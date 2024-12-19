#include "Smoke.h"
#include "TheApp.h"
#include <algorithm>

extern TheApp theApp;
extern float degrees2radians;

uint32_t Smoke::windsUpdateframeN;
std::vector<Wind*> Smoke::winds;

float Wind::maxVortexSpeed=1.5;
float Wind::maxWndSpeed=1.5;

int Smoke::modelNcloud = -1;

float Smoke::splitSize = 200;
int Smoke::transitAlphaFramesN = 30;
uint32_t Smoke::lastSplitframeN=0;

int Smoke::init() {
	modelNcloud = ModelLoader::loadModelOnce0(&theApp.models, NULL, NULL, "/dt/md/smoke01.txt", "", "Smoke");

	//init winds in all corners
	float* rads = theApp.gameTable.worldBox.bbRad;
	for (int wN =0; wN < 4; wN++) {
		Wind* pW = new Wind();
		winds.push_back(pW);
		for (int i = 0; i < 3; i += 2) {
			float r = rads[i];
			pW->pos[i] = getRandom(-r, r);
			r = pW->maxVortexSpeed;
			pW->posSpeed[i] = getRandom(-r, r);
		}
		pW->rad = getRandom(10.0f, rads[0]);
		pW->windSpeed = pW->maxWndSpeed;
		if (getRandom(0, 2) == 0)
			pW->windSpeed *= -1;
	}
	return 1;
}
int Smoke::cleanUp() {
	for (int i = winds.size() - 1; i >= 0; i--) {
		Wind* pW = winds.at(i);
		delete pW;
	}
	winds.clear();
	return 1;
}
int Smoke::getWindDir(float* vOut, float* pos) {
	float yawSpeedMax = 2;
	float yawAcceleration = 2;
	float* rads = theApp.gameTable.worldBox.bbRad;
	float worldRad = rads[0];
	int windsN = winds.size();
	if (windsUpdateframeN != theApp.frameN) {
		windsUpdateframeN = theApp.frameN;
		for (int wN = 0; wN < windsN; wN++) {
			Wind* pW = winds.at(wN);
			pW->rad += getRandom(-1.0f, 1.0f);
			pW->rad = minimax(pW->rad, worldRad * 0.2, worldRad * 0.6);
			//pW->windSpeed += getRandom(-1.0f, 1.0f)*0.1;
			//pW->windSpeed = minimax(pW->windSpeed, -pW->maxWndSpeed, pW->maxWndSpeed);
			for (int i = 0; i < 3; i += 2) {
				pW->posSpeed[i] += getRandom(-1.0f, 1.0f);
				v3max(pW->posSpeed, pW->maxVortexSpeed);
				pW->pos[i] += pW->posSpeed[i];
				float r = rads[i];
				pW->pos[i] = minimax(pW->pos[i], -r, r);
			}
		}
	}

	float hypotenuseSquare = 0;
	for (int wN = 0; wN < windsN; wN++) {
		Wind* pW = winds.at(wN);
		float leg = worldRad - v3lengthFromToXZ(pos, pW->pos);
		leg = fmax(leg, 1);
		pW->legSquare = leg * leg;
		hypotenuseSquare += pW->legSquare;
	}
	float controlWeightsSum = 0;//result must be 1
	v3setAll(vOut,0);
	for (int wN = 0; wN <windsN; wN++) {
		Wind* pW = winds.at(wN);

		float w = pW->legSquare / hypotenuseSquare;
		controlWeightsSum += w;

		float d = v3lengthFromToXZ(pos, pW->pos);
		float k = 1;
		if (d <= pW->rad)
			k = d / pW->rad;
		else {
			d=d - pW->rad;//dist from rad
			float r = worldRad;
			k = 1.0-d / r;
			k = fmax(k * w, 0.2);
		}

		float yaw = v3yawDgFromTo(pW->pos, pos);
		float ang = (yaw+90) * degrees2radians;
		vOut[0] += (sin(ang) * k*pW->windSpeed);
		vOut[2] += (cos(ang) * k*pW->windSpeed);
	}
	v3max(vOut, Wind::maxWndSpeed);
	return 1;
}

