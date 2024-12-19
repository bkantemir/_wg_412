#include "TheApp.h"
#include "platform.h"
#include "utils.h"
#include "linmath.h"
#include "Texture.h"
#include "DrawJob.h"
#include "ModelBuilder.h"
#include "TexCoords.h"
#include "Shadows.h"
#include "ProgressBar.h"
//#include "gltf/GLTFparser.h"
//#include "gltf/GLTFskinShader.h"
#include "model_car/CarWheel.h"
#include "rr/RollingStock.h"
#include "rr/WheelPair.h"
#include "rr/Gear.h"
#include "rr/Bell.h"
#include "rr/Whistle.h"
#include "rr/Coupler.h"
#include "rr/Gangway.h"
#include "rr/RRUI.h"
#include "rr/ModelLoaderRR.h"
#include "rr/ExhaustLoco.h"
#include "subjs/Exhaust.h"
#include "subjs/DirLight.h"
#include "subjs/Interior.h"
#include "subjs/Fan.h"
#include "subjs/Smoke.h"
#include "MySound.h"
#include "CameraMan.h"
#include "ScreenLine.h"


extern std::string filesRoot;
extern float degrees2radians;

Camera TheApp::collisionCamera;

int TheApp::getReady() {
    cleanUp();

    Shader::loadBasicShaders();
    UISubj::init();

    ProgressBar* pPBar = new ProgressBar(filesRoot + "/dt/cfg/progressbar00.bin");

    bPause = false;
    bExitApp = false;
    frameN = 0;
    Camera::setCollisionCamera(&collisionCamera);// , & gameTable.worldBox);

    Shader::loadShaders(pPBar);
    //GLTFskinShader::loadGLTFshaders(pPBar);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);//default
    glFrontFace(GL_CCW);//default

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_TRUE);

    std::vector<SceneSubj*>* pSubjs = &sceneSubjs;

    int subjN = 0;
    SceneSubj* pSS = NULL;

    gameTable.reInitLayout(&gameTable, 35,false);
    gameTable.basicOval6x6();

    mylog("basicOval: %d\n",gameTable.railsMap.size());

    gameTable.addVirtualRails();

    mylog("VirtualRails: %d\n", gameTable.railsMap.size());

    {
        std::vector<std::string> locoOpts;
        
        locoOpts.push_back("/dt/md/rr/us/lc/999_0-6-0dockside/01reading/root01.txt");
        locoOpts.push_back("/dt/md/rr/us/lc/999_0-6-0dockside/02atsf/root01.txt");
        /*
        locoOpts.push_back("/dt/md/rr/us/lc/998_alc-42/300blue/root01.txt");
        locoOpts.push_back("/dt/md/rr/us/lc/998_alc-42/301black/root01.txt");
        locoOpts.push_back("/dt/md/rr/us/lc/997_sc-44/2111surf/root01.txt");
        locoOpts.push_back("/dt/md/rr/us/lc/997_sc-44/1401wsdot/root01.txt");
        */
        subjN = ModelLoaderRR::loadModelRR(pSubjs, NULL, NULL, locoOpts.at(getRandom(0, locoOpts.size() - 1)), "", "RollingStock", "", pPBar);
        pSS = pSubjs->at(subjN);
        gameTable.placeAt(pSS->ownCoords.pos, getRandom(0.0f, (float)gameTable.tableTiles[0]), gameTable.groundLevel0, getRandom(0.0f, (float)gameTable.tableTiles[1]));//Rail::railsLevel
        //mylog("RollingStock at %dx%d\n", (int)pSS->ownCoords.pos[0], (int)pSS->ownCoords.pos[2]);
        pSS->ownCoords.setEulerDg(0, getRandom(0.0f, 360.0f), 0);
        pSS->deployModel(pSS, "");
        pSS->desirableZdir = 1;
        if (getRandom(0, 1) == 0)
            pSS->desirableZdir *= -1;
        RollingStock* pLoco = (RollingStock*)pSS;
        pLoco->activeLoco = true;
        RollingStock::reinspectTrain(pLoco->pCouplerFront);
    }

    std::vector<std::string> wagonOpts;

    wagonOpts.push_back("/dt/md/rr/us/fr/998_box-sigar/01dutch/root01.txt");
    
    wagonOpts.push_back("/dt/md/rr/us/fr/999_tank3dome/01gulf/root01.txt");
    wagonOpts.push_back("/dt/md/rr/us/fr/999_tank3dome/02goodyear/root01.txt");
    wagonOpts.push_back("/dt/md/rr/us/fr/999_tank3dome/03protex/root01.txt");
    
    /*
    wagonOpts.push_back("/dt/md/rr/us/ps/996_round/82666coach/root01.txt");  
    wagonOpts.push_back("/dt/md/rr/us/ps/997_surfliner/6906surf-rear/root01.txt");
    wagonOpts.push_back("/dt/md/rr/us/ps/997_surfliner/6402surf-coach/root01.txt");
    wagonOpts.push_back("/dt/md/rr/us/ps/998_romb/61017baggage-amtrak/root01.txt");
    wagonOpts.push_back("/dt/md/rr/us/ps/999_superliner/39029transition/root01.txt");
    wagonOpts.push_back("/dt/md/rr/us/ps/999_superliner/33023lounge/root01.txt");
    wagonOpts.push_back("/dt/md/rr/us/ps/999_superliner/38016dining/root01.txt");
    wagonOpts.push_back("/dt/md/rr/us/ps/999_superliner/34058coach/root01.txt");
    */

    int totalWagons = 5;// 7;
    for (int i = 0; i < totalWagons; i++) {
        subjN = ModelLoaderRR::loadModelRR(pSubjs, NULL, NULL, wagonOpts.at(getRandom(0, wagonOpts.size() - 1)), "", "RollingStock", "", pPBar);
        pSS = pSubjs->at(subjN);
        gameTable.placeAt(pSS->ownCoords.pos, getRandom(0.0f, (float)gameTable.tableTiles[0]), gameTable.groundLevel0, getRandom(0.0f, (float)gameTable.tableTiles[1]));//Rail::railsLevel
        pSS->ownCoords.setEulerDg(0, getRandom(0.0f, 360.0f), 0);
        pSS->deployModel(pSS, "coupleTo=back");
        /*
        pSS->ownSpeed.pos[2] = 1;
        if (getRandom(0, 1) == 0)
            pSS->ownSpeed.pos[2] = -1;
            */
    }


    if(totalWagons>2)
        gameTable.reInitLayout(&gameTable, pSubjs);
    else
        gameTable.reInitLayout(&gameTable, 60,false);// 35);// pSubjs);

    //pSS->setHighLight(0.2, MyColor::getUint32(1.0f, 0.0f, 1.0f));

    //showHierarchy(pSS);

    /*
    pSubjs = &staticSubjs;

    std::vector<std::string> carOpts;
    carOpts.push_back("/dt/md/cars/us/999_1935_deusenberg_ssj/root01.txt");
    carOpts.push_back("/dt/md/cars/us/998_1929_deusenberg_j/root01.txt");

    float scale = 64.0f / 150;
    scale *= 1.1;
    subjN = ModelLoader::loadModelStandard(pSubjs, NULL, NULL, carOpts.at(getRandom(0, carOpts.size() - 1)), "", "", "", pPBar);
    pSS = pSubjs->at(subjN);
    pSS->scaleMe(scale);
    pSS->ownCoords.setEulerDg(0, getRandom(0.0f, 360.0f), 0);
    pSS->deployModel(pSS, "tableAt='3.5,2.5'");

    subjN = ModelLoader::loadModelStandard(pSubjs, NULL, NULL, carOpts.at(getRandom(0, carOpts.size() - 1)), "", "", "", pPBar);
    pSS = pSubjs->at(subjN);
    pSS->scaleMe(scale);
    pSS->ownCoords.setEulerDg(0, getRandom(0.0f, 360.0f), 0);
    pSS->deployModel(pSS, "tableAt='2.5,2.5'");

    subjN = ModelLoader::loadModelStandard(pSubjs, NULL, NULL, carOpts.at(getRandom(0, carOpts.size() - 1)), "", "", "", pPBar);
    pSS = pSubjs->at(subjN);
    pSS->scaleMe(scale);
    pSS->ownCoords.setEulerDg(0, getRandom(0.0f, 360.0f), 0);
    pSS->deployModel(pSS, "tableAt='1,4'");

    subjN = ModelLoader::loadModelStandard(pSubjs, NULL, NULL, "/dt/md/misc/us/999_marlboro01red/root00.txt", "", "", "", pPBar);
    pSS = pSubjs->at(subjN);
    pSS->ownCoords.setEulerDg(0, getRandom(0, 360), 0);
    pSS->deployModel(pSS, "tableAt='1.5,2' align=bottom");
    */
    //=============arrays to process
    pSubjArrays2process.push_back(&sceneSubjs);
    pSubjArrays2process.push_back(&clouds);

    pSubjArrays2draw.push_back(&gameTable.tableParts);
    pSubjArrays2draw.push_back(&gameTable.railsMap);
    pSubjArrays2draw.push_back(&sceneSubjs);
    pSubjArrays2draw.push_back(&staticSubjs);
    pSubjArrays2draw.push_back(&clouds);

    pSubjArraysPairs4collisionDetection.push_back(&sceneSubjs);
    pSubjArraysPairs4collisionDetection.push_back(&sceneSubjs);
    //===== set up light
    v3set(dirToMainLight, getRandom(-1.f, 1.f), 1.f, getRandom(0.f, 1.f));
    //v3set(dirToMainLight, 0.4,0.7,0.6);//0.3,0.7,0.7 //-0.7,0.7,0.3
    v3norm(dirToMainLight);

    pPBar->nextStep(pPBar);
    Smoke::init();
    pPBar->nextStep(pPBar);
    Exhaust::init();
    pPBar->nextStep(pPBar);

    Shadows::init();
    pPBar->nextStep(pPBar);

    //===== set up MAIN camera
    myPollEvents();
    mainCamera.ownCoords.setEulerDg(Camera::pitchDefault, 165, 0); //set camera angles/orientation
    mat4x4_from_quat(mainCamera.ownCoords.rotationMatrix, mainCamera.ownCoords.getRotationQuat());
    mainCamera.viewRangeDg = 30;
    v3set(mainCamera.lookAtPoint, 14, -18, 10);
    float stageSizeX = gameTable.suggestedStageSize();

    mainCamera.focusDistance = Camera::getDistance4stage(&mainCamera, stageSizeX);

    Camera::refocus2ground(&mainCamera);

    Camera::reset(&mainCamera, &gameTable.worldBox);

    pPBar->nextStep(pPBar);
    
    MySound::init(pPBar);
    
    pPBar->nextStep(pPBar);

    delete pPBar;
    UISubj::clear();

    //======= set UI
    //UISubj::init();
    //UISubj::addClrSubj("test", 10, 10, 100, 60, "top left", MyColor::getUint32(1.0f, 0.0f, 0.0f, 1.0f));
    RRUI::initPlayScreen();

    return 1;
}

