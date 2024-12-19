#pragma once
#include "SceneSubj.h"
#include "Smoke.h"

class Exhaust : public SceneSubj
{
public:
	int lastExhaustN = -1;
	uint32_t lastExhaustFrameN = 0;

	//int uTex3N = -1;
	float pressure = 0.5;
	float dencity = 1;

	float phase = 0;//0-idle, 1-running
	int countdown = 0;

	static int modelNcloud;
	static int uTex0Nwn;
	static int uTex3N4black;

public:
	Exhaust() {};
	Exhaust(Exhaust* pSS0) { memcpy((void*)this, (void*)pSS0, sizeof(Exhaust)); };
	virtual Exhaust* clone() {
		if (strcmp(this->className, "Exhaust") != 0)
			return NULL;
		return new Exhaust(this);
	};
	static int init();
	//virtual int moveSubj() { return moveExhaust(this); };
	//static int moveExhaust(Exhaust* pGS);
	static int startSingleCloud(Smoke* pModel, float errorLevel);
	static int startCloud(Exhaust* pEx, float linSpeed, float startSize, int renderType, unsigned int refN, float pressure = 1, float dencity = 1, float tex3framePixelWidth = 1);
	static int startCloudRing(Exhaust* pEx, float linSpeed, float startSize, int renderType, unsigned int refN, float pressure = 1, float dencity = 1, float tex3framePixelWidth = 1);
	static Smoke* prepareCloudModel(Exhaust* pEx, float linSpeed, float startSize, int renderType, unsigned int refN, float pressure = 1, float dencity = 1, float tex3framePixelWidth = 1);
};
