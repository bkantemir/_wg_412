#include "ButtonMap.h"
#include "TheApp.h"
#include "MySound.h"

extern TheApp theApp;

int ButtonMap::texNmap=-1;
int ButtonMap::texNnomap=-1;

int ButtonMap::texNhint_map = -1;
int ButtonMap::texNhint_table = -1;


int ButtonMap::renderButtonMap(ButtonMap* pUI) {

	float camPitch = theApp.mainCamera.ownCoords.getEulerDg(0);
	pUI->mapMode = (camPitch>70);

	if (pUI->mapMode) {
		pUI->mt0.uTex0 = texNmap;
		pUI->hintTexN = texNhint_table;
		//flashing
		int nOn = 10;
		int nTotal = nOn * 2;
		if(theApp.frameN%nTotal<nOn)
			pUI->mt0.uTex0 = texNnomap;
	}
	else {
		pUI->mt0.uTex0 = texNnomap;
		pUI->hintTexN = texNhint_map;
	}

	if (TouchScreen::pSelected == pUI) {
		if (TouchScreen::cursorStatus == 0)
			pUI->mt0.uAlphaFactor = 0.85;//hover
		else //pressing
			pUI->mt0.uAlphaFactor = 1;
	}
	else
		pUI->mt0.uAlphaFactor = 0.7;

	return renderStandard(pUI);
}
int ButtonMap::switchMapMode(ButtonMap* pUI) {
	MySound::playSound(MySound::soundNclick01);

	float camPitch = theApp.mainCamera.ownCoords.getEulerDg(0);
	pUI->mapMode = (camPitch > 70);//current mode
	pUI->mapMode = !pUI->mapMode;//new mode

	CameraMan::setView(pUI->mapMode);

	return pUI->onFocusOut();
}



