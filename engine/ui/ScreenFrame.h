#pragma once
#include "UISubj.h"

class ScreenFrame : UISubj
{
public:
	static float screenFrameMargin;

public:
	ScreenFrame();
	ScreenFrame(std::string name0, std::vector<UISubj*>* pSubjs = NULL, std::vector<DrawJob*>* pDJs = NULL)
		: UISubj(name0, pSubjs, pDJs) {
		strcpy_s(className, 32, "ScreenFrame");
		djStartN = djNframe;
		memcpy(&mt0, &pDrawJobs->at(djStartN)->mt, sizeof(Material));
		mt0.uColor.setRGBA(1.0f, 1.0f, 1.0f, 0.3f );
		mt0.uAlphaBlending = 1;
		mt0.lineWidth = 1;
	};
	virtual int render() { return renderScreenFrame(this); };
	static int renderScreenFrame(ScreenFrame* pUI);
};

