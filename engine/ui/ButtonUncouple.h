#pragma once
#include "UISubj.h"
#include <vector>
#include "Texture.h"
#include "rr/Coupler.h"

class ButtonUncouple : public UISubj
{
public:
	Coupler* pCp = NULL;
public:
	ButtonUncouple();
	ButtonUncouple(Coupler* pCp) : UISubj("ButtonUncouple", NULL, NULL) {
		strcpy_s(className, 32, "ButtonUncouple");

		ButtonUncouple* pUI = this;
		pUI->pCp = pCp;
		pCp->pUIuncoupleButton = pUI;

		pUI->djStartN = djNround;
		memcpy((void*)&pUI->mt0, (void*)&pUI->pDrawJobs->at(pUI->djStartN)->mt, sizeof(Material));
		pUI->mt0.uTex0 = Texture::loadTexture("/dt/ui/signs/lock01.png");;

		setCoords(pUI, 0, 0, buttonsH, buttonsH, "top left");

		pUI->mt0.uAlphaFactor = 0.66;

	};
	virtual int render() { return renderButtonUncouple(this); };
	static int renderButtonUncouple(ButtonUncouple* pUI);
	virtual bool isDraggable() { return false; };
	virtual bool isClickable() { return true; };
	virtual bool isResponsive() { return true; };
	virtual int onClick() { return onLeftButtonUp(); };
	virtual int onLeftButtonUp() { return uncouple(this); };
	virtual int onFocusOut();
	static int uncouple(ButtonUncouple* pUI);
};
