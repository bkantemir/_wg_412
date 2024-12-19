#include "Rail.h"
#include "Shadows.h"
#include "TheApp.h"
#include "ScreenLine.h"
#include "MyColor.h"
#include "MySound.h"
#include "MeltingSign.h"

extern float PI;
extern float degrees2radians;
extern TheApp theApp;

float Rail::railroadGauge = 9;
float Rail::railWidth = 1;
float Rail::railHeight = 2;
float Rail::railSleeperLength = 16;
float Rail::railSleeperWidth = 2;
float Rail::railSleeperHeight = 1;
float Rail::fastenerWidth = 1.5;
float Rail::railSleepersInterval = 5;
float Rail::couplerY = 5;
//map-related
float Rail::railsLevel = 1;
float Rail::curveRadius = 1;
float Rail::tileCenter2curveCenter = 1;
float Rail::railLenghtLong = 1;
float Rail::railLenghtShort = 1;
float Rail::railLenghtCurved = 1;
//guides
float Rail::railGuideLineWidth = 0.5;
float Rail::railGuideGaugeWidth = railroadGauge * 0.5;
float Rail::railGuideLength= railLenghtLong*0.45;
float Rail::railGuideAngle = railGuideLength*360/(PI*2* curveRadius);
float Rail::railGuideShortLength= railLenghtLong*0.15;
float Rail::railGuideShortAngle = railGuideShortLength * 360 / (PI * 2 * curveRadius);
float Rail::railGuideBedDy = -1.2;

//wheel-related
float Rail::wheelWidth = 1.6;
float Rail::rimBorderLineWidth = 0.5;
//models
int Rail::railModelSnowFlakeN = -1;
int Rail::railModelSnowFlakeSmallN = -1;

int Rail::railModelStraightVirtualN = -1;
int Rail::railModelShortVirtualN = -1;
int Rail::railModelCurvedVirtualN = -1;

int Rail::railModelStraightN = -1;
int Rail::railModelShortN = -1;
int Rail::railModelCurvedN = -1;

int Rail::railModelStraightNsmall = -1;
int Rail::railModelShortNsmall = -1;
int Rail::railModelCurvedNsmall = -1;

int Rail::railModelStraightNtiny = -1;
int Rail::railModelShortNtiny = -1;
int Rail::railModelCurvedNtiny = -1;

int Rail::railModelGuideStraightN;
int Rail::railModelGuideCurvedN;
int Rail::railModelGuideBedStraightN;
int Rail::railModelGuideBedCurvedN;
int Rail::railModelGuideShortStraightN;
int Rail::railModelGuideShortCurvedN;


int Rail::couplerModelN = -1;
int Rail::couplerShaftModelN = -1;
int Rail::couplersCoupleModelN = -1;

int Rail::gangwaysCoupleModelN = -1;

std::vector<SceneSubj*> Rail::railModels;


int Rail::cleanUp() {
    for (int i = railModels.size()-1; i >=0; i--) {
        SceneSubj* pSS = railModels.at(i);
        if (pSS != NULL)
            delete pSS;
    }
    railModels.clear();
    return 1;
}
void RailEnd::toLog(std::string title, RailEnd* pRE) {
    mylog("%s:rail %d.%d @tile %.1fx%.1f\n", title.c_str(), pRE->toRailN,pRE->toRailEndN, pRE->toTileIndexXZ[0], pRE->toTileIndexXZ[1]);
}

