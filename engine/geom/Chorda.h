#pragma once
#include "linmath.h"
#include "Gabarites.h"

class Chorda
{
public:
	int chordType = -1;//-1-don't need, 0-round, 2-square
	LineXY axis[3];
	float sizeVector[3][4];
	float sizeDirXY[3][4];
	float sizeXY[3];
	float chordR;
	LineXY chord;
	int chordaQuality = 1; //1-good, 0-so0so, -1-bad

public:
	static int buildChorda(Chorda* pCh, Gabarites* pGB00, mat4x4 mMVP, float* targetRads, float nearClip, float farClip);

};

