#pragma once
#include "SceneSubj.h"
#include "geom/LineXY.h"
#include "geom/ArcXY.h"

class RailEnd
{
public:
	float toTileIndexXZ[2];
	float endYaw = 0;
	int toRailN = -1;
	int toRailEndN = -1; //0 or 1 (start or end)
	bool isSwitch = false;
	bool isOn = false;
	int altRailN = -1;
	int altRailEndN = -1;
	int ownRailN = -1;
	int highlight = 0;
	float tremble = 0;
	bool isBusy = false;

public:
	static void toLog(std::string title, RailEnd* pRE);
};

class Rail : public SceneSubj
{
public:
	int refModelN = -1;
	float tileIndexXZ[2];
	float yawInOut[2]; //orientation degrees: 0-down, CCW
	float railLength;
	int railType = 0;//0-straight, 1-curve
	int railStatus = 0; //0-normal, 1-virtual, -1-snowFlake
	LineXY line2d;
	ArcXY arc2d;
	RailEnd railEnd[2];
	float p0[4] = { 0,0,0,0 }; //pointIn
	float p1[4] = { 0,0,0,0 }; //pointOut
	int crossingRailN = -1;
	float crossingPercent = 0.5;
	float crossingPoint[4] = { 0,0,0,0 };
	float crossingYaw;
	//curve-specific
	float curveCenter[4] = { 0,0,0,0 };
	int CCW = 1;//-1-CW
	float radiusAngle[2]; //orientation degrees: 0-down, CCW

	int sizeMode = 0; //-1-small, -2-tiny

	int switchEndN = -1;
	int highlight = 0;
	bool switchSelected = false;
	bool switchWaits4click = false;

	bool isBusy = false;

	int priority = 0;

	//models sizes
	static float railroadGauge;
	static float railWidth;
	static float railHeight;
	static float railSleeperLength;
	static float railSleeperWidth;
	static float railSleeperHeight;
	static float fastenerWidth;
	static float railSleepersInterval;
	static float couplerY;
	//map-related
	static float railsLevel;
	static float curveRadius;
	static float tileCenter2curveCenter;
	static float railLenghtLong;	//minimum 27
	static float railLenghtShort;
	static float railLenghtCurved;
	
	//guide
	static float railGuideLineWidth;
	static float railGuideGaugeWidth;
	static float railGuideAngle;
	static float railGuideLength;
	static float railGuideShortAngle;
	static float railGuideShortLength;
	static float railGuideBedDy;

	//wheel related
	static float wheelWidth;
	static float rimBorderLineWidth;

	//models
	static int railModelSnowFlakeN;
	static int railModelSnowFlakeSmallN;

	static int railModelStraightVirtualN;
	static int railModelShortVirtualN;
	static int railModelCurvedVirtualN;

	static int railModelStraightN;
	static int railModelShortN;
	static int railModelCurvedN;

	static int railModelStraightNsmall;
	static int railModelShortNsmall;
	static int railModelCurvedNsmall;

	static int railModelStraightNtiny;
	static int railModelShortNtiny;
	static int railModelCurvedNtiny;

	static int railModelGuideStraightN;
	static int railModelGuideCurvedN;
	static int railModelGuideBedStraightN;
	static int railModelGuideBedCurvedN;
	static int railModelGuideShortStraightN;
	static int railModelGuideShortCurvedN;


	static int couplerModelN;
	static int couplersCoupleModelN;
	static int couplerShaftModelN;

	static int gangwaysCoupleModelN;

	static std::vector<SceneSubj*> railModels;

public:
	Rail() {};
	Rail(Rail* pR0) { memcpy(this, pR0, sizeof(Rail)); };
	static int cleanUp();

	virtual int render(Camera* pCam, float* dirToMainLight, float* dirToTranslucent, int renderFilter, bool forDepthMap) {
		return renderRail(this, pCam, dirToMainLight, dirToTranslucent, renderFilter, forDepthMap);
	};
	static int renderRail(Rail* pGS, Camera* pCam, float* dirToMainLight, float* dirToTranslucent, int renderFilter, bool forDepthMap);
	static int renderRailEnd(Rail* pR, int endN, Camera* pCam, float* dirToMainLight);
	static int sizeModeModelN(Rail* pR, int sizeMode);
	static int adjustModel4size(Rail* pSS, float sizeUnitPixelsSize);
	float dist2point3d(float* pos) { return dist2point3d(this,pos); };
	static float dist2point3d(Rail* pR,float* pos);
	static void toLog(std::string title, Rail* pR);
	static std::string ang2dir(float ang);
	static Rail* pointerOnRail();// std::vector<SceneSubj*>* pRailsMap);
	
	virtual bool isClickable();
	virtual int onFocus() { return 0; };
	virtual int onClick() { return onLeftButtonUp();};
	virtual int onLeftButtonUp();
	static int convertRail(Rail* pR);
	static int releaseConnectedRails(Rail* pR, int endN);
	static int switchRail (Rail * pR);
};
