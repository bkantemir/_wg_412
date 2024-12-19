#include "Camera.h"
#include "TheApp.h"
#include "Shadows.h"
#include "utils.h"
#include "rr/RollingStock.h"


extern TheApp theApp;
extern float degrees2radians;

float Camera::pitchDefault = 30;// 20;

Camera::~Camera(){ 
	//clearRays(this); 
}
void Camera::clearRays(Camera* pCam) {
	for (int i = pCam->visibleRays.size() - 1; i >= 0; i--) {
		Line3D* pL = pCam->visibleRays.at(i);
		if (pL != NULL)
			delete pL;
	}
	pCam->visibleRays.clear();
}

void Camera::setCameraPosition(Camera* pCam) {
	v3set(pCam->ownCoords.pos, 0, 0, -pCam->focusDistance);
	mat4x4_mul_vec4plus(pCam->ownCoords.pos, pCam->ownCoords.rotationMatrix, pCam->ownCoords.pos, 1);
	for (int i = 0; i < 3; i++)
		pCam->ownCoords.pos[i] += pCam->lookAtPoint[i];
}
void Camera::buildLookAtMatrix(Camera* pCam) {
	float cameraUp[4] = { 0,1,0,0 }; //y - up
	mat4x4_mul_vec4plus(cameraUp, pCam->ownCoords.rotationMatrix, cameraUp, 0);
	mat4x4_look_at(pCam->lookAtMatrix, pCam->ownCoords.pos, pCam->lookAtPoint, cameraUp);
}
void Camera::onTargetResize(Camera* pCam, int width, int height, Gabarites* pWorldBox) {
	pCam->targetDims[0] = (float)width;
	pCam->targetDims[1] = (float)height;
	pCam->targetRads[0] = pCam->targetDims[0] / 2;
	pCam->targetRads[1] = pCam->targetDims[1] / 2;
	pCam->targetAspectRatio = (float)width / (float)height;
	//refocus with new targetAspectRatio
	pCam->focusDistance= getDistance4stage(pCam,pCam->viewRx*2);
	pCam->reset(pCam, pWorldBox);
}
void Camera::reset(Camera* pCam, Gabarites* pWorldBox) {
	if (pCam->focusDistance == 0)
		return;
	if (pCam->targetDims[0] < 20)
		return;

	if (pCam == &theApp.mainCamera)
		outOfLimits(pCam, pWorldBox);

	setCameraPosition(pCam);
	buildLookAtMatrix(pCam);
	setNearAndFarClips(pCam, pWorldBox);
	buildViewProjectionWithClips(pCam, pWorldBox);
	
	if (pCam == &theApp.mainCamera) {
		Shadows::resetShadowsFor(pCam, pWorldBox);
		RollingStock::resetMaxSpeed();
	}
	mat4x4_invert(pCam->mVPinverted, pCam->mViewProjection);
}
void Camera::buildViewProjectionNoClips(Camera* pCam,Gabarites* pViewBox) {
	setNearAndFarClips(pCam, pViewBox);
	buildViewProjectionWithClips(pCam, pViewBox);
}
void Camera::buildViewProjectionWithClips(Camera* pCam, Gabarites* pViewBox) {
	mat4x4 mProjection;
	if (pCam->viewRangeDg == 0) {
		float w = setRs4distance(pCam);// pViewBox->bbRadpCam->stageSize[0] / 2 + 10.0f;
		float h = w / pCam->targetAspectRatio;
		mat4x4_ortho(mProjection, -w, w, -h, h, pCam->nearClip, pCam->farClip);
		mat4x4_mul(pCam->mViewProjection, mProjection, pCam->lookAtMatrix);
	}
	else {
		mat4x4_perspective(mProjection, pCam->viewRangeDg * degrees2radians, pCam->targetAspectRatio, pCam->nearClip, pCam->farClip);
		mat4x4_mul(pCam->mViewProjection, mProjection, pCam->lookAtMatrix);
		pCam->mViewProjection[1][3] = 0; //keystone effect
	}
}
void Camera::setCollisionCamera(Camera* pCam){//}, Gabarites* pViewBox) {
	pCam->ownCoords.setEulerDg(90, 180, 0); //set camera angles/orientation
	pCam->viewRangeDg = 0;

	v3setAll(pCam->lookAtPoint, 0);
	pCam->focusDistance = 100;// pViewBox->bbRad[1] * 2;
	setCameraPosition(pCam);
	buildLookAtMatrix(pCam);
	m16copy(pCam->mViewProjection, pCam->lookAtMatrix);
	mat4x4_invert(pCam->mVPinverted, pCam->mViewProjection);
}