int TheApp::drawFrame() {
    /*
    if(frameN==10){//test
        SceneSubj ss;
        v3set(ss.scale, 2, 3, 4);
        v3set(ss.ownCoords.pos, 20, 30, 40);
        float pitch = 12;
        float yaw = 13;
        float roll = 14;
        ss.ownCoords.setEulerDg(pitch,yaw,roll);
        ss.buildModelMatrix();
        mat4x4 mMVP;
        mat4x4_mul(mMVP, mainCamera.mViewProjection, ss.absModelMatrix);
        mylog_mat4x4("before:", mMVP);

        float dir[3];
        for (int i = 0; i < 3; i++)
            dir[i] = mainCamera.ownCoords.pos[i] - ss.absCoords.pos[i];
        pitch = v3pitchDg(dir);
        yaw = v3yawDg(dir);
        ss.ownCoords.setEulerDg(pitch, yaw, roll);
        ss.buildModelMatrix();
        mat4x4_mul(mMVP, mainCamera.mViewProjection, ss.absModelMatrix);
        mylog_mat4x4("after:", mMVP);
    }
    */

    checkGameStatus();

    myPollEvents(); 
    TouchScreen::getInput();
    /*
    {//debug
        int n = 0;
        for (int i = clouds.size() - 1; i >= 0; i--) {
            SceneSubj* pS = clouds.at(i);
            if (pS != NULL)
                n++;
        }
        mylog("%d clouds %d of %d\n",(int)frameN, n, clouds.size());
    }
    */
    if (!bPause) {
        MySound::checkSound();
        gameTable.freeRails();
        //scan subjects
        for (int arrayN = 0; arrayN < pSubjArrays2process.size(); arrayN++) {
            std::vector<SceneSubj*>* pSubjs = pSubjArrays2process.at(arrayN);
            int subjsN = pSubjs->size();
            for (int subjN = 0; subjN < subjsN; subjN++) {
                SceneSubj* pSS = pSubjs->at(subjN);
                if (pSS == NULL)
                    continue;

                //re-check root
                if (pSS->d2parent == 0)
                    pSS->rootN = pSS->nInSubjsSet;
                else {//is a child
                    SceneSubj* pParent = pSS->getParent();
                    pSS->rootN = pParent->rootN;
                    if (pSS->tremble < pParent->tremble)
                        pSS->tremble = pParent->tremble;
                }
                pSS->tremble = SceneSubj::trembleFade(pSS->tremble);

                if (pSS->processSubj() < 1)
                    continue;

                if (pSS->d2parent == 0)
                    pSS->assemblyGabaritesPrepareRoot(pSS);

                if (pSS->hidden() > 0)
                    continue;
                if (pSS->djTotalN < 1)
                    continue;

                //prepare for fit screen check and for collision detection
                if (pSS->pSubjsSet != &staticSubjs) {
                    SceneSubj::fillWorldGabarites(pSS);
                    pSS->assemblyGabaritesUpdateRoot(pSS);
                }
            }
        }
        if (frameN > 0) {
            RollingStock::checkTrains(&sceneSubjs);
            SceneSubj::checkCollisions(pSubjArraysPairs4collisionDetection);
            Coupler::drawCouplerButtons(&sceneSubjs);

        }
    }

    cameraMan.process();

    //translucent light
    float vecToLightInEyeSpace[4];
    mat4x4_mul_vec4plus(vecToLightInEyeSpace, (vec4*)mainCamera.mViewProjection, dirToMainLight, 0);
    if (vecToLightInEyeSpace[2] <= 0) { //light from behind
        for (int i = 0; i < 3; i++) {
            if (i == 2)
                dirToTranslucent[i] = dirToMainLight[i];
            else
                dirToTranslucent[i] = -dirToMainLight[i];
        }
    }
    else { //light to eyes
        for (int i = 0; i < 3; i++) {
            dirToTranslucent[i] = -dirToMainLight[i];
        }
    }
    dirToTranslucent[2] *= 0.5;
    v3norm(dirToTranslucent);

    float dir2camera[3];
    for (int i = 0; i < 3; i++)
        dir2camera[i] = mainCamera.ownCoords.pos[i]-mainCamera.lookAtPoint[i];
    v3norm(dir2camera);
    isDark = (v3dotProduct(dirToMainLight, dir2camera) <= 0);

    int arraysN = pSubjArrays2draw.size();
    for (int nA = 0; nA < arraysN; nA++) {
        std::vector<SceneSubj*>* pSubjs = pSubjArrays2draw.at(nA);
        int subjsN = pSubjs->size();
        //calculate screen coords/gabarites
        int checkRootN = -1;
        //SceneSubj* pRoot=NULL;
        bool rootInView = true;
        for (int subjN = 0; subjN < subjsN; subjN++) {
            SceneSubj* pSS = pSubjs->at(subjN);
            if (pSS == NULL)
                continue;
            if (pSS->hidden() > 0)
                continue;
 
            //if (strcmp(pSS->className, "Smoke") == 0)
            //    int a = 0;

            if (checkRootN != pSS->rootN) {
                checkRootN = pSS->rootN;
                rootInView = true;
                SceneSubj* pRoot = pSubjs->at(pSS->rootN);
                //if assembly gabarites in a view box
                if (Gabarites::boxesIntersect(&pRoot->gabaritesWorldAssembly, &mainCamera.visibleBox))
                    rootInView = true; //in view area
                else
                    rootInView = false;
                if (rootInView) {
                    if (Gabarites::pointInBox(mainCamera.ownCoords.pos, &pRoot->gabaritesWorldAssembly,10))
                        rootInView = false; //camera inside of root
                }
            }
            if (!rootInView) {
                pSS->gabaritesOnScreen.isInViewRange = -1;
                continue;
            }
            
            if (Gabarites::boxesIntersect(&pSS->gabaritesWorld, &mainCamera.visibleBox)==false) {
                pSS->gabaritesOnScreen.isInViewRange = -1;
                continue;
            }
           
            pSS->chordaScreen.chordType = pSS->chordaCollision.chordType;
            
            SceneSubj::fillScreenGabarites(pSS);
            /*
            //debug
            if(pSS->gabaritesOnScreen.isInViewRange>=0){
                if(pSS->chordaScreen.chordaQuality>=0)
                    ScreenLine::addLine2queue(pSS->chordaScreen.chord.p0, pSS->chordaScreen.chord.p1,
                        MyColor::getUint32(255, 0, 0), 2, true);
            }
            */
        }
    }

    Shadows::addToShadowsQueue(pSubjArrays2draw);
    Shadows::renderDepthMap();
			checkGLerrors("after renderDepthMap");
 
    //set render to screen
    glViewport(0, 0, mainCamera.targetDims[0], mainCamera.targetDims[1]);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glDepthMask(GL_TRUE);
    //glClearColor(0.0, 0.0, 0.0, 1.0);
    glClearColor(0.5, 0.5, 0.5, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //sort for rendering
    std::vector<SceneSubj*> renderQueueOpaque;
    std::vector<SceneSubj*> renderQueueTransparent;
    arraysN = pSubjArrays2draw.size();
    for (int nA = 0; nA < arraysN; nA++) {
        std::vector<SceneSubj*>* pSubjs = pSubjArrays2draw.at(nA);
        SceneSubj::addToRenderQueue(&renderQueueOpaque, &renderQueueTransparent, pSubjs);
    }
    //draw opaque subjects
    SceneSubj::sortRenderQueue(&renderQueueOpaque, 0); //0-closer 1st
    int subjsN = renderQueueOpaque.size();
		//mylog("frame%d, %d opaques\n",frameN,subjsN);
    for (int subjN = 0; subjN < subjsN; subjN++) {
        SceneSubj* pSS = renderQueueOpaque.at(subjN);
        pSS->render(&mainCamera, dirToMainLight, dirToTranslucent, 0,false); //0 - render opaque only
 			checkGLerrors("after lrender opaque");
   }
    renderQueueOpaque.clear();

    //Smoke::drawDebug();

    //draw transparent subjects
    //SceneSubj::sortRenderQueue(&renderQueueTransparent, 1); //1-farther 1st
    subjsN = renderQueueTransparent.size();
		//mylog("%d tranparents\n",subjsN);
    for (int subjN = 0; subjN < subjsN; subjN++) {
        SceneSubj* pSS = renderQueueTransparent.at(subjN);
        pSS->render(&mainCamera, dirToMainLight, dirToTranslucent, 1,false); //1 - render transparent only
 			checkGLerrors("after lrender transparent");
    }
    renderQueueTransparent.clear();

    UISubj::renderAll();
    //mylog("after render UI\n");
			

    //synchronization
    uint64_t currentMillis = getSystemMillis();
    uint64_t millisSinceLastFrame = currentMillis - lastFrameMillis;
    cpuLoad = (float)millisSinceLastFrame / (float)millisPerFrame;
    //cpuLoadAverage
    int cpuLoadsSizeLimit = 30;
    if(cpuLoads.size() >= cpuLoadsSizeLimit)
        cpuLoads.erase(cpuLoads.begin());
    cpuLoads.push_back(cpuLoad);
    cpuLoadAverage = 0;
    for (int i = cpuLoads.size() - 1; i >= 0; i--)
        cpuLoadAverage += cpuLoads.at(i);
    cpuLoadAverage /= cpuLoads.size();

    //mylog("%d millis=%d cpuLoad=%f avg=%f\n", frameN, (int)millisSinceLastFrame, cpuLoad, cpuLoadAverage);

    while (1) {
        currentMillis = getSystemMillis();
        millisSinceLastFrame = currentMillis - lastFrameMillis;
        if (millisSinceLastFrame >= millisPerFrame) {
            lastFrameMillis = currentMillis;
            break;
        }
    }
    mySwapBuffers();
    frameN++;
    return 1;
}

int TheApp::cleanUp() {
    //delete all sceneSubjs
    int itemsN = sceneSubjs.size();
    for (int i = 0; i < itemsN; i++) {
        SceneSubj* pSS = sceneSubjs.at(i);
        if (pSS != NULL)
            delete pSS;
    }
    sceneSubjs.clear();
    //delete staticSubjs
    itemsN = staticSubjs.size();
    for (int i = 0; i < itemsN; i++) {
        SceneSubj* pSS = staticSubjs.at(i);
        if (pSS != NULL)
            delete pSS;
    }
    staticSubjs.clear();
    //delete clouds
    itemsN = clouds.size();
    for (int i = 0; i < itemsN; i++) {
        SceneSubj* pSS = clouds.at(i);
        if (pSS != NULL)
            delete pSS;
    }
    clouds.clear();
    //delete models
    itemsN = models.size();
    for (int i = 0; i < itemsN; i++) {
        SceneSubj* pSS = models.at(i);
        if (pSS != NULL)
            delete pSS;
    }
    models.clear();

    //delete UISubjs
    itemsN = UISubjs.size();
    for (int i = 0; i < itemsN; i++) {
        UISubj* pSS = UISubjs.at(i);
        if (pSS != NULL)
            delete pSS;
    }
    UISubjs.clear();
    //delete drawJobs
    itemsN = drawJobs.size();
    for (int i = 0; i < itemsN; i++) {
        DrawJob* pDJ = drawJobs.at(i);
        delete pDJ;
    }
    drawJobs.clear();
    //delete Buffers
    itemsN = buffersIds.size();
    for (int i = 0; i < itemsN; i++) {
        unsigned int id = buffersIds.at(i);
        glDeleteBuffers(1, &id);
    }
    buffersIds.clear();

    std::vector<DrawJob*> drawJobs;
    std::vector<unsigned int> buffersIds;

    //clear all other classes
    MaterialAdjust::cleanUp();
    Texture::cleanUp();
    Shader::cleanUp();
    DrawJob::cleanUp();
    MyColor::cleanUp();
    gameTable.cleanUpLayout();
    Rail::cleanUp();
    TouchScreen::cleanUp();
    MySound::cleanUp();
    Smoke::cleanUp();
    return 1;
}
int TheApp::onScreenResize(int width, int height) {
			//mylog("onScreenResize %d x %d\n",width,height);
    if (mainCamera.targetDims[0] == width && mainCamera.targetDims[1] == height)
        return 0;
    if (gameTable.worldBox.boxRad != 0)
        Camera::onTargetResize(&mainCamera, width, height, &gameTable.worldBox);
    UISubj::onScreenResize(width, height);
    mylog(" screen size %d x %d\n", width, height);

    return 1;
}
void TheApp::mylogObjSize(std::string objName, int objSize) {
    mylog("%s %d", objName.c_str(), objSize);
    int divider = 2;
    while (1) {
        int rest = objSize % divider;
        if (rest != 0) {
            mylog(" dividable by %d\n", divider / 2);
            break;
        }
        divider *= 2;
    }
}
void TheApp::showHierarchy(SceneSubj* pSS) {
    //std::vector<SceneSubj*>* pSubjsSet = pSS->pSubjsSet;
    int subjN = pSS->nInSubjsSet;
    int acanTo = subjN+pSS->totalElements;
    mylog("------------%s-----\n", pSS->source256);
    for (int elN = subjN; elN < acanTo; elN++) {
        int eN = elN;
        SceneSubj* pSS0 = pSS->pSubjsSet->at(eN);
        mylog("<%d:%s:%s:%s>", elN, pSS0->className, pSS0->name64, pSS0->source256);
        if (pSS0->d2parent != 0) {
            int pN = pSS0->nInSubjsSet - pSS0->d2parent;
            SceneSubj* pParent = pSS->pSubjsSet->at(pN);
            mylog(" parent <%d:%s:%s>",pN, pParent->className, pParent->name64);

            int a = 0;
        }
        else
            mylog(" root");
        mylog("\n");
    }
    mylog("/////////////////////////////////////\n");
}

SceneSubj* TheApp::newSceneSubj(std::string subjClass, std::string sourceFile,
    std::vector<SceneSubj*>* pSubjsSet0, std::vector<DrawJob*>* pDrawJobs0) {
    SceneSubj* pSS = NULL;
    if (subjClass.compare("") == 0)
        pSS = (new SceneSubj());
    else if (subjClass.compare("Smoke") == 0)
        pSS = (new Smoke());
    else if (subjClass.compare("Exhaust") == 0)
        pSS = (new Exhaust());
    else if (subjClass.find("Car") == 0) {
        if (subjClass.compare("CarWheel") == 0)
            pSS = (new CarWheel());
    }
    else if (subjClass.compare("Coupler") == 0)
        pSS = (new Coupler());
    else if (subjClass.compare("whistle") == 0)
        pSS = (new Whistle());
    else if (subjClass.compare("bell") == 0)
        pSS = (new Bell());
    else if (subjClass.compare("DirLight") == 0)
        pSS = (new DirLight());
    else if (subjClass.compare("Gear") == 0)
        pSS = (new Gear());
    else if (subjClass.compare("OnRails") == 0)
        pSS = (new OnRails());
    else if (subjClass.compare("WheelPair") == 0)
        pSS = (new WheelPair());
    else if (subjClass.find("RollingStock") == 0) {
        if (subjClass.compare("RollingStock") == 0)
            pSS = (new RollingStock());
    }
    else if (subjClass.compare("Interior") == 0)
        pSS = (new Interior());
    else if (subjClass.compare("Fan") == 0)
        pSS = (new Fan());
    else if (subjClass.compare("Gangway") == 0)
        pSS = (new Gangway());
    else if (subjClass.compare("ExhaustLoco") == 0)
        pSS = (new ExhaustLoco());

    if (pSS == NULL) {
        mylog("ERROR in TheProject::newSceneSubj. %s class not found\n", subjClass.c_str());
        return NULL;
    }
    strcpy_s(pSS->className, 32, subjClass.c_str());
    strcpy_s(pSS->source256, 256, sourceFile.c_str());
    if (pDrawJobs0 != NULL)
        pSS->pDrawJobs = pDrawJobs0;
    if (pSubjsSet0 != NULL) {
        pSS->nInSubjsSet = pSubjsSet0->size();
        pSS->pSubjsSet = pSubjsSet0;
        pSubjsSet0->push_back(pSS);
    }
    return pSS;
}

int TheApp::checkGameStatus() {
    if (frameN == 0)
        return 0;
    if (trainIsReady)
        return 0;
    std::vector<SceneSubj*>* pSubjs = &sceneSubjs;
    int trainRootN = gameTable.primeLocoN;
    if (trainRootN < 0)
        return 0;
    RollingStock* pTrainRoot = (RollingStock*)pSubjs->at(0);
    if (pTrainRoot->tr.divorcing)
        return 0;
    Coupler* pCp = pTrainRoot->pCouplerFront;
    if (pCp->connected > 0) {
        RollingStock::decouple(pCp);
        pTrainRoot->desirableZdir = -1;
        return 1;
    }
    for (int sN = pSubjs->size() - 1; sN >= 0; sN--) {
        SceneSubj* pSS = pSubjs->at(sN);
        if (pSS == NULL)
            continue;
        if (pSS->d2parent != 0)
            continue;
        if (strstr(pSS->className, "RollingStock") == NULL)
            continue;
        RollingStock* pWagon = (RollingStock*)pSS;
        if (pWagon->trainRootN != trainRootN) {
            pTrainRoot->desirableZdir = -1;
            return 1;
        }
    }
    //here - trainIsReady
    if (!trainIsReady) {
        trainIsReady = true;
        pTrainRoot->desirableZdir = 1;
        return 1;
    }
    //ok
    return 0;
}

int TheApp::run() {
    /*
    {//test
        float p0[4] = { 498,232,0,0 };
        float p1[4] = { 197,238,0,0 };
        float pc[3] = { 295,241,0 };
        Chorda chorda;
        LineXY* pLn = &chorda.chord;
        LineXY::initLineXY(pLn, p0, p1);
        chorda.chordR = 50;
        float dist = LineXY::dist_l2p(pLn, pc);

        mylog("dist=%d  length=%d a_slope=%f b_intercept=%d isVertical=%d x_vertical=%d isHorizontal=%d isDot=%d\n",(int)dist, 
            (int)pLn->length, pLn->a_slope, (int)pLn->b_intercept, pLn->isVertical, (int)pLn->x_vertical, pLn->isHorizontal, pLn->isDot);
        //return 1;
    }
    */
    {//random pick
        int choice = getRandom(0, 6);
        if (choice == 0) { //ebay
            printf("ebay.");
            choice = getRandom(0, 4);
            if (choice == 0) printf("uk ");
            else if (choice == 1) printf("de ");
            else printf("com ");
            printf("page %d item %d", getRandom(1, 10), getRandom(1, 10));
        }
        else if (choice == 1) { //pict
            printf("picts ");
            for (int lvN = 0; lvN < 6; lvN++) {
                printf("%d ", getRandom(1, 10));
            }
        }
        else { //real
            std::vector<std::string> terms;
            terms.push_back("top box,mid box,lower box,table");
            terms.push_back("South,SE,East,NE,North,NW,West,SW,center");
            terms.push_back("South,SE,East,NE,North,NW,West,SW,center");
            terms.push_back("layer 1,layer 1,layer 2,layer 3,junk,bulk");
            for (int lvN = 0; lvN < terms.size(); lvN++) {
                std::vector<std::string>* pOpts = splitString(terms.at(lvN), ",");
                int optN = getRandom(0, pOpts->size() - 1);
                printf("%s -> ", pOpts->at(optN).c_str());
            }
            printf("#%d in box", getRandom(1, 6));
        }
        printf("\n");
    }

    getReady();
    while (!bExitApp) {
        drawFrame();
    }
    //gameTable.map2log();
    cleanUp();
    return 1;
}