int Smoke::drawDebug() {

	std::vector<SceneSubj*>* pSubjs = &theApp.models;
	float y0 = 60;
	SceneSubj* pS = ModelLoader::loadModelOnce(pSubjs, NULL, NULL, "/dt/md/cylinder1.txt", "", "");
	for (int n = winds.size() - 1; n >= 0; n--) {
		Wind* pW = winds.at(n);
		pS->ownCoords.setPitch(-90);
		v3copy(pS->ownCoords.pos, pW->pos);
		pS->ownCoords.pos[1] = y0;
		v3set(pS->scale, pW->rad*2, pW->rad * 2, 1);

		singleDraw3D(pS);
	}
	pS = ModelLoader::loadModelOnce(pSubjs, NULL, NULL, "/dt/md/windvane1.txt", "", "");
	float* rads = theApp.gameTable.worldBox.bbRad;
	float stepSize = 25;
	for (float z = -rads[2]; z <= rads[2]; z += stepSize) {
		for (float x = -rads[0]; x <= rads[0]; x += stepSize) {
			pS->ownCoords.pos[0] = x;
			pS->ownCoords.pos[1] = y0;
			pS->ownCoords.pos[2] = z;

			float windDid[4];
			getWindDir(windDid, pS->ownCoords.pos);
			float yaw = v3yawDg(windDid);
			pS->ownCoords.setYaw(yaw);

			float ln = v3length(windDid);
			v3set(pS->scale, 1, 2, ln*5);

			singleDraw3D(pS);

		}

	}
	return 1;
}


