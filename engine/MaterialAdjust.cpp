#include "MaterialAdjust.h"
#include "utils.h"

std::vector<MaterialAdjust*> MaterialAdjust::materialAdjustsList;


int MaterialAdjust::cleanUp() {
	int itemsN = materialAdjustsList.size();
	//delete all materialAdjusts
	for (int i = 0; i < itemsN; i++)
		delete materialAdjustsList.at(i);
	materialAdjustsList.clear();
	return 1;
}

int MaterialAdjust::adjust(Material* pMT, MaterialAdjust* pMA) {
	if (pMA->b_name)
		strcpy_s(pMT->materialName32, 32, pMA->materialName32);

	if (pMA->b_shaderType)
		pMT->setShaderType(pMT, pMA->shaderType32);
	if (pMA->b_primitiveType)
		pMT->primitiveType = pMA->primitiveType;
	if (pMA->b_uColor) {
		memcpy(&pMT->uColor, &pMA->uColor, sizeof(MyColor));
		pMT->uTex0 = -1;
		pMT->uTex0translateChannelN = -1; //translate tex0 to tex3 by channelN. Default -1 - don't translate
		pMT->uTex3 = -1;
		pMT->uColor1.clear();
		pMT->uColor2.clear();
	}
	if (pMA->b_uTex0) {
		pMT->uTex0 = pMA->uTex0;
		pMT->uColor.clear();
	}
	if (pMA->b_uColor1)
		memcpy(&pMT->uColor1, &pMA->uColor1, sizeof(MyColor));
	if (pMA->b_uColor2)
		memcpy(&pMT->uColor2, &pMA->uColor2, sizeof(MyColor));
	if (pMA->b_uTex1mask)
		pMT->uTex1mask = pMA->uTex1mask;
	if (pMA->b_uTex2nm)
		pMT->uTex2nm = pMA->uTex2nm;
	if (pMA->b_uTex3)
		pMT->uTex3 = pMA->uTex3;
	if (pMA->b_uTex1alphaChannelN)
		pMT->uTex1alphaChannelN = pMA->uTex1alphaChannelN;
	if (pMA->b_uTex1alphaNegative)
		pMT->uTex1alphaNegative = pMA->uTex1alphaNegative;
	if (pMA->b_uTex0translateChannelN)
		pMT->uTex0translateChannelN = pMA->uTex0translateChannelN;
	if (pMA->b_uTexMods)
		m16copy((float*)pMT->uTexMods, (float*)pMA->uTexMods);
	if (pMA->b_uAlphaBlending) {
		pMT->uAlphaBlending = pMA->uAlphaBlending;
		if (pMT->uAlphaBlending == 0)
			pMT->uAlphaFactor = 0;
	}

	if (pMA->b_uAlphaFactor) {
		pMT->uAlphaFactor = pMA->uAlphaFactor;
		if (pMT->uAlphaFactor != 1)
			pMT->uAlphaBlending = 1;
	}
	if (pMA->b_uAmbient)
		pMT->uAmbient = pMA->uAmbient;
	if (pMA->b_uSpecularIntencity) {
		pMT->uSpecularIntencity = pMA->uSpecularIntencity;
		if (pMT->uSpecularIntencity == 0) {
			pMT->uSpecularMinDot[0] = 0.8f;
			pMT->uSpecularMinDot[1] = 1.0f;
			pMT->uSpecularPowerOf = 20.0f;
		}
	}
	if (pMA->b_uSpecularMinDot) {
		pMT->uSpecularMinDot[0] = pMA->uSpecularMinDot[0];
		pMT->uSpecularMinDot[1] = pMA->uSpecularMinDot[1];
	}
	if (pMA->b_uTranslucency)
		pMT->uTranslucency = pMA->uTranslucency;

	if (pMA->b_uSpecularPowerOf)
		pMT->uSpecularPowerOf = pMA->uSpecularPowerOf;
	if (pMA->b_uBleach)
		pMT->uBleach = pMA->uBleach;
	if (pMA->b_uShadingK)
		pMT->uShadingK = pMA->uShadingK;
	if (pMA->b_uEdgeAlpha)
		pMT->uEdgeAlpha = pMA->uEdgeAlpha;

	if (pMA->b_layer2as)
		strcpy_s(pMT->layer2as, 32, pMA->layer2as);

	if (pMA->b_forElement32)
		strcpy_s(pMT->forElement32, 32, pMA->forElement32);

	if (pMA->b_lineWidth)
		pMT->lineWidth = pMA->lineWidth;

	if (pMA->b_dropsShadow)
		pMT->dropsShadow = pMA->dropsShadow;
	if (pMA->b_dontRender)
		pMT->dontRender = pMA->dontRender;
	if (pMA->b_noStickers)
		pMT->noStickers = pMA->noStickers;
	return 1;
}
int MaterialAdjust::setWhat2adjust(MaterialAdjust* pMA, std::string tagStr) {
	if (tagStr.find("name_too") != std::string::npos)
		pMA->b_name = true;
	if (tagStr.find("uTex0") != std::string::npos)
		pMA->b_uTex0 = true;
	if (tagStr.find("uTex1mask") != std::string::npos)
		pMA->b_uTex1mask = true;
	if (tagStr.find("uTex2nm") != std::string::npos)
		pMA->b_uTex2nm = true;
	if (tagStr.find("uTex3") != std::string::npos)
		pMA->b_uTex3 = true;

	if (tagStr.find("mt_type") != std::string::npos)
		pMA->b_shaderType = true;
	if (tagStr.find("uColor") != std::string::npos)
		pMA->b_uColor = true;
	if (tagStr.find("uColor1") != std::string::npos)
		pMA->b_uColor1 = true;
	if (tagStr.find("uColor2") != std::string::npos)
		pMA->b_uColor2 = true;
	if (tagStr.find("primitiveType") != std::string::npos)
		pMA->b_primitiveType = true;
	if (tagStr.find("uTex1alphaChannelN") != std::string::npos)
		pMA->b_uTex1alphaChannelN = true;
	if (tagStr.find("uTex0translateChannelN") != std::string::npos)
		pMA->b_uTex0translateChannelN = true;
	if (tagStr.find("uTexMods") != std::string::npos)
		pMA->b_uTexMods = true;

	if (tagStr.find("uAlphaBlending") != std::string::npos)
		pMA->b_uAlphaBlending = true;
	if (tagStr.find("uAlphaFactor") != std::string::npos)
		pMA->b_uAlphaFactor = true;
	if (tagStr.find("uAmbient") != std::string::npos)
		pMA->b_uAmbient = true;
	if (tagStr.find("uSpecularIntencity") != std::string::npos)
		pMA->b_uSpecularIntencity = true;
	if (tagStr.find("uSpecularMinDot") != std::string::npos)
		pMA->b_uSpecularMinDot = true;
	if (tagStr.find("uTranslucency") != std::string::npos)
		pMA->b_uTranslucency = true;
	if (tagStr.find("uSpecularPowerOf") != std::string::npos)
		pMA->b_uSpecularPowerOf = true;
	if (tagStr.find("uBleach") != std::string::npos)
		pMA->b_uBleach = true;
	if (tagStr.find("uShadingK") != std::string::npos)
		pMA->b_uShadingK = true;
	if (tagStr.find("uEdgeAlpha") != std::string::npos)
		pMA->b_uEdgeAlpha = true;

	if (tagStr.find("layer2as") != std::string::npos)
		pMA->b_layer2as = true;
	if (tagStr.find("lineWidth") != std::string::npos)
		pMA->b_lineWidth = true;

	if (tagStr.find("noShadow") != std::string::npos)
		pMA->b_dropsShadow = true;
	if (tagStr.find("dontRender") != std::string::npos)
		pMA->b_dontRender = true;
	if (tagStr.find("noStickers") != std::string::npos)
		pMA->b_noStickers = true;

	if (tagStr.find("sticker") != std::string::npos) {
		pMA->b_dropsShadow = true;
		pMA->b_uAlphaBlending = true;
	}
	if (tagStr.find("4wire") != std::string::npos) {
		pMA->b_uColor = true;
		pMA->b_shaderType = true;
		pMA->b_uSpecularIntencity = true;
	}
	if (tagStr.find("forElement") != std::string::npos)
		pMA->b_forElement32 = true;

	return 1;
}

MaterialAdjust* MaterialAdjust::findMaterialAdjust(const char adjustmentName[], std::vector<MaterialAdjust*>* pList) {
	if (pList != NULL) {
		for (int maN = pList->size() - 1; maN >= 0; maN--) {
			MaterialAdjust* pMA = pList->at(maN);
			if (strcmp(pMA->materialName32, adjustmentName) == 0)
				return pMA;
		}
	}
	if (pList != &MaterialAdjust::materialAdjustsList) {
		pList = &MaterialAdjust::materialAdjustsList;
		for (int maN = pList->size() - 1; maN >= 0; maN--) {
			MaterialAdjust* pMA = pList->at(maN);
			if (strcmp(pMA->materialName32, adjustmentName) == 0)
				return pMA;
		}
	}
	return NULL;
}

