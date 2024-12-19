#pragma once
#include <vector>
#include "geom/LineXY.h"
#include "utils.h"

class Gabarites
{
public:
	//bounding box
	float bbMin[3] = { 1000000,1000000,1000000 };
	float bbMax[3] = { -1000000,-1000000,-1000000 };

	float bbMid[4];
	float bbRad[3];

	float boxRad = 0;
	int isInViewRange = 0; //1-completely on-screen, 0 - partially, -1 - off-screen

public:
	void clear() { clear(this); };
	static void clear(Gabarites* pGB) { v3setAll(pGB->bbMin, 1000000); v3setAll(pGB->bbMax, -1000000); };
	static void adjustMidRad(Gabarites* pGB);
	static void adjustMinMaxByPoint(Gabarites* pGB, float* newPoint);
	static void adjustMinMaxByBBox(Gabarites* pGBdst, Gabarites* pGBsrc);

	static void fillBBox(Gabarites* pGB, Gabarites* pGabarites00, mat4x4 mMVP, float* targetRads, float nearClip, float farClip);
	//static int fillGabarites(Gabarites* pGB, mat4x4 absModelMatrix, Gabarites* pGB00, mat4x4 mViewProjection, float* targetRads, float nearClip, float farClip);
	//static float checkCollision(float* collisionPoint, Gabarites* pGB1, Gabarites* pGB2);
	static void toLog(std::string title,Gabarites* pGB);
	static bool boxesIntersect(Gabarites* pBox1, Gabarites* pBox2);
	static bool pointInBox(float* p, Gabarites* pBox,float margin=0);
};