int Smoke::moveSmoke(Smoke* pSS) {
	if (pSS->birthFrameN == theApp.frameN)
		return 1;

	if (pSS->gabaritesOnScreen.isInViewRange < 0) {
		//out of screen

		//debug
		float f = theApp.mainCamera.ownCoords.pos[1];
		float a = theApp.mainCamera.visibleBox.bbMax[1];
		float b = pSS->absCoords.pos[1];
		float c = pSS->gabaritesWorld.bbMin[1];
		bool d = Gabarites::boxesIntersect(&pSS->gabaritesWorldAssembly, &theApp.mainCamera.visibleBox);
		bool e = Gabarites::boxesIntersect(&pSS->gabaritesWorld, &theApp.mainCamera.visibleBox);

		pSS->deleteMe();
		return 0;
	}
	pSS->age += 1.0f;
	if (pSS->age >= pSS->lifeSpan) {
		//too old
		pSS->deleteMe();
		return 0;
	}
	if (pSS->transitAlphaSpeed != 0) {
		pSS->transitAlpha += pSS->transitAlphaSpeed;
		if (pSS->transitAlpha >= 1) {
			pSS->transitAlpha = 1;
			pSS->transitAlphaSpeed = 0;
		}
		if (pSS->transitAlpha <=0) {
			pSS->deleteMe();
			return 0;
		}
	}

	float lifePercent = pSS->age/pSS->lifeSpan;
	//rendering
	Material* pMt0 = &pSS->mt0;
	if (pSS->renderType <2 )
		pMt0->uAlphaFactor = (1.0 - lifePercent)*pSS->dencity * pSS->transitAlpha;
	else {//renderType==2-tex0 translated
		pMt0->uAlphaFactor = pSS->dencity * pSS->transitAlpha;
		float tex3shift = lifePercent * pSS->tex3widthK; //width correction (minus last transparent pixel)// ((pSS->tex3width - 1) / pSS->tex3width);
		//uTex3
		float* pChannel = &pMt0->uTexMods[3][0];
		pChannel[2] = tex3shift; //shift right up to age
	}
	//shift tex0 or tex1mask
	if (pSS->renderType >0) {
		float* pChannel = &pMt0->uTexMods[0][0];
		for (int i = 0; i < 2; i++)
			pChannel[2 + i] += pSS->tex0speed[i];
	}

	//speed
	pSS->ownSpeedValue *= pSS->speedValueDrag;
	float windSpeedValue = 1.0 - pSS->ownSpeedValue;

	//windSpeedValue = 0;

	float windDir[4] = { 0,0,0,0 };
	getWindDir(windDir, pSS->absCoords.pos);
	for (int i = 0; i < 3; i++)
		pSS->ownSpeed.pos[i] = pSS->startSpeedPos[i] * pSS->ownSpeedValue + windDir[i] * windSpeedValue;
	//size
	pSS->growthSpeed *= pSS->growthSpeedDrag;
	float diam = pSS->scale[0] + pSS->growthSpeed;
	v3setAll(pSS->scale, diam);
	pSS->rad = diam / 2;


	if (pSS->transitAlphaSpeed == 0) {
		//find master
		std::vector<SceneSubj*>* pSubjs = pSS->pSubjsSet;
		Smoke* pMaster = NULL;
		float masterAgeDifference = 1000000;
		for (int n = 0; n < pSS->nInSubjsSet; n++) {
			Smoke* pS2 = (Smoke*)pSubjs->at(n);
			if (pS2 == NULL)
				continue;
			if (pS2->transitAlphaSpeed != 0)
				continue;
			float dMax = pSS->rad + pS2->rad;
			bool bMaybe = true;
			for (int i = 0; i < 3; i++) {
				if (abs(pSS->absCoords.pos[i] - pS2->absCoords.pos[i]) > dMax) {
					bMaybe = false;
					break;
				}
			}
			if (!bMaybe)
				continue;
			float d = v3lengthFromTo(pSS->absCoords.pos, pS2->absCoords.pos);
			if (d > dMax)
				continue;
			//have a candidate
			float ageDifference = abs(pSS->age - pS2->age);
			if (masterAgeDifference < ageDifference)
				continue;
			//new choice
			masterAgeDifference = ageDifference;
			pMaster = pS2;
			if (masterAgeDifference < 1)
				break;
		}
		if (pMaster != NULL) {
			//have master
			float dirFromMaster[3];
			for (int i = 0; i < 3; i++)
				dirFromMaster[i] = pSS->ownCoords.pos[i] - pMaster->ownCoords.pos[i];
			float d = v3length(dirFromMaster);
			if (abs(d - pMaster->rad) > 1) {
				//needs correction
				v3setLength(dirFromMaster, pMaster->rad);
				float dstPoint[3];
				for (int i = 0; i < 3; i++)
					dstPoint[i] = pMaster->ownCoords.pos[i] + dirFromMaster[i];
				float slaveDir[3];
				for (int i = 0; i < 3; i++)
					slaveDir[i] = dstPoint[i] - pSS->ownCoords.pos[i];
				if (v3equals(slaveDir, 0))
					slaveDir[1] = 1;
				v3max(slaveDir, 0.25);

				for (int i = 0; i < 3; i++)
					pSS->ownCoords.pos[i] += slaveDir[i];
				//pSS->mt0.uColor.setRGBA(255, 0, 0);
			}
		}
	}
	applySpeedsStandard(pSS);
	
	splitIfTooBig(pSS);

	return 1;
}
int Smoke::splitIfTooBig(Smoke* pSS00) {
	if (lastSplitframeN == theApp.frameN)
		return 0; //only 1 split per frame
	if (pSS00->transitAlphaSpeed != 0)
		return 0;
	if (pSS00->scale[0] < 10)
		return 0;
	float screenSize = getUnitPixelsSize(pSS00, &theApp.mainCamera); //counts scale too
	if (screenSize < splitSize)
		return 0;
	if (pSS00->age > pSS00->lifeSpan * 0.7)
		return 0;
	int realAge = (int)(theApp.frameN - pSS00->birthFrameN);
	if (realAge < 3)
		return 0;
	
	lastSplitframeN = theApp.frameN;
		
	Smoke splitModel;
	memcpy(&splitModel, pSS00, sizeof(Smoke));
	v3setAll(splitModel.scale, pSS00->scale[0] * 0.6);
	splitModel.transitAlpha = 0;
	splitModel.transitAlphaSpeed = 1.0 / transitAlphaFramesN;
	splitModel.mt0.uAlphaFactor = 0;
	splitModel.growthSpeed = pSS00->growthSpeed * 0.5;
	splitModel.generation++;

	//random spin matrix
	float eulerDg[3] = { 0,0,0 };
	for (int i = 0; i < 3; i++)
		eulerDg[i] = getRandom(0, 180);
	quat q = { 0,0,0,1 };
	Coords::eulerDg2quaternion(q, eulerDg);
	mat4x4 randomSpinMatrix = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };
	mat4x4_from_quat(randomSpinMatrix, q);

	float pointShift = pSS00->scale[0] * 0.3;
	float outSpeed = splitModel.growthSpeed/pSS00->ownSpeedValue;

	float vIn[4] = { 0,0,1,0 };
	float vOut[4];
	for (float pitch = -30; pitch < 180; pitch += 120)
		for (float yaw = 0; yaw < 360; yaw += 120) {
			v3set(eulerDg,pitch, yaw,0);
			Coords::eulerDg2quaternion(q, eulerDg);
			mat4x4 rotationMatrix = { 1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1 };
			mat4x4_from_quat(rotationMatrix, q);
			mat4x4_mul_vec4plus(vOut, rotationMatrix, vIn, 0);
			mat4x4_mul_vec4plus(vOut, randomSpinMatrix, vOut, 0);
			v3setLength(vOut, pointShift);
			for (int i = 0; i < 3; i++)
				splitModel.ownCoords.pos[i] += vOut[i];
			v3setLength(vOut, outSpeed);
			for (int i = 0; i < 3; i++)
				splitModel.startSpeedPos[i] += vOut[i];
			int subjN = Exhaust::startSingleCloud(&splitModel, 0.15);
			/*
			//debug
			Smoke* pS = (Smoke*)theApp.clouds.at(subjN);
			pS->mt0.uColor.setRGBA(0,255,0);
			*/
			if (pitch == 90)
				break;
		}
	//dissolve original

	pSS00->transitAlpha = 1;
	pSS00->transitAlphaSpeed = -1.0 / transitAlphaFramesN;
	
	//pSS00->mt0.uColor.setRGBA(255,0,0);
	/*
	//debug
	float dist2cam = v3lengthFromTo(pSS00->absCoords.pos, theApp.mainCamera.ownCoords.pos); //counts scale too
	mylog("%d: %d clouds. pS0 gen=%d age=%d of %d realAge=%d size=%d screenSize=%d dist2cam=%d growthSpeed=%f\n", 
		(int)theApp.frameN, theApp.clouds.size(), pSS00->generation,
		(int)pSS00->age,(int)pSS00->lifeSpan, (int)(theApp.frameN-pSS00->birthFrameN), (int)pSS00->scale[0],(int)screenSize,
		(int)dist2cam, pSS00->growthSpeed);
	*/
	return 1;
}
int Smoke::fillWithGas(SceneSubj* pS0) {
	std::vector<SceneSubj*>* pSubjs = pS0->pSubjsSet;
	std::vector<int>* pSubjNs = entireFamily(pS0);
	for (int sN = pSubjNs->size() - 1; sN >= 0; sN--) {
		int subjN = pSubjNs->at(sN);
		SceneSubj* pS = pSubjs->at(subjN);
		if (pS->djTotalN == 0)
			continue;
		Gabarites gb;
		memcpy(&gb, &pS->gabaritesOnLoad,sizeof(Gabarites));
		if (v3equals(pS->scale, 1) == false) {
			for (int i = 0; i < 3; i++) {
				gb.bbMin[i] * pS->scale[i];
				gb.bbMax[i] * pS->scale[i];
			}
			gb.adjustMidRad(&gb);
		}
		float speed[3];
		for (int i = 0; i < 3; i++)
			speed[i] = pS->absCoords.pos[i] - pS->absCoordsPrev.pos[i];

		fillBoxWithGas(&gb, pS->absModelMatrixUnscaled, speed);

	}
	pSubjNs->clear();

	return 1;
}


