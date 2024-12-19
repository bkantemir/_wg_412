#pragma once
#include "Coords.h"
#include "Camera.h"
#include "MaterialAdjust.h"
#include <string>
#include <vector>
#include "DrawJob.h"
#include "geom/Gabarites.h"
#include "geom/Chorda.h"
#include "Shader.h"
#include "ScreenSubj.h"

class SceneSubj : public ScreenSubj
{
public:
    std::vector<DrawJob*>* pDrawJobs = NULL; //which vector/set this subj belongs to
    std::vector<SceneSubj*>* pSubjsSet = NULL; //which vector/set this subj belongs to
    int nInSubjsSet = 0; //subj's number in pSubjsSet
    int rootN = 0; //model's root N
    int d2parent = 0; //shift to parent object
    int d2headTo = 0; //shift to headTo object
    bool place2middle = false; // used with headTo only
    float posOnHeadTo[4] = { 0,0,0,0 };
    SceneSubj* pStickTo = NULL;
    int totalElements = 0; //elements N in the model
    int totalNativeElements = 0; //elements N in the model when initially loaded
    char source256[256] = "";
    char source256root[256] = "";
    Coords absCoords;
    Coords absCoordsPrev;
    mat4x4 absModelMatrixUnscaled = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };
    mat4x4 absModelMatrix = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };
    mat4x4 ownModelMatrixUnscaled = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };
    uint32_t absCoordsUpdateFrameN = -1;

    Coords ownCoords;
    Coords ownSpeed;
    //Coords desirableSpeed;
    float desirableZdir = 0;
    float desirableZspeed = 0;
    float ownZspeed = 0;
    float accelerationLin = 0.5;
    float accelerationAng = 0.5;

    float scale[3] = { 1,1,1 };
    int djStartN = 0; //first DJ N in DJs array (DrawJob::drawJobs)
    int djTotalN = 0; //number of DJs
    Material mt0;
    Material mt0Layer2; //second layer
    int mt0isSet = 0;
    int dropsShadow = 1;

    float zOffsetFromRoot = 0;
    int alignedWithRoot = 1;//pr -1

    char fileOnDeploy[256] = "";
    std::vector<MyColor*>* pCustomColors = NULL;
    std::vector<MaterialAdjust*>* pCustomMaterials = NULL;

    Gabarites gabaritesOnLoad;
    Gabarites gabaritesWorld;
    Gabarites gabaritesWorldAssembly;
    //Gabarites gabaritesOnScreen;
    Chorda chordaCollision;
    Chorda chordaScreen;

    int renderOrder = -1; //-1-unknown, 0-render 1st (no alpha or b/w), 1-render last (has alpa), 2-render last (disordered mixed alpha)

    float lever = 0;
    int renderMirrorAxis = -1; //-1 - no mirroring
    int cullFace=GL_BACK; //or GL_FRONT

    float uHighLightLevel = 0.0;
    MyColor uHighLightColor;

    bool bilboard = false;

    float tremble = 0;

    static SceneSubj* pSelectedSceneSubj;
    static SceneSubj* pSelectedSceneSubj00;

private:
    int hide = 0;//1-hide, 2-hide forever (delete)