void Camera::setNearAndFarClips(Camera* pCam, Gabarites* pViewBox) {
	//set temporary mProjection matrix with unreasonable near and far clips
	float worldRadTmp = pViewBox->boxRad * 2;
	pCam->nearClip = pCam->focusDistance - worldRadTmp;
	if (pCam->nearClip < 1) pCam->nearClip = 1;
	pCam->farClip = pCam->focusDistance + worldRadTmp;
	pCam->buildViewProjectionWithClips(pCam, pViewBox);
	//reset clips
	pCam->nearClip = 1000000;
	pCam->farClip = -1000000;
	v3setAll(pCam->visibleBox.bbMin, 1000000);
	v3setAll(pCam->visibleBox.bbMax, -1000000);
	pCam->clearRays(pCam);

	mat4x4 mInvertedCamViewProjection;
	mat4x4_invert(mInvertedCamViewProjection, pCam->mViewProjection);
	float vIn[4] = { 0,0,0,1 };
	for (float y = -1.0; y <= 1.0; y += 0.1) {
		for (float x = -1.0; x <= 1.0; x += 0.1) {
			//set nearClip
			float p0[4];
			
			v3set(vIn, x, y, -1);
			if (mat4x4_mul_vec4plus(p0, mInvertedCamViewProjection, vIn, 1, true) < 1)
				continue;
				
			//set farClip
			float p1[4];
			v3set(vIn, x, y, 1);
			if (mat4x4_mul_vec4plus(p1, mInvertedCamViewProjection, vIn, 1, true) < 1)
				continue;
			Line3D raySegment;
			Line3D::initLine3D(&raySegment, p0, p1);
			if (Line3D::clipLineByBox(&raySegment, pViewBox,true) < 1)
				continue; //line is out of box
			pCam->visibleRays.push_back(new Line3D(raySegment));
			Gabarites::adjustMinMaxByPoint(&pCam->visibleBox, raySegment.p0);
			Gabarites::adjustMinMaxByPoint(&pCam->visibleBox, raySegment.p1);
			//update near and far clips
			float dist = v3lengthFromTo(pCam->ownCoords.pos, raySegment.p0);
			if (pCam->nearClip > dist)
				pCam->nearClip = dist;
			if (pCam->farClip < dist)
				pCam->farClip = dist;
			dist = v3lengthFromTo(pCam->ownCoords.pos, raySegment.p1);
			if (pCam->nearClip > dist)
				pCam->nearClip = dist;
			if (pCam->farClip < dist)
				pCam->farClip = dist;
		}
	}
	
	//adjust visibleBox
	
	float d = 40;
	for (int i = 0; i < 3; i ++){//} = 2) {
		pCam->visibleBox.bbMin[i] -= d;
		pCam->visibleBox.bbMax[i] += d;
	}
	//pCam->visibleBox.bbMax[1] += d;
	
	Gabarites::adjustMidRad(&pCam->visibleBox);
	//setup/adjust clips
	float range = pCam->farClip - pCam->nearClip;
	float adjust = range * 0.07;
	pCam->farClip += adjust;
	pCam->nearClip-=adjust;
	if (pCam->nearClip < 0) 
		pCam->nearClip = 0;
}

void Camera::copyParams(Camera* pCamTo, Camera* pCamFrom) {
	memcpy(&pCamTo->ownCoords, &pCamFrom->ownCoords,sizeof(Coords));
	v3copy(pCamTo->lookAtPoint, pCamFrom->lookAtPoint);
	pCamTo->focusDistance = pCamFrom->focusDistance;
	pCamTo->viewRangeDg = pCamFrom->viewRangeDg;
	pCamTo->viewRx = pCamFrom->viewRx;
	v3copy(pCamTo->lookAtGround, pCamFrom->lookAtGround);
	pCamTo->viewRxGround = pCamFrom->viewRxGround;
	v2copy(pCamTo->targetDims, pCamFrom->targetDims);
	v2copy(pCamTo->targetRads, pCamFrom->targetRads);
	pCamTo->targetAspectRatio = pCamFrom->targetAspectRatio;
}

