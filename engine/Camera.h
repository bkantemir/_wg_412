#pragma once
#include "Coords.h"
#include "geom/Gabarites.h"
#include "linmath.h"
#include "geom/Line3D.h"

class Camera
{
public:
	Coords ownCoords;
	mat4x4 lookAtMatrix; //view?
	mat4x4 mViewProjection;
	mat4x4 mVPinverted;
	float lookAtPoint[4] = { 0,0,0, 0 };
	float focusDistance = 100;
	float viewRangeDg = 30;
	//float stageSize[2] = { 500, 375 };
	float viewRx;
	float viewRxGround;
	float dist2ground;
	float lookAtGround[4] = { 0,0,0, 0 };
	float nearClip = 0;
	float farClip = 100;
	float targetDims[2] = { 10,10 };
	float targetRads[2] = { 5,5 };
	float targetAspectRatio = 1;
	Gabarites visibleBox;
	std::vector<Line3D*> visibleRays;

	static float pitchDefault;
public:
	virtual ~Camera();
	static void clearRays(Camera* pCam);
	static void copyParams(Camera* pCamTo, Camera* pCamFrom);
	static float getDistance4stage(Camera* pCam, float ssX, float ssY=0);
	static void setCameraPosition(Camera* pCam);
	static void buildLookAtMatrix(Camera* pCam);
	static void onTargetResize(Camera* pCam, int width, int height, Gabarites* pWorldBox);
	static void buildViewProjectionNoClips(Camera* pCam, Gabarites* pViewBox);
	static void buildViewProjectionWithClips(Camera* pCam, Gabarites* pViewBox);
	static void reset(Camera* pCam, Gabarites* pWorldBox);
	static void setCollisionCamera(Camera* pCam);// , Gabarites* viewBox);
	static void setNearAndFarClips(Camera* pCam, Gabarites* pWorldBox);
	static float setRs4distance(Camera* pCam);
	static float getRground4distance(Camera* pCam);
	static float refocus2ground(Camera* pCam);
	static bool outOfLimits(Camera* pCam, Gabarites* pWorldBox);
};
