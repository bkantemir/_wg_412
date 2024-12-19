#include "Exhaust.h"
#include "subjs/Smoke.h"
#include "rr/RollingStock.h"
#include "Texture.h"
#include "TheApp.h"

extern TheApp theApp;
extern float degrees2radians;
extern float PI;

int Exhaust::modelNcloud = -1;
int Exhaust::uTex0Nwn = -1;
int Exhaust::uTex3N4black = -1;

int Exhaust::init() {
	modelNcloud = ModelLoader::loadModelOnce0(&theApp.models, NULL, NULL, "/dt/md/smoke01.txt", "", "Smoke");
	unsigned int ckey = 0xff00ff00;
	uTex0Nwn = Texture::loadTexture("/dt/mt/wn64_blur3.bmp");
	uTex3N4black = Texture::loadTexture("/dt/mt/smoke4black.bmp", ckey, GL_CLAMP_TO_EDGE, GL_MIRRORED_REPEAT);
	return 1;
}
int Exhaust::startSingleCloud(Smoke* pModel,float errorLevel) {

	if (theApp.cpuLoad > 0.98) {
		if (findSlotIn(&theApp.clouds, 1) > 50) {
			//mylog("%d. Exhaust::startSingleCloud overload - skip\n",(int)theApp.frameN);
			return 0;
		}
	}

	float cloudSize = pModel->scale[0];
	Smoke* pS = (Smoke*)copyModel(&theApp.clouds, pModel);
	v3setAll(pS->scale,cloudSize);

	Material* pMt0 = &pS->mt0;
	pS->mt0isSet = 1;
	if (pS->renderType == 1)//color over mask
		pMt0->uTex1alphaChannelN = getRandom(0, 3);
	else if (pS->renderType == 2)//tex0
		pMt0->uTex0translateChannelN = getRandom(0, 3);
	pS->ownSpeed.setEulerDg(0, 0, getRandom(-5, 5));

	//set up textures
	Material* pMt = &pS->mt0;
	if (pS->renderType > 0) {
		float* pChannel = &pMt->uTexMods[0][0];
		pChannel[0] = 1;
		pChannel[1] = 0.5;
		for (int i = 0; i < 2; i++)
			pS->tex0speed[i] = getRandom(-1.0f, 1.0f);
		v2max(pS->tex0speed, 0.01);//full texture hange in 3 second
		pMt->uTex0translateChannelN = getRandom(0, 2);
	}
	if (errorLevel > 0) {
		pS->lifeSpan *= (1.0 + getRandom(-errorLevel, errorLevel));
		//start speed
		float linSpeed = v3length(pS->startSpeedPos);
		float speedErrorLim = fmax(0.2, linSpeed * errorLevel);
		float speedError[3];
		for (int i = 0; i < 3; i++)
			speedError[i] = getRandom(-speedErrorLim, speedErrorLim);
		v3max(speedError, speedErrorLim);
		for (int i = 0; i < 3; i++)
			pS->startSpeedPos[i] += speedError[i];

		//growth speed
		pS->growthSpeed *= (1.0 + getRandom(-errorLevel, errorLevel));
	}
	return pS->nInSubjsSet;
}
int Exhaust::startCloud(Exhaust* pEx, float linSpeed, float size,int renderType, unsigned int refN, float pressure, float dencity,float tex3framePixelWidth) {
	//renderType: 0-uColor,1-uColor over whitenoise Mask,2-whitenoise tex withuTex3 tanslate
	if (theApp.frameN < 10)
		return 1;

	Smoke* pS0 = prepareCloudModel(pEx, linSpeed, size, renderType, refN, pressure, dencity, tex3framePixelWidth);
	
	//check if need in-between points
	if (pEx->lastExhaustFrameN == theApp.frameN - 1) {
		float vIn[4] = { 0,0,1,0 };
		float ownSpeed[4];
		mat4x4_mul_vec4plus(ownSpeed, pEx->absModelMatrixUnscaled, vIn, 0);
		v3setLength(ownSpeed, linSpeed);
		for (int i = 0; i < 3; i++)
			ownSpeed[i] += (pEx->absCoords.pos[i] - pEx->absCoordsPrev.pos[i]);
		float ownLinSpeed = v3length(ownSpeed);
		float step = size;
		if (ownLinSpeed > step) {
			int pointsN = round(ownLinSpeed / step);
			if (pointsN > 1) { //render in-between points
				pS0->dencity /= pointsN;
				for (int pointN = pointsN - 1; pointN > 0; pointN--) {
					float v1 = (float)pointN / pointsN;
					for (int i = 0; i < 3; i++)
						pS0->ownCoords.pos[i] = pEx->absCoords.pos[i] + ownSpeed[i] * v1;
					int subjN = startSingleCloud(pS0, 0.2);
				}
			}
		}
	}
	v3copy(pS0->ownCoords.pos, pEx->absCoords.pos);
	int subjN = pEx->lastExhaustN = startSingleCloud(pS0,0.2);

	pEx->lastExhaustFrameN = theApp.frameN;
	return 1;
}

