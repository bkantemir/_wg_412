#pragma once
#include "LineXY.h"

class ArcXY //circular arc
{
public:
	float centerPos[2] = { 0,0 };
	float p0[2] = { 0,0 };
	float p1[2] = { 0,0 };
	float a0 = 0; //start angle
	float a1 = 0; //end angle
	int arcDir = 1; //1-CCW, -1-CW
	float angleDgCCWstart;
	float angleDgCCWrange;
	float radius = 0;
	bool fullCircle = false;
	float closestPoint[2] = { 0,0 };
public:
	static void initArcXY(ArcXY* pA, float centerX, float centerY, float rad, float a0, float a1, int circleDir);

	float dist_arc2point(float* p2d) { return dist_arc2point(this, p2d); };
	static float dist_arc2point(ArcXY* pA, float* p2d);
	static int circleCrossLinePick1(float* vOut2d, ArcXY* pA, LineXY* pL, int* solutionPreference);
	static int circleCrossLine(float (*points)[2], ArcXY* pA, LineXY* pL);
	static int arcCrossLine(float(*points)[2], ArcXY* pA, LineXY* pL, bool printDegug);
	static bool pointWithinArc(ArcXY* pA, float* p2d);
	static bool angleWithinArc(ArcXY* pA, float angleDg,bool printDrbug);
	static float arcPercent(ArcXY* pA, float* p2d);
	static int arcCrossLineSegment(float(*points)[2], ArcXY* pA, LineXY* pL);
	static int circleCrossCircle(float(*points)[2], ArcXY* pA, ArcXY* pA2);
	static int arcCrossArc(float(*points)[2], ArcXY* pA, ArcXY* pA2);
};

