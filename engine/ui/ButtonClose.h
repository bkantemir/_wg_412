#pragma once
#include "UISubj.h"
#include <vector>
#include "Texture.h"


class ButtonClose : public UISubj
{

public:
	ButtonClose() : UISubj("ButtonClose", NULL, NULL) {
		strcpy_s(className, 32, "ButtonClose");

		UISubj* pUI = this;
		float h = buttonsH;
		float w = h;
		float x =buttonsMargin;
		float y = buttonsMargin;
		setCoords(pUI, x, y, w, h, "top right");

		pUI->djStartN = djNround;
		memcpy((void*)&pUI->mt0, (void*)&pUI->pDrawJobs->at(pUI->djStartN)->mt, sizeof(Material));
		pUI->mt0.uTex0 = Texture::loadTexture("/dt/ui/buttons/close01.png");

		pUI->mt0.uAlphaFactor = 0.4;
	};
	virtual int render() { return renderButtonClose(this); };
	static int renderButtonClose(ButtonClose* pUI);
	virtual int onClick() { return onLeftButtonUp(); };
	virtual int onLeftButtonUp() { return exitApp(); };
	static int exitApp();
	virtual bool isDraggable() { return false; };
	virtual bool isClickable() { return true; };
	virtual bool isResponsive() { return true; };
};
