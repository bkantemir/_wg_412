#include "CameraMan.h"
#include "TheApp.h"
#include "rr/RollingStock.h"
#include "ButtonTableDrag.h"
#include "MySound.h"


extern TheApp theApp;

float CameraMan::closeViewMargin = 20;

int CameraMan::zoomInOut(SceneSubj* pS) {
	if (pS == NULL)
		pS = &theApp.gameTable;

	bool sameSubj = false;
	if (subj2followN == pS->nInSubjsSet)
		if (pSubjs == pS->pSubjsSet)
			if (birthFrameN == pS->birthFrameN)
				sameSubj = true;

	if (pS == &theApp.gameTable)
		task=0;//change position
	else
		task=1;//focus and follow subj

	pSubjs = pS->pSubjsSet;
	subj2followN = pS->nInSubjsSet;
	birthFrameN = pS->birthFrameN;

	Camera* pMainCam = &theApp.mainCamera;
	Camera::copyParams(&startCamera, pMainCam);
	Camera::copyParams(&endCamera, pMainCam);
	Camera camClose;
	Camera camFar;
	Camera::copyParams(&camClose, pMainCam);
	Camera::copyParams(&camFar, pMainCam);
	//decide zoom in or zoom out:
	if (task==0) {//table - just refocus
		zoom = 0;
		Camera* pCam= &endCamera;
		Camera::copyParams(pCam, pMainCam);
		TheTable::getCursorAncorPointTable(pCam->lookAtPoint, TouchScreen::cursorPos, &theApp.gameTable);
		pCam->focusDistance = Camera::getDistance4stage(pCam, theApp.gameTable.suggestedStageSize());
		pCam->ownCoords.setPitch(Camera::pitchDefault);
		//pCam->ownCoords.setYaw(pCam->ownCoords.getEulerDg(1)+10);
		Camera::setCameraPosition(pCam);
		float d = v3lengthFromTo(pMainCam->ownCoords.pos, pCam->ownCoords.pos);
		if (d < 50) {
			//MySound::playSound(MySound::soundNwhat01, NULL, false);
			task = -1;
			return 0;
		}
	}
	else {//task==1? - SceneSubj
		Gabarites::adjustMidRad(&pS->gabaritesWorldAssembly);
		//near cam
		Camera* pCam = &camClose;
		v3copy(pCam->lookAtPoint, pS->gabaritesWorldAssembly.bbMid);
		pCam->ownCoords.setPitch(20);// Camera::pitchDefault);
		float stageRad = fmax(pS->gabaritesWorldAssembly.bbRad[0], pS->gabaritesWorldAssembly.bbRad[2]) + closeViewMargin;
		pCam->focusDistance = Camera::getDistance4stage(pCam, stageRad*2, pS->gabaritesWorldAssembly.bbRad[1]*2 + closeViewMargin);

		if (pS->pSubjsSet == &theApp.staticSubjs) {

			float yaw00 = pMainCam->ownCoords.getEulerDg(1);
			float targetYaw = 0;
			float bestD = 1000000;
			float y00 = pS->absCoords.getEulerDg(1);
			for (int y0 = 0; y0 < 360; y0 += 90) {
				float y = y00+y0 + 45;
				float dy = angleDgFromTo(yaw00, y);
				if (abs(bestD) > abs(dy)) {
					targetYaw = y;
					bestD = dy;
				}
			}
			pCam->ownCoords.setYaw(targetYaw);
		}
		Camera::setCameraPosition(pCam);

		//far cam
		pCam = &camFar;
		v3copy(pCam->lookAtPoint, pS->gabaritesWorldAssembly.bbMid);
		pCam->ownCoords.setPitch(Camera::pitchDefault);
		pCam->focusDistance = Camera::getDistance4stage(pCam, theApp.gameTable.suggestedStageSize());
		Camera::setCameraPosition(pCam);

		pCam = &endCamera;
		if (zoom!=1 || !sameSubj) { //subj zoomIn
			Camera::copyParams(pCam, &camClose);
			zoom = 1;//close up
			//strcpy_s(ButtonTableDrag::defaultMode64, 64, "drag_table_spin");
		}
		else { //subj zoomOut
			Camera::copyParams(pCam, &camFar);
			zoom = 0;//table view
			//strcpy_s(ButtonTableDrag::defaultMode64, 64, "drag_table_xz");
			task = 0;
		}
		//release subj
		pS->onFocusOut();
		SceneSubj::pSelectedSceneSubj=NULL;
		SceneSubj::pSelectedSceneSubj00 = NULL;
	}
	Camera* pCam = &endCamera;
	Camera::setRs4distance(pCam);
	progress = 0;
	progressSpeed = 0;
	return 1;
}