void Rail::toLog(std::string title, Rail* pR) {
    mylog("%s:%d tile=%.1fx%.1f %s->%s l=%d\n", title.c_str(), pR->nInSubjsSet, pR->tileIndexXZ[0], pR->tileIndexXZ[1], ang2dir(pR->yawInOut[0]).c_str(), ang2dir(pR->yawInOut[1]).c_str(), (int)pR->railLength);
    RailEnd::toLog("from", &pR->railEnd[0]);
    RailEnd::toLog("  to", &pR->railEnd[1]);
}
std::string Rail::ang2dir(float ang) {
    int a = (int)angleDgNorm360(ang);
    switch (a) {
    case 0: return "S";
    case 45: return "SE";
    case 90: return "E";
    case 135: return "NE";
    case 180: return "N";
    case 225: return "NW";
    case 270: return "W";
    case 315: return "SW";
    default: return "HZ";
    }
}
Rail* Rail::pointerOnRail() {
    RailMap45* pTable = (RailMap45*)&theApp.gameTable;
    std::vector<SceneSubj*>* pRailsMap = &pTable->railsMap;
    float tablePoint[4];
    if (TheTable::getCursorAncorPointTable(tablePoint, TouchScreen::cursorPos, pTable) < 1) {
        float a = theApp.mainCamera.mVPinverted[0][0];
        float b = theApp.mainCamera.mViewProjection[0][0];
        return NULL;
    }
    //check if out of table
    for (int i = 0; i < 3; i += 2) {
        if (abs(tablePoint[i]) >= pTable->worldBox.bbRad[i])
            return NULL;
    }
    //scan rails
    //check if in tile first
    float tileR = pTable->tileSize*0.6;
    float maxD = pTable->tileSize * 0.6;
    float railR = railroadGauge * 1.0;
    Rail* pBest = NULL;
    float bestDist2cursor = maxD;
    for (int rN = pRailsMap->size() - 1; rN >= 0; rN--) {
        Rail* pR = (Rail*)pRailsMap->at(rN);
        if (pR == NULL)
            continue;
        if (pR->isClickable() == false)
            continue;
        if (pR->gabaritesOnScreen.isInViewRange < 0)
            continue;
        //check gabarites
        bool tooFar = false;
        for (int i = 0; i < 3; i += 2) {
            if (abs(pR->ownCoords.pos[i] - tablePoint[i]) >= maxD) {
                tooFar =true;
                break;
            }
        }
        if (tooFar)
            continue;
        //get precise distance
        float d = dist2point3d(pR, tablePoint);
        if (d > railR)
            continue;
        if (pBest != NULL) {
            if (pBest->railStatus != 0) {//virtual
                if (pR->railStatus == 0) //real rail preferred
                    bestDist2cursor = 1000000;
                else if (pBest->priority < pR->priority)
                    bestDist2cursor = 1000000;
                else if (pBest->priority > pR->priority)
                    d = 1000000;
            }
        }
        if (bestDist2cursor <= d)
            continue;
        bestDist2cursor = d;
        pBest = pR;
    }
    /*
    //debug
    if (pBest != NULL) {
        Rail* pR = pBest;
        if (bestDist2cursor < maxD)
            if (pR->railStatus == 0)
                if (pR->railType == 1) {
                    float p0[4];
                    float p1[4];
                    //mat4x4_mul_vec4screen(p0, theApp.mainCamera.mViewProjection, pR->p0, theApp.mainCamera.targetRads, theApp.mainCamera.nearClip, theApp.mainCamera.farClip);
                    //mat4x4_mul_vec4screen(p1, theApp.mainCamera.mViewProjection, pR->p1, theApp.mainCamera.targetRads, theApp.mainCamera.nearClip, theApp.mainCamera.farClip);
                    float p00[4];
                    float p01[4];
                    p00[0] = pR->arc2d.p0[0];
                    p00[1] = pR->ownCoords.pos[1];
                    p00[2] = pR->arc2d.p0[1];
                    p01[0] = pR->arc2d.p1[0];
                    p01[1] = pR->ownCoords.pos[1];
                    p01[2] = pR->arc2d.p1[1];
                    mat4x4_mul_vec4screen(p0, theApp.mainCamera.mViewProjection, p00, theApp.mainCamera.targetRads, theApp.mainCamera.nearClip, theApp.mainCamera.farClip);
                    mat4x4_mul_vec4screen(p1, theApp.mainCamera.mViewProjection, p01, theApp.mainCamera.targetRads, theApp.mainCamera.nearClip, theApp.mainCamera.farClip);
                    ScreenLine::addLine2queue(p0, p1, MyColor::getUint32(0, 255, 0), 7, true);
 
                    mat4x4_mul_vec4screen(p0, theApp.mainCamera.mViewProjection, tablePoint, theApp.mainCamera.targetRads, theApp.mainCamera.nearClip, theApp.mainCamera.farClip);
                    p01[0] = pR->arc2d.closestPoint[0];
                    p01[1] = pR->ownCoords.pos[1];
                    p01[2] = pR->arc2d.closestPoint[1];
                    mat4x4_mul_vec4screen(p1, theApp.mainCamera.mViewProjection, p01, theApp.mainCamera.targetRads, theApp.mainCamera.nearClip, theApp.mainCamera.farClip);
                    ScreenLine::addLine2queue(p0, p1, MyColor::getUint32(255, 0, 0), 3, true);

                }

    }
    */
    
    return pBest;
}
bool Rail::isClickable() { 
    Rail* pR = this;
    if (pR->railStatus < 0)
        return false;//snowflake
    if(theApp.bEditMode)
        return true;
    /*
    if (pR->railEnd[0].isSwitch)
        return true;
    if (pR->railEnd[1].isSwitch)
        return true;
        */
    if (pR->switchEndN>=0)
        return true;
    return false;
}
float Rail::dist2point3d(Rail* pR, float* pos3d) {
    float p2d[2];
    p2d[0] = pos3d[0]; //x
    p2d[1] = pos3d[2]; //z
    float dist;
    if (pR->railType == 0) {//straight
        dist = pR->line2d.dist_l2p(p2d);
        /*
        //debug
        if (pR->yawInOut[0] == 0) {
            float p0[4];
            float p1[4];
            mat4x4_mul_vec4screen(p0, theApp.mainCamera.mViewProjection, pR->p0, theApp.mainCamera.targetRads, theApp.mainCamera.nearClip, theApp.mainCamera.farClip);
            mat4x4_mul_vec4screen(p1, theApp.mainCamera.mViewProjection, pR->p1, theApp.mainCamera.targetRads, theApp.mainCamera.nearClip, theApp.mainCamera.farClip);
            ScreenLine::addLine2queue(p0, p1, MyColor::getUint32(255, 0, 0), 9, true);

            mat4x4_mul_vec4screen(p0, theApp.mainCamera.mViewProjection, pos3d, theApp.mainCamera.targetRads, theApp.mainCamera.nearClip, theApp.mainCamera.farClip);
            float pC[4];
            pC[0] = pR->line2d.closestPoint[0];
            pC[1] = pR->ownCoords.pos[1];
            pC[2] = pR->line2d.closestPoint[1];
            mat4x4_mul_vec4screen(p1, theApp.mainCamera.mViewProjection, pC, theApp.mainCamera.targetRads, theApp.mainCamera.nearClip, theApp.mainCamera.farClip);
            ScreenLine::addLine2queue(p0, p1, MyColor::getUint32(0, 0, 255), 3, true);
        }
        */
    }
    else {//curve
        dist = pR->arc2d.dist_arc2point(p2d);
        /*
        //debug
        if (pR->nInSubjsSet == 10) {
            float p0[4];
            float p1[4];
            mat4x4_mul_vec4screen(p0, theApp.mainCamera.mViewProjection, pR->p0, theApp.mainCamera.targetRads, theApp.mainCamera.nearClip, theApp.mainCamera.farClip);
            mat4x4_mul_vec4screen(p1, theApp.mainCamera.mViewProjection, pR->p1, theApp.mainCamera.targetRads, theApp.mainCamera.nearClip, theApp.mainCamera.farClip);
            ScreenLine::addLine2queue(p0, p1, MyColor::getUint32(255, 0, 0), 3, true);

            mat4x4_mul_vec4screen(p0, theApp.mainCamera.mViewProjection, pos3d, theApp.mainCamera.targetRads, theApp.mainCamera.nearClip, theApp.mainCamera.farClip);
            float pC[4];
            pC[0] = pR->arc2d.closestPoint[0];
            pC[1] = pR->ownCoords.pos[1];
            pC[2] = pR->arc2d.closestPoint[1];
            mat4x4_mul_vec4screen(p1, theApp.mainCamera.mViewProjection, pC, theApp.mainCamera.targetRads, theApp.mainCamera.nearClip, theApp.mainCamera.farClip);
            ScreenLine::addLine2queue(p0, p1, MyColor::getUint32(0, 0, 255), 3, true);
        }
        */
    }
    return dist;
}
int Rail::sizeModeModelN(Rail* pSS, int needSizeMode) {
    int modelN = -1;
    if (pSS->railType == 0) {//0-straight, 1-curve)
        if (pSS->railLength == railLenghtLong) {
            if (needSizeMode == -2)
                modelN = railModelStraightNtiny;
            else if (needSizeMode == -1)
                modelN = railModelStraightNsmall;
            else
                modelN = railModelStraightN;
        }
        else { //short
            if (needSizeMode == -2)
                modelN = railModelShortNtiny;
            else if (needSizeMode == -1)
                modelN = railModelShortNsmall;
            else
                modelN = railModelShortN;
        }
    }
    else { //curve
        if (needSizeMode == -2)
            modelN = railModelCurvedNtiny;
        else if (needSizeMode == -1)
            modelN = railModelCurvedNsmall;
        else
            modelN = railModelCurvedN;
    }
    return modelN;
}
int Rail::adjustModel4size(Rail* pSS, float sizeUnitPixelsSize) {
    int needSizeMode = 0;
    if (sizeUnitPixelsSize * Rail::railWidth < 1)
        needSizeMode = -2;
    else if (sizeUnitPixelsSize * Rail::railWidth < 3)
        needSizeMode = -1;
    if (pSS->sizeMode == needSizeMode)
        return 0;    
    pSS->sizeMode = needSizeMode;
    int modelN = sizeModeModelN(pSS, needSizeMode);
    Rail* pR0 = (Rail*)railModels.at(modelN);
    pSS->djStartN = pR0->djStartN;
    pSS->djTotalN = pR0->djTotalN;
    pSS->mt0isSet = 0;    
    return 1;
}
int Rail::convertRail(Rail* pR) {
    //remove from table
    pR->pSubjsSet->at(pR->nInSubjsSet) = NULL;
    if (pR->railStatus == 0) { //real rail - remove
        //release neighbours
        if (pR->crossingRailN >= 0) {
            Rail* pR2 = (Rail*)pR->pSubjsSet->at(pR->crossingRailN);
            pR2->crossingRailN = -1;
        }
        releaseConnectedRails(pR, 0);
        releaseConnectedRails(pR, 1);
        theApp.gameTable.trimTable();
    }
    else {
        //virtual rail - add real one
        theApp.gameTable.setRail(pR->tileIndexXZ[0], pR->tileIndexXZ[1], pR->yawInOut[0], pR->yawInOut[1], 0);
    }
    theApp.gameTable.addVirtualRails();

    TouchScreen::abandonSelected();

    if (pR->railStatus == 0)//real removed
        MySound::playSound(MySound::soundNrailout, pR);
    else //rail added
        MySound::playSound(MySound::soundNrailin, pR);
    //delete original object
    delete pR;
    return 1;
}
int Rail::renderRailEnd(Rail* pR, int endN, Camera* pCam, float* dirToMainLight) {
    RailEnd* pRE = &pR->railEnd[endN];
    if (pRE->isSwitch == false)
        return 0;

    if (pRE->isBusy==false) {
        //check alt rail end
        Rail* pR2 = (Rail*)pR->pSubjsSet->at(pRE->altRailN);
        RailEnd* pRE2 = &pR2->railEnd[pRE->altRailEndN];
        pRE->isBusy = pRE2->isBusy;
    }

    int modelN;
    if (pRE->isOn) {
        if (pR->railType == 0)
            modelN = railModelGuideStraightN;
        else
            modelN = railModelGuideCurvedN;
    }
    else {//off
        if (pR->railType == 0)
            modelN = railModelGuideBedStraightN;
        else
            modelN = railModelGuideBedCurvedN;
    }
    SceneSubj* pS = railModels.at(modelN);


    if (pRE->highlight > 0) {
        if(pRE->isBusy)
            pS->setHighLight(0.5, MyColor::getUint32(255, 0, 0));
        else
            pS->setHighLight(0.5, MyColor::getUint32(255, 255, 255));
        //blinking?
        if (TouchScreen::cursorStatus == 1) {
            int framesOn = 5;
            int framesOff = framesOn;
            int framesCircle = framesOn + framesOff;
            int circleFrameN = theApp.frameN % framesCircle;
            if (circleFrameN < framesOff)
                pS->setHighLight(0);
        }
    }

    //find position
    float progress = railGuideLength / 2 / pR->railLength;
    RailCoord rc;
    rc.railN = pR->nInSubjsSet;
    if (endN == 0)
        rc.percentOnRail = progress;
    else
        rc.percentOnRail = 1.0 - progress;
    rc.alignedWithRail = -pR->CCW;
    theApp.gameTable.fillRailCoordsFromPercent(&rc, pR);

    v3copy(pS->ownCoords.pos, rc.xyz);
    pS->ownCoords.setEulerDg(0,rc.currentYaw,0);
    pS->buildModelMatrix();

    pS->tremble=pRE->tremble;
    pRE->tremble = trembleFade(pRE->tremble);

    pS->render(pCam, dirToMainLight, NULL, 0, false);

    pRE->highlight = 0;
    pS->setHighLight(0);
    return 1;
}

