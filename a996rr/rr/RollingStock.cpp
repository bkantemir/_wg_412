#include "RollingStock.h"
#include "TheApp.h"
#include "TouchScreen.h"
#include "ScreenLine.h"
#include "WheelPair.h"
#include "MeltingSign.h"
#include "MySound.h"


extern TheApp theApp;
extern float displayDPI;

float RollingStock::accelerationLinPassive = 0.005;
float RollingStock::accelerationLinActive = 0.01;
float RollingStock::accelerationOnDrag = 0.2;
float RollingStock::divorceSpeed = 0.8;
float RollingStock::maxTrainSpeed = 3;
float RollingStock::maxDragSpeed = 3;

int RollingStock::onDeployRS(RollingStock* pRS, std::string tagStr) {
    SceneSubj::onDeployStandatd(pRS, tagStr);
    if (pRS->d2parent != 0)
        return 0;
    //find couplers
    for (int elN = 1; elN < pRS->totalElements; elN++) {
        SceneSubj* pSS = pRS->pSubjsSet->at(pRS->nInSubjsSet + elN);
        if (pSS == NULL)
            continue;
        if (strcmp(pSS->className, "Coupler") == 0) {
            Coupler* pCp = (Coupler*)pSS;
            pCp->connected = -1;
            pCp->pCounterCoupler = NULL;
            if (pSS->alignedWithRoot > 0)
                pRS->pCouplerFront = (Coupler*)pSS;
            else
                pRS->pCouplerBack = (Coupler*)pSS;
        }
        else if (strcmp(pSS->className, "Gangway") == 0) {
            if (pSS->alignedWithRoot > 0)
                pRS->pGangwayFront = (Gangway*)pSS;
            else
                pRS->pGangwayBack = (Gangway*)pSS;
        }
    }
    if (XMLparser::varExists("coupleTo", tagStr)) {
        std::string coupleTo = XMLparser::getStringValue("coupleTo", tagStr);
        Coupler* pCp = pRS->pCouplerFront;
        //take 1st train
        RollingStock* pRS0 = (RollingStock*)pRS->pSubjsSet->at(0);
        RollingStock* pTR0 = (RollingStock*)pRS0->pSubjsSet->at(pRS0->trainRootN);
        Train* pTr = &pTR0->tr;
        Coupler* pCp0 = pTr->pCouplerBack;
        if (pCp0->connected > 0)
            pCp0 = pTr->pCouplerFront;
        if (pCp0->connected <= 0) {
            pCp->couple(pCp,pCp0,false);
            return 1;
        }
    }
    //train info
    RollingStock::reinspectTrain(pRS->pCouplerFront);
    //place to nearest rail
    RailCoord* pRC=&pRS->railCoord;
                                pRC->railN = -1;
    if (pRC->railN < 0) {
        int rN = theApp.gameTable.findClosestRail(pRC, &pRS->ownCoords);
        if (rN < 0) //no rail found
            return -1;
        //populate to ownCoords
        v3copy(pRS->ownCoords.pos, pRC->xyz);
        pRS->ownCoords.setEulerDg(0, pRC->currentYaw, 0);
     }
    return 1;
}

int RollingStock::onLoadRS(RollingStock* pSS00, std::string tagStr) {
    if (pSS00->d2parent != 0)
        return 0;
    //inspect RS
    pSS00->wheelBase = 0;
    pSS00->wheelBaseZ = 0;
    pSS00->zOffsetFromRoot = 0;
    pSS00->alignedWithRoot = 1;
    pSS00->maxWheelRad = 0;
    for (int elN = 1; elN < pSS00->totalElements; elN++) {
        SceneSubj* pSS = pSS00->pSubjsSet->at(pSS00->nInSubjsSet + elN);
        if (pSS == NULL)
            continue;
        SceneSubj::buildModelMatrixStandard(pSS);
        pSS->zOffsetFromRoot = pSS->absCoords.pos[2];

        float vIn[4] = { 0,0,1,0 };
        float vOut[4];
        mat4x4_mul_vec4plus(vOut, pSS->absModelMatrixUnscaled, vIn, 0);
        if(vOut[2]<0)
            pSS->alignedWithRoot = -1;
        else
            pSS->alignedWithRoot = 1;

        if (strcmp(pSS->className, "Coupler") == 0) {
            if (pSS->absCoords.pos[1] != Rail::couplerY) {
                float dy = Rail::couplerY - pSS->absCoords.pos[1];
                pSS->ownCoords.pos[1] += dy;
            }
            if (pSS->alignedWithRoot > 0)
                pSS00->pCouplerFront = (Coupler*)pSS;
            else
                pSS00->pCouplerBack = (Coupler*)pSS;
        }
        else if (strcmp(pSS->className, "WheelPair") == 0) {
            WheelPair* pWP = (WheelPair*)pSS;
            if(pSS00->maxWheelRad < pWP->wheelRadius)
                pSS00->maxWheelRad = pWP->wheelRadius;
        }
    }
    pSS00->unitLength = pSS00->pCouplerFront->zOffsetFromRoot - pSS00->pCouplerBack->zOffsetFromRoot +
        pSS00->pCouplerFront->lever + pSS00->pCouplerBack->lever;
    pSS00->maxSectionLength = pSS00->unitLength;
    return 1;
}

