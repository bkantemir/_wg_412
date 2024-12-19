#include "ButtonUncouple.h"
#include "TheApp.h"
#include "rr/RollingStock.h"
#include "MeltingSign.h"

extern TheApp theApp;

int ButtonUncouple::renderButtonUncouple(ButtonUncouple* pUI) {
	Coupler* pCp = pUI->pCp;
	Coupler* pCp2 = pCp->pCounterCoupler;
	Camera* pCam = &theApp.mainCamera;
	float vIn[4];
	for (int i = 0; i < 3; i ++)
		vIn[i] = (pCp->absCoords.pos[i] + pCp2->absCoords.pos[i]) / 2;
	float screenPos[4];
	if (mat4x4_mul_vec4screen(screenPos, pCam->mViewProjection, vIn, pCam->targetRads, pCam->nearClip, pCam->farClip, 0) < 0) {
		return -1;
	}
	v3copy(pUI->ownCoords.pos, screenPos);
	for (int i = 0; i < 2; i++)
		pUI->ownCoords.pos[i] -= buttonsH / 2;


	pUI->transformMatrixIsReady = false;
	buildModelMatrix(pUI);

	if (TouchScreen::pSelected == pUI) {
		if (TouchScreen::cursorStatus == 0) {
			pUI->mt0.uAlphaFactor = 0.8; //hover
			//highlight involved cars
			pCp->getRoot()->setHighLight(0.2, MyColor::getUint32(255, 255, 255));
			pCp2->getRoot()->setHighLight(0.2, MyColor::getUint32(255, 255, 255));
		}
		else {
			pUI->mt0.uAlphaFactor = 1;
			//highlight involved cars
			int nOn = 3;
			int nTotal = nOn * 2;
			if (theApp.frameN % nTotal < nOn) {
				pCp->getRoot()->setHighLight(0.2, MyColor::getUint32(255, 255, 255));
				pCp2->getRoot()->setHighLight(0.2, MyColor::getUint32(255, 255, 255));
			}
			else {
				pCp->getRoot()->setHighLight(0);
				pCp2->getRoot()->setHighLight(0);
			}
		}
	}
	else
		pUI->mt0.uAlphaFactor = 0.6;

	renderStandard(pUI);
	return 1;
}
int ButtonUncouple::onFocusOut() {
	pCp->getRoot()->setHighLight(0);
	pCp->pCounterCoupler->getRoot()->setHighLight(0);
	UISubj::onFocusOut();
	return 1;
}
int ButtonUncouple::uncouple(ButtonUncouple* pUI) {
	Coupler* pCp = pUI->pCp;

	pCp->getRoot()->setHighLight(0);
	pCp->pCounterCoupler->getRoot()->setHighLight(0);

	RollingStock::decouple(pCp);

	{//split away?
		RollingStock* pWagon = (RollingStock*)pCp->getRoot();
		RollingStock* pTrainRoot = (RollingStock*)pWagon->pSubjsSet->at(pWagon->trainRootN);
		Coupler* pCp2 = pCp->pCounterCoupler;
		RollingStock* pWagon2 = (RollingStock*)pCp2->getRoot();
		RollingStock* pTrainRoot2 = (RollingStock*)pWagon2->pSubjsSet->at(pWagon2->trainRootN);
		float outSpeed = -0.3;
		pTrainRoot->ownZspeed = outSpeed * pCp->alignedWithRoot * pWagon->alignedWithTrainHead * pTrainRoot->alignedWithTrainHead;
		pTrainRoot2->ownZspeed = outSpeed * pCp2->alignedWithRoot * pWagon2->alignedWithTrainHead * pTrainRoot2->alignedWithTrainHead;
	}


	//MeltingSign::addMeltingSign("/dt/ui/signs/lock02.png", TouchScreen::cursorPos[0], TouchScreen::cursorPos[1]);// pUI->ownCoords.pos[0], pUI->ownCoords.pos[1]);

	//delete button
	TouchScreen::abandonSelected();
	pUI->deleteMe();
	pCp->pUIuncoupleButton = NULL;

	mylog("%d delete uncouple button, pSelectedNULL=%d\n",theApp.frameN,TouchScreen::pSelected==NULL);
	SceneSubj* pT = &theApp.gameTable;;

	return 1;
}