int CameraMan::process() {
	if (task < 0)//no task
		return 0;
	if (task > 0) {//subj: focus+follow | just follow
		//re-check subj
		SceneSubj* pS = pSubjs->at(subj2followN);
		if (pS == NULL)
			task = -1;
		else if (pS->birthFrameN != birthFrameN)
			task = -1;
		if (task < 0) //subj lost
			return 0;
		//update target
		Gabarites::adjustMidRad(&pS->gabaritesWorldAssembly);
		v3copy(endCamera.lookAtPoint, pS->gabaritesWorldAssembly.bbMid);
		float stageRadNow = fmax(pS->gabaritesWorldAssembly.bbRad[0], pS->gabaritesWorldAssembly.bbRad[2]) + closeViewMargin;
		float newDist = Camera::getDistance4stage(&endCamera, stageRadNow*2);
		if (task == 2) {//just follow
			Camera* pCam = &theApp.mainCamera;
			pCam->focusDistance = fmax(pCam->focusDistance, newDist);
			v3copy(pCam->lookAtPoint, endCamera.lookAtPoint);
		}
		else //focus and follow
			endCamera.focusDistance = fmax(endCamera.focusDistance, newDist);		//process
	}
	if (task < 2) {//focus first
		Camera::setCameraPosition(&endCamera);
		Camera* pCam = &theApp.mainCamera;
		float brakingDistance = (progressSpeed * progressSpeed) / (progressAcceleration * 2);
		if (brakingDistance >= (1.0 - progress))
			progressSpeed -= progressAcceleration;
		else
			progressSpeed += progressAcceleration;
		progress += progressSpeed;
		if (progress >= 1) {//target point reached
			Camera::copyParams(pCam, &endCamera);
			if (task == 0)
				task = -1;//focus reached
			else {
				SceneSubj* pS = pSubjs->at(subj2followN);
				if (pS->pSubjsSet == &theApp.staticSubjs)
					task = -1;
				else
					task = 2; //from now - just follow
			}
		}
		else {//still moving
			float k0 = 1.0 - progress;
			float k1 = progress;
			float pitch = startCamera.ownCoords.getEulerDg(0) * k0 + endCamera.ownCoords.getEulerDg(0) * k1;
			float angleD = angleDgFromTo(startCamera.ownCoords.getEulerDg(1), endCamera.ownCoords.getEulerDg(1));
			float yaw = startCamera.ownCoords.getEulerDg(1) + angleD * k1;
			float roll = startCamera.ownCoords.getEulerDg(2) * k0 + endCamera.ownCoords.getEulerDg(2) * k1;
			pCam->ownCoords.setEulerDg(pitch, yaw, roll);
			for (int i = 0; i < 3; i++) {
				//pCam->ownCoords.pos[i] = startCamera.ownCoords.pos[i] * k0 + endCamera.ownCoords.pos[i] * k1;
				pCam->lookAtPoint[i] = startCamera.lookAtPoint[i] * k0 + endCamera.lookAtPoint[i] * k1;
			}
			pCam->focusDistance = startCamera.focusDistance * k0 + endCamera.focusDistance * k1;
		}
	}
	Camera::reset(&theApp.mainCamera, &theApp.gameTable.worldBox);

	//mylog("%d task=%d\n", (int)theApp.frameN, task);
	return 1;
}