bool RollingStock::ignoreCollisionRollingStock(RollingStock* pRS1, SceneSubj* pS2) {
    if (strstr(pS2->className, "RollingStock") != pS2->className)
        return false;
    //dealing with RollingStock, check if coupled with it
    Coupler* pCp = pRS1->pCouplerFront;
    if (pCp != NULL)
        if (pCp->pCounterCoupler != NULL) {
            Coupler* pCp2 = pCp->pCounterCoupler;
            if (pCp2->rootN == pS2->rootN)
                return true;
        }
    pCp = pRS1->pCouplerBack;
    if (pCp != NULL)
        if (pCp->pCounterCoupler != NULL) {
            Coupler* pCp2 = pCp->pCounterCoupler;
            if (pCp2->rootN == pS2->rootN)
                return true;
        }
    return false;
}
int RollingStock::processCollisionRollingStock(float penetrationDepth, RollingStock* pS1, SceneSubj* pS2, float* collisionPointWorld, float* hitPointNormal, float* hitSpeed) {
    RollingStock* pTrainRoot = (RollingStock*)pS1->pSubjsSet->at(pS1->trainRootN);
    if (strstr(pS2->className, "RollingStock") == pS2->className) {
        RollingStock* pTrainRoot = (RollingStock*)pS1->pSubjsSet->at(pS1->trainRootN);
        if (penetrationDepth > 3 || pTrainRoot->tr.divorcing || !theApp.trainIsReady) {
            if (pS1->nInSubjsSet > pS2->nInSubjsSet)
                return 0;
            divorceTrains(pS1, (RollingStock*)pS2);
            return 1;
        }
    }
    if (pTrainRoot->ownZspeed != 0) {
        float dir2hitpoint[4] = { 0,0,0,0 };
        for (int i = 0; i < 3; i += 2)
            dir2hitpoint[i] = collisionPointWorld[i] - pS1->absCoords.pos[i];
        //calculate wagon's z-direction
        float dir2head[4] = { 0,0,0,0 };
        for (int i = 0; i < 3; i += 2)
            dir2head[i] = pS1->pCouplerFront->absCoords.pos[i] - pS1->absCoords.pos[i];
        float setZspeed = fmax(abs(pTrainRoot->ownZspeed), 1);
        float dotP = v3dotProduct(dir2hitpoint, dir2head);
        if (dotP >= 0)
            setZspeed=-setZspeed;
        pTrainRoot->ownZspeed = setZspeed * pTrainRoot->alignedWithTrainHead * pS1->alignedWithTrainHead * 0.6;
        /*
        //debug
        if (pTrainRoot->nInSubjsSet == 0)
            mylog("%d alignedWithZ=%d pTrainRoot->ownZspeed=%.2f\n", theApp.frameN, alignedWithZ, pTrainRoot->ownZspeed);
            */
    }
        
    pS1->tremble = 1;
    pTrainRoot->desirableZdir = 0;
    pTrainRoot->desirableZspeed = 0;
    if (pS1->nInSubjsSet > pS2->nInSubjsSet)
        MySound::playSound(MySound::soundNhit01, pS1, false);
    
    TouchScreen::cancelDrag();
    return 1;
}

int RollingStock::checkTrains(std::vector<SceneSubj*>* pSceneSubjs) {
    int subjsN = pSceneSubjs->size();
    for (int sN = 0; sN < subjsN; sN++) {
        SceneSubj* pS = pSceneSubjs->at(sN);
        if (pS == NULL)
            continue;
        if (pS->d2parent != 0)
            continue;
        if (strstr(pS->className, "RollingStock") != pS->className)
            continue;
        RollingStock* pRS = (RollingStock*)pS;
        pRS->powerOn = false;
        if (pRS->trainRootN != pRS->nInSubjsSet)
            continue;
        Train* pTr = &pRS->tr;
        couplerClearance(pTr->pCouplerFront);
        couplerClearance(pTr->pCouplerBack);
    }
    //turn lights on?
    int primeLocoN = theApp.gameTable.primeLocoN;
    if (primeLocoN >= 0) {
        RollingStock* pRS = (RollingStock*)pSceneSubjs->at(primeLocoN);
        if (pRS->powered) {
            pRS->powerOn = true;
            Coupler* pCp = pRS->pCouplerFront;
            while (pCp->connected > 0 && pCp->couplersInvolved > 1) {
                pCp = pCp->pCounterCoupler;
                pRS = (RollingStock*)pSceneSubjs->at(pCp->rootN);
                pRS->powerOn = true;
                //next coupler
                if (pCp == pRS->pCouplerFront)
                    pCp = pRS->pCouplerBack;
                else
                    pCp = pRS->pCouplerFront;
            }
            pRS = (RollingStock*)pSceneSubjs->at(primeLocoN);
            pCp = pRS->pCouplerBack;
            while (pCp->connected > 0 && pCp->couplersInvolved > 1) {
                pCp = pCp->pCounterCoupler;
                pRS = (RollingStock*)pSceneSubjs->at(pCp->rootN);
                pRS->powerOn = true;
                //next coupler
                if (pCp == pRS->pCouplerFront)
                    pCp = pRS->pCouplerBack;
                else
                    pCp = pRS->pCouplerFront;
            }
        }
    }
    return 1;
}
float RollingStock::couplerClearance(Coupler* pCoupler) {
    return 100;
}
int RollingStock::readClassPropsRS(RollingStock* pRS, std::string tagStr) {
    readClassPropsSceneSubj(pRS, tagStr);

    if (strcmp(pRS->type16, "loco") == 0) {
        pRS->powered = 1;
    }
    return 1;
}