int Rail::renderRail(Rail* pSS, Camera* pCam, float* dirToMainLight, float* dirToTranslucent, int renderFilter, bool forDepthMap) {
    if (TouchScreen::pSelected == pSS) {
        if (pSS->railStatus == 1) {
            //virtual rail
            if (TouchScreen::cursorStatus == 0)
                pSS->highlight = 1;
        }
        if (pSS->railStatus == 0) {//real rail only
            if (pSS->switchEndN >= 0)
            if (pSS->switchSelected == false)
                {//rail has a switch
                if(theApp.bEditMode==false)
                    pSS->switchSelected = true;
                else {
                    RailEnd* pRE = &pSS->railEnd[pSS->switchEndN];
                    if(pRE->highlight>0)
                        pSS->switchSelected = true;
                    else {
                        float tablePoint[4];
                        TheTable::getCursorAncorPointTable(tablePoint, TouchScreen::cursorPos, &theApp.gameTable);
                        float d2end0 = v3lengthFromToXZ(tablePoint, pSS->p0);
                        float d2end1 = v3lengthFromToXZ(tablePoint, pSS->p1);
                        float d2switch = d2end0;
                        float d2opposite = d2end1;
                        if (pSS->switchEndN == 1) {
                            d2switch = d2end1;
                            d2opposite = d2end0;
                        }
                        if (d2switch < d2opposite)
                            pSS->switchSelected = true;
                    }
                }
            }
            if (pSS->switchSelected) {
                //hightlight switch on both rails
                RailEnd* pRE = &pSS->railEnd[pSS->switchEndN];
                pRE->highlight = 1;
                Rail* pR2 = (Rail*)pSS->pSubjsSet->at(pRE->altRailN);
                RailEnd* pRE2 = &pR2->railEnd[pRE->altRailEndN];
                pRE2->highlight = 1;
            }
            else if (TouchScreen::cursorStatus == 0) {//hover
                if(pSS->isBusy)
                    pSS->setHighLight(0.4, MyColor::getUint32(255, 0, 0));
                else
                    pSS->setHighLight(0.4, MyColor::getUint32(255, 255, 255));
            }
        }

        //blinking?
        if (TouchScreen::cursorStatus == 1) {
            if(pSS->switchSelected == false)
            if (theApp.bEditMode) {
                //pressing on this rail
                int framesOn = 5;
                int framesOff = framesOn;
                int framesCircle = framesOn + framesOff;
                int circleFrameN = theApp.frameN % framesCircle;
                if (circleFrameN < framesOff)
                    return 0;
                Rail r1;
                memcpy(&r1, pSS, sizeof(Rail));
                pSS = &r1;
                pSS->railStatus = 0;
                pSS->sizeMode = 22;
                pSS->ownCoords.pos[1] = Rail::railsLevel;
                pSS->buildModelMatrix();
                pSS->uHighLightLevel = 0;
            }
        }
    }
    if (pSS->highlight > 0) {
        float highLightLevel = 0.9;
        if (pSS->railStatus == 0)//real rail
            highLightLevel = 0.5;
        if(pSS->isBusy)
            pSS->setHighLight(highLightLevel, MyColor::getUint32(255,0,0));
        else
        pSS->setHighLight(highLightLevel, MyColor::getUint32(255,255,255));
    }

    mat4x4 mMVP;
    mat4x4 mMVP4dm;
    float sizeUnitPixelsSize;
    renderStandard_prepare(pSS, pCam, dirToMainLight, dirToTranslucent, mMVP, mMVP4dm, &sizeUnitPixelsSize, forDepthMap);

    if (pSS->railStatus == 0) {//real rails only, not virtual
        adjustModel4size(pSS, sizeUnitPixelsSize);
    }
    renderStandard_execute(pSS, mMVP, mMVP4dm, pCam, dirToMainLight, dirToTranslucent, sizeUnitPixelsSize, renderFilter, forDepthMap);
    
    if (pSS->switchSelected)
        pSS->switchWaits4click = true;
    else
        pSS->switchWaits4click = false;
    pSS->highlight = 0;
    pSS->setHighLight(0);
    pSS->switchSelected = false;

    if (!forDepthMap) {
        renderRailEnd(pSS, 0, pCam, dirToMainLight);
        renderRailEnd(pSS, 1, pCam, dirToMainLight);
        //render crossing?
        if (pSS->crossingRailN >= 0) {
            int modelN = railModelGuideShortStraightN;
            if (pSS->railType > 0)
                modelN = railModelGuideShortCurvedN;
            SceneSubj* pS = railModels.at(modelN);
            v3copy(pS->ownCoords.pos, pSS->crossingPoint);
            pS->ownCoords.setEulerDg(0, pSS->crossingYaw, 0);
            pS->buildModelMatrix();
            pS->render(pCam, dirToMainLight, NULL, 0, false);
        }
    }

    return 1;
}
int Rail::onLeftButtonUp() {
    Rail* pR = this;
    onFocusOut(); 
    if (switchWaits4click) {
        //check if rail end is busy
        RailEnd* pRE = &pR->railEnd[pR->switchEndN];
        if (pRE->isBusy) {
            MySound::playSound(MySound::soundNfart01, pR, false);
            MeltingSign::addMeltingSign("/dt/ui/signs/noway.png", TouchScreen::cursorPos[0], TouchScreen::cursorPos[1]);
            return 0;
        }
        return switchRail(pR);
    }
    else {
        if (pR->isBusy) {
            MySound::playSound(MySound::soundNfart01, pR, false);
            MeltingSign::addMeltingSign("/dt/ui/signs/noway.png", TouchScreen::cursorPos[0], TouchScreen::cursorPos[1]);
            return 0;
        }
        return convertRail(pR);
    }
}
int Rail::switchRail(Rail* pR) {
    if (pR->switchEndN < 0)
        return 0;
    RailEnd* pRE = &pR->railEnd[pR->switchEndN];
    Rail* pR2 = (Rail*)pR->pSubjsSet->at(pRE->altRailN);
    RailEnd* pRE2 = &pR2->railEnd[pRE->altRailEndN];
    pRE->isOn = pRE2->isOn;
    pRE2->isOn = !pRE->isOn;
    float trembleAmp = 0.5;
    if (pRE->isOn)
        pRE->tremble = trembleAmp;
    else
        pRE2->tremble = trembleAmp;
    MySound::playSound(MySound::soundNrailswitch, pR);
    TouchScreen::abandonSelected();
    return 1;
}
int Rail::releaseConnectedRails(Rail* pR, int endN) {
    RailEnd* pRE = &pR->railEnd[endN];
    //release connected rail end first
    if (pRE->toRailN >= 0) {
        Rail* pR2 = (Rail*)pR->pSubjsSet->at(pRE->toRailN);
        RailEnd* pRE2 = &pR2->railEnd[pRE->toRailEndN];
        if (pRE->isSwitch) { //redirect connected rail to my alt rail
            Rail* pR2 = (Rail*)pR->pSubjsSet->at(pRE->toRailN);
            RailEnd* pRE2 = &pR2->railEnd[pRE->toRailEndN];
            pRE2->toRailN = pRE->altRailN;
            pRE2->toRailEndN = pRE->altRailEndN;
        }
        else //or release end
            pRE2->toRailN = -1;
        if (pRE2->isSwitch) { //populate to alt rail end too
            Rail* pR3 = (Rail*)pR->pSubjsSet->at(pRE2->altRailN);
            RailEnd* pRE3 = &pR3->railEnd[pRE2->altRailEndN];
            pRE3->toRailN = pRE2->toRailN;
            pRE3->toRailEndN = pRE2->toRailEndN;
        }

        //if pR2 - short diagonal
        if (pR2->railLength == Rail::railLenghtShort) {
            //check both ends if connected
            pRE2 = &pR2->railEnd[0];
            if (pRE2->toRailN < 0) {
                pRE2 = &pR2->railEnd[1];
                if (pRE2->toRailN < 0) {
                    //both ends are free - delete rail
                    if (pR2->crossingRailN >= 0) {
                        //release crossing diagonal
                        Rail* pR3 = (Rail*)pR->pSubjsSet->at(pR2->crossingRailN);
                        pR3->crossingRailN = -1;
                    }
                    pR2->pSubjsSet->at(pR2->nInSubjsSet) = NULL;
                    delete pR2;
                }
            }
        }
    }
    
    //release own alt rail?
    if (pRE->isSwitch) {
        Rail* pR2 = (Rail*)pR->pSubjsSet->at(pRE->altRailN);
        RailEnd* pRE2 = &pR2->railEnd[pRE->altRailEndN];
        pRE2->isSwitch = false;
        pRE2->isOn = false;
        pRE2->altRailN = -1;
        pR2->switchEndN = -1;
    }
    
    return 1;
}


