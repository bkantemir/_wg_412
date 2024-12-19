#pragma once
#include "UISubj.h"

class UIhint : public UISubj
{
public:
	static int hintSubjN;
public:
	UIhint();
	UIhint(std::string name0, std::vector<UISubj*>* pSubjs = NULL, std::vector<DrawJob*>* pDJs = NULL)
		: UISubj(name0, pSubjs, pDJs) {
		strcpy_s(className, 32, "UIhint");
	};
	static int init();
	static int clear();
	virtual int render() { return renderHint(this); };
	static int renderHint(UIhint* pUI);

};