int RollingStock::moveRS(RollingStock* pRS) {
    //if (theApp.frameN == 0)
    //    return 1;
    RollingStock* pRSroot = (RollingStock*)pRS->pSubjsSet->at(pRS->trainRootN);
    Train* pTr = &pRSroot->tr;
    if (pRS->trainRootN == pRS->nInSubjsSet) {
        //is train root
        //check if dragged
        if(TouchScreen::cursorStatus==2) {
            if (pSelectedSceneSubj != NULL)
                if (strcmp(pSelectedSceneSubj->className, "RollingStock") == 0) {
                    RollingStock* pDraged = (RollingStock*)pSelectedSceneSubj;
                    if (pDraged->trainRootN == pRS->trainRootN)
                        return 1;
                }
        }
        if (pTr->poweredLength == 0 || theApp.bEditMode) {
            pRSroot->desirableZdir = 0;
            pRSroot->desirableZspeed = 0;
            pRSroot->accelerationLin = RollingStock::accelerationLinPassive;
            if(theApp.bEditMode)
                pRSroot->accelerationLin = RollingStock::accelerationLinActive;
        }
        else { // powered root
            pRSroot->accelerationLin = RollingStock::accelerationLinActive;
            /*
            //pick desirable speed from prime loco
            RollingStock* pLoco = (RollingStock*)pRS->pSubjsSet->at(pTr->primeLocoN);
            pRS->desirableZdir = pLoco->desirableZdir * pRS->alignedWithTrainHead * pLoco->alignedWithTrainHead;
            */
            pRS->desirableZspeed  = pRS->desirableZdir * RollingStock::maxTrainSpeed;

            //mylog("pRS->desirableZspeed=%f pRS->ownZspeed=%f\n", pRS->desirableZspeed, pRS->ownZspeed);
        }
        approveMoveIntent(pRS, &theApp.gameTable);
        if (pRS->ownZspeed != 0 || pRS->desirableZspeed != 0) {
            if (applySpeedOnRails(pRS) < 0) {
                pRS->deleteMe(true);
                return -1;//out of rail
            }
            pTr->divorcing = false;
        }
    }
    else {//follow root
        float shiftFromRoot = (pRSroot->zOffsetFromHead- pRS->zOffsetFromHead)* pRSroot->alignedWithTrainHead;
        theApp.gameTable.shiftRailCoord(&pRS->railCoord, &pRSroot->railCoord, &theApp.gameTable, shiftFromRoot,true);
        if (pRS->alignedWithTrainHead != pRSroot->alignedWithTrainHead)
            pRS->railCoord.alignedWithRail *= -1;
    }
    //calc ownSpeed
    for (int i = 0; i < 3; i++)
        pRS->ownSpeed.pos[i] = pRS->railCoord.xyz[i] - pRS->railCoordOld.xyz[i];

    if (pRS->powered)
    if(pRS->powerOn) {
        float heatSpeed = 1.0 / 33 / 10;
        float chillSpeed = 1.0 / 33 / 20;
        if (pRS->desirableZspeed == 0) {
            pRS->engineHeat -= chillSpeed;
            if (pRS->engineHeat < 0)
                pRS->engineHeat = 0;
        }
        else {
            pRS->engineHeat += heatSpeed;
            if (pRS->engineHeat >1)
                pRS->engineHeat = 1;
        }
        /*
        if (abs(pRS->desirableZspeed) > abs(pRS->ownZspeed)) {
            pRS->tremble = 0.5;
        }
        */
        //mylog("%d desirableZspeed=%f ownZspeed=%f tremble=%f\n", theApp.frameN, pRS->desirableZspeed, pRS->ownZspeed, pRS->tremble);
    }
    return 1;
}

