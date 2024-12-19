#include "SceneSubj.h"
#include "platform.h"
#include "utils.h"
#include "TheApp.h"
#include "DrawJob.h"
#include "Shadows.h"
#include "ModelLoaderCmd.h"
#include "XMLparser.h"
#include <algorithm>
#include "ScreenLine.h"
#include "CameraMan.h"
#include "MySound.h"

extern TheApp theApp;
extern float degrees2radians;

SceneSubj* SceneSubj::pSelectedSceneSubj =NULL;
SceneSubj* SceneSubj::pSelectedSceneSubj00 = NULL;


SceneSubj::SceneSubj() {
}
SceneSubj::~SceneSubj() {
    if (pCustomColors != NULL) {
        int totalN = pCustomColors->size();
        for (int i = 0; i < totalN; i++)
            delete pCustomColors->at(i);
        pCustomColors->clear();
        delete pCustomColors;
        pCustomColors = NULL;
    }
    if (pCustomMaterials != NULL) {
        int totalN = pCustomMaterials->size();
        for (int i = 0; i < totalN; i++)
            delete pCustomMaterials->at(i);
        pCustomMaterials->clear();
        delete pCustomMaterials;
        pCustomMaterials = NULL;
    }
}
void SceneSubj::buildModelMatrixStandard(SceneSubj* pSS) {

    SceneSubj* pParent = pSS->getParent();

    if (pSS->d2parent != 0 && pSS->d2headTo != 0) {
        if (v3equals(pSS->ownCoords.pos, 0))
            v3copy(pSS->absCoords.pos, pParent->absCoords.pos);
        else
            mat4x4_mul_vec4plus(pSS->absCoords.pos, pParent->absModelMatrixUnscaled, pSS->ownCoords.pos, 1);
        SceneSubj* pHeadTo = pSS->pSubjsSet->at(pSS->nInSubjsSet - pSS->d2headTo);
        float posHeadTo[4];
        if (v3equals(pSS->posOnHeadTo, 0))
            v3copy(posHeadTo, pHeadTo->absCoords.pos);
        else
            mat4x4_mul_vec4plus(posHeadTo, pHeadTo->absModelMatrixUnscaled, pSS->posOnHeadTo, 1);
        float yaw = v3yawDgFromTo(pSS->absCoords.pos, posHeadTo);
        pSS->absCoords.setYaw(yaw);
        if (pSS->place2middle)
            for (int i = 0; i < 3; i+=2)
                pSS->absCoords.pos[i] = (pSS->absCoords.pos[i] + posHeadTo[i]) / 2;
        //ignore ownModelMatrixUnscaled
        mat4x4_translate(pSS->absModelMatrixUnscaled, pSS->absCoords.pos[0], pSS->absCoords.pos[1], pSS->absCoords.pos[2]);
        //rotation order: Z-X-Y
        mat4x4_mul(pSS->absModelMatrixUnscaled, pSS->absModelMatrixUnscaled, pSS->absCoords.rotationMatrix);
    }
    else {//normal case
        mat4x4_translate(pSS->ownModelMatrixUnscaled, pSS->ownCoords.pos[0], pSS->ownCoords.pos[1], pSS->ownCoords.pos[2]);
        //rotation order: Z-X-Y
        mat4x4_mul(pSS->ownModelMatrixUnscaled, pSS->ownModelMatrixUnscaled, pSS->ownCoords.rotationMatrix);

        if (pSS->renderMirrorAxis >= 0)
            pSS->ownModelMatrixUnscaled[pSS->renderMirrorAxis][pSS->renderMirrorAxis] *= -1;

        if (pSS->d2parent != 0) {
            SceneSubj* pParent = pSS->pSubjsSet->at(pSS->nInSubjsSet - pSS->d2parent);
            mat4x4_mul(pSS->absModelMatrixUnscaled, pParent->absModelMatrixUnscaled, pSS->ownModelMatrixUnscaled);
        }
        else if (pSS->pStickTo != NULL)
            mat4x4_mul(pSS->absModelMatrixUnscaled, pSS->pStickTo->absModelMatrixUnscaled, pSS->ownModelMatrixUnscaled);
        else
            memcpy(pSS->absModelMatrixUnscaled, pSS->ownModelMatrixUnscaled, sizeof(mat4x4));
    }
    if (v3equals(pSS->scale, 1))
        memcpy(pSS->absModelMatrix, pSS->absModelMatrixUnscaled, sizeof(mat4x4));
    else
        mat4x4_scale_aniso(pSS->absModelMatrix, pSS->absModelMatrixUnscaled, pSS->scale[0], pSS->scale[1], pSS->scale[2]);

    //update absCoords
    if (pSS->d2parent == 0)
        memcpy(&pSS->absCoords, &pSS->ownCoords, sizeof(Coords));
    else {
        Coords::getPositionFromMatrix(pSS->absCoords.pos, pSS->absModelMatrixUnscaled);
        /*
        //get abs quaternion+rotationMatrix     
        quat q2;
        //quat_from_mat4x4(q2, pSS->absModelMatrixUnscaled);
        quat_mul(q2, pSS->ownCoords.getRotationQuat(), pParent->absCoords.getRotationQuat());
        pSS->absCoords.setQuaternion(q2);
        */
    }

    pSS->absCoordsUpdateFrameN = theApp.frameN;
}

int SceneSubj::moveStandard(SceneSubj* pSS) {
    pSS->applySpeeds();
    return 1;
}
int SceneSubj::applySpeedsStandard(SceneSubj* pSS) {
    //set up speeds here or use pre-set
    if (pSS->ownSpeed.hasRotation()) {
        quat q2;
        quat_mul(q2, pSS->ownSpeed.getRotationQuat(), pSS->ownCoords.getRotationQuat());
        pSS->ownCoords.setQuaternion(q2);
    }
    for (int i = 0; i < 3; i++)
        pSS->ownCoords.pos[i] += pSS->ownSpeed.pos[i];
    return 1;
}



int SceneSubj::scaleStandard(SceneSubj* pSS, float k) {
    for (int i = 0; i < 3; i++)
        pSS->scale[i] *= k;
    int subjsN = pSS->pSubjsSet->size();
    for (int sN = pSS->nInSubjsSet + 1; sN < subjsN; sN++) {
        SceneSubj* pSS2 = pSS->pSubjsSet->at(sN);
        if (pSS2 == NULL)
            continue;
        if (pSS2->nInSubjsSet - pSS2->d2parent != pSS->nInSubjsSet)
            continue;
        for (int i = 0; i < 3; i++)
            pSS2->ownCoords.pos[i] *= k;
        pSS2->zOffsetFromRoot *= k;
        pSS2->lever *= k;
        pSS2->scaleMe(k);
    }
    return 1;
}
int SceneSubj::processSubjStandard(SceneSubj* pSS) {
    memcpy(&pSS->absCoordsPrev, &pSS->absCoords,sizeof(Coords));
    if (pSS->moveSubj() < 1)
        return 0; //subj was deleted
    if (pSS->absCoordsUpdateFrameN != theApp.frameN)
        pSS->buildModelMatrix();
    return 1;
}