int Smoke::fillBoxWithGas(Gabarites* pGB, mat4x4 absModelMatrix, float* speed) {

	float rads[3];
	v3copy(rads, pGB->bbRad);

	sort(rads, rads + sizeof(rads), std::greater<float>());
	float diam = rads[1] + rads[2];
	if (diam < 5)
		return 0;//too small
	diam = fmin(diam, 20);
	float rad = diam / 2;
	int dimPoints[3];
	float dimStart[3];
	for (int i = 0; i < 3; i++) {
		dimPoints[i] = (int)round(pGB->bbRad[i]*2 / rad-1);
		if (dimPoints[i] < 1)
			dimPoints[i] = 1;
		dimStart[i] = pGB->bbMid[i] - rad * 0.5*(dimPoints[i]-1);
	}

	Smoke* pS0 = Exhaust::prepareCloudModel(NULL, 0, diam, 1, MyColor::getUint32(200,200,220), 0.01, 0.3, 1);
	v3copy(pS0->ownSpeed.pos, speed);

	float offset[4];
	offset[0] = dimStart[0];
	int cloudsN = 0;
	for (int ix = 0; ix < dimPoints[0]; ix++) {
		offset[1] = dimStart[1];
		for (int iy = 0; iy < dimPoints[1]; iy++) {
			offset[2] = dimStart[2];
			for (int iz = 0; iz < dimPoints[2]; iz++) {
				//float pos[4];
				mat4x4_mul_vec4plus(pS0->ownCoords.pos, absModelMatrix, offset, 1);
				Exhaust::startSingleCloud(pS0, 0.3);
				//to next cloud
				cloudsN++;
				offset[2] += rad;
			}
			offset[1] += rad;
		}
		offset[0] += rad;
	}
	return cloudsN;
}
