#include "TouchScreen.h"
#include "utils.h"
#include "TheApp.h"
#include "input.h"
#include "ScreenLine.h"
#include "Shadows.h"
//#include "rr/RollingStock.h"

extern TheApp theApp;
extern float degrees2radians;

ScreenSubj* TouchScreen::pSelected = NULL;

std::string TouchScreen::capturedCode = "NONE";
uint32_t TouchScreen::captureFrameN;
float TouchScreen::captureCursorPos[2];
float TouchScreen::ancorPoint[4] = { 0,0,0,0 };
float TouchScreen::ancorPointOnScreen[4] = { 0,0,0,0 };
uint16_t TouchScreen::clickMillisMax = 200;
bool TouchScreen::confirmedDrag = false;

int TouchScreen::cursorStatus; //-1 - off screen, 0 - hover, 1 - on, 2 - dragging
float TouchScreen::cursorPos[3] = { 0,0,0 };
float TouchScreen::holdPos[3] = { 0,0,0 };
int TouchScreen::holdFramesN = 0;
uint32_t TouchScreen::cursorFrameN=0;

std::vector<TouchScreen*> TouchScreen::touchScreenEvents;

LineXY TouchScreen::ax0[3];
LineXY TouchScreen::ax1[3];
LineXY TouchScreen::ax2[3];

uint64_t TouchScreen::leftButtonDownTime = 0;
float TouchScreen::leftButtonDownCursorPos[2];

void TouchScreen::cleanUp() {
	for (int eN = touchScreenEvents.size() - 1; eN >= 0; eN--)
		delete touchScreenEvents.at(eN);
	touchScreenEvents.clear();
}
void TouchScreen::addEvent(InputEvent eventCode00, float x, float y) {
	if (eventCode00 == MOVE_CURSOR) {
		cursorPos[0] = x;
		cursorPos[1] = y;
	}

	//debug
	//mylog("%d cursor %dx%d\n",theApp.frameN, (int)cursorPos[0], (int)cursorPos[1]);

	TouchScreen* pE = new TouchScreen;
	pE->eventCode = eventCode00;
	pE->pos[0] = x;
	pE->pos[1] = y;
	touchScreenEvents.insert(touchScreenEvents.begin(), pE);
}
void TouchScreen::getInput() {

	while (touchScreenEvents.size() > 0) {
		TouchScreen* pE = touchScreenEvents.back();
		touchScreenEvents.pop_back();
		switch (pE->eventCode) {
		case MOVE_CURSOR:
			continue;
		case SCREEN_IN:
			if (cursorStatus <0) {
				cursorStatus = 0;
			}
			continue;
		case SCREEN_OUT:
			cursorStatus = -1;
			capturedCode.assign("NONE");
			if (pSelected != NULL) {
				pSelected->onFocusOut();
				abandonSelected();
			}
			continue;
		case LEFT_BUTTON_DOWN:
			leftButtonDown();
			continue;
		case LEFT_BUTTON_UP:
			leftButtonUp();
			continue;
		case RIGHT_BUTTON_DOWN:
			continue;
		case RIGHT_BUTTON_UP:
			continue;
		case SCROLL:
			processScroll(pE->pos[0], pE->pos[1]);
			continue;
		default:
			continue;
		}
	}
	
	if (cursorFrameN != theApp.frameN) {
		cursorFrameN = theApp.frameN;
		//notable move?
		float d = v2lengthFromTo(cursorPos, holdPos);
		float cursorShiftTolerance = fmax(theApp.mainCamera.targetDims[0], theApp.mainCamera.targetDims[1]) * 0.02;
		if (d < cursorShiftTolerance)
			holdFramesN++;
		else {
			holdFramesN = 0;
			v2copy(holdPos, cursorPos);
		}
	}
	

	if (cursorStatus < 0) //if cursor out of screen
		return;

	if (cursorStatus == 2) {
		//dragging
		setCursor("fist");

		if(isDragAlready())
			if(pSelected!=NULL)
				pSelected->onDrag();
	}
	else{
		pointerOnSubj();
		if (cursorStatus == 1) {
			//left button down non-draggable
			if(pSelected==NULL)
				setCursor("arrow");
			else {
				setCursor("fingerdown");
			}
		}
		else if (cursorStatus == 0) {
			//hover;
			if (pSelected == NULL) {
				setCursor("palm");// hand");

				pSelected = &theApp.gameTable;
				SceneSubj::pSelectedSceneSubj = &theApp.gameTable;
				SceneSubj::pSelectedSceneSubj00 = &theApp.gameTable;
			}
			else if (pSelected->isDraggable())
				setCursor("vfingers");
			else if (pSelected->isClickable())
				setCursor("finger");
			else
				setCursor("arrow");
		}
	}

	//mylog("%d TouchScreen::capturedCode=%s\n", (int)theApp.frameN, TouchScreen::capturedCode.c_str());
}