int SceneSubj::onDeployStandatd(SceneSubj* pSS,std::string tagStr) {
    
    if (strlen(pSS->fileOnDeploy) > 0) {
        std::string fileOnDeployStr(pSS->fileOnDeploy);
        ModelLoaderCmd* pML = new ModelLoaderCmd(pSS, fileOnDeployStr);
        pML->processSource(pML);
        delete pML;
    }
    //tag instructions
    if (tagStr.length() == 0)
        return 0;

    float pos[2] = { 0,0 };
    XMLparser::setFloatArray(pos, 2, "tableAt", tagStr);
    float posY = theApp.gameTable.groundLevel0;
    std::string alignStr = XMLparser::getStringValue("align", tagStr);
    if (alignStr.find("bottom") != std::string::npos)
        posY += (pSS->gabaritesOnLoad.bbRad[1] * pSS->scale[1]);
    theApp.gameTable.placeAt(pSS->ownCoords.pos, pos[0], posY, pos[1]);

    if (XMLparser::varExists("ay", tagStr)) {
        float ay = XMLparser::getFloatValue("ay", tagStr);
        pSS->ownCoords.setEulerDg(0, ay, 0);
        mat4x4_from_quat(pSS->ownCoords.rotationMatrix, pSS->ownCoords.getRotationQuat());
    }


    return 1;
}
int SceneSubj::addToRenderQueue(std::vector<SceneSubj*>* pQueueOpaque, std::vector<SceneSubj*>* pQueueTransparent, std::vector<SceneSubj*>* pSubjs) {
    int subjsN = pSubjs->size();
    int added = 0;
    for (int sN = 0; sN < subjsN; sN++) {
        SceneSubj* pSS = pSubjs->at(sN);
        if (pSS == NULL)
            continue;
        if (pSS->hide > 0)
            continue;
        if (pSS->djTotalN < 1)
            continue;
        if (pSS->gabaritesOnScreen.isInViewRange < 0)
            continue;
        //check render order/priority
        if (pSS->renderOrder < 0) {//-1-unknown
            int lastOpaqueN = -1;
            int lastAlphaN = -1;
            for (int djN = 0; djN < pSS->djTotalN; djN++) {
                DrawJob* pDJ = pSS->pDrawJobs->at(pSS->djStartN + djN);
                Material* pMt = &pDJ->mt;
                if (pMt->uAlphaBlending > 0) //transparent
                    lastAlphaN = djN;
                else //opaque
                    lastOpaqueN = djN;
            }
            if (lastAlphaN < 0)//all DJs opaque
                pSS->renderOrder = 0;
            else if (lastOpaqueN < 0)
                pSS->renderOrder = 1; //all w/alpha
            else
                pSS->renderOrder = 2; //some DJs w/alpha, some - opaque, mixed
        }

        if (pQueueTransparent == NULL)
            pQueueOpaque->push_back(pSS);
        else if (pSS->renderOrder == 0)
            pQueueOpaque->push_back(pSS);
        else if (pSS->renderOrder == 1)
            pQueueTransparent->push_back(pSS);
        else { //mixed
            pQueueOpaque->push_back(pSS);
            pQueueTransparent->push_back(pSS);
        }
        added++;
    }
    return added;
}
int SceneSubj::sortRenderQueue(std::vector<SceneSubj*>* pQueue, int direction) {
    //direction: 0-closer 1st, 1-farther 1st
    if (direction == 0) //0-closer 1st
        std::sort(pQueue->begin(), pQueue->end(),
            [](SceneSubj* pSS0, SceneSubj* pSS1) {
                //if < - go 1st
                return pSS0->gabaritesOnScreen.bbMin[2] < pSS1->gabaritesOnScreen.bbMin[2];
            });
    else //1-farther 1st
        std::sort(pQueue->begin(), pQueue->end(),
            [](SceneSubj* pSS0, SceneSubj* pSS1) {
                //if > - go 1st
                return pSS0->gabaritesOnScreen.bbMin[2] > pSS1->gabaritesOnScreen.bbMin[2];
            });
    return 1;
}

