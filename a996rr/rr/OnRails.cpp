#include "OnRails.h"
#include "TheApp.h"
#include "RollingStock.h"

extern TheApp theApp;

int OnRails::moveOnRails(OnRails* pOR) {
    //if (theApp.frameN == 0)
    //    return 1;
    if (theApp.gameTable.railsMap.size() < 1)
        return moveStandard(pOR);

    if (applySpeedOnRails(pOR) < 0) {
        SceneSubj* pRoot = pOR->pSubjsSet->at(pOR->rootN);
        pRoot->deleteMe();
        return 0;//subj deleted
    }
    return 1;
}
int OnRails::applySpeedOnRails(OnRails* pOR) {
    memcpy(&pOR->railCoordOld, &pOR->railCoord, sizeof(RailCoord));
    if (pOR->d2parent == 0) {    //apply speed
        if (pOR->ownZspeed != pOR->desirableZspeed) {
            float d = pOR->desirableZspeed - pOR->ownZspeed;
            if (abs(d) < pOR->accelerationLin)
                pOR->ownZspeed = pOR->desirableZspeed;
            else
                pOR->ownZspeed = pOR->ownZspeed + pOR->accelerationLin * signOf(d);
        }
        if (pOR->ownZspeed != 0) {
            if (theApp.gameTable.shiftRailCoord00(&pOR->railCoord, &theApp.gameTable, pOR->ownZspeed,true) < 0) {
                return -1;//out of rail
            }
        }
    }
    else{//if (pOR->d2parent != 0) {
        OnRails* pRoot = (OnRails*)pOR->pSubjsSet->at(pOR->rootN);
        memcpy(&pOR->railCoord, &pRoot->railCoord, sizeof(RailCoord));
        if (pOR->zOffsetFromRoot != 0) {

            //debug
            if (pRoot->railCoord.railN < 0) {
                RollingStock* pRS = (RollingStock*)pRoot;
                RollingStock* pTR = (RollingStock*)pRS->pSubjsSet->at(pRS->trainRootN);
                int a = theApp.frameN;
            }


            if (theApp.gameTable.shiftRailCoord00(&pOR->railCoord, &theApp.gameTable, pOR->zOffsetFromRoot, true) < 0) {
                return -1;//out of rail
            }
        }
        //adjust yaw if not aligned with root
        if (pOR->alignedWithRoot < 0)
            pOR->railCoord.currentYaw = angleDgNorm180(pOR->railCoord.currentYaw + 180);
    }

    if (pOR->wheelBase == 0) {
        //clone to ownCoord
        v3copy(pOR->ownCoords.pos, pOR->railCoord.xyz);
        pOR->ownCoords.setEulerDg(0, pOR->railCoord.currentYaw, 0);
    }
    else { //have wheelBase
        RailCoord toFront;
        RailCoord* p0=&toFront;
        float offset = pOR->wheelBase / 2*pOR->alignedWithRoot;
        memcpy(p0, &pOR->railCoord, sizeof(RailCoord));
        if (theApp.gameTable.shiftRailCoord00(p0, &theApp.gameTable, offset, true) < 0) {
            return -1;//out of rail
        }
        RailCoord toBack;
        p0 = &toBack;
        offset = -pOR->wheelBase / 2 * pOR->alignedWithRoot;
        memcpy(p0, &pOR->railCoord, sizeof(RailCoord));
        if (theApp.gameTable.shiftRailCoord00(p0, &theApp.gameTable, offset, true) < 0) {
            return -1;//out of rail
        }
        pOR->ownCoords.pos[1] = pOR->railCoord.xyz[1];
        for (int i = 0; i < 3; i+=2)
            pOR->ownCoords.pos[i] = (toFront.xyz[i] + toBack.xyz[i]) * 0.5;
        float yaw = v3yawDgFromTo(toBack.xyz, toFront.xyz);
        pOR->ownCoords.setEulerDg(0, yaw, 0);

    }
    return 1;
}



void OnRails::buildModelMatrixOnRails(OnRails* pSS) {

    mat4x4_translate(pSS->ownModelMatrixUnscaled, pSS->ownCoords.pos[0], pSS->ownCoords.pos[1], pSS->ownCoords.pos[2]);
    //rotation order: Z-X-Y
    mat4x4_mul(pSS->ownModelMatrixUnscaled, pSS->ownModelMatrixUnscaled, pSS->ownCoords.rotationMatrix);

    // ignore parent and stickTo
    memcpy(pSS->absModelMatrixUnscaled, pSS->ownModelMatrixUnscaled, sizeof(mat4x4));

    if (v3equals(pSS->scale, 1))
        memcpy(pSS->absModelMatrix, pSS->absModelMatrixUnscaled, sizeof(mat4x4));
    else
        mat4x4_scale_aniso(pSS->absModelMatrix, pSS->absModelMatrixUnscaled, pSS->scale[0], pSS->scale[1], pSS->scale[2]);

    //update absCoords
    memcpy(&pSS->absCoords, &pSS->ownCoords, sizeof(Coords));

    pSS->absCoordsUpdateFrameN = theApp.frameN;
}
int OnRails::onLoadOR(OnRails* pOR, std::string tagStr) {
    //tag instructions
    if (tagStr.length() == 0)
        return 0;

    pOR->wheelBase = XMLparser::getFloatValue("wheelBase", tagStr);
    int wheelPairsN = XMLparser::getFloatValue("wheelPairs", tagStr);
    if (wheelPairsN > 2) {
        if (wheelPairsN % 2 == 0)
            pOR->wheelBase *= 0.78;
        else
            pOR->wheelBase *= 0.67;
    }
    return 1;
}

