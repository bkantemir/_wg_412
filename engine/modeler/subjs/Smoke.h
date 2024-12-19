#pragma once
#include "SceneSubj.h"


class Wind //vortex
{
public:
	float pos[3];
	float posSpeed[3];
	float rad;
	float windSpeed = 0;

	float legSquare;

	static float maxVortexSpeed;
	static float maxWndSpeed;
};

class Smoke : public SceneSubj
{
public:
	int renderType = 0;
	float age = 0;
	float lifeSpan = 1;
	float startSpeedPos[4];
	float ownSpeedValue = 1;
	float speedValueDrag = 0.5;
	float growthSpeed = 1;
	float growthSpeedDrag = 0.5;
	float rad = 1;
	float tex0speed[2];
	float dencity = 1;
	float tex3widthK = 1;

	float transitAlpha = 1;
	float transitAlphaSpeed=0;
	int generation = 0;
	
	static uint32_t lastSplitframeN;
	static float splitSize;
	static int transitAlphaFramesN;// = 10;

	static int modelNcloud;
	static uint32_t windsUpdateframeN;
	static std::vector<Wind*> winds;

public:
	Smoke() { bilboard = true; };
	Smoke(Smoke* pSS0) { 
		memcpy((void*)this, (void*)pSS0, sizeof(Smoke)); 
		int a = 0;
	};
	virtual Smoke* clone() {
		if (strcmp(this->className, "Smoke") != 0)
			return NULL;
		return new Smoke(this);
	};
	static int init();
	static int cleanUp();
	virtual int moveSubj() { return moveSmoke(this); };
	static int moveSmoke(Smoke* pSS);
	static int getWindDir(float* vOut, float* pos);
	static int drawDebug();
	static int splitIfTooBig(Smoke* pSS);
	static int fillWithGas(SceneSubj* pS0);
	static int fillBoxWithGas(Gabarites* pGB, mat4x4 absModelMatrix, float* speed);
};