float Camera::getDistance4stage(Camera* pCam,float ssX,float ssY) {
	float viewRangeDg = pCam->viewRangeDg;
	if (viewRangeDg == 0)
		viewRangeDg = 10;
	float cotangentA = 1.0f / tanf(degrees2radians * viewRangeDg/2);
	float cameraDistanceV = ssY / 2 * cotangentA;
	float cameraDistanceH = ssX / 2 * cotangentA / pCam->targetAspectRatio;
	float dMax=(float)fmax(cameraDistanceV, cameraDistanceH);
	return dMax;
}
float Camera::setRs4distance(Camera* pCam) {
	float cotangentA = 1.0f / tanf(degrees2radians * pCam->viewRangeDg/2);
	pCam->viewRx = pCam->focusDistance / cotangentA *pCam->targetAspectRatio;
	//check viewRground
	float groundLevel = theApp.gameTable.groundLevel0;
	if (pCam->lookAtPoint[1] == groundLevel) {
		pCam->viewRxGround = pCam->viewRx;
		pCam->dist2ground = pCam->focusDistance;
	}
	else {//not on groundLevel
		//find cam position (a real one can be uninitiated yet)
		float camPos[4];
		v3set(camPos, 0, 0, -pCam->focusDistance);
		mat4x4_mul_vec4plus(camPos, pCam->ownCoords.rotationMatrix, camPos, 1);
		for (int i = 0; i < 3; i++)
			camPos[i] += pCam->lookAtPoint[i];
		//find view ray
		Line3D viewRay;
		Line3D::initLine3D(&viewRay, camPos, pCam->lookAtPoint);
		//find ground point
		Line3D::crossPlane(pCam->lookAtGround, &viewRay, 1, groundLevel);
		pCam->dist2ground = v3lengthFromTo(camPos, pCam->lookAtGround);
		//find viewRxGround
		pCam->viewRxGround = pCam->dist2ground / cotangentA *pCam->targetAspectRatio;

	}
	return pCam->viewRx;
}
float Camera::getRground4distance(Camera* pCam) {
	setRs4distance(pCam);
	return pCam->viewRxGround;
}
float Camera::refocus2ground(Camera* pCam) {
	setRs4distance(pCam);
	if (pCam->lookAtPoint[1] == pCam->lookAtGround[1])
		return 0;
	v3copy(pCam->lookAtPoint, pCam->lookAtGround);
	pCam->focusDistance = pCam->dist2ground;
	return pCam->focusDistance;
}


bool Camera::outOfLimits(Camera* pCam, Gabarites* pWorldBox){
	//set zoom limits
	float diamFar = fmax(pWorldBox->bbRad[0], pWorldBox->bbRad[2]) * 2;
	float maxDistFromGround = getDistance4stage(pCam, diamFar, diamFar);
	float minDist = 200;

	bool wasAdjusted = false;

	//check distance limits
	if (pCam->focusDistance < minDist) {
		pCam->focusDistance = minDist;
		wasAdjusted = true;
	}
	else {
		setRs4distance(pCam);
		if (pCam->dist2ground > maxDistFromGround) {
			float d = pCam->dist2ground - maxDistFromGround;
			pCam->focusDistance -= d;
			wasAdjusted = true;
		}
	}
	//lookAtPoint limits
	float r = pCam->getRground4distance(pCam) * 0.2;
	for (int i = 0; i < 3; i += 2) {
		if (pWorldBox->bbRad[i] <= r) {
			pCam->lookAtPoint[i] = 0;
			wasAdjusted = true;
		}
		else {
			if (pCam->lookAtPoint[i] < pWorldBox->bbMin[i] + r) {
				pCam->lookAtPoint[i] = pWorldBox->bbMin[i] + r;
				wasAdjusted = true;
			}
			if (pCam->lookAtPoint[i] > pWorldBox->bbMax[i] - r) {
				pCam->lookAtPoint[i] = pWorldBox->bbMax[i] - r;
				wasAdjusted = true;
			}
		}
	}
	return wasAdjusted;
}