Smoke* Exhaust::prepareCloudModel(Exhaust* pEx, float linSpeed, float size, int renderType, unsigned int refN, float pressure, float dencity, float tex3framePixelWidth) {
	//renderType: 0-uColor,1-uColor over whitenoise Mask,2-whitenoise tex withuTex3 tanslate

	Smoke* pS0 = (Smoke*)theApp.models.at(modelNcloud);
	pS0->renderType = renderType;
	pS0->dencity = dencity;
	Material* pMt0 = &pS0->mt0;
	pMt0->uAlphaFactor = dencity;
	//clear first
	pMt0->uColor.clear();
	pMt0->uTex0 = -1;
	pMt0->uTex1mask = -1;
	pMt0->uTex3 = -1;
	pMt0->uEdgeAlpha = -1;
	pMt0->bTexMod = 0;
	pMt0->uSpecularIntencity = 0;
	pMt0->uAlphaBlending = 1;
	pMt0->uAmbient = 0.8;
	pMt0->uTex1alphaChannelN = 3; //default - alpha channel for mask
	pMt0->uTex0translateChannelN = -1; //translate tex0 to tex3 by channelN. Default -1 - don't translate, 4-7 - 2-tone

	if (renderType == 0) {//0-uColor
		pMt0->uColor.setUint32(refN);
		if (pMt0->uColor.getChannelInt(3) == 0)
			pMt0->uColor.setChannelInt(3, 255);
	}
	else if (renderType == 1) { //1-uColor over whitenoise Mask
		pMt0->uColor.setUint32(refN);
		if (pMt0->uColor.getChannelInt(3) == 0)
			pMt0->uColor.setChannelInt(3, 255);
		pMt0->uTex1mask = uTex0Nwn;
		pMt0->bTexMod = 1;
		float* pChannel = &pMt0->uTexMods[0][0];//uTuv used for both tex0 and tex1
		pChannel[0] = 1;
		pChannel[1] = 0.5;
	}
	else if (renderType == 2) {//2-whitenoise tex0 with uTex3 translate
		pMt0->uTex0 = uTex0Nwn;
		pMt0->bTexMod = 1;
		float* pChannel = &pMt0->uTexMods[0][0];
		pChannel[0] = 1;
		pChannel[1] = 0.5;
		//tex3
		pMt0->uTex0translateChannelN = 0;
		pMt0->uTex3 = refN;
		pChannel = &pMt0->uTexMods[3][0];
		pChannel[0] = 1;
		Texture* pTx = Texture::textures.at(pMt0->uTex3);
		pChannel[1] = tex3framePixelWidth / pTx->size[0];//frame width
		pS0->tex3widthK = (float)(pTx->size[0] - 1) / pTx->size[0];
	}
	Material::assignShader(pMt0, "phong");
	pMt0->shaderNshadow = Material::getShaderNumber(pMt0, "depthmap");
	pS0->mt0isSet = 1;

	if (size <= 0) {//if startSize unknown - take from emitter
		if (pEx != NULL)
			size = pEx->scale[0];
		else
			size = 1;
	}
	v3setAll(pS0->scale, size); //start size

	pS0->lifeSpan = sqrt(size) * sqrt(pressure) * 60;// 50;

	pS0->speedValueDrag = fmax(0.5, 1.0 - 0.04 / size / pressure);// / dencity);
	pS0->growthSpeedDrag = 0.96;

	pS0->growthSpeed = sqrt(size * pressure);

	if (pEx != NULL) {
		float vIn[4] = { 0,0,1,0 };
		float ownSpeed[4];
		mat4x4_mul_vec4plus(ownSpeed, pEx->absModelMatrixUnscaled, vIn, 0);
		v3setLength(ownSpeed, linSpeed);
		//add train speed
		buildModelMatrixStandard(pEx);
		for (int i = 0; i < 3; i++)
			pS0->startSpeedPos[i] = ownSpeed[i] + (pEx->absCoords.pos[i] - pEx->absCoordsPrev.pos[i]);
	}
	return pS0;
}

int Exhaust::startCloudRing(Exhaust* pEx, float linSpeed, float size, int renderType, unsigned int refN, float pressure, float dencity, float tex3framePixelWidth) {
	//renderType: 0-uColor,1-uColor over whitenoise Mask,2-whitenoise tex withuTex3 tanslate
	if (theApp.frameN < 10)
		return 1;

	Smoke* pS0 = prepareCloudModel(pEx, linSpeed, size, renderType, refN, pressure, dencity, tex3framePixelWidth);
	float pos00[3];
	float speed00[3];
	v3copy(pos00, pEx->absCoords.pos);
	v3copy(speed00, pS0->startSpeedPos);

	
	size = pS0->scale[0] / 8;
	v3setAll(pS0->scale, size);
	pS0->lifeSpan = pS0->lifeSpan * 2;
	pS0->growthSpeed= pS0->growthSpeed*0.5;
	pS0->speedValueDrag = sqrt(pS0->speedValueDrag);
	

	int pointsN = 12;
	float pointSpeed = pS0->growthSpeed;
	pS0->growthSpeedDrag = pS0->speedValueDrag;
	float circumference = size * pointsN / 2;	
	float pointShift = circumference / PI / 2;
	for (int pointN = 0; pointN < pointsN; pointN++) {
		float angle = (360.0 / pointsN) * pointN;
		float vIn[4] = { 0,0,0,0 };
		vIn[0] = cos(degrees2radians * angle);
		vIn[1] = sin(degrees2radians * angle);
		float dir[4];
		mat4x4_mul_vec4plus(dir, pEx->absModelMatrixUnscaled, vIn, 0);
		v3setLength(dir, pointSpeed);
		for (int i = 0; i < 3; i++)
			pS0->startSpeedPos[i] = speed00[i]+dir[i];
		v3setLength(dir, pointShift);
		for (int i = 0; i < 3; i++)
			pS0->ownCoords.pos[i] = pos00[i]+dir[i];

		int subjN = pEx->lastExhaustN = startSingleCloud(pS0, 0);
	}

	pEx->lastExhaustFrameN = theApp.frameN;
	return 1;
}