int RollingStock::emergencyTrainShift(RollingStock* pW1, float divorceDir, RollingStock* pW2) {
    RollingStock* pRoot1 = (RollingStock*)pW1->pSubjsSet->at(pW1->trainRootN);
    RollingStock* pRoot2 = (RollingStock*)pW2->pSubjsSet->at(pW2->trainRootN);
    Train* pTr1 = &pRoot1->tr;
    Train* pTr2 = &pRoot2->tr;
    if (pTr1->divorcing) { //already divorcing
        return 0;
    }
    //drop tail
    Coupler* pCp2free = pW1->pCouplerBack;
    if(divorceDir<0)
        pCp2free = pW1->pCouplerFront;
    if (pCp2free->connected > 0) {
        Coupler* pCp3 = pCp2free->pCounterCoupler;
        pCp3->connected = 0;
        reinspectTrain(pCp3);
        RollingStock* pW3 = (RollingStock*)pCp3->pSubjsSet->at(pCp3->rootN);
        RollingStock* pRoot3 = (RollingStock*)pW2->pSubjsSet->at(pW3->trainRootN);
        pRoot3->ownZspeed = 0;

        pCp2free->connected = 0;
        reinspectTrain(pCp2free);
        pRoot1 = (RollingStock*)pW1->pSubjsSet->at(pW1->trainRootN);
        pTr1 = &pRoot1->tr;
    }
    if (pW1->alignedWithTrainHead != pRoot1->alignedWithTrainHead)
        divorceDir *= -1;

     pRoot1->tr.divorcing = true;
     pRoot1->ownZspeed = divorceDir*divorceSpeed;
     /*
     {
         mylog("emergencyTrainShift car%d:%d->root %d:%d->train %d dir %d\n",pW1->nInSubjsSet,pW1->alignedWithTrainHead, 
             pRoot1->nInSubjsSet, pRoot1->alignedWithTrainHead,pTr1->trainId, (int)divorceDir);
     }
     */
     return 1;
}
int RollingStock::trainToLog(Coupler* pCp) {
    RollingStock* pW = (RollingStock*)pCp->pSubjsSet->at(pCp->rootN);
    return trainToLog(pW);
}
int RollingStock::trainToLog(RollingStock* pW0) {
    RollingStock* pRoot = (RollingStock*)pW0->pSubjsSet->at(pW0->trainRootN);
    mylog("%d.%s:rootN %d, units %d: ", pW0->nInSubjsSet, path2title(pW0->source256).c_str(), pW0->trainRootN, pRoot->tr.carsTotal);
    Coupler* pCp = pRoot->tr.pCouplerFront;
    int carsN = 0;
    while (1) {
        RollingStock* pW = (RollingStock*)pRoot->pSubjsSet->at(pCp->rootN);
        if (pW->alignedWithTrainHead > 0)
            mylog("+");
        else
            mylog("-");
        mylog("%d.%s:%d", pW->nInSubjsSet,path2title(pW->source256).c_str(),(int)pW->zOffsetFromHead);
        if (pW == pW0)
            mylog("!!");
        carsN++;
        //to next coupler
        if (pCp->alignedWithRoot > 0)
            pCp = pW->pCouplerBack;
        else
            pCp = pW->pCouplerFront;
        if (pCp->connected < 1)
            break;//tail reached
        //go to next car
        pCp = pCp->pCounterCoupler;
        mylog(" -> ");
    }
    mylog("\n");
    if (carsN != pRoot->tr.carsTotal) {
        mylog("ERROR: not declared cars N. Slow-mo:\n");
        Coupler* pCp = pRoot->tr.pCouplerFront;
        int carsN = 0;
        while (1) {
            mylog("checking coupler %d:%d\ncar:", pCp->rootN, pCp->alignedWithRoot);
            RollingStock* pW = (RollingStock*)pRoot->pSubjsSet->at(pCp->rootN);
            if (pW->alignedWithTrainHead > 0)
                mylog("+");
            else
                mylog("-");
            mylog("%d.%s:%d", pW->nInSubjsSet, path2title(pW->source256).c_str(), (int)pW->zOffsetFromHead);
            if (pW == pW0)
                mylog("!!");
            mylog("\n");
            carsN++;
            //to next coupler
            if (pCp->alignedWithRoot > 0)
                pCp = pW->pCouplerBack;
            else
                pCp = pW->pCouplerFront;
            mylog("opposite coupler %d:%d\n", pCp->rootN, pCp->alignedWithRoot);
            if (pCp->connected < 1) {
                mylog("The last one\n");
                break;//tail reached
            }
            //go to next car
            pCp = pCp->pCounterCoupler;
            mylog("connected to %d:%d\n", pCp->rootN, pCp->alignedWithRoot);
        }
        int a = 0;
    }
    return 1;
}
std::string RollingStock::path2title(std::string path) {
    int lastSlashPos = path.find_last_of("/");
    std::string path2 = path.substr(0, lastSlashPos);
    lastSlashPos = path2.find_last_of("/");
    std::string name = path2.substr(lastSlashPos+1);
    return name;
}
bool RollingStock::trainRootIsWrong(RollingStock* pW0) {
    RollingStock* pRoot = (RollingStock*)pW0->pSubjsSet->at(pW0->trainRootN);
    Coupler* pCp = pRoot->tr.pCouplerFront;
    while (1) {
        RollingStock* pW = (RollingStock*)pCp->pSubjsSet->at(pCp->rootN);
        if (pW->nInSubjsSet == pW0->nInSubjsSet)
            return false;
        //switch to opposite coupler
        if (pCp->alignedWithRoot > 0)
            pCp = pW->pCouplerBack;
        else
            pCp = pW->pCouplerFront;
        //to next car
        if (pCp->connected < 1)
            return true;//tail reached
        pCp = pCp->pCounterCoupler;
    }
}
int RollingStock::reinspectTrain(Coupler* pCoupler) {
    SceneSubj* pSS = pCoupler->pSubjsSet->at(pCoupler->rootN);
    if (strstr(pSS->className, "RollingStock") != pSS->className)
        return 0;//not a wagon
    pCoupler = findHeadCoupler(pCoupler);
    return assignNewTrainHead(pCoupler);
}
Coupler* RollingStock::findHeadCoupler(Coupler* pCp) {
    while (pCp->connected > 0) {
 
        //mylog("findHeadCoupler checking Coupler %d:%d\n", pCp->rootN,pCp->alignedWithRoot);

        //coupler connected
        Coupler* pCpConnected = pCp->pCounterCoupler;
 
        //mylog("findHeadCoupler connected to %d:%d\n", pCpConnected->rootN, pCpConnected->alignedWithRoot);
        
        RollingStock* pWagonNext = (RollingStock*)pCpConnected->pSubjsSet->at(pCpConnected->rootN);

        //proceed to opposite coupler
        if (pCpConnected->alignedWithRoot > 0)
            pCp = pWagonNext->pCouplerBack;
        else
            pCp = pWagonNext->pCouplerFront;

        //mylog("findHeadCoupler proceed to nextCoupler %d:%d\n", pCp->rootN, pCp->alignedWithRoot);
    }
    //mylog("findHeadCoupler . Head coupler %d:%d\n", pCp->rootN, pCp->alignedWithRoot);

    return pCp;
}
int RollingStock::assignNewTrainHead(Coupler* pCp) {
    //pCp-train front coupler
    if(pCp->connected > 0) {
        mylog("ERROR in RollingStock::assignNewTrainHead: not a head coupler\n");
        return -1;
    }
    Train tr;
    int trainRootN = pCp->rootN;
    tr.trainId = newId();
    tr.primeLocoN = -1;
    tr.carsTotal = 0;
    tr.pCouplerFront = pCp;
    float trainLength = 0;
    float poweredLength = 0;
    //just in case
    tr.divorcing = false;

    //mylog("assignNewTrainHead set HEAD coupler %d.%d\n", pCp->rootN, pCp->alignedWithRoot);

    std::vector<int> trainWagonNs;
    float zOffsetFromHead = 0;
    int alignedWithTrainHead = pCp->alignedWithRoot;
    RollingStock* pWagon = NULL;
    while(1){

        //mylog("assignNewTrainHead coupler %d.%d\n", pCp->rootN, pCp->alignedWithRoot);

        pWagon = (RollingStock*)pCp->pSubjsSet->at(pCp->rootN);
        //count current wagon
        pWagon->nInTrain = trainWagonNs.size();
        trainWagonNs.push_back(pWagon->nInSubjsSet);
        pWagon->alignedWithTrainHead = alignedWithTrainHead;
        pWagon->zOffsetFromHead = zOffsetFromHead;
        if (trainRootN > pCp->rootN)
            trainRootN = pCp->rootN;
        if (pWagon->activeLoco) {
            tr.primeLocoN = pWagon->nInSubjsSet;
            tr.poweredLength = pWagon->unitLength;
        }
        //move to opposite coupler
        if (pCp->alignedWithRoot > 0)
            pCp = pWagon->pCouplerBack;
        else
            pCp = pWagon->pCouplerFront;

        //mylog("assignNewTrainHead opposite coupler %d.%d\n", pWagon->nInSubjsSet, pCp->alignedWithRoot);

        zOffsetFromHead += abs(pCp->zOffsetFromRoot);
        if (pCp->connected < 1) {
            //mylog("That's the last coupler\n");
            //it's a last coupler
            break;
        }
        //if here - coupler connected
        //add dist between wagons
        zOffsetFromHead += Coupler::couplingDistances[pCp->couplersInvolved];
        zOffsetFromHead += pCp->lever;
        Coupler* pCpConnected = pCp->pCounterCoupler;
        if (pCp->alignedWithRoot == pCpConnected->alignedWithRoot)
            alignedWithTrainHead *= -1;
        //proceed to connected coupler
        if (pCpConnected->pCounterCoupler != pCp) {
            Coupler* pCp2 = pCpConnected->pCounterCoupler;
            mylog("Wrong cross-connection: %d:%d to %d:%d when %d:%d to %d:%d\n", 
                pCp->rootN, pCp->alignedWithRoot, pCpConnected->rootN, pCpConnected->alignedWithRoot,
                pCpConnected->rootN, pCpConnected->alignedWithRoot, pCp2->rootN, pCp2->alignedWithRoot);
            return -1;
        }

        pCp = pCpConnected;
        zOffsetFromHead += abs(pCp->zOffsetFromRoot);
        zOffsetFromHead += pCp->lever;
        /*
        //mylog("assignNewTrainHead proceed to connected coupler %d.%d\n", pCp->rootN, pCp->alignedWithRoot);

        if (trainWagonNs.size() > 10) {
            mylog("Short cirquit\n");
            trainToLog(pWagon);
            int a = 0;
        }
        */
    }
    tr.pCouplerBack = pCp;
    tr.carsTotal = trainWagonNs.size();
    tr.trainLength = zOffsetFromHead;
    //reassign trainRoot
    //mylog("assignNewTrainHead assigning new root %d to %d/%d wagons:", trainRootN, trainWagonNs.size(), tr.carsTotal);
    for (int wN = 0; wN < tr.carsTotal; wN++) {
        pWagon = (RollingStock*)pWagon->pSubjsSet->at(trainWagonNs.at(wN));
        //mylog(" %d,", pWagon->nInSubjsSet);
        pWagon->trainRootN = trainRootN;
    }
    //mylog("\n");

    RollingStock* pRoot = (RollingStock*)pWagon->pSubjsSet->at(trainRootN);
    memcpy(&pRoot->tr, &tr, sizeof(Train));
    if (tr.primeLocoN >= 0 && tr.primeLocoN != trainRootN){
        RollingStock* pLoco = (RollingStock*)pWagon->pSubjsSet->at(tr.primeLocoN);
        pRoot->desirableZdir = pLoco->desirableZdir;
        if (pRoot->alignedWithTrainHead != pLoco->alignedWithTrainHead)
            pRoot->desirableZdir *= -1;
    }
    //mylog("%d wagons\n", tr.carsTotal);
    return tr.carsTotal;
}
int RollingStock::divorceTrains(RollingStock* pW1, RollingStock* pW2) {
    /*
    {
        mylog("---%d divorceTrains:\n", (int)theApp.frameN);
        trainToLog(pW1);
        trainToLog(pW2);
        mylog("Solution:\n");
    }
    */
    if (pW1->trainRootN == pW2->trainRootN) {
        //same train
        //mylog("Same train, skip:\n");
        return 1;
    }
    float dir2to1[4];
    v3dirFromTo(dir2to1, pW2->absCoords.pos, pW1->absCoords.pos);
    float vIn[4] = { 0,0,1,0 };
    float r1dir[4];
    mat4x4_mul_vec4plus(r1dir, pW1->ownCoords.rotationMatrix, vIn, 0);
    if (v3length(dir2to1)<0.5)
        v3copy(dir2to1, r1dir);
    float dot1 = v3dotProductNorm(dir2to1, r1dir);
    float w1shift = 1;
    if (dot1 < 0)
        w1shift *= -1;
    emergencyTrainShift(pW1, w1shift, pW2);

    float dir1to2[4];
    for (int i = 0; i < 3; i++)
        dir1to2[i] = -dir2to1[i];
    float r2dir[4];
    mat4x4_mul_vec4plus(r2dir, pW2->ownCoords.rotationMatrix, vIn, 0);
    float dot2 = v3dotProductNorm(dir1to2, r2dir);
    if (abs(dot2) > 0.3) {
        float w2shift = 1;
        if (dot2 < 0)
            w2shift *= -1;
        emergencyTrainShift(pW2, w2shift, pW1);
    }
    return 1;
}
int RollingStock::decouple(Coupler* pCp2free) {
    if (pCp2free->connected < 1)
        return 0;
    std::vector<SceneSubj*>* pSubjs = pCp2free->pSubjsSet;
    RollingStock* pWg00 = (RollingStock*)pSubjs->at(pCp2free->rootN);
    RollingStock* pRoot00 = (RollingStock*)pSubjs->at(pWg00->trainRootN);
    //involved couplers
    Coupler* pCps[2];
    pCps[0] = pCp2free;
    pCps[1] = pCp2free->pCounterCoupler;
    for (int wN = 0; wN < 2; wN++) {
        Coupler* pCp=pCps[wN];
        RollingStock* pWg = (RollingStock*)pSubjs->at(pCp->rootN);
        pCp->connected = 0;
        pCp->tremble = 0.5;
        reinspectTrain(pCp);
        if (pWg->trainRootN == pRoot00->nInSubjsSet)
            continue;
        //new root
        int alignedWithRoot00 = pWg->alignedWithTrainHead * pRoot00->alignedWithTrainHead;
        RollingStock* pRoot = (RollingStock*)pSubjs->at(pWg->trainRootN);
        int alignedRoots = pWg->alignedWithTrainHead * pRoot->alignedWithTrainHead * alignedWithRoot00;
        pRoot->desirableZdir = pRoot00->desirableZdir * alignedRoots;
        pRoot->ownZspeed = pRoot00->ownZspeed * alignedRoots;
    }
    MySound::playSound(MySound::soundNuncoupling, pCp2free, false);
    return 1;
}
/*
bool RollingStock::isDraggable() {
    if(theApp.bEditMode)
        return true;
    //only powered trains
    RollingStock* pRS = this;
    RollingStock* pRoot = (RollingStock*)pRS->pSubjsSet->at(pRS->trainRootN);
    if (pRoot->tr.poweredLength > 0)
        return true;
    return false;
}
*/
int RollingStock::onDragRS(RollingStock* pRS) {
    pRS->dragging = true;
    RollingStock* pRoot = (RollingStock*)pRS->pSubjsSet->at(pRS->trainRootN);

    if (pRoot->tr.poweredLength == 0 && theApp.bEditMode == false) {
        pRoot->desirableZdir = 0;
        pRoot->desirableZspeed = 0;
        pRS->dragging = false;
        TouchScreen::cancelDrag();
        MySound::playSound(MySound::soundNfart01, pRS, false);
        MeltingSign::addMeltingSign("/dt/ui/signs/noway.png", TouchScreen::cursorPos[0], TouchScreen::cursorPos[1]);
        return 0;
    }

    // highlight
    int nOn = 4;
    int nTotal = nOn * 2;
    if(theApp.frameN%nTotal<nOn)
        pRS->setHighLight(0.4, MyColor::getUint32(1.0f, 1.0f, 1.0f));
    else
        pRS->setHighLight(0);

    //drag direction
    mat4x4 mMVP;
    mat4x4_mul(mMVP, theApp.mainCamera.mViewProjection, pSelectedSceneSubj00->absModelMatrix);
    Camera* pCam = &theApp.mainCamera;
    if (mat4x4_mul_vec4screen(TouchScreen::ancorPointOnScreen, mMVP, TouchScreen::ancorPoint, pCam->targetRads, pCam->nearClip, pCam->farClip) < 1) {
        //point out of reach
        TouchScreen::cancelDrag();
        return 0;        
    }

    float dragDir[2];
    for (int i = 0; i < 2; i++)
        dragDir[i] = TouchScreen::cursorPos[i] - TouchScreen::ancorPointOnScreen[i];
    float wagonDir[2];
    for (int i = 0; i < 2; i++)
        wagonDir[i] = pRS->pCouplerFront->gabaritesOnScreen.bbMid[i] - pRS->pCouplerBack->gabaritesOnScreen.bbMid[i];
    float shiftDir=0;//stop train
    float shiftDistance = 0;
    if (v3lengthXY(dragDir) > 3) {
        if (v3lengthXY(wagonDir) > 3) {
            float dp = v2dotProduct(wagonDir, dragDir);
            if (abs(dp) > 0.1) {
                shiftDistance = dp * v3lengthXY(dragDir);
                if (abs(shiftDistance) > 10){//displayDPI*0.5) {
                    shiftDir = signOf(shiftDistance);
                }
            }
        }
    }
    pRoot->desirableZdir = shiftDir * pRoot->alignedWithTrainHead * pRS->alignedWithTrainHead;
    pRoot->desirableZspeed = abs(shiftDistance)*pRoot->desirableZdir;
    if(theApp.bEditMode)
        pRoot->accelerationLin = accelerationOnDrag;
    else
        pRoot->accelerationLin = accelerationLinActive;

    if (approveMoveIntent(pRoot, &theApp.gameTable)) {
        float brakingDistance = (pRoot->ownZspeed * pRoot->ownZspeed) / pRoot->accelerationLin;
        float sizeUnitPixelsSize = getUnitPixelsSize(pRS, &theApp.mainCamera);
        float brakingDistancePx = brakingDistance * sizeUnitPixelsSize;
        if (abs(shiftDistance) <= brakingDistancePx)
            pRoot->desirableZspeed = 0;
        else {
            if (theApp.bEditMode)
                pRoot->desirableZspeed = pRoot->desirableZdir * RollingStock::maxDragSpeed;
            else
                pRoot->desirableZspeed = pRoot->desirableZdir * RollingStock::maxTrainSpeed;
        }
    }
    else {//obstacle ahead
        TouchScreen::cancelDrag();
        return 1;
    }
    if (pRoot->ownZspeed != 0 || pRoot->desirableZspeed != 0) {
        if (applySpeedOnRails(pRoot) < 0) {
            return -1;
        }
    }

    //add drag ribbon
    if (v3lengthFromToXY(TouchScreen::ancorPointOnScreen, TouchScreen::cursorPos) > 30) {
        float dp = abs(v2dotProduct(wagonDir, dragDir));
        if (dp < 0.1) {
            if (v3lengthXY(dragDir) > UISubj::screenSize[1] * 0.1) {
                TouchScreen::cancelDrag();
                return 1;
            }
        }
        float g = fmin(1.0, dp * 2);
        float r = fmin(1.0, (1.0 - dp) * 2);
        MyColor color;
        color.setRGBA(r, g, 0.0f, 0.5f);

        float ribbonL = v3lengthXY(dragDir);
        float screenHpercent = ribbonL / UISubj::screenSize[1];
        float maxAllowedPercent = 0.6;
        if (screenHpercent> maxAllowedPercent) {
            TouchScreen::cancelDrag();
            return 1;
        }
        float ribbonW = 10.0 * (1.0- screenHpercent/maxAllowedPercent );
        if (ribbonW < 1.0)
            ribbonW = 1.0;
        ScreenLine::addLine2queue(TouchScreen::ancorPointOnScreen, TouchScreen::cursorPos, color.getUint32(), ribbonW, true);
        /*
        {//debug
            mylog("%d ancor[ %d x %d ]. ",(int)theApp.frameN,(int)TouchScreen::ancorPointOnScreen[0], (int)TouchScreen::ancorPointOnScreen[1]);
            mylog("R=%d ",(int)pSelectedSceneSubj00->gabaritesOnScreen.chordR);
            float gl[4];
            mat4x4_mul_vec4plus(gl, theApp.mainCamera.mViewProjection, pSelectedSceneSubj00->absCoords.pos,1, true);
            mylog("GL[ %f x %f x %f x %f ]\n", gl[0], gl[1], gl[2], gl[3]);
        }
        */
    }
    return 1;
}