void TouchScreen::leftButtonDown() {
	if (cursorStatus == 3)
		return; //multifinger (android)

	v2copy(leftButtonDownCursorPos, cursorPos);
	leftButtonDownTime = getSystemMillis();
	captureFrameN = theApp.frameN;
	v2copy(captureCursorPos, cursorPos);
	pointerOnSubj();
	if (pSelected == NULL) {
		pSelected = &theApp.gameTable;
		SceneSubj::pSelectedSceneSubj = &theApp.gameTable;
		SceneSubj::pSelectedSceneSubj00 = &theApp.gameTable;
		cursorStatus = 2; //table draggable
	}
	else {//have subj
		if (pSelected->isDraggable()) {
			cursorStatus = 2; //draggable subj
		}
		else if (pSelected->isClickable()) {
			cursorStatus = 1; //Clickable subj
		}
	}
	confirmedDrag = false;
	pSelected->onLeftButtonDown();

}
int TouchScreen::processScroll(float x, float y) {
	//ignore x, y<0-zoom out, >0-zoom in
	Camera* pCam = &theApp.mainCamera;
	float k = 1.0 + y * 0.025;
	for (int i = 0; i < 2; i++)
		pCam->focusDistance *= k;
	pCam->reset(pCam, &theApp.gameTable.worldBox);

	return 1;
}
bool TouchScreen::isDragAlready() {
	if (cursorStatus != 2) {
		confirmedDrag = false;
		return false;
	}
	if (!confirmedDrag) {
		uint64_t currentMillis = getSystemMillis();
		uint16_t dMillis = currentMillis - leftButtonDownTime;
		if (dMillis >= clickMillisMax) {
			confirmedDrag = true;
		}
		else {
			float mouseShift[2];
			for (int i = 0; i < 2; i++)
				mouseShift[i] = cursorPos[i] - captureCursorPos[i];
			float d = v2length(mouseShift);
			float cursorShiftTolerance = fmax(theApp.mainCamera.targetDims[0], theApp.mainCamera.targetDims[1]) * 0.02;
			if (d > cursorShiftTolerance) {
				confirmedDrag = true;
			}
		}
	}
	return confirmedDrag;
}
void TouchScreen::leftButtonUp() {

	if (pSelected != NULL) {
		/*
		{//debug
			uint64_t currentMillis = getSystemMillis();
			uint16_t dMillis = currentMillis - leftButtonDownTime;
			mylog("%d click? dMillis=%d confirmedDrag=%d cursorStatus=%d\n", theApp.frameN, dMillis, confirmedDrag, cursorStatus);
		}
		*/
		if (isDragAlready() == false) {
			//is a click?
			if (confirmedClick()) {
				pSelected->onClick();
				/*
				{//debug
					uint64_t currentMillis = getSystemMillis();
					uint16_t dMillis = currentMillis - leftButtonDownTime;
					mylog("%d click\n", theApp.frameN);
				}
				*/
			}
			else {
				pSelected->onLeftButtonUp();
				/*
				{//debug
					uint64_t currentMillis = getSystemMillis();
					uint16_t dMillis = currentMillis - leftButtonDownTime;
					mylog("%d ButtonUp\n", theApp.frameN);
				}
				*/
			}
		}
		else {//not draggable
			pSelected->onLeftButtonUp();
			/*
			{//debug
				uint64_t currentMillis = getSystemMillis();
				uint16_t dMillis = currentMillis - leftButtonDownTime;
				mylog("%d ButtonUp\n", theApp.frameN);
			}
			*/
		}
	}
	cursorStatus = 0;
	confirmedDrag = false;
	pSelected = NULL;
	capturedCode.assign("NONE");
	theApp.bPause = false;
	SceneSubj::pSelectedSceneSubj = NULL;
	UISubj::pSelectedUISubj = NULL;
}
bool TouchScreen::confirmedClick() {
	uint64_t currentMillis = getSystemMillis();
	uint16_t dMillis = currentMillis - leftButtonDownTime;
	if (dMillis >= clickMillisMax)
		return false;
	return true;
}

