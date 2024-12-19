#pragma once
#include "SceneSubj.h"
#include "RailCoord.h"

class Coupler : public SceneSubj
{
public:
	RailCoord railCoord;
	RailCoord railCoordOld;
	Coupler* pCounterCoupler = NULL;
	int connected = -1; //-1-not connected, 0-coupler ahead, 1-connected
	int couplersInvolved = 0;
	float distance = 1000000;//dist to CounterCoupler
	float distanceBefore = 1000000;//dist to CounterCoupler in prev frame
	int hook = 0; //1-has hook

	UISubj* pUImoveButton = NULL;
	UISubj* pUIuncoupleButton = NULL;

	static float couplingDistances[3];
	static float couplersPairLength;


public:
	Coupler() {};
	Coupler(Coupler* pSS0) { memcpy((void*)this, (void*)pSS0, sizeof(Coupler)); };
	virtual Coupler* clone() {
		if (strcmp(this->className, "Coupler") != 0)
			return NULL;
		return new Coupler(this);
	};
	virtual int render(Camera* pCam, float* dirToMainLight, float* dirToTranslucent, int renderFilter, bool forDepthMap) {
		return renderCoupler(this, pCam, dirToMainLight, dirToTranslucent, renderFilter, forDepthMap);
	};
	static int renderCoupler(Coupler* pCp, Camera* pCam, float* dirToMainLight, float* dirToTranslucent, int renderFilter, bool forDepthMap);

	virtual int moveSubj() { checkCoupler(this); return 1; };
	static int checkCoupler(Coupler* pGS);
	static int checkCouplerButtons(Coupler* pGS);
	virtual int deleteMe(bool withChilds = true);
	virtual int onLoad(std::string tagStr) { return onLoadCoupler(this, tagStr); };
	static int onLoadCoupler(Coupler* pCp, std::string tagStr);
	static int couple(Coupler* pCp, Coupler* pCp2, bool withSound);
	static int drawCouplerButtons(std::vector<SceneSubj*>* pSceneSubjs);

};

