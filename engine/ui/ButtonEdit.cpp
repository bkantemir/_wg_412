#include "ButtonEdit.h"
#include "TheApp.h"
#include "TouchScreen.h"
#include "MySound.h"

extern TheApp theApp;

int ButtonEdit::texNedit = -1;
int ButtonEdit::texNnoedit = -1;

int ButtonEdit::texNhint_edit = -1;
int ButtonEdit::texNhint_play = -1;

int ButtonEdit::renderButtonEdit(ButtonEdit* pUI) {
	if (theApp.bEditMode) {
		pUI->mt0.uTex0 = texNedit;
		pUI->hintTexN = texNhint_play;
		//flashing
		int nOn = 10;
		int nTotal = nOn * 2;
		if (theApp.frameN % nTotal < nOn)
			pUI->mt0.uTex0 = texNnoedit;
	}
	else {
		pUI->mt0.uTex0 = texNnoedit;
		pUI->hintTexN = texNhint_edit;
	}

	if (TouchScreen::pSelected == pUI) {
		if(TouchScreen::cursorStatus==0)
			pUI->mt0.uAlphaFactor = 0.85;//hover
		else
			pUI->mt0.uAlphaFactor = 1;//pressing
	}
	else
		pUI->mt0.uAlphaFactor = 0.7;

	renderStandard(pUI);
	return 1;
}
int ButtonEdit::switchMode(ButtonEdit* pUI) {
	MySound::playSound(MySound::soundNclick01);
	theApp.trainIsReady = true;
	theApp.bEditMode = !theApp.bEditMode;
	theApp.gameTable.reInitLayout(&theApp.gameTable, theApp.gameTable.tileSize, true);
	theApp.gameTable.addVirtualRails();
	return 1;
}