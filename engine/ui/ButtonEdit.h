#pragma once
#include "UISubj.h"
//#include <vector>
#include "Texture.h"
//#include "rr/Coupler.h"

class ButtonEdit : public UISubj
{
public:
	bool bEditMode = false;

	static int texNedit;
	static int texNnoedit;

	static int texNhint_edit;
	static int texNhint_play;

public:
	ButtonEdit() : UISubj("ButtonEdit", NULL, NULL) {
		strcpy_s(className, 32, "ButtonEdit");

		ButtonEdit* pUI = this;

		texNedit = Texture::loadTexture("/dt/ui/buttons/edit01.png");
		texNnoedit = Texture::loadTexture("/dt/ui/buttons/edit02.png");

		texNhint_edit = Texture::loadTexture("/dt/ui/buttons/hint_edit_mode.bmp");
		texNhint_play = Texture::loadTexture("/dt/ui/buttons/hint_play_mode.bmp");

		pUI->djStartN = djNround;
		memcpy((void*)&pUI->mt0, (void*)&pUI->pDrawJobs->at(pUI->djStartN)->mt, sizeof(Material));
		float h = buttonsH;// *0.9;
		float thisButtonMargin = (buttonsH - h) / 2;
		float w = h;//
		float x = buttonsH * 2 + buttonsMargin * 4+ thisButtonMargin;
		float y = buttonsMargin+ thisButtonMargin;
		pUI->mt0.uTex0 = texNedit;
		pUI->mt0.uAlphaFactor = 0.6;

		setCoords(pUI, x,y, w,h, "top left");

		pUI->hintTexN = texNhint_edit;
		Texture* pHintTx = Texture::textures.at(pUI->hintTexN);
		pUI->hintSize[1] = hintRowH * 2;//2 rows
		pUI->hintSize[0] = pUI->hintSize[1] * ((float)pHintTx->size[0] / pHintTx->size[1]);

	};
	virtual bool isDraggable() { return false; };
	virtual bool isClickable() { return true; };
	virtual bool isResponsive() { return true; };
	virtual int onClick() { return onLeftButtonUp(); };
	virtual int render() { return renderButtonEdit(this); };
	static int renderButtonEdit(ButtonEdit* pUI);
	virtual int onLeftButtonUp() { return switchMode(this); };
	static int switchMode(ButtonEdit* pUI);
};