int TouchScreen::pointerOnSubj() {
	/*
	//debug
	mylog("%d pSelected ",theApp.frameN);
	if(pSelected==NULL)
		mylog("NULL", theApp.frameN);
	else
		mylog("<%s>", pSelected->className);
		*/
	ScreenSubj* pCandidate = UISubj::pointerOnUI();
	if (pCandidate == NULL) {
		SceneSubj* pS00 = NULL;
		pS00 = SceneSubj::pickFromCandidates(pS00, SceneSubj::pointerOnSceneSubj00(&theApp.sceneSubjs));
		pS00 = SceneSubj::pickFromCandidates(pS00, SceneSubj::pointerOnSceneSubj00(&theApp.staticSubjs));
		pS00 = SceneSubj::pickFromCandidates(pS00, Rail::pointerOnRail());
		pCandidate = SceneSubj::updateSelected00(pS00);
		/*
		//debug
		if (SceneSubj::pSelectedSceneSubj != NULL && SceneSubj::pSelectedSceneSubj00 == NULL)
			int a = 0;
			*/
	}

	if (pSelected != pCandidate) {

		//debug
		//int a = theApp.frameN;

		if (pSelected != NULL) {
			pSelected->onFocusOut();
		}
		pSelected = pCandidate;
		if (pSelected != NULL) {
			pSelected->onFocus();
		}
		//clean up previous selections
		if (pSelected != UISubj::pSelectedUISubj)
			UISubj::pSelectedUISubj = NULL;
		if (pSelected != SceneSubj::pSelectedSceneSubj) {
			SceneSubj::pSelectedSceneSubj = NULL;
			SceneSubj::pSelectedSceneSubj00 = NULL;
		}
	}
	/*
	//debug
	if (pSelected != NULL)
		if (strcmp(pSelected->className, "Rail") == 0) {
			float tablePoint00[4];
			//TheTable::getCursorAncorPointTable(tablePoint00, TouchScreen::cursorPos, &theApp.gameTable);
			//int TheTable::getCursorAncorPointTable(float* ancorPoint, float* cursorPos, TheTable * pTable) {
			float* ancorPoint = tablePoint00;
			float* cursorPos = TouchScreen::cursorPos;
			TheTable* pTable = &theApp.gameTable;
			v3setAll(ancorPoint, 0);
			float* targetRads = theApp.mainCamera.targetRads;
			//cursor position in GL range
			float cursorGLpos[4];
			cursorGLpos[0] = (TouchScreen::cursorPos[0] - targetRads[0]) / targetRads[0];
			cursorGLpos[1] = -(TouchScreen::cursorPos[1] - targetRads[1]) / targetRads[1];
			cursorGLpos[2] = 1;
			//cursor position in world coords
			float cursor3Dpos[4];
			//mat4x4_mul_vec4plus(cursor3Dpos, theApp.mainCamera.mVPinverted, cursorGLpos, 1, true);

			//rebuild mVPinverted WITHOUT keystone suppression
			Camera* pCam = &theApp.mainCamera;
			mat4x4 mVP;
			mat4x4 mProjection;
			mat4x4_perspective(mProjection, pCam->viewRangeDg * degrees2radians, pCam->targetAspectRatio, pCam->nearClip, pCam->farClip);
			mat4x4_mul(mVP, mProjection, pCam->lookAtMatrix);
			mat4x4 mVPinverted;
			mat4x4_invert(mVPinverted, mVP);
			mat4x4_mul_vec4plus(cursor3Dpos, mVPinverted, cursorGLpos, 1, true);

			Line3D* pL3D = new Line3D(theApp.mainCamera.ownCoords.pos, cursor3Dpos);
			float y0 = pTable->groundLevel0;
			if (Line3D::crossPlane(ancorPoint, pL3D, 1, y0) == 0) {
				mylog("ERROR in TheTable::getCursorAncorPointTable - no crossing\n");
				return -1;
			}

			mylog("%d cursorGLpos %.2fx%.2fx%.2f cursor3Dpos %.2fx%.2fx%.2f NaN=%dx%dx%d\n", theApp.frameN,
				cursorGLpos[0], cursorGLpos[1], cursorGLpos[2],
				cursor3Dpos[0], cursor3Dpos[1], cursor3Dpos[2],
			cursor3Dpos[0] != cursor3Dpos[0], cursor3Dpos[1] != cursor3Dpos[1], cursor3Dpos[2]!= cursor3Dpos[2]);
			mylog("%d pL3D %.2fx%.2fx%.2f to %.2fx%.2fx%.2f\n", theApp.frameN,
				pL3D->p0[0], pL3D->p0[1], pL3D->p0[2],
				pL3D->p1[0], pL3D->p1[1], pL3D->p1[2]);
			mylog("%d crossPoint %.2fx%.2fx%.2f\n", theApp.frameN,
				ancorPoint[0], ancorPoint[1], ancorPoint[2]);



			float tablePoint[2];
			tablePoint[0] = tablePoint00[0];
			tablePoint[1] = tablePoint00[2];
			Rail* pR = (Rail*)pSelected;
			float d;
			if (pR->railType == 0)//straight
				d = LineXY::dist_l2p(&pR->line2d, tablePoint);
			else //curved
				d = ArcXY::dist_arc2point(&pR->arc2d, tablePoint);
			mylog("%d rail#%d pos %dx%d cursor %dx%d => table %dx%d. D=%.2f\n", theApp.frameN, pR->nInSubjsSet,
				(int)pR->absCoords.pos[0], (int)pR->absCoords.pos[2],
				(int)TouchScreen::cursorPos[0], (int)TouchScreen::cursorPos[1],
				(int)tablePoint[0], (int)tablePoint[1], d);
		}
*/
	/*
//debug
	mylog(",then ", theApp.frameN);
	if (pSelected == NULL)
		mylog("NULL", theApp.frameN);
	else
		mylog("<%s>", pSelected->className);
	mylog("\n");
	*/

	return (pSelected != NULL);
}
void TouchScreen::abandonSelected() {
	pSelected = NULL;
	UISubj::pSelectedUISubj = NULL;
	SceneSubj::pSelectedSceneSubj = NULL;
	SceneSubj::pSelectedSceneSubj00 = NULL;
}
void TouchScreen::cancelDrag() {
	if (pSelected != NULL)
		pSelected->onFocusOut();
	abandonSelected();
}

