#pragma once
#include "UISubj.h"
#include <vector>
#include "Texture.h"
#include "rr/Coupler.h"

class ButtonMove : public UISubj
{
public:
	Coupler* pCp = NULL;
	RailEnd* pSwitchAhead = NULL;
public:
	ButtonMove();
	ButtonMove(Coupler* pCp) : UISubj("ButtonMove", NULL, NULL) {
		strcpy_s(className, 32, "ButtonMove");

		ButtonMove* pUI = this;
		pUI->pCp = pCp;
		pCp->pUImoveButton = pUI;

		pUI->djStartN = djNround;
		memcpy((void*)&pUI->mt0, (void*)&pUI->pDrawJobs->at(pUI->djStartN)->mt, sizeof(Material));
		pUI->mt0.uTex0 = Texture::loadTexture("/dt/ui/signs/arrow01.png");;

		setCoords(pUI, 0,0, buttonsH, buttonsH, "top left");

		pUI->mt0.uAlphaFactor = 0.66;

	};
	virtual int render() { return renderButtonMove(this); };
	static int renderButtonMove(ButtonMove* pUI);
	virtual bool isDraggable() { return false; };
	virtual bool isClickable() { return true; };
	virtual bool isResponsive() { return true; };
	virtual int onClick() { return onLeftButtonUp(); };
	virtual int onLeftButtonUp() { return runTrain(this); };
	static int runTrain(ButtonMove* pUI);
};
