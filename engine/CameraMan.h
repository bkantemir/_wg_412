#pragma once
#include "Camera.h"
#include "SceneSubj.h"

class CameraMan
{
public:
	int task=-1; //-1-no task, 0-change position, 1-focus and follow subj, 2-just follow subj
	int zoom = 0;//0-medium, 1-close up, -1-far
	Camera startCamera;
	Camera endCamera;
	int subj2followN=-1;
	std::vector<SceneSubj*>* pSubjs;
	unsigned int birthFrameN; //subj2follow
	float progress=0;
	float progressSpeed=0;
	float progressAcceleration = 0.01;

	static float closeViewMargin;
public:
	int process();
	int zoomInOut(SceneSubj* pS);
	int refocus(float* pos);
	static int setView(bool mapView);
	int setZoomTo(SceneSubj* pS);

};


