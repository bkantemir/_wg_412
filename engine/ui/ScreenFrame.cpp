#include "ScreenFrame.h"
#include "TheApp.h"
#include "TouchScreen.h"

extern TheApp theApp;

float ScreenFrame::screenFrameMargin = 10;

int ScreenFrame::renderScreenFrame(ScreenFrame* pUI) {
	TheTable* pTable = &theApp.gameTable;
	float ancorPoint[4];
	pTable->cursorOnTableGood=(pTable->getCursorAncorPointTable(ancorPoint, TouchScreen::cursorPos, pTable) > 0);
	if (pTable->cursorOnTableGood == false)
		return 0;
	screenFrameMargin = buttonsH + buttonsMargin * 2;
	float d = screenFrameMargin * 2;
	pUI->setCoords(pUI, 0,0, screenSize[0]-d, screenSize[1] - d, "");
	renderStandard(pUI);
	return 1;
}