int CameraMan::refocus(float* pos) {
	task = 0;//change position

	Camera::copyParams(&startCamera, &theApp.mainCamera);
	Camera::copyParams(&endCamera, &theApp.mainCamera);
	Camera* pMainCam = &theApp.mainCamera;
	Camera* pCam = &endCamera;
	if (zoom >= 0)//just refocus
		v3copy(pCam->lookAtPoint, pos);
	else{//full top view
		pCam->focusDistance = Camera::getDistance4stage(pCam,
			fmax(theApp.gameTable.worldBox.bbRad[0], theApp.gameTable.worldBox.bbRad[2]) * 2,
			fmin(theApp.gameTable.worldBox.bbRad[0], theApp.gameTable.worldBox.bbRad[2]) * 2);
		pCam->ownCoords.setPitch(85);

		float yaw00 = pMainCam->ownCoords.getEulerDg(1);
		float targetYaw = 0;
		float bestD = 1000000;
		for (int y = 0; y < 360; y += 90) {
			if (theApp.gameTable.worldBox.bbRad[0] != theApp.gameTable.worldBox.bbRad[2]) {
				if (theApp.gameTable.worldBox.bbRad[0] < theApp.gameTable.worldBox.bbRad[2])
					if (y % 180 == 0)
						continue;
				if (theApp.gameTable.worldBox.bbRad[0] > theApp.gameTable.worldBox.bbRad[2])
					if (y % 180 != 0)
						continue;
			}
			float dy = angleDgFromTo(yaw00, y);
			if (abs(bestD) > abs(dy)) {
				targetYaw = y;
				bestD = dy;
			}
		}
		pCam->ownCoords.setYaw(targetYaw);

		v3setAll(pCam->lookAtPoint, 0);
	}
	//strcpy_s(ButtonTableDrag::defaultMode64, 64, "drag_table_xz");
	Camera::setCameraPosition(pCam);
	Camera::setRs4distance(pCam);
	progress = 0;
	progressSpeed = 0;
	return 1;
}
int CameraMan::setView(bool mapView) {
	TheTable* pT = &theApp.gameTable;
	CameraMan* pCM = &theApp.cameraMan;
	pCM->task = 0;//change position
	Camera::copyParams(&pCM->startCamera, &theApp.mainCamera);
	Camera::copyParams(&pCM->endCamera, &theApp.mainCamera);
	Camera* pCam = &pCM->endCamera;
	v3set(pCam->lookAtPoint, 0, pT->groundLevel0, 0);

	if (mapView) {//switch to Map view
		pCM->zoom = -1;
		pCam->focusDistance = Camera::getDistance4stage(pCam,
			fmax(theApp.gameTable.worldBox.bbRad[0], theApp.gameTable.worldBox.bbRad[2]) * 2,
			fmin(theApp.gameTable.worldBox.bbRad[0], theApp.gameTable.worldBox.bbRad[2]) * 2);
		pCam->ownCoords.setPitch(85);

		float yaw00 = pCam->ownCoords.getEulerDg(1);
		float targetYaw = 0;
		float bestD = 1000000;
		for (int y = 0; y < 360; y += 90) {
			if (theApp.gameTable.worldBox.bbRad[0] < theApp.gameTable.worldBox.bbRad[2])
				if (y % 180 == 0)
					continue;
			if (theApp.gameTable.worldBox.bbRad[0] > theApp.gameTable.worldBox.bbRad[2])
				if (y % 180 != 0)
					continue;
			float dy = angleDgFromTo(yaw00, y);
			if (abs(bestD) > abs(dy)) {
				targetYaw = y;
				bestD = dy;
			}
		}
		pCam->ownCoords.setYaw(targetYaw);
	}
	else {//switch to table view
		pCM->zoom = 0;
		//pT->getCursorAncorPointTable(pCam->lookAtPoint, TouchScreen::cursorPos, pT);
		pCam->focusDistance = Camera::getDistance4stage(pCam, theApp.gameTable.suggestedStageSize());
		pCam->ownCoords.setPitch(Camera::pitchDefault);
		pCam->ownCoords.setYaw(pCam->ownCoords.getEulerDg(1) - 10);
	}
	Camera::setCameraPosition(pCam);
	Camera::setRs4distance(pCam);
	pCM->progress = 0;
	pCM->progressSpeed = 0;
	return 1;
}
int CameraMan::setZoomTo(SceneSubj* pS) {
	task = 1;//focus and follow subj
	zoom = 1;
	pSubjs = pS->pSubjsSet;
	subj2followN = pS->nInSubjsSet;
	birthFrameN = pS->birthFrameN;

	Camera* pMainCam = &theApp.mainCamera;
	Camera::copyParams(&startCamera, pMainCam);
	Camera::copyParams(&endCamera, pMainCam);
	Gabarites::adjustMidRad(&pS->gabaritesWorldAssembly);
	Camera* pCam = &endCamera;
	v3copy(pCam->lookAtPoint, pS->gabaritesWorldAssembly.bbMid);
	pCam->ownCoords.setPitch(20);// Camera::pitchDefault);
	float stageRad = fmax(pS->gabaritesWorldAssembly.bbRad[0], pS->gabaritesWorldAssembly.bbRad[2]) + closeViewMargin;
	pCam->focusDistance = Camera::getDistance4stage(pCam, stageRad * 2, pS->gabaritesWorldAssembly.bbRad[1] * 2 + closeViewMargin);

	if (pS->pSubjsSet == &theApp.staticSubjs) {
		float yaw00 = pMainCam->ownCoords.getEulerDg(1);
		float targetYaw = 0;
		float bestD = 1000000;
		float y00 = pS->absCoords.getEulerDg(1);
		for (int y0 = 0; y0 < 360; y0 += 90) {
			float y = y00 + y0 + 45;
			float dy = angleDgFromTo(yaw00, y);
			if (abs(bestD) > abs(dy)) {
				targetYaw = y;
				bestD = dy;
			}
		}
		pCam->ownCoords.setYaw(targetYaw);
	}
	Camera::setCameraPosition(pCam);

	//release subj
	pS->onFocusOut();
	TouchScreen::abandonSelected();

	Camera::setRs4distance(pCam);
	progress = 0;
	progressSpeed = 0;
	return 1;
}


