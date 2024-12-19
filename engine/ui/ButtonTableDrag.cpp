
#include "ButtonTableDrag.h"
#include "ScreenFrame.h"
#include "ButtonMap.h"
#include "Texture.h"
#include "UIhint.h"
#include "TheApp.h"

extern TheApp theApp;

int ButtonTableDrag::drag_table_xy = -1;
int ButtonTableDrag::drag_table_xz = -1;
int ButtonTableDrag::drag_table_spin = -1;
int ButtonTableDrag::drag_table_sun=-1;

/*
int ButtonTableDrag::addTableButtons() {
	ScreenFrame* pSF = new ScreenFrame("ScreenFrame", NULL, NULL);
	ButtonMap* pBM = new ButtonMap("ButtonMap", NULL, NULL);
	float r = buttonsH;
	float x = r + buttonsMargin * 3;
	//init table drag group
	drag_table_spin = addTableButton("drag_table_spin", x, buttonsMargin, r,r, "bottom right", "/dt/ui/buttons/drag_table_spin.bmp", "/dt/ui/buttons/hint_turn_table.bmp");
	drag_table_xz = addTableButton("drag_table_xz", x, r + buttonsMargin * 3, r,r, "bottom right", "/dt/ui/buttons/drag_table_xz.bmp", "/dt/ui/buttons/hint_drag_table.bmp");
	//drag_table_xz = addTableButton("drag_table_xy", x, y += d, w, h, "", "/dt/ui/buttons/drag_table_xy.bmp", "/dt/ui/buttons/hint_lift_table.bmp");
	//drag_table_sun = addTableButton("drag_table_sun", x, y += d, w, h, "", "/dt/ui/buttons/drag_table_sun.bmp", "/dt/ui/buttons/hint_shift_light.bmp");
	return 1;
}
*/
int ButtonTableDrag::addTableButton(std::string uiName, float x, float y, float w, float h, std::string alignTo, std::string src, std::string hintSrc,int hintRowsN) {

	UISubj* pUI = new ButtonTableDrag(uiName, NULL, NULL);
	//strcpy_s(pUI->name, 32, uiName.c_str());

	std::vector<DrawJob*>* pDrawJobs = pUI->pDrawJobs;

	setCoords(pUI, x, y, w, h, alignTo);

	attachHint(pUI, hintSrc, hintRowsN);

	pUI->djStartN = djNround;
	memcpy((void*)&pUI->mt0, (void*)&pDrawJobs->at(pUI->djStartN)->mt, sizeof(Material));

	int uTex0 = Texture::loadTexture(src);

	pUI->mt0.uTex0 = uTex0;

	pUI->mt0.uAlphaFactor = 0.4;


	return pUI->nInSubjs;
}
int ButtonTableDrag::renderButtonTable(UISubj* pUI) {
	TheTable* pTable = &theApp.gameTable;
	if (strcmp(pUI->name64, "drag_table_xz") == 0) {
		if (pTable->cursorOnTableGood == false)
			return 0;
	}
	float alpha = 0.4;
	int selectedDragMode = 0;//xz
	if (TouchScreen::pSelected == pTable) {
		selectedDragMode = 0;//xz
		if (TouchScreen::cursorStatus == 2)//dragging already
			selectedDragMode = pTable->dragMode;
		else {//hover
			selectedDragMode = 0;//xz
			if (pTable->cursorOnTableGood == false)
				selectedDragMode = 1;//spin only
			else {//check cursor position
				float* cPos = TouchScreen::cursorPos;
				if (TouchScreen::cursorStatus == 1)
					cPos = TouchScreen::captureCursorPos;
				for (int i = 0; i < 2; i++) {
					if (cPos[i] < ScreenFrame::screenFrameMargin) {
						selectedDragMode = 1;//spin - out of frame
						break;
					}
					else if (cPos[i] > screenSize[i] - ScreenFrame::screenFrameMargin) {
						selectedDragMode = 1;//spin - out of frame
						break;
					}
				}
			}
		}
		bool thisButton = false;
		if (selectedDragMode == 0 && strcmp(pUI->name64, "drag_table_xz") == 0)
			thisButton = true;
		else if (selectedDragMode == 1 && strcmp(pUI->name64, "drag_table_spin") == 0)
			thisButton = true;

		if (thisButton) {
			if (TouchScreen::cursorStatus == 0)
				alpha = 0.7;//hover
			else if (TouchScreen::cursorStatus == 2)
				alpha = 1;//drag
			
			//hint?
			float curPos[2];
			curPos[0] = TouchScreen::cursorPos[0] - screenSize[0] / 2;
			curPos[1] = -TouchScreen::cursorPos[1] + screenSize[1] / 2;
			bool cursorOn = true;
			for (int i = 0; i < 2; i++) {
				if (curPos[i] < pUI->absCoords.pos[i] - pUI->scale[i] / 2)
					cursorOn = false;
				else if (curPos[i] > pUI->absCoords.pos[i] + pUI->scale[i] / 2)
					cursorOn = false;
			}
			/*
			//debug
			mylog("%d button %dx%d, cursor %dx%d, curPos %dx%d \n",theApp.frameN,
				(int)pUI->absCoords.pos[0], (int)pUI->absCoords.pos[1],
				(int)TouchScreen::cursorPos[0], (int)TouchScreen::cursorPos[1],
				(int)curPos[0], (int)curPos[1]);
			*/
			if (cursorOn) {
				UISubj* pUIhint = pUI->pSubjs->at(UIhint::hintSubjN);
				pUIhint->mt0.uTex0 = pUI->hintTexN;
				v2copy(pUIhint->scale, pUI->hintSize);
				renderStandard(pUIhint);
			}
			
		}
		else {//wrong button
			alpha = 0.4;
		}
	}
	else {//not a table
		alpha = 0.4;
	}
	/*
	//debug
	if (strcmp(pUI->name64, "drag_table_spin") == 0)
		mylog("%d selectedDragMode=%d cursorStatus=%d\n", theApp.frameN, selectedDragMode, TouchScreen::cursorStatus);
	*/

	pUI->mt0.uAlphaFactor = alpha;
	return renderStandard(pUI);
}