int RollingStock::onClick() {
    onFocusOut();
    if (theApp.cameraMan.zoom == 1) {//refocus?
        if (theApp.cameraMan.subj2followN >= 0) {
            SceneSubj* pS = theApp.cameraMan.pSubjs->at(theApp.cameraMan.subj2followN);
            if (pS == this)
                theApp.cameraMan.setView(false);
            else
                theApp.cameraMan.setZoomTo(this);
        }
        else
            theApp.cameraMan.setView(false);
        return 1;
    }
    if (theApp.gameTable.stopAllTrains() > 0)
        return 1;
    theApp.cameraMan.setZoomTo(this);
    return 1;
}
int RollingStock::resetMaxSpeed() {
    //find closest to camera visible rail
    std::vector<SceneSubj*>* pRails=&theApp.gameTable.railsMap;
    float* camPos = theApp.mainCamera.ownCoords.pos;
    float closestDist = 1000000;
    for (int rN = pRails->size() - 1; rN >= 0; rN--) {
        Rail* pRail = (Rail*)pRails->at(rN);
        if (pRail == NULL)
            continue;
        if (pRail->railStatus != 0)
            continue;
        if (pRail->gabaritesOnScreen.isInViewRange < 0)
            continue;
        float d = v3lengthFromTo(camPos, pRail->absCoords.pos);
        if (closestDist > d)
            closestDist = d;
    }

    //float fd = sqrt(closestDist);// theApp.mainCamera.focusDistance;
    float fd = sqrt(theApp.mainCamera.focusDistance);
    RollingStock::maxTrainSpeed = fd * 0.1;
    RollingStock::maxDragSpeed = RollingStock::maxTrainSpeed * 3;
    RollingStock::accelerationLinActive = RollingStock::maxTrainSpeed * 0.02;
    RollingStock::accelerationLinPassive = RollingStock::accelerationLinActive / 3;
    RollingStock::accelerationOnDrag = RollingStock::accelerationLinActive * 10;
    return 1;
}

