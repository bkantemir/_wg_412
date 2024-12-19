#include "ButtonClose.h"
#include "TheApp.h"
#include "MySound.h"

extern TheApp theApp;


int ButtonClose::renderButtonClose(ButtonClose* pUI) {

	if (TouchScreen::pSelected == pUI) {
		if (TouchScreen::cursorStatus == 0)
			pUI->mt0.uAlphaFactor = 0.7;//hover
		else //pressing
			pUI->mt0.uAlphaFactor = 1;
	}
	else
		pUI->mt0.uAlphaFactor = 0.4;

	return renderStandard(pUI);
}
int ButtonClose::exitApp() {
	MySound::playSound(MySound::soundNclick01);

	theApp.bExitApp = true;

	return 1;
}