public:
    SceneSubj();
    SceneSubj(SceneSubj* pSS0) {
        memcpy((void*)this, (void*)pSS0, sizeof(SceneSubj)); 
    };
    virtual SceneSubj* clone() { 
        if (strcmp(this->className, "") != 0 && strcmp(this->className, "SceneSubj") != 0)
            return NULL; 
        return new SceneSubj(this); 
    };
    virtual ~SceneSubj();
    virtual void buildModelMatrix() { buildModelMatrixStandard(this); };
    static void buildModelMatrixStandard(SceneSubj* pGS);
    virtual int moveSubj() { return moveStandard(this); };
    static int moveStandard(SceneSubj* pGS);
    virtual int applySpeeds() { return applySpeedsStandard(this); };
    static int applySpeedsStandard(SceneSubj* pGS);
    virtual int deleteMe(bool withChilds=true);
    static int deleteMeSimple(SceneSubj* pSS);
    virtual int render(Camera* pCam, float* dirToMainLight, float* dirToTranslucent, int renderFilter,bool forDepthMap) {
        return renderStandard(this, pCam, dirToMainLight, dirToTranslucent, renderFilter, forDepthMap);
    };
    static int renderStandard(SceneSubj* pGS, Camera* pCam, float* dirToMainLight, float* dirToTranslucent, int renderFilter, bool forDepthMap);
    static float getUnitPixelsSize(SceneSubj* pSS, Camera* pCam);
    static int renderStandard_prepare(SceneSubj* pSS, Camera* pCam, float* dirToMainLight, float* dirToTranslucent,
        mat4x4 mMVP, mat4x4 mMVP4dm, float* pSizeUnitPixelsSize, bool forDepthMap);
    static int renderStandard_execute(SceneSubj* pSS, mat4x4 mMVP, mat4x4 mMVP4dm, Camera* pCam, float* dirToMainLight,
        float* dirToTranslucent, float sizeUnitPixelsSize, int renderFilter, bool forDepthMap);
    static int executeDJstandard2(DrawJob* pDJ, float* uMVP, float* uMV3x3, float* uMM, float* uMVP4dm, 
        float* uVectorToLight, float* uVectorToTranslucent, float* uCameraPosition, float line1pixSize, Material* pMt,int shaderN, 
        float uFogLevel = 0, unsigned int uFogColor32 = 0);

    virtual int scaleMe(float k) { return scaleStandard(this, k); };
    static int scaleStandard(SceneSubj* pGS, float k);
    static void deployModel(SceneSubj* pSS, std::string tagStr);
    virtual int onDeploy(std::string tagStr) { return onDeployStandatd(this, tagStr); };
    static int onDeployStandatd(SceneSubj* pSS, std::string tagStr);
    virtual int onLoad(std::string tagStr) { return onLoadStandard(this, tagStr); };
    static int onLoadStandard(SceneSubj* pSS, std::string tagStr);
    virtual int readClassProps(std::string tagStr) { return readClassPropsSceneSubj(this, tagStr); };
    static int readClassPropsSceneSubj(SceneSubj* pSS,std::string tagStr);
    virtual int processSubj() { return processSubjStandard(this); };
    static int processSubjStandard(SceneSubj* pGS);

    static int addToRenderQueue(std::vector<SceneSubj*>* pQueueOpaque, std::vector<SceneSubj*>* pQueueTransparent, std::vector<SceneSubj*>* pSubjs);
    static int sortRenderQueue(std::vector<SceneSubj*>* pQueue, int direction); //0-closer 1st, 1-farther 1st

    static int customizeMaterial(Material** ppMt, Material** ppMt2, Material** ppAltMt, Material** ppAltMt2, SceneSubj* pSS);
    static bool lineWidthIsImportant(int primitiveType);
    static int executeDJcommon(DrawJob* pDJ, Shader* pShader,
        float* uVectorToLight, float* uVectorToTranslucent, float* uCameraPosition, Material* pMt, float uFogLevel=0, unsigned int uFogColor32=0);
    void setHighLight(float highLightlevel, unsigned int uHighLightColor32 = 0) { setHighLight(this,highLightlevel, uHighLightColor32); };
    static void setHighLight(SceneSubj* pSS, float highLightlevel, unsigned int uHighLightColor32 = 0);
    static std::vector<int>* entireFamily(SceneSubj* pSS);
    static std::vector<int>* allChilds(SceneSubj* pSS);
    void setHide(int val) { setHide(this, val); };
    static void setHide(SceneSubj* pSS, int val);
    int hidden() { return this->hide; };
    void resetRoot(int rootN = -1) { resetRoot(this,rootN); };
    static void resetRoot(SceneSubj* pSS, int rootN=-1);
    static int checkCollisions(std::vector<std::vector<SceneSubj*>*> pSubjArraysPairs4collisionDetection);
    static float checkMacroObjsCollision(SceneSubj* pS1, SceneSubj* pS2);
    virtual bool ignoreCollision(SceneSubj* pS2) { return false; };
    virtual int processCollision(float penetrationDepth, SceneSubj* pS2, float* collisionPointWorld, float* hitPointNormal, 
        float* hitSpeed) {
        return processCollisionStandard(penetrationDepth,this, pS2, collisionPointWorld, hitPointNormal, hitSpeed);
    };
    static int processCollisionStandard(float penetrationDepth,SceneSubj* pS1, SceneSubj* pS2, float* collisionPointWorld, 
        float* hitPointNormal, float* hitSpeed);
    static float checkSingleSubjsCollision2D(SceneSubj* pS1, SceneSubj* pS2);

    SceneSubj* getParent() { return getParent(this); };
    static SceneSubj* getParent(SceneSubj* pSS);
    SceneSubj* getRoot() { return getRoot(this); };
    static SceneSubj* getRoot(SceneSubj* pSS);
    static int newId();
    static int subj2log(SceneSubj* pSS);
    static int assemblyGabaritesPrepareRoot(SceneSubj* pSS);
    static int assemblyGabaritesUpdateRoot(SceneSubj* pSS);
    static int singleDraw3D(SceneSubj* pSS00);


    static SceneSubj* copyModel(std::vector<SceneSubj*>* pToSet, SceneSubj* pModel, bool nativeOnly = true);
    static SceneSubj* copyModel(std::vector<SceneSubj*>* pToSet, std::vector<SceneSubj* >* pFrom, int modelN, bool nativeOnly = true) {
        return copyModel(pToSet, pFrom->at(modelN), nativeOnly = true); };
    static int findSlotIn(std::vector<SceneSubj*>* pSet, int modelSize=1, int searchFrom=0);
    static int fillWorldGabarites(SceneSubj* pS);
    static int fillScreenGabarites(SceneSubj* pS);
    static int initGabarites(SceneSubj* pS);

    virtual int onInput() { return onInputStandard(this); };
    static int onInputStandard(SceneSubj* pSS);
    virtual bool isClickable() { if (d2parent == 0) return true; else return false; };
    virtual bool isDraggable() { return false; };
    virtual bool isResponsive() { return (isClickable() || isDraggable()); };

    virtual int onDrag() { return 1; };
    virtual int onClick() { onFocusOut();  cameraMan_zoomInOut(this); return 1;};//tremble = 5; return 1; };// 
    virtual int onLeftButtonUp(){ onFocusOut(); return 1; };
    virtual int onFocusOut();
    virtual int onFocus() { setHighLight(0.2, MyColor::getUint32(1.0f, 1.0f, 1.0f)); return 1; };
    virtual int onLeftButtonDown();
    static int cameraMan_zoomInOut(SceneSubj* pS);
    static int getCursorAncorPointStandard(float* ancorPoint, float* cursorPos, SceneSubj* pSS);
    virtual SceneSubj* getResponsiveSubj() {return getResponsiveSceneSubj(this);};
    static SceneSubj* getResponsiveSceneSubj(SceneSubj* pSS);
    static SceneSubj* pointerOnSceneSubj00(std::vector<SceneSubj*>* pSubjs);
    static SceneSubj* pickFromCandidates(SceneSubj* pCandidate1, SceneSubj* pCandidate2);
    static SceneSubj* updateSelected00(SceneSubj* pCandidate00);
    static float trembleFade(float tremble);
};