int SceneSubj::customizeMaterial(Material** ppMt, Material** ppMt2, Material** ppAltMt, Material** ppAltMt2, SceneSubj* pSS) {
    Material* pMt = *ppMt;
    Material* pMt2 = *ppMt2;
    Material* pAltMt = *ppAltMt;
    Material* pAltMt2 = *ppAltMt2;

    SceneSubj* pRoot = pSS->pSubjsSet->at(pSS->rootN);
    if (pRoot->pCustomMaterials != NULL) {
        int customMaterialsN = pRoot->pCustomMaterials->size();
        if (strlen(pMt->materialName32) != 0) {
            for (int oN = 0; oN <= customMaterialsN; oN++) {
                if (oN == customMaterialsN)
                    break;
                MaterialAdjust* pMA = pRoot->pCustomMaterials->at(oN); //custom color option
                if (strcmp(pMA->materialName32, pMt->materialName32) == 0) {
                    pAltMt = new Material(pMt);
                    pMt = pAltMt;
                    pMA->adjust(pMt, pMA);
                    pMt->shaderN = pMt->getShaderNumber(pMt, pMt->shaderType32);
                    //2-nd layer
                    if (strlen(pMt->layer2as) == 0)
                        //pMt2 = NULL;
                        pMt2->shaderN = -1;
                    else if (strcmp(pMt->layer2as, pMt2->materialName32) != 0) {
                        pAltMt2 = new Material(pMt);
                        pMt2 = pAltMt2;
                        pMt2->shaderN = -1;
                        strcpy_s(pMt2->materialName32, 32, pMt->layer2as);
                        //look in
                        std::vector<MaterialAdjust*>* pMAlist = &MaterialAdjust::materialAdjustsList;
                        customMaterialsN = pMAlist->size();
                        for (int oN = 0; oN < customMaterialsN; oN++) {
                            MaterialAdjust* pMA = pMAlist->at(oN); //custom color option
                            if (strcmp(pMA->materialName32, pMt2->materialName32) == 0) {
                                pMA->adjust(pMt2, pMA);
                                pMt2->shaderN = pMt2->getShaderNumber(pMt2, pMt2->shaderType32);
                            }
                        }
                    }
                    break;
                }
            }
        }
    }

    if (pRoot->pCustomColors != NULL) {
        int customColorsN = pRoot->pCustomColors->size();
        //customize colors?
        MyColor* pColors[3];
        pColors[0] = &pMt->uColor;
        pColors[1] = &pMt->uColor1;
        pColors[2] = &pMt->uColor2;
        for (int cN = 0; cN < 3; cN++) {
            MyColor* pCL = pColors[cN];
            if (strlen(pCL->colorName) == 0)
                continue;
            for (int oN = 0; oN <= customColorsN; oN++) {
                if (oN == customColorsN)
                    break;
                MyColor* pCC = pRoot->pCustomColors->at(oN); //custom color option
                if (strcmp(pCC->colorName, pCL->colorName) == 0) {
                    memcpy(pCL, pCC, sizeof(MyColor));
                    break;
                }
            }
        }
    }
    *ppMt = pMt;
    *ppMt2 = pMt2;
    *ppAltMt = pAltMt;
    *ppAltMt2 = pAltMt2;

    return 1;
}
bool SceneSubj::lineWidthIsImportant(int primitiveType) {
    if (primitiveType == GL_TRIANGLES) return false;
    if (primitiveType == GL_TRIANGLE_STRIP) return false;
    if (primitiveType == GL_TRIANGLE_FAN) return false;
    return true;
}
int SceneSubj::executeDJstandard2(DrawJob* pDJ,
    float* uMVP, float* uMV3x3, float* uMM, float* uMVP4dm,
    float* uVectorToLight, float* uVectorToTranslucent, float* uCameraPosition, float line1pixSize, Material* pMt, int shaderN,
    float uFogLevel, unsigned int uFogColor32) {

    if (pMt == NULL)
        pMt = &(pDJ->mt);

    if (shaderN < 0)
        return 0;

    if (lineWidthIsImportant(pMt->primitiveType)) {
        if(pMt->lineWidthFixed==1) 
            glLineWidth(pMt->lineWidth);
        else {
            float lw = line1pixSize * pMt->lineWidth;
             if (lw < 1.0) {
                if (pMt->lineWidthFixed == 2) //not less than 1
                    lw = 1;
                else {
                    return 0;
                }
            }
            else {
                lw = fmin(lw, 10);
            }
            glLineWidth(lw);
        }
    }

    Shader* pShader = Shader::shaders.at(shaderN);
    glUseProgram(pShader->GLid);

    //input uniform matrices
    if (pShader->l_uMVP >= 0)
        glUniformMatrix4fv(pShader->l_uMVP, 1, GL_FALSE, (const GLfloat*)uMVP);
    if (pShader->l_uMV3x3 >= 0)
        glUniformMatrix3fv(pShader->l_uMV3x3, 1, GL_FALSE, (const GLfloat*)uMV3x3);
    if (pShader->l_uMM >= 0)
        glUniformMatrix4fv(pShader->l_uMM, 1, GL_FALSE, (const GLfloat*)uMM);
    //shadow uniform matrix
    if (pShader->l_uTex4dm >= 0)
        glUniformMatrix4fv(pShader->l_uMVP4dm, 1, GL_FALSE, (const GLfloat*)uMVP4dm);

    executeDJcommon(pDJ, pShader, uVectorToLight, uVectorToTranslucent, uCameraPosition, pMt, uFogLevel, uFogColor32);
    return 1;
}
int SceneSubj::executeDJcommon(DrawJob* pDJ, Shader* pShader,
    float* uVectorToLight, float* uVectorToTranslucent, float* uCameraPosition, Material* pMt, float uFogLevel, unsigned int uFogColor32) {
    //assuming that shader program set, transform matrices provided
    glBindVertexArray(pDJ->glVAOid);

    if (pShader->l_uDepthBias >= 0) //for depthMap
        glUniformMatrix4fv(pShader->l_uDepthBias, 1, GL_FALSE, (const GLfloat*)Shadows::uDepthBias);

    //shadow uniforms
    if (pShader->l_uTex4dm >= 0) {
        //translate to texture
        int textureId = Texture::getGLid(Shadows::depthMapTexN);
        //pass textureId to shader program
        glActiveTexture(GL_TEXTURE4); // activate the texture unit first before binding texture
        glBindTexture(GL_TEXTURE_2D, textureId);
        // Tell the texture uniform sampler to use this texture in the shader by binding to texture unit 4.
        glUniform1i(pShader->l_uTex4dm, 4);

        glUniform1f(pShader->l_uShadow, Shadows::shadowLight);
    }

    if (pShader->l_uVectorToLight >= 0)
        glUniform3fv(pShader->l_uVectorToLight, 1, (const GLfloat*)uVectorToLight);

    if (pShader->l_uTranslucency >= 0) {
        glUniform1f(pShader->l_uTranslucency, pMt->uTranslucency);
        if (pMt->uTranslucency > 0)
            glUniform3fv(pShader->l_uVectorToTranslucent, 1, (const GLfloat*)uVectorToTranslucent);
    }

    if (pShader->l_uCameraPosition >= 0)
        glUniform3fv(pShader->l_uCameraPosition, 1, (const GLfloat*)uCameraPosition);

    //attach textures
    if (pShader->l_uTex0 >= 0) {
        int textureId = Texture::getGLid(pMt->uTex0);
        //pass textureId to shader program
        glActiveTexture(GL_TEXTURE0); // activate the texture unit first before binding texture
        glBindTexture(GL_TEXTURE_2D, textureId);
        // Tell the texture uniform sampler to use this texture in the shader by binding to texture unit 0.    
        glUniform1i(pShader->l_uTex0, 0);
    }
    if (pShader->l_uTex1mask >= 0) {
        int textureId = Texture::getGLid(pMt->uTex1mask);
        //pass textureId to shader program
        glActiveTexture(GL_TEXTURE1); // activate the texture unit first before binding texture
        glBindTexture(GL_TEXTURE_2D, textureId);
        // Tell the texture uniform sampler to use this texture in the shader by binding to texture unit 1.    
        glUniform1i(pShader->l_uTex1mask, 1);
    }
    if (pShader->l_uTex2nm >= 0) {
        int textureId = Texture::getGLid(pMt->uTex2nm);
        //pass textureId to shader program
        glActiveTexture(GL_TEXTURE2); // activate the texture unit first before binding texture
        glBindTexture(GL_TEXTURE_2D, textureId);
        // Tell the texture uniform sampler to use this texture in the shader by binding to texture unit 2.    
        glUniform1i(pShader->l_uTex2nm, 2);
    }
    if (pShader->l_uTex0translateChannelN >= 0) {
        glUniform1i(pShader->l_uTex0translateChannelN, pMt->uTex0translateChannelN);
        if (pMt->uTex0translateChannelN >= 4) {
            //translate to 2-tone
            if (pShader->l_uColor1 >= 0) {
                glUniform4fv(pShader->l_uColor1, 1, pMt->uColor1.forGL());
                glUniform4fv(pShader->l_uColor2, 1, pMt->uColor2.forGL());
            }
        }
        else
            if (pShader->l_uTex3 >= 0 && pMt->uTex3 >= 0) {
                //translate to texture
                int textureId = Texture::getGLid(pMt->uTex3);
                //pass textureId to shader program
                glActiveTexture(GL_TEXTURE3); // activate the texture unit first before binding texture
                glBindTexture(GL_TEXTURE_2D, textureId);
                // Tell the texture uniform sampler to use this texture in the shader by binding to texture unit 3.    
                glUniform1i(pShader->l_uTex3, 3);
            }
    }

    //tex coords modifiers
    if (pShader->l_uTexMods >= 0)
		glUniformMatrix4fv(pShader->l_uTexMods, 1, GL_FALSE, (const GLfloat*)pMt->uTexMods);

    //for normal map
    if (pShader->l_uHaveBinormal >= 0)
        glUniform1i(pShader->l_uHaveBinormal, pDJ->uHaveBinormal);

    //material uniforms
    if (pShader->l_uTex1alphaChannelN >= 0)
        glUniform1i(pShader->l_uTex1alphaChannelN, pMt->uTex1alphaChannelN);
    if (pShader->l_uTex1alphaNegative >= 0)
        glUniform1i(pShader->l_uTex1alphaNegative, pMt->uTex1alphaNegative);
    if (pShader->l_uColor >= 0)
        glUniform4fv(pShader->l_uColor, 1, pMt->uColor.forGL());
    if (pShader->l_uAlphaFactor >= 0)
        glUniform1f(pShader->l_uAlphaFactor, pMt->uAlphaFactor);
    if (pShader->l_uAlphaBlending >= 0)
        glUniform1i(pShader->l_uAlphaBlending, pMt->uAlphaBlending);
    if (pShader->l_uAmbient >= 0)
        glUniform1f(pShader->l_uAmbient, pMt->uAmbient);
    if (pShader->l_uSpecularIntencity >= 0)
        glUniform1f(pShader->l_uSpecularIntencity, pMt->uSpecularIntencity);
    if (pShader->l_uSpecularMinDot >= 0)
        glUniform2fv(pShader->l_uSpecularMinDot, 1, pMt->uSpecularMinDot);
    if (pShader->l_uSpecularPowerOf >= 0)
        glUniform1f(pShader->l_uSpecularPowerOf, pMt->uSpecularPowerOf);
    if (pShader->l_uBleach >= 0)
        glUniform1f(pShader->l_uBleach, pMt->uBleach);
    if (pShader->l_uShadingK >= 0)
        glUniform1f(pShader->l_uShadingK, pMt->uShadingK);
    if (pShader->l_uEdgeAlpha >= 0)
        glUniform1f(pShader->l_uEdgeAlpha, pMt->uEdgeAlpha);
    if (pShader->l_uDiscardNormalsOut >= 0)
        glUniform1i(pShader->l_uDiscardNormalsOut, pMt->uDiscardNormalsOut);
    if (pShader->l_uConstZ >= 0) {
        float constZ = Shadows::uConstZ * 2;
        if (pMt->isLine())
            constZ = constZ + Shadows::uConstZ * pMt->lineWidth;
        glUniform1f(pShader->l_uConstZ, constZ);
    }
    if (pShader->l_uHighLightLevel >= 0)
        glUniform1f(pShader->l_uHighLightLevel, uFogLevel);
    if (pShader->l_uHighLightColor >= 0) {
        MyColor uHighLightColor;
        uHighLightColor.setUint32(uFogColor32);
        glUniform4fv(pShader->l_uHighLightColor, 1, uHighLightColor.forGL());
    }

    //adjust render settings
    if (pMt->zBuffer > 0) {
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
    }
    else
        glDisable(GL_DEPTH_TEST);

    if (pMt->zBufferUpdate > 0)
        glDepthMask(GL_TRUE);
    else
        glDepthMask(GL_FALSE);

    if (pShader->l_uAlphaBlending >= 0 && pMt->uAlphaBlending > 0 || pMt->uAlphaFactor < 1) {
        glDepthMask(GL_FALSE); //don't update z-buffer
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    else
        glDisable(GL_BLEND);

    //execute
    if (pDJ->glEBOid > 0)
        glDrawElements(pMt->primitiveType, pDJ->pointsN, GL_UNSIGNED_SHORT, 0);
    else
        glDrawArrays(pMt->primitiveType, 0, pDJ->pointsN);

    glBindVertexArray(0);


    //checkGLerrors("DrawJob::executeDrawJob end");

    return 1;
}

int SceneSubj::onLoadStandard(SceneSubj* pSS, std::string tagStr) {
    return 1;
}
void SceneSubj::setHighLight(SceneSubj* pSS00, float highLightlevel, unsigned int highLightColor32) {
    std::vector<int>* pFamily=entireFamily(pSS00);
    while (pFamily->size()>0) {
        int subjN = pFamily->back();
        pFamily->pop_back();
        SceneSubj* pSS = pSS00->pSubjsSet->at(subjN);
        pSS->uHighLightLevel = highLightlevel;
        pSS->uHighLightColor.setUint32(highLightColor32);
    }
    delete pFamily;
}
int SceneSubj::renderStandard_prepare(SceneSubj* pSS, Camera* pCam, float* dirToMainLight, float* dirToTranslucent, 
    mat4x4 mMVP, mat4x4 mMVP4dm, float* pSizeUnitPixelsSize,bool forDepthMap) {
    
    if(pSS->bilboard){ //
        float dir[3];
        for (int i = 0; i < 3; i++)
            dir[i] = pCam->ownCoords.pos[i] - pSS->absCoords.pos[i];
        float pitch = v3pitchDg(dir);
        float yaw = v3yawDg(dir);
        pSS->absCoords.setEulerDg(pitch, yaw, pSS->ownCoords.getEulerDg(2));
        mat4x4_translate(pSS->absModelMatrixUnscaled, pSS->absCoords.pos[0], pSS->absCoords.pos[1], pSS->absCoords.pos[2]);
        mat4x4_mul(pSS->absModelMatrixUnscaled, pSS->absModelMatrixUnscaled, pSS->absCoords.rotationMatrix);
        mat4x4_scale_aniso(pSS->absModelMatrix, pSS->absModelMatrixUnscaled, pSS->scale[0], pSS->scale[1], pSS->scale[2]);
    }
    
    if (!forDepthMap) {
        //build mMVP4dm (shadow MVP) matrix for given subject
        mat4x4_mul(mMVP4dm, Shadows::shadowCamera.mViewProjection, pSS->absModelMatrix);
    }
    //build MVP matrix for given subject
    mat4x4_mul(mMVP, pCam->mViewProjection, pSS->absModelMatrix);

    float sizeUnitPixelsSize = getUnitPixelsSize(pSS, pCam);
    *pSizeUnitPixelsSize = sizeUnitPixelsSize;
    return 1;
}

float SceneSubj::getUnitPixelsSize(SceneSubj * pSS, Camera * pCam){
    //subj's distance from camera
    float cameraSpacePos[4];
    mat4x4_mul_vec4plus(cameraSpacePos, pCam->lookAtMatrix, pSS->absCoords.pos, 1);
    float zDistance = abs(cameraSpacePos[2]);
    float viewRangeDg = pCam->viewRangeDg;
    if (viewRangeDg == 0)
        viewRangeDg = 30;
    float cotangentA = 1.0f / tanf(degrees2radians * viewRangeDg / 2.0f);
    float halfScreenVertSizeInUnits = zDistance / cotangentA;
    float sizeUnitPixelsSize = (float)theApp.mainCamera.targetRads[1] / halfScreenVertSizeInUnits;
    sizeUnitPixelsSize *= pSS->scale[0];
    return sizeUnitPixelsSize;
}
int SceneSubj::renderStandard(SceneSubj* pSS, Camera* pCam, float* dirToMainLight, float* dirToTranslucent, int renderFilter, bool forDepthMap) {
    if (pSS->djTotalN < 1)
        return 0;
    if (forDepthMap && pSS->dropsShadow < 1)
        return 0;

    mat4x4 mMVP;
    mat4x4 mMVP4dm;
    float sizeUnitPixelsSize;
    renderStandard_prepare(pSS, pCam, dirToMainLight, dirToTranslucent, mMVP, mMVP4dm, &sizeUnitPixelsSize, forDepthMap);
    renderStandard_execute(pSS, mMVP, mMVP4dm, pCam, dirToMainLight, dirToTranslucent, sizeUnitPixelsSize,renderFilter, forDepthMap);

    if (pSS->tremble > 0)
    if(renderFilter==0)//opaque only
    if(!forDepthMap){
        float trembleAnglePerFrame = 5;
        float angle = angleDgNorm360(trembleAnglePerFrame*theApp.frameN);
        float trembleD[3] = {0,0,0};
        trembleD[0] = cos(degrees2radians * angle);
        trembleD[2] = sin(degrees2radians * angle);
        v3setLength(trembleD, pSS->tremble);
        if (theApp.frameN % 2 == 0) {
            trembleD[0] *= -1;
            trembleD[2] *= -1;
        }
        pSS->absModelMatrix[3][0] += trembleD[0];
        pSS->absModelMatrix[3][2] += trembleD[2];
        mat4x4_mul(mMVP, pCam->mViewProjection, pSS->absModelMatrix);
        renderStandard_execute(pSS, mMVP, mMVP4dm, pCam, dirToMainLight, dirToTranslucent, sizeUnitPixelsSize, renderFilter, forDepthMap);
    }
    return 1;
}
int SceneSubj::renderStandard_execute(SceneSubj* pSS, mat4x4 mMVP, mat4x4 mMVP4dm, Camera* pCam, float* dirToMainLight, float* dirToTranslucent,
    float sizeUnitPixelsSize, int renderFilter, bool forDepthMap){

    if (renderFilter == pSS->renderOrder)
        renderFilter = -1; //render all

    glCullFace(pSS->cullFace);

    //build Model-View (rotation) matrix for normals
    mat4x4 mMV4x4;
    mat4x4_mul(mMV4x4, pCam->lookAtMatrix, pSS->absModelMatrixUnscaled);
    //convert to 3x3 matrix
    float mMV3x3[3][3];
    for (int y = 0; y < 3; y++)
        for (int x = 0; x < 3; x++)
            mMV3x3[y][x] = mMV4x4[y][x];

    if (pSS->pDrawJobs == NULL)
        pSS->pDrawJobs = &theApp.drawJobs;

    for (int i0 = 0; i0 < pSS->djTotalN; i0++) {
        int i = i0;
        if (renderFilter > 0) //render transparent only, most likely - GLTF reversed order
            i = pSS->djTotalN - 1 - i0;

        DrawJob* pDJ = pSS->pDrawJobs->at(pSS->djStartN + i);

        if (forDepthMap) {
            if (pDJ->mt.dropsShadow < 1)
                continue;
            Material* pMT = &pDJ->mt;
            if(i==0 && pSS->mt0isSet)
                pMT = &pSS->mt0;

            executeDJstandard2(pDJ, (float*)mMVP, *mMV3x3, NULL, NULL, NULL, NULL, NULL,
                Shadows::sizeUnitPixelsSize * pSS->scale[0], pMT,pMT->shaderNshadow);
            continue;
        }

        Material* pMt = &pDJ->mt;

        if (renderFilter >= 0)
            if (renderFilter != pMt->uAlphaBlending > 0)
                continue;

        Material* pMt2 = &pDJ->mtLayer2;
        if (i == 0) {
            Material mt;
            memcpy(&mt, &pDJ->mt, sizeof(Material));
            pMt = &mt;
            //2-nd layer
            Material mt2;
            memcpy(&mt2, &pDJ->mtLayer2, sizeof(Material));
            pMt2 = &mt2;
        }
        Material* pAltMt = NULL;
        Material* pAltMt2 = NULL;
        if (i == 0 && pSS->mt0isSet > 0) { //already customized
            pMt = &pSS->mt0;
            pMt2 = &pSS->mt0Layer2;
        }
        else { //customize?
            customizeMaterial(&pMt, &pMt2, &pAltMt, &pAltMt2, pSS);
            if (i == 0) {
                memcpy(&pSS->mt0, pMt, sizeof(Material));
                memcpy(&pSS->mt0Layer2, pMt2, sizeof(Material));
                pSS->mt0isSet = 1;
            }
        }
 
        executeDJstandard2(pDJ, (float*)mMVP, *mMV3x3, (float*)pSS->absModelMatrix, (float*)mMVP4dm, dirToMainLight, dirToTranslucent,
            pCam->ownCoords.pos, sizeUnitPixelsSize, pMt, pMt->shaderN, pSS->uHighLightLevel, pSS->uHighLightColor.getUint32());

        //have 2-nd layer ?
        if (pMt2 != NULL) {
            if (pMt2->shaderN >= 0) {
                executeDJstandard2(pDJ, (float*)mMVP, *mMV3x3, (float*)pSS->absModelMatrix, (float*)mMVP4dm, dirToMainLight, dirToTranslucent,
                    pCam->ownCoords.pos, sizeUnitPixelsSize, pMt2, pMt2->shaderN, pSS->uHighLightLevel, pSS->uHighLightColor.getUint32());
            }
        }
        if (pAltMt != NULL) {
            delete pAltMt;
            pAltMt = NULL;
        }
        if (pAltMt2 != NULL) {
            delete pAltMt2;
            pAltMt2 = NULL;
        }
    }
    return 1;
}
void SceneSubj::deployModel(SceneSubj * pSS00, std::string tagStr){
    for (int elN = pSS00->totalElements - 1; elN >= 0; elN--) {
        SceneSubj* pSS = pSS00->pSubjsSet->at(pSS00->nInSubjsSet + elN);
        if(pSS == NULL)
            continue;
        if(elN==0)
            pSS->onDeploy(tagStr);
        else
            pSS->onDeploy("");
    }
    pSS00->initGabarites(pSS00);
}
std::vector<int>* SceneSubj::entireFamily(SceneSubj* pSS00) {
    std::vector<int>* pv = new std::vector<int>;
    pv->push_back(pSS00->nInSubjsSet);
    if(pSS00->pSubjsSet==NULL)
        return pv;
    //array limits
    //SceneSubj* pRoot = pSS00->pSubjsSet->at(pSS00->rootN);
    //int lastN2check = pRoot->nInSubjsSet + pRoot->totalElements - 1;
    int lastN2check = pSS00->pSubjsSet->size() - 1;
    for (int checkSlotN = 0; checkSlotN < pv->size(); checkSlotN++) {
        int parentN = pv->at(checkSlotN);
        for (int elN = lastN2check; elN > parentN; elN--) {
            SceneSubj* pSS = pSS00->pSubjsSet->at(elN);
            if (pSS == NULL)
                continue;
            if (elN - pSS->d2parent != parentN)
                continue;
            //here-have a child
            pv->push_back(elN);
        }
    }
    return pv;
}
std::vector<int>* SceneSubj::allChilds(SceneSubj* pSS00) {
    std::vector<int>* pv = new std::vector<int>;
    //array limits
    int lastN2check = pSS00->pSubjsSet->size() - 1;
    int parentN = pSS00->nInSubjsSet;
    for (int elN = lastN2check; elN > parentN; elN--) {
        SceneSubj* pSS = pSS00->pSubjsSet->at(elN);
        if (pSS == NULL)
            continue;
        if (elN - pSS->d2parent != parentN)
            continue;
        //here-is a child
        pv->push_back(elN);
    }
    return pv;
}

void SceneSubj::setHide(SceneSubj* pSS00, int val) {
    std::vector<int>* pFamilyList = entireFamily(pSS00);
    for (int slotN = pFamilyList->size() - 1; slotN >= 0; slotN--) {
        int elN = pFamilyList->at(slotN);
        SceneSubj* pSS = pSS00->pSubjsSet->at(elN);
        pSS->hide = val;
    }
    pFamilyList->clear();
    delete pFamilyList;
}

void SceneSubj::resetRoot(SceneSubj* pSS00,int rootN) {
    if (rootN < 0) {    //find root
        rootN = pSS00->nInSubjsSet;
        while (pSS00->d2parent != 0) {
            rootN = pSS00->nInSubjsSet - pSS00->d2parent;
            pSS00 = pSS00->pSubjsSet->at(rootN);
        }
    }
    //reset
    std::vector<int>* pFamilyList = entireFamily(pSS00);
    for (int slotN = pFamilyList->size() - 1; slotN >= 0; slotN--) {
        int elN = pFamilyList->at(slotN);
        SceneSubj* pSS = pSS00->pSubjsSet->at(elN);
        pSS->rootN = rootN;
    }
    pFamilyList->clear();
    delete pFamilyList;
}
int SceneSubj::checkCollisions(std::vector<std::vector<SceneSubj*>*> pSubjArraysPairs4collisionDetection) {
    for (int arrayN = 0; arrayN < pSubjArraysPairs4collisionDetection.size(); arrayN += 2) {
        std::vector<SceneSubj*>* pSet1 = pSubjArraysPairs4collisionDetection.at(arrayN);
        std::vector<SceneSubj*>* pSet2 = pSubjArraysPairs4collisionDetection.at(arrayN+1);
        int set1size = pSet1->size();
        int set2size = pSet2->size();
        bool sameSet = (pSet1 == pSet2);
        for (int n1 = 0; n1 < set1size; n1++) {
            SceneSubj* pS1 = pSet1->at(n1);
            if (pS1 == NULL)
                continue;
            if (pS1->hidden() > 0)
                continue;
            if (pS1->d2parent != 0)
                continue;
            int startFrom = 0;
            if (sameSet)
                startFrom = n1 + 1;
            for (int n2 = startFrom; n2 < set2size; n2++) {
                SceneSubj* pS2 = pSet2->at(n2);
                if (pS2 == NULL)
                    continue;
                if (pS2->hidden() > 0)
                    continue;
                if (pS2->d2parent != 0)
                    continue;
                if (pS1->ignoreCollision(pS2))
                    continue;
                bool crossing = Gabarites::boxesIntersect(&pS1->gabaritesWorldAssembly, &pS2->gabaritesWorldAssembly);

                //mylog("%d crossing=%d\n", (int)theApp.frameN, crossing);

                if (!crossing)
                    continue;
                float penetrationDepth = checkMacroObjsCollision(pS1, pS2);
                if (penetrationDepth > 0)
                    break;
            }
        }
    }
    return 1;
}
float SceneSubj::checkMacroObjsCollision(SceneSubj* pS01, SceneSubj* pS02) {
    for (int n1 = 0; n1 < pS01->totalElements; n1++) {
        int sN1 = n1 + pS01->nInSubjsSet;
        SceneSubj* pS1 = pS01->pSubjsSet->at(sN1);
        if (pS1 == NULL)
            continue;
        if (pS1->hidden() > 0)
            continue;
        if (pS1->djTotalN<1)
            continue;
        if (pS1->chordaCollision.chordType < 0)
            continue;
        if (pS1->rootN != pS01->rootN)
            continue;
        for (int n2 = 0; n2 < pS02->totalElements; n2++) {
            int sN2 = n2 + pS02->nInSubjsSet;
            SceneSubj* pS2 = pS02->pSubjsSet->at(sN2);
            if (pS2 == NULL)
                continue;
            if (pS2->hidden() > 0)
                continue;
            if (pS2->djTotalN < 1)
                continue;
            if (pS2->chordaCollision.chordType < 0)
                continue;
            if (pS2->rootN != pS02->rootN)
                continue;

            /*
            if (sN1 == 13 && sN2 == 45) {
                mat4x4 mMVP;
                float* targetRads = theApp.mainCamera.targetRads;
                float vIn[4];
                float p0[4];
                float p1[4];
                float y0 = 20;
                SceneSubj* pS = pS1;
                mat4x4_mul(mMVP, theApp.mainCamera.mViewProjection, pS->absModelMatrix);
                //draw z-axis
                v3setAll(vIn, 0);
                vIn[2] = -pS->gabaritesOnLoad.bbMin[2];
                mat4x4_mul_vec4screen(p0, mMVP, vIn, targetRads);
                vIn[2] = -pS->gabaritesOnLoad.bbMax[2];
                mat4x4_mul_vec4screen(p1, mMVP, vIn, targetRads);
                ScreenLine::addLine2queue(p0, p1, MyColor::getUint32(1.0f, 0.0f, 0.0f, 1.0f), 2, true);
                //draw chord
                y0 = pS->absCoords.pos[1] + 10;// gabaritesWorld.bbMid[1];
                vIn[0] = pS->gabaritesWorld.chord.p0[0];
                vIn[1] = y0;
                vIn[2] = -pS->gabaritesWorld.chord.p0[1];
                mat4x4_mul_vec4screen(p0, theApp.mainCamera.mViewProjection, vIn, targetRads);
                vIn[0] = pS->gabaritesWorld.chord.p1[0];
                vIn[1] = y0;
                vIn[2] = -pS->gabaritesWorld.chord.p1[1];
                mat4x4_mul_vec4screen(p1, theApp.mainCamera.mViewProjection, vIn, targetRads);
                ScreenLine::addLine2queue(p0, p1, MyColor::getUint32(0.0f, 1.0f, 0.0f, 1.0f), 3, true);

                pS = pS2;
                mat4x4_mul(mMVP, theApp.mainCamera.mViewProjection, pS->absModelMatrix);
                //draw z-axis
                v3setAll(vIn, 0);
                vIn[2] = -pS->gabaritesOnLoad.bbMin[2];
                mat4x4_mul_vec4screen(p0, mMVP, vIn, targetRads);
                vIn[2] = -pS->gabaritesOnLoad.bbMax[2];
                mat4x4_mul_vec4screen(p1, mMVP, vIn, targetRads);
                ScreenLine::addLine2queue(p0, p1, MyColor::getUint32(1.0f, 0.0f, 0.0f, 1.0f), 2, true);
                //draw chord
                y0 = pS->absCoords.pos[1] + 10;// gabaritesWorld.bbMid[1];
                vIn[0] = pS->gabaritesWorld.chord.p0[0];
                vIn[1] = y0;
                vIn[2] = -pS->gabaritesWorld.chord.p0[1];
                mat4x4_mul_vec4screen(p0, theApp.mainCamera.mViewProjection, vIn, targetRads);
                vIn[0] = pS->gabaritesWorld.chord.p1[0];
                vIn[1] = y0;
                vIn[2] = -pS->gabaritesWorld.chord.p1[1];
                mat4x4_mul_vec4screen(p1, theApp.mainCamera.mViewProjection, vIn, targetRads);
                ScreenLine::addLine2queue(p0, p1, MyColor::getUint32(0.0f, 1.0f, 0.0f, 1.0f), 3, true);

                int a = 0;
            }
            */

            float penetrationDepth2d = checkSingleSubjsCollision2D(pS1, pS2);
            if (penetrationDepth2d <= 0)
               continue;
           
            /*
            {//debug
                float p0[4];
                float p1[4];
                mat4x4_mul_vec4plus(p0, theApp.collisionCamera.mVPinverted, pS1->chordaCollision.chord.p0, 1, true);
                mat4x4_mul_vec4plus(p1, theApp.collisionCamera.mVPinverted, pS1->chordaCollision.chord.p1, 1, true);
                ScreenLine::add3Dline2queue(p0, p1, MyColor::getUint32(255, 0, 0), 3, true);
                mat4x4_mul_vec4plus(p0, theApp.collisionCamera.mVPinverted, pS2->chordaCollision.chord.p0, 1, true);
                mat4x4_mul_vec4plus(p1, theApp.collisionCamera.mVPinverted, pS2->chordaCollision.chord.p1, 1, true);
                ScreenLine::add3Dline2queue(p0, p1, MyColor::getUint32(255, 0, 0), 3, true);
                mylog("%d chordaR1=%.2f chordaR2=%.2f penetrationDepth=%.2f\n",theApp.frameN,
                    pS1->chordaCollision.chordR, pS2->chordaCollision.chordR, penetrationDepth);
                //theApp.bPause = true;
            }
            */

            //have 2 objs collision here
            float closestPoint1[4];
            float closestPoint2[4];
            mat4x4_mul_vec4plus(closestPoint1, theApp.collisionCamera.mVPinverted, pS1->chordaCollision.chord.closestPoint, 1, true);
            mat4x4_mul_vec4plus(closestPoint2, theApp.collisionCamera.mVPinverted, pS2->chordaCollision.chord.closestPoint, 1, true);
            //set y
            float yMin = fmax(pS1->gabaritesWorld.bbMin[1], pS2->gabaritesWorld.bbMin[1]);
            float yMax = fmin(pS1->gabaritesWorld.bbMax[1], pS2->gabaritesWorld.bbMax[1]);
            float y = (yMin + yMax) / 2;
            closestPoint1[1] = y;
            closestPoint2[1] = y;


            float chordsRR = pS1->chordaCollision.chordR + pS2->chordaCollision.chordR;
            float k1 = pS2->chordaCollision.chordR / chordsRR;
            float k2 = 1.0f - k1;
            float collisionPointWorld[4];
            for (int i = 0; i < 3; i++)
                collisionPointWorld[i] = closestPoint1[i] * k1 + closestPoint2[i] * k2;


            float hitPointNormal1[4];
            float hitPointNormal2[4];
            for (int i = 0; i < 3; i++) {
                hitPointNormal1[i] = collisionPointWorld[i] - closestPoint1[i];
                hitPointNormal2[i] = collisionPointWorld[i] - closestPoint2[i];
            }
            v3norm(hitPointNormal1);
            v3norm(hitPointNormal2);
            //hit speed
            float speedS1[3];
            float speedS2[3];
            float hitSpeed2S1[3];
            float hitSpeed2S2[3];
            for (int i = 0; i < 3; i++) {
                speedS1[i] = pS1->absCoords.pos[i] - pS1->absCoordsPrev.pos[i];
                speedS2[i] = pS2->absCoords.pos[i] - pS2->absCoordsPrev.pos[i];
                hitSpeed2S1[i] = -speedS1[i] + speedS2[i];
                hitSpeed2S2[i] = -hitSpeed2S1[i];
            }
            SceneSubj* pS1base = pS1;
            while (pS1base->d2parent != 0) {
                pS1base = pS1base->pSubjsSet->at(pS1base->nInSubjsSet - pS1base->d2parent);
            }
            SceneSubj* pS2base = pS2;
            while (pS2base->d2parent != 0) {
                pS2base = pS2base->pSubjsSet->at(pS2base->nInSubjsSet - pS2base->d2parent);
            }
            pS1base->processCollision(penetrationDepth2d,pS2base, collisionPointWorld, hitPointNormal1, hitSpeed2S1);
            pS2base->processCollision(penetrationDepth2d,pS1base, collisionPointWorld, hitPointNormal2, hitSpeed2S2);

            //mylog("%d collision %d/%d : %d/%d penetrationDepth=%f\n", (int)theApp.frameN, pS01->nInSubjsSet, pS02->nInSubjsSet, sN1, sN2,penetrationDepth);

            return penetrationDepth2d;
        }
    }
    return 0;
}
int SceneSubj::processCollisionStandard(float penetrationDepth, SceneSubj* pS1, SceneSubj* pS2, float* collisionPointWorld, float* hitPointNormal, float* hitSpeed) {
 
    if (pS1->pSubjsSet == &theApp.staticSubjs) {
        Smoke::fillWithGas(pS1);
        MySound::playSound(MySound::soundNpop01, pS1, false);
        pS1=pS1->getRoot();
        pS1->deleteMe(true);
        return 0;
    }
    
    return 1;
}
int lastId = 0;
int SceneSubj::newId() {
    lastId++;
    return lastId;
}
int SceneSubj::subj2log(SceneSubj* pSS) {
    mylog("subj:");
    if (pSS == NULL) {
        mylog("NULL\n");
        return 0;
    }
    if (strlen(pSS->name64) > 0)
        mylog(" %s", pSS->name64);
    if (strlen(pSS->className) > 0)
        mylog(" %s", pSS->className);
    if (strlen(pSS->source256) > 0)
        mylog(" %s", pSS->source256);
    if (strlen(pSS->source256root) > 0)
        mylog(" / %s", pSS->source256root);
    mylog("\n");
    return 1;
}
int SceneSubj::onInputStandard(SceneSubj* pSS) {
    //delegate to parent
    if (pSS->d2parent == 0)
        return 0;
    SceneSubj* pParent = pSS->pSubjsSet->at(pSS->nInSubjsSet - pSS->d2parent);
    pParent->onInput();
    return 1;
}
int SceneSubj::getCursorAncorPointStandard(float* ancorPoint, float* cursorPos, SceneSubj* pSS) {
    //mylog("----------------------\n");
    mat4x4 mMVP;
    Camera* pCam = &theApp.mainCamera;
    mat4x4_mul(mMVP, pCam->mViewProjection, pSS->absModelMatrix);
    //center point
    float p00[4];
    float vIn[4] = { 0,0,0,0 };
    mat4x4_mul_vec4screen(p00, mMVP, vIn, pCam->targetRads, pCam->nearClip, pCam->farClip);
    //axis dirs sizes
    float dir10[3][4];
    float dirSize = 10;
    float axK[3];
    for (int axN = 0; axN < 3; axN++) {
        v3setAll(vIn, 0);
        vIn[axN] = dirSize;
        mat4x4_mul_vec4screen(dir10[axN], mMVP, vIn, pCam->targetRads, pCam->nearClip, pCam->farClip);
        for (int i = 0; i < 2; i++)
            dir10[axN][i] -= p00[i];
        axK[axN] = dirSize / v3lengthXY(dir10[axN]);
    }
    //float bestShift = 1000000;
    for (int ax0N = 2; ax0N > 0; ax0N--) {
        LineXY ax0;
        v2copy(ax0.p0, p00);
        v2copy(ax0.p1, p00);
        for (int i = 0; i < 2; i++)
            ax0.p1[i] += dir10[ax0N][i];
        ax0.calculateLineXY();
        bool goodEnough = false;
        for (int ax1N = ax0N - 1; ax1N >= 0; ax1N--) {
            LineXY ax1;
            v2copy(ax1.p0, cursorPos);
            v2copy(ax1.p1, cursorPos);
            for (int i = 0; i < 2; i++)
                ax1.p1[i] += dir10[ax1N][i];
            ax1.calculateLineXY();
            float vOut[4];
            if (LineXY::linesIntersectionXY(vOut, &ax0, &ax1) < 1) {
                mylog("skip axes %d/%d\n", ax0N, ax1N);
                continue;
            }
            //have intersection - find shifts
            float shift[3];
            v3setAll(shift, 0);
            float d = v3lengthFromToXY(p00, vOut) * axK[ax0N];
            if (d != 0) {
                float vDir[4] = { 0,0,0,0 };
                for (int i = 0; i < 2; i++)
                    vDir[i] = vOut[i] - p00[i];
                if (v2dotProductNorm(vDir, dir10[ax0N]) < 0)
                    d = -d;
            }
            shift[ax0N] = d;
            //now - for ax1N
            d = v3lengthFromToXY(vOut, cursorPos) * axK[ax1N];
            if (d != 0) {
                float vDir[4] = { 0,0,0,0 };
                for (int i = 0; i < 2; i++)
                    vDir[i] = cursorPos[i] - vOut[i];
                if (v2dotProductNorm(vDir, dir10[ax1N]) < 0)
                    d = -d;
            }
            shift[ax1N] = d;
            goodEnough = true;

            for (int i = 0; i < 3; i++) {
                if (abs(shift[i]) > pSS->gabaritesOnLoad.bbRad[i] * 2) {
                    goodEnough = false;
                    break;
                }
            }
            if (!goodEnough)
                continue;

            v3copy(ancorPoint, shift);
            break;
        }
        if (goodEnough)
            break;
    }
    //mylog_v3("ancor", ancorPoint);
    /*
    {
        for (int axN = 0; axN < 3; axN++) {
            for (int i = 0; i < 3; i++) {
                TouchScreen::ax0[axN].p0[i] = p00[i] - dir10[axN][i];
                TouchScreen::ax0[axN].p1[i] = p00[i] + dir10[axN][i];

                TouchScreen::ax1[axN].p0[i] = cursorPos[i] - dir10[axN][i];
                TouchScreen::ax1[axN].p1[i] = cursorPos[i] + dir10[axN][i];

            }
        }

        float shift[4] = { 0,0,0,0 };
        float vOut[4];
        for (int axN = 2; axN >= 0; axN--) {
            //p0 - before shift
            mat4x4_mul_vec4plus(TouchScreen::ax2[axN].p0, mMVP, shift, 1, true);
            //GL to screen coords
            TouchScreen::ax2[axN].p0[0] = TouchScreen::ax2[axN].p0[0] * targetRads[0] + targetRads[0];
            TouchScreen::ax2[axN].p0[1] = -TouchScreen::ax2[axN].p0[1] * targetRads[1] + targetRads[1];
            //p1 - after shift
            shift[axN] = ancorPoint[axN];
            mat4x4_mul_vec4plus(TouchScreen::ax2[TouchScreen::axN].p1, mMVP, shift, 1, true);
            //GL to screen coords
            ax2[axN].p1[0] = ax2[axN].p1[0] * targetRads[0] + targetRads[0];
            ax2[axN].p1[1] = -ax2[axN].p1[1] * targetRads[1] + targetRads[1];

        }
    }
    */
    return 1;
}
int SceneSubj::onLeftButtonDown(){
    getCursorAncorPointStandard(TouchScreen::ancorPoint, TouchScreen::cursorPos, (SceneSubj*)pSelectedSceneSubj00);
    return 1;
}
int SceneSubj::onFocusOut(){
    setHighLight(0);
    TouchScreen::abandonSelected();
    /*
    TouchScreen::pSelected = NULL;
    //UISubj::pSelectedUISubj = NULL;
    SceneSubj::pSelectedSceneSubj = NULL;
    SceneSubj::pSelectedSceneSubj00 = NULL;
    */
    return 1;
}
SceneSubj* SceneSubj::getResponsiveSceneSubj(SceneSubj* pCandidate) {
    if (pCandidate == NULL)
        return NULL;    
    while (!pCandidate->isResponsive()) {
        if (pCandidate->d2parent == 0)
            return pCandidate;
        pCandidate = pCandidate->pSubjsSet->at(pCandidate->nInSubjsSet - pCandidate->d2parent);
    }    
    return pCandidate;
}

SceneSubj* SceneSubj::getParent(SceneSubj* pSS) {
    if (pSS->d2parent == 0)
        return pSS;
    return pSS->pSubjsSet->at(pSS->nInSubjsSet - pSS->d2parent);
}
SceneSubj* SceneSubj::getRoot(SceneSubj* pSS) {
    if (pSS->d2parent == 0)
        return pSS;
    return pSS->pSubjsSet->at(pSS->rootN);
}
SceneSubj* SceneSubj::copyModel(std::vector<SceneSubj*>* pToSet, SceneSubj* pModelRoot, bool nativeOnly) {
    std::vector<SceneSubj* >* pFrom = pModelRoot->pSubjsSet;
    int modelSize = pModelRoot->totalElements;
    if (nativeOnly)
        modelSize = pModelRoot->totalNativeElements;

    int slotStarts = findSlotIn(pToSet, modelSize);

    for (int sN = 0; sN < modelSize; sN++) {
        SceneSubj* pS00 = pModelRoot;
        if(sN>0)
            pS00 = pFrom->at(pModelRoot->nInSubjsSet + sN);
        SceneSubj* pS = pS00->clone();
        int slotN = slotStarts + sN;
        if (slotN < pToSet->size())
            pToSet->at(slotN) = pS;
        else
            pToSet->push_back(pS);
        pS->pSubjsSet = pToSet;
        pS->nInSubjsSet = slotN;
        pS->pCustomColors = NULL;
        pS->pCustomMaterials = NULL;
        pS->mt0isSet = 0;
        if (pS->hidden())
            pS->setHide(0);
    }
    SceneSubj* pS = pToSet->at(slotStarts);
    pS->totalElements = modelSize;
    pS->totalNativeElements = pModelRoot->totalNativeElements;
    pS->d2parent = 0;
    pS->d2headTo = 0;
    if (pS->scale[0] != 1.0)
        pS->scaleMe(1.0f / pS->scale[0]);
    pS->resetRoot();

    pS->birthFrameN = theApp.frameN;

    return pS;
}
int SceneSubj::findSlotIn(std::vector<SceneSubj*>* pSet, int modelSize, int searchFrom) {
    //find slot
    int subjsN = pSet->size();
    int slotSize = 0;
    int slotStarts = -1;
    for (int sN = searchFrom; sN < subjsN; sN++) {
        SceneSubj* pS = pSet->at(sN);
        if (pS != NULL) {
            slotSize = 0;
            slotStarts = -1;
            continue;
        }
        //potential slot
        if (slotStarts < 0)
            slotStarts = sN;
        slotSize++;
        if (slotSize >= modelSize)
            break;
    }
    if (slotStarts < 0)
        slotStarts = subjsN;
    return slotStarts;
}
int SceneSubj::singleDraw3D(SceneSubj* pSS00) {
    for (int n = 0; n < pSS00->totalElements; n++) {
        SceneSubj* pSS = pSS00->pSubjsSet->at(pSS00->nInSubjsSet + n);
        pSS->buildModelMatrix();
        pSS->render(&theApp.mainCamera, theApp.dirToMainLight, theApp.dirToTranslucent, 0, false);
    }
    return 1;
}
int SceneSubj::readClassPropsSceneSubj(SceneSubj* pSS,std::string tagStr) {
    XMLparser::setCharsValue(pSS->name64, 64, "name", tagStr);
    XMLparser::setCharsValue(pSS->type16, 16, "type", tagStr);
    XMLparser::setCharsValue(pSS->subtype16, 16, "subtype", tagStr);
    return 1;
}
int SceneSubj::assemblyGabaritesPrepareRoot(SceneSubj* pSS) {
    if (pSS->d2parent != 0)
        return 1;
    Gabarites::adjustMidRad(&pSS->gabaritesWorldAssembly);
    pSS->gabaritesWorldAssembly.clear();
    //v3copy(pSS->gabaritesWorldAssembly.bbMin, pSS->absCoords.pos);
    //v3copy(pSS->gabaritesWorldAssembly.bbMax, pSS->absCoords.pos);
    return 1;
}

int SceneSubj::assemblyGabaritesUpdateRoot(SceneSubj* pSS) {
    SceneSubj* pRoot = pSS->getRoot();
    if (pSS->chordaCollision.chordType < 0 && pRoot->totalElements>1)
        return 0;
    Gabarites::adjustMinMaxByBBox(&pRoot->gabaritesWorldAssembly, &pSS->gabaritesWorld);
    return 1;
}
int SceneSubj::cameraMan_zoomInOut(SceneSubj* pS) {
    return theApp.cameraMan.zoomInOut(pS);
}
int SceneSubj::initGabarites(SceneSubj* pS) {
    //world gabarites and assembly gabarites
    std::vector<SceneSubj*>* pSubjs=pS->pSubjsSet;
    if (pS->d2parent == 0)
        pS->rootN = pS->nInSubjsSet;
    if (pS->totalNativeElements == 0) {
        pS->totalNativeElements = 1;
        pS->totalElements = 1;
    }
    SceneSubj* pRoot = pS->getRoot(pS);
    pRoot->assemblyGabaritesPrepareRoot(pRoot);
    for (int i = 0; i < pRoot->totalElements; i++) {
        pS = pSubjs->at(pRoot->nInSubjsSet + i);
        if (pS == NULL)
            continue;
        if (pS->d2parent != 0) {
            SceneSubj* pParent = pS->getParent(pS);
            pS->rootN = pParent->rootN;
        }
        if (pS->rootN != pRoot->nInSubjsSet)
            continue;
        if (pS->hide > 0)
            continue;
        if (pS->djTotalN < 1)
            continue;
        buildModelMatrixStandard(pS);
        //world gabarites
        fillWorldGabarites(pS);
        Gabarites::adjustMinMaxByBBox(&pRoot->gabaritesWorldAssembly, &pS->gabaritesWorld);
    }
    return 1;
}

int SceneSubj::fillWorldGabarites(SceneSubj* pS){//Gabarites* pG, mat4x4 absModelMatrix, Gabarites* pG00, mat4x4 mViewProjection, float* targetRads, float nearClip, float farClip) {
    //build MVP matrix for given subject
    Gabarites::fillBBox(&pS->gabaritesWorld, &pS->gabaritesOnLoad, pS->absModelMatrix, NULL, 0, 0);
    if (pS->chordaCollision.chordType < 0)
        return 0; //don't need chord

    Camera* pCam = &theApp.collisionCamera;
    mat4x4 mMVP;
    mat4x4_mul(mMVP, pCam->mViewProjection, pS->absModelMatrix);
    /*
    if (strcmp(pS->name64, "body") == 0) {
        float vIn[4] = { 0,0,100,0 };
        float vOut[4];
        mat4x4_mul_vec4plus(vOut, mMVP, vIn, 1, true);
        float vOut2[4];
        mat4x4_mul_vec4plus(vOut2, mMVP, vIn, 1, false);

        int a = 0;
    }
    */
    Chorda::buildChorda(&pS->chordaCollision, &pS->gabaritesOnLoad, mMVP, NULL, 0, 0);

    //debug
    if (pS->chordaCollision.chordR != pS->chordaCollision.chordR)
        int a = 0;

    return 1;
}
int SceneSubj::fillScreenGabarites(SceneSubj* pS) {//Gabarites* pG, mat4x4 absModelMatrix, Gabarites* pG00, mat4x4 mViewProjection, float* targetRads, float nearClip, float farClip) {
    //build MVP matrix for given subject
    Camera* pCam = &theApp.mainCamera;
    mat4x4 mMVP;
    mat4x4_mul(mMVP, pCam->mViewProjection, pS->absModelMatrix);

    Gabarites::fillBBox(&pS->gabaritesOnScreen, &pS->gabaritesOnLoad, mMVP, pCam->targetRads, pCam->nearClip, pCam->farClip);
    if (pS->gabaritesOnScreen.isInViewRange < 0)
        return 0;
    if (pS->chordaScreen.chordType < 0)
        return 0; //don't need chord
    Chorda::buildChorda(&pS->chordaScreen, &pS->gabaritesOnLoad, mMVP, pCam->targetRads, pCam->nearClip, pCam->farClip);
    return 1;
}
float SceneSubj::checkSingleSubjsCollision2D(SceneSubj* pS1, SceneSubj* pS2) {
    //returns penetration depth
    //check bounding box first
    Gabarites* pGb1 = &pS1->gabaritesWorld;
    Gabarites* pGb2 = &pS2->gabaritesWorld;

    bool crossing = Gabarites::boxesIntersect(pGb1, pGb2);
    if(!crossing)
        return 0;
    //if here - bounding boxes overlap
    Chorda* pCh1 = &pS1->chordaCollision;
    Chorda* pCh2 = &pS2->chordaCollision;
    float chordsRR = pCh1->chordR + pCh2->chordR;
    float dist = LineXY::dist_l2l(&pCh1->chord, &pCh2->chord);
    float overlap = chordsRR - dist;

    /*
    if (strcmp(pS1->name64, "body") == 0 && strcmp(pS2->name64, "body") == 0) {
        mylog("%d body overlap=%d\n", (int)theApp.frameN, (int)overlap);
    }
    */
    if (overlap <= 0)//no collision
        return 0;

    return overlap;
}
SceneSubj* SceneSubj::pointerOnSceneSubj00(std::vector<SceneSubj*>* pSubjs) {
    SceneSubj* pCandidate00 = NULL;
    int subjsN = pSubjs->size();

    for (int sN = 0; sN < subjsN; sN++) {
        SceneSubj* pSS = pSubjs->at(sN);
        if (pSS == NULL)
            continue;
        if (pSS->chordaScreen.chordType < 0)
            continue;
        if (pSS->gabaritesOnScreen.isInViewRange < 0)
            continue;
        if (pSS->chordaScreen.chordaQuality < 0)
            continue;
        //has chord, on screen
        float dist2cursor = LineXY::dist_l2p(&pSS->chordaScreen.chord, TouchScreen::cursorPos);
        /*
        //debug
        if (pSS->rootN == 0 && strcmp(pSS->name64, "body") == 0) {
            mylog("---%d dist=%d chordR=%d ", (int)theApp.frameN, (int)dist2cursor, (int)pSS->chordaScreen.chordR);
            mylog("chorda %d,%d,%d-%d,%d,%d length=%d a_slope=%f b_intercept=%d isVertical=%d x_vertical=%d isHorizontal=%d isDot=%d >>> cursor:%d,%d,%d\n",
                (int)pSS->chordaScreen.chord.p0[0], (int)pSS->chordaScreen.chord.p0[1], (int)pSS->chordaScreen.chord.p0[2],
                (int)pSS->chordaScreen.chord.p1[0], (int)pSS->chordaScreen.chord.p1[1], (int)pSS->chordaScreen.chord.p1[2],
                (int)pSS->chordaScreen.chord.length,pSS->chordaScreen.chord.a_slope, (int)pSS->chordaScreen.chord.b_intercept, pSS->chordaScreen.chord.isVertical, (int)pSS->chordaScreen.chord.x_vertical, pSS->chordaScreen.chord.isHorizontal, pSS->chordaScreen.chord.isDot,
                (int)TouchScreen::lastCursorPos[0], (int)TouchScreen::lastCursorPos[1], (int)TouchScreen::lastCursorPos[2]);
        }
        */
        if (dist2cursor > pSS->chordaScreen.chordR)
            continue; //too far
        if (pCandidate00 == NULL) {
            pCandidate00 = pSS;
            continue;
        }
        if (pCandidate00->gabaritesOnScreen.bbMid[2] < pSS->gabaritesOnScreen.bbMid[2])
            continue;
        pCandidate00 = pSS;
    }
    return pCandidate00;
}
SceneSubj* SceneSubj::updateSelected00(SceneSubj* pCandidate00) {
    if (pCandidate00 == NULL) {
        pSelectedSceneSubj00 = NULL;
        pSelectedSceneSubj = NULL;
        return NULL;
    }
    if (pSelectedSceneSubj00 != pCandidate00) {
        //new candidate
        pSelectedSceneSubj00 = pCandidate00;
        pSelectedSceneSubj = pSelectedSceneSubj00->getResponsiveSubj();
    }
    return pSelectedSceneSubj;
}
SceneSubj* SceneSubj::pickFromCandidates(SceneSubj* pCandidate1, SceneSubj* pCandidate2) {
    if (pCandidate1 == NULL)
        return pCandidate2;
    if (pCandidate2 == NULL)
        return pCandidate1;
    if (pCandidate1->gabaritesOnScreen.bbMin[2] < pCandidate2->gabaritesOnScreen.bbMin[2])
        return pCandidate1;
    return pCandidate2;
}
float SceneSubj::trembleFade(float tremble) {
    if (tremble == 0)
        return 0;
    float trembleFadeSpeed = 0.01;
    tremble -= trembleFadeSpeed;
    if (tremble < 0)
         tremble = 0;
    return tremble;
}

int SceneSubj::deleteMeSimple(SceneSubj* pSS) {
    if (pSS->totalElements > 1) {
        mylog("ERROR in SceneSubj::deleteMeSimple: %d elements\n", pSS->totalElements);
        return -1;
    }
    //single element
    std::vector<SceneSubj*>* pSubjs = pSS->pSubjsSet;
    int subjN = pSS->nInSubjsSet;
    pSubjs->at(subjN) = NULL;
    delete pSS;
    return 1;
}

int SceneSubj::deleteMe(bool withChilds) {
    SceneSubj* pSS = this;

    //if (pSS->totalElements > 1) {
        std::vector<SceneSubj*>* pSubjs = pSS->pSubjsSet;
        std::vector<int>* pSubjNs = allChilds(pSS);
        for (int sN = pSubjNs->size() - 1; sN >= 0; sN--) {
            int subjN = pSubjNs->at(sN);
            SceneSubj* pS = pSubjs->at(subjN);
            if(withChilds)
                pS->deleteMe(withChilds);
            else { //release child
                pS->d2parent = 0;
                pS->rootN = subjN;
            }
        }
        pSubjNs->clear();
    //}

    pSS->totalElements = 1;
    deleteMeSimple(pSS);
    return 1;
}