int RollingStock::onLeftButtonUp() { 
    dragging = false;
    float cursorShiftTolerance = fmax(theApp.mainCamera.targetDims[0], theApp.mainCamera.targetDims[1]) * 0.01;

    //mylog(">>>>>>>>>>>> %d holdFramesN=%d\n", theApp.frameN, TouchScreen::holdFramesN);

    if (TouchScreen::holdFramesN>10) {
        //stop train
        RollingStock* pRS = this;
        RollingStock* pRoot = (RollingStock*)pRS->pSubjsSet->at(pRS->trainRootN);
        if (pRoot->desirableZdir != 0)
            MeltingSign::addMeltingSign("/dt/ui/signs/stop.png", TouchScreen::cursorPos[0], TouchScreen::cursorPos[1]);
        pRoot->desirableZdir = 0;
        pRoot->desirableZspeed = 0;
    }
    return onFocusOut(); 
}
int RollingStock::deleteMe(bool withChilds) {
    RollingStock* pRS = this;
    if (pRS->nInSubjsSet==theApp.gameTable.primeLocoN) {
        theApp.gameTable.primeLocoN = -1;
    }
    Smoke::fillWithGas(pRS);
    MySound::playSound(MySound::soundNpop01, pRS, false);
    SceneSubj::deleteMe(withChilds);
    return 1;
}

