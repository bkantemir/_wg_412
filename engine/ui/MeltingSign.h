#pragma once
#include "UISubj.h"
//#include <string>

class MeltingSign : public UISubj
{
public:
	MeltingSign() {};
	MeltingSign(std::string name0, std::vector<UISubj*>* pSubjs = NULL, std::vector<DrawJob*>* pDJs = NULL)
		: UISubj(name0, pSubjs, pDJs) {
		strcpy_s(className, 32, "MeltingSign");
	};
	static int addMeltingSign(std::string srcFile, float x, float y);
	virtual int render() { return renderMeltingSign(this); };
	static int renderMeltingSign(MeltingSign* pUI);
	virtual bool isDraggable() { return false; };
	virtual bool isResponsive() { return false; };
};

