#include "ButtonMove.h"
#include "TheApp.h"
#include "rr/RollingStock.h"

extern TheApp theApp;

int ButtonMove::renderButtonMove(ButtonMove* pUI) {
	Coupler* pCp = pUI->pCp;
	Camera* pCam = &theApp.mainCamera;
	float vIn[4] = { 0,16,4,0 };
	float vOut[4];
	mat4x4_mul_vec4plus(vOut, pCp->absModelMatrix, vIn, 1, false);
	float screenPos[4];
	if (mat4x4_mul_vec4screen(screenPos, pCam->mViewProjection, vOut, pCam->targetRads, pCam->nearClip, pCam->farClip, 0) < 0) {
		return -1;
	}
	v3copy(pUI->ownCoords.pos, screenPos);
	for (int i = 0; i < 2; i++)
		pUI->ownCoords.pos[i] -= buttonsH / 2;

	{//arrow direction
		//coupler screen pos 
		if (mat4x4_mul_vec4screen(screenPos, pCam->mViewProjection, pCp->railCoord.xyz, pCam->targetRads, pCam->nearClip, pCam->farClip, 0) < 0) {
			return -1;
		}
		//point ahead
		RailCoord rc;
		RailMap45::shiftRailCoord(&rc, &pCp->railCoord, &theApp.gameTable, 10, false);
		//rc.xyz[1] = pCp->absCoords.pos[1];

		float screenPos2[4];
		if (mat4x4_mul_vec4screen(screenPos2, pCam->mViewProjection, rc.xyz, pCam->targetRads, pCam->nearClip, pCam->farClip, 0) < 0) {
			return -1;
		}
		pUI->ownCoords.aZdg = v2dirDgFromToDownCCW(screenPos, screenPos2);
	}

	pUI->transformMatrixIsReady = false;
	buildModelMatrix(pUI);

	if (TouchScreen::pSelected == pUI) {
		if(TouchScreen::cursorStatus==0)
			pUI->mt0.uAlphaFactor = 0.8; //hover
		else
		pUI->mt0.uAlphaFactor = 1;
	}
	else
		pUI->mt0.uAlphaFactor = 0.6;

	renderStandard(pUI);
	return 1;
}
int ButtonMove::runTrain(ButtonMove* pUI) {
	if (pUI->pSwitchAhead != NULL) {
		//switch ahead
		if (pUI->pSwitchAhead->isOn == false) {
			Rail* pR = (Rail*)theApp.gameTable.railsMap.at(pUI->pSwitchAhead->ownRailN);
			Rail::switchRail(pR);
		}
	}
	Coupler* pCp = pUI->pCp;
	RollingStock* pRS = (RollingStock*)pCp->getRoot();
	RollingStock* pTrainRoot = (RollingStock*)pRS->pSubjsSet->at(pRS->trainRootN);
	pTrainRoot->desirableZdir = pCp->alignedWithRoot * pRS->alignedWithTrainHead * pTrainRoot->alignedWithTrainHead;
	//delete button
	TouchScreen::abandonSelected();
	pUI->deleteMe();
	pCp->pUImoveButton = NULL;
	return 1;
}