bool RollingStock::obstacleAhead(RollingStock* pTrainRoot) {
    std::vector<SceneSubj*>* pSubjs = pTrainRoot->pSubjsSet;
    if (pTrainRoot->nInSubjsSet != pTrainRoot->trainRootN)
        pTrainRoot = (RollingStock*)pSubjs->at(pTrainRoot->trainRootN);
    if (pTrainRoot->ownZspeed == 0)
        return false;
    // s = (v0 * v0) / (a * 2)
    float v0 = pTrainRoot->ownZspeed;
    float brakingDistance = (v0 * v0) / (pTrainRoot->accelerationLin * 2);
    Train* pTr = &pTrainRoot->tr;
    //select train forward coupler
    Coupler* pFwdCp = pTr->pCouplerFront;
    if(pTrainRoot->ownZspeed < 0)
        pFwdCp = pTr->pCouplerBack;
    //check clearance
    float d2obstacle = brakingDistance+20;
    if (pFwdCp->connected >= 0)
        d2obstacle = pFwdCp->distance;
    d2obstacle = theApp.gameTable.checkClearanceAhead(&pFwdCp->railCoord,&theApp.gameTable, d2obstacle)-10;

    if (d2obstacle <= brakingDistance)
        return true;
    return false;
}
bool RollingStock::approveMoveIntent(RollingStock* pRS,RailMap45* pMap) {
    std::vector<SceneSubj*>* pSubjs = pRS->pSubjsSet;
    //make sure it's a train root
    if (pRS->nInSubjsSet != pRS->trainRootN)
        pRS = (RollingStock*)pSubjs->at(pRS->trainRootN);
    float checkZspeed = pRS->ownZspeed;
    if (checkZspeed == 0)
        checkZspeed = pRS->desirableZspeed;
    if (checkZspeed == 0)
        return true;
    //select train forward coupler
    Train* pTr = &pRS->tr;
    Coupler* pFwdCp = pTr->pCouplerFront;
    if (checkZspeed*pRS->alignedWithTrainHead < 0)
        pFwdCp = pTr->pCouplerBack;
    //check clearance
    float brakingDistance = 0;
    if (pTr->poweredLength > 0) {
        // brakingDistance: s = (v0 * v0) / (a * 2)
        float v0 = pRS->ownZspeed;
        brakingDistance = (v0 * v0) / (pRS->accelerationLin * 2);
    }
    float bounceBackDist = 10;
    float d2check = brakingDistance+ pFwdCp->lever+bounceBackDist * 2;

    float d2obstacle = theApp.gameTable.checkClearanceAhead(&pFwdCp->railCoord, pMap, d2check);

    //bounce back?
    if (d2obstacle <= bounceBackDist) {
        pRS->desirableZspeed = 0;
        pRS->desirableZdir = 0;
        if (pRS->ownZspeed != 0) {
            float maxBBspeed = maxTrainSpeed * 0.3;//max bounce back speed
            pRS->ownZspeed = minimax(-pRS->ownZspeed*0.8,-maxBBspeed, maxBBspeed);
            //tremble?
            float amp = fmin(1.0, abs(pRS->ownZspeed) * 0.5);
            if (amp > 0.1) {
                pRS = (RollingStock*)pFwdCp->getRoot();
                pRS->tremble = amp;
            }
            //stop sign
            RailCoord rc;
            RailMap45::shiftRailCoord(&rc, &pFwdCp->railCoord, pMap, d2obstacle + pFwdCp->lever + bounceBackDist, false);
            float vIn[4];
            v3copy(vIn, rc.xyz);
            vIn[1] += 10;
            float vOut[4];
            Camera* pCam = &theApp.mainCamera;
            mat4x4_mul_vec4screen(vOut, pCam->mViewProjection, vIn, pCam->targetRads, pCam->nearClip, pCam->farClip);
            MeltingSign::addMeltingSign("/dt/ui/signs/noway.png", vOut[0], vOut[1]);
        }
        return false;
    }
    //slow down?
    if (pRS->desirableZspeed != 0) {
        if (pTr->poweredLength > 0) {
            if (d2obstacle <= brakingDistance + pFwdCp->lever + bounceBackDist) {//
                pRS->desirableZspeed = 0;
                pRS->desirableZdir = 0;
                return false;
            }
            //if coupler ahead
            if (pFwdCp->connected >= 0) {
                if (pFwdCp->distance <= brakingDistance) {
                    pRS->desirableZspeed = 0;
                    return true;
                }
            }
        }
    }
    return true;
}



