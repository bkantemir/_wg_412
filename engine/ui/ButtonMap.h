#pragma once
#include "UISubj.h"
#include <vector>
#include "Texture.h"


class ButtonMap : public UISubj
{
public:
	bool mapMode = false;

	static int texNmap;
	static int texNnomap;

	static int texNhint_map;
	static int texNhint_table;

public:
	ButtonMap();
	ButtonMap(std::string name0, std::vector<UISubj*>* pSubjs = NULL, std::vector<DrawJob*>* pDJs = NULL)
		: UISubj(name0, pSubjs, pDJs) {
		strcpy_s(className, 32, "ButtonMap");

		texNmap = Texture::loadTexture("/dt/ui/buttons/map01.png");
		texNnomap = Texture::loadTexture("/dt/ui/buttons/map02.png");

		texNhint_map = Texture::loadTexture("/dt/ui/buttons/hint_map_view.bmp");
		texNhint_table = Texture::loadTexture("/dt/ui/buttons/hint_table_view.bmp");

		UISubj* pUI = this;
		float h = buttonsH;// *0.9;
		float thisButtonMargin = (buttonsH - h) / 2;
		float w = h;//
		float x = buttonsH + buttonsMargin * 3 + thisButtonMargin;
		float y = buttonsMargin + thisButtonMargin;
		setCoords(pUI, x, y, w, h, "top left");

		//attachHint(pUI, hintSrc, hintRowsN);

		pUI->djStartN = djNround;
		memcpy((void*)&pUI->mt0, (void*)&pUI->pDrawJobs->at(pUI->djStartN)->mt, sizeof(Material));
		pUI->mt0.uTex0 = texNmap;

		pUI->mt0.uAlphaFactor = 0.4;

		pUI->hintTexN = texNhint_map;
		Texture* pHintTx = Texture::textures.at(pUI->hintTexN);
		pUI->hintSize[1] = hintRowH * 2;//2 rows
		pUI->hintSize[0] = pUI->hintSize[1] * ((float)pHintTx->size[0] / pHintTx->size[1]);
	};
	virtual int render() { return renderButtonMap(this); };
	static int renderButtonMap(ButtonMap* pUI);
	virtual int onClick() { return onLeftButtonUp(); };
	virtual int onLeftButtonUp() { return switchMapMode(this); };
	static int switchMapMode(ButtonMap* pUI);
	virtual bool isDraggable() { return false; };
	virtual bool isClickable() { return true; };
	virtual bool isResponsive() { return true; };
};
