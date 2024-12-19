#include "TheTable.h"
#include "ModelBuilder.h"
#include "Texture.h"
#include "TheApp.h"
#include "geom/Line3D.h"
#include "ButtonTableDrag.h"
#include "utils.h"
#include "Shadows.h"
#include "Input.h"
#include "ScreenFrame.h"
#include "MySound.h"

extern TheApp theApp;
extern std::string filesRoot;
extern float degrees2radians;

TheTable::~TheTable() {
	cleanUpTable();
}



void TheTable::cleanUpTable() {
	for (int i = tableParts.size() - 1; i >= 0; i--)
		delete tableParts.at(i);
	tableParts.clear();

	for (int i = table_drawJobs.size() - 1; i >= 0; i--)
		delete table_drawJobs.at(i);
	table_drawJobs.clear();

	for (int i = table_buffersIds.size() - 1; i >= 0; i--) {
		unsigned int id = table_buffersIds.at(i);
		glDeleteBuffers(1, &id);
	}
	table_buffersIds.clear();
}
int TheTable::initTable(float tileSizeXZ, float tileSizeUp, float tileSizeDown, int tilesNx, int tilesNz) {
	tileSize = tileSizeXZ;
	tableTiles[0] = tilesNx;
	tableTiles[1] = tilesNz;
	groundLevel0 = 0;
	seaDepth=tileSizeDown;
	skyHeight = tileSizeUp;

	//define worldBox 
	worldBox.bbMin[1] = -seaDepth;
	worldBox.bbMax[1] = skyHeight;
	worldBox.bbMax[0] = tileSizeXZ * 0.5 * tilesNx;
	worldBox.bbMax[2] = tileSizeXZ * 0.5 * tilesNz;
	worldBox.bbMin[0] = -worldBox.bbMax[0];
	worldBox.bbMin[2] = -worldBox.bbMax[2];
	Gabarites::adjustMidRad(&worldBox);

	if(theApp.bEditMode)
		buildTablePartsGrid();
	else
		buildTableParts();
	return 1;
}
void TheTable::placeAt(float* pos, float x, float y, float z) {
	pos[0] = x * tileSize - worldBox.bbRad[0];
	pos[1] = y;
	pos[2] = z * tileSize - worldBox.bbRad[2];
}
int TheTable::onLeftButtonDown() {
	//TouchScreen::capturedCode.assign(ButtonTableDrag::defaultMode64);
	cursorOnTableGood = (getCursorAncorPointTable(TouchScreen::ancorPoint, TouchScreen::cursorPos, this)>0);

	if (cursorOnTableGood) {
		dragMode = 0;//grag xz
		float* cPos = TouchScreen::captureCursorPos;
		for (int i = 0; i < 2; i++) {
			if (cPos[i] < ScreenFrame::screenFrameMargin) {
				dragMode = 1; //spin
				break;
			}
			else if (cPos[i] > UISubj::screenSize[i] - ScreenFrame::screenFrameMargin) {
				dragMode = 1; //spin
				break;
			}
		}
	}
	else
		dragMode = 1;//spin only


	return 1;
};

int TheTable::getCursorAncorPointTable(float* ancorPoint, float* cursorPos, TheTable* pTable) {
	v3setAll(ancorPoint, 0);
    float* targetRads = theApp.mainCamera.targetRads;
    //cursor position in GL range
    float cursorGLpos[4];
    cursorGLpos[0] = (TouchScreen::cursorPos[0] - targetRads[0]) / targetRads[0];
    cursorGLpos[1] = -(TouchScreen::cursorPos[1] - targetRads[1]) / targetRads[1];
    cursorGLpos[2] = 1;
    //cursor position in world coords
    float cursor3Dpos[4];
	//mat4x4_mul_vec4plus(cursor3Dpos, theApp.mainCamera.mVPinverted, cursorGLpos, 1, true);
	
	//rebuild mVPinverted WITHOUT keystone suppression
	Camera* pCam = &theApp.mainCamera;
	mat4x4 mVP;
	mat4x4 mProjection;
	mat4x4_perspective(mProjection, pCam->viewRangeDg * degrees2radians, pCam->targetAspectRatio, pCam->nearClip, pCam->farClip);
	mat4x4_mul(mVP, mProjection, pCam->lookAtMatrix);
	mat4x4 mVPinverted;
	mat4x4_invert(mVPinverted, mVP);
	if (mat4x4_mul_vec4plus(cursor3Dpos, mVPinverted, cursorGLpos, 1, true) < 1) {
		//mylog("ERROR in TheTable::getCursorAncorPointTable - bad cursor3Dpos\n");
		return -1;
	}
	
	Line3D* pL3D = new Line3D(theApp.mainCamera.ownCoords.pos, cursor3Dpos);
    float y0 = pTable->groundLevel0;
    if (Line3D::crossPlane(ancorPoint, pL3D, 1, y0) == 0) {
        mylog("ERROR in TheTable::getCursorAncorPointTable - no crossing\n");
        return -1;
    }
    return 1;
}
int TheTable::onDrag() {
    theApp.bPause = true;
	if (v2match(TouchScreen::captureCursorPos, TouchScreen::cursorPos))
		return 0;

	Camera* pCam = &theApp.mainCamera;

	if (TouchScreen::capturedCode.compare("drag_table_sun") == 0) {//Light-SPIN
		float d[2];
		for (int i = 0; i < 2; i++)
			d[i] = TouchScreen::cursorPos[i] - TouchScreen::captureCursorPos[i];
		//d[0] *= -1;
		float* lightDir=theApp.dirToMainLight;
		float yaw0 = v3yawDg(lightDir);
		float pitch0 = v3pitchDg(lightDir);

		float yaw = angleDgNorm180(yaw0 + d[0] / UISubj::screenSize[0] * 180);
		//dragging front(closest) table rib
		float pitchD = d[1] / UISubj::screenSize[1] * 20;
		float pitch = pitch0 + pitchD;
		pitch = minimax(pitch, -80, -40);

		Coords lt;
		lt.setEulerDg(pitch, yaw, 0);
		float v1[4] = { 0,0,1,0 };
		mat4x4_mul_vec4plus(lightDir, lt.rotationMatrix, v1, 0);

		Shadows::resetShadowsFor(&theApp.mainCamera, &theApp.gameTable.worldBox);

	}
	//else if (TouchScreen::capturedCode.compare("drag_table_spin") == 0) {//TABLE-SPIN
	else if (dragMode ==1) {//TABLE-SPIN
		float d[2];
		for (int i = 0; i < 2; i++)
			d[i] = TouchScreen::cursorPos[i] - TouchScreen::captureCursorPos[i];
		d[0] *= -1;
		float yaw = angleDgNorm180(pCam->ownCoords.getEulerDg(1) + d[0] / UISubj::screenSize[0] * 180);
		//dragging front(closest) table rib
		float pitchD = d[1] / UISubj::screenSize[1] * 45;
		float pitch = pCam->ownCoords.getEulerDg(0) + pitchD;
		pitch = minimax(pitch, 5, 80);

		pCam->ownCoords.setEulerDg(pitch, yaw, 0);
		Camera::reset(pCam, &worldBox);

	}
	//else if (TouchScreen::capturedCode.compare("drag_table_xz") == 0) {
	else if (dragMode == 0) {
		theApp.cameraMan.task = -1;
		float newAncor[4];
		if (getCursorAncorPointTable(newAncor, TouchScreen::cursorPos, this) < 1) {
			TouchScreen::abandonSelected();
			setCursor("arrow");
			TouchScreen::capturedCode.assign("");
			return -1;
		}
		float shift[4];
		for (int i = 0; i < 3; i++)
			shift[i] = TouchScreen::ancorPoint[i] - newAncor[i];

		//mylog("%d: %d x %d shift %d x %d\n", (int)theApp.frameN, (int)newAncor[0], (int)newAncor[2], (int)shift[0], (int)shift[2]);

		Camera testCam;
		Camera::copyParams(&testCam, &theApp.mainCamera);
		//adjust cam lookAtPoint
		testCam.lookAtPoint[0] += shift[0];
		testCam.lookAtPoint[2] += shift[2];
		if (Camera::outOfLimits(&testCam, &worldBox) == false) {
			Camera::copyParams(&theApp.mainCamera ,&testCam);
			Camera::reset(pCam, &worldBox);
		}
		getCursorAncorPointTable(TouchScreen::ancorPoint, TouchScreen::cursorPos, this);
	}
	else if (TouchScreen::capturedCode.compare("drag_table_xy") == 0) {
		theApp.cameraMan.task = -1;
		float shift[4] = { 0,0,0,0 };
		for (int i = 0; i < 2; i++)
			shift[i] = TouchScreen::cursorPos[i] - TouchScreen::captureCursorPos[i];
		
		//Camera Y Position limits
		if (shift[1] > 0) {//limit up
			if (pCam->lookAtPoint[1] + shift[1] > worldBox.bbMax[1])
				shift[1] = 0;
		}
		else if (shift[1] < 0) {//limit down
			float limY = theApp.gameTable.groundLevel0 + 20;// pCam->ownCoords.getEulerDg(0) * 4;
			if (pCam->ownCoords.pos[1] + shift[1] < limY)
				shift[1] = 0;
		}

		Camera testCam;
		Camera::copyParams(&testCam, &theApp.mainCamera);

		float vAdjust[4];
		if (mat4x4_mul_vec4plus(vAdjust, testCam.ownCoords.rotationMatrix, shift, 1, true) < 1) {
			TouchScreen::abandonSelected();
			setCursor("arrow");
			TouchScreen::capturedCode.assign("");
			return -1;
		}
		testCam.lookAtPoint[0] += vAdjust[0];
		if (shift[1] != 0) {
			float cameraUp[4];
			float vIn[4] = { 0,1,0,0 };
			mat4x4_mul_vec4plus(cameraUp, testCam.ownCoords.rotationMatrix, vIn, 0);
			
			for (int i = 0; i < 3; i++)
				testCam.lookAtPoint[i] += cameraUp[i] * shift[1];
		}
		Camera::setCameraPosition(&testCam);
		Camera::refocus2ground(&testCam);

		//adjust cam lookAtPoint
		if (Camera::outOfLimits(&testCam, &worldBox) == false) {
			Camera::copyParams(&theApp.mainCamera, &testCam);
			Camera::reset(&theApp.mainCamera, &worldBox);
		}
	}
	v2copy(TouchScreen::captureCursorPos, TouchScreen::cursorPos);
	return 1;
}

float TheTable::suggestedStageSize() {
	float tilesN = fmax(tableTiles[0], tableTiles[1])-0.5;
	float size = tileSize * tilesN;
	size = 600;// minimax(size, 350, 700);
	return size;
}
int TheTable::buildTablePartsGrid() {
	ModelBuilder* pMB = new ModelBuilder();
	Material mt;
	strcpy_s(mt.shaderType32, 32, "phong");
	strcpy_s(mt.materialName32, 32, "table");
	mt.dropsShadow = 0;
	mt.uSpecularIntencity = 0;
	mt.uAmbient = 0.6;
	VirtualShape vs;

	
	pMB->lockGroup(pMB); {//entire table		
		pMB->lockGroup(pMB); {//table top
			v3set(vs.whl, tileSize, 0, tileSize);
			for (int zN = 0; zN < tableTiles[1]; zN++)
				for (int xN = 0; xN < tableTiles[0]; xN++) {
					
					SceneSubj* pSS = new SceneSubj();
					pSS->nInSubjsSet = tableParts.size();
					tableParts.push_back(pSS);
					pSS->pDrawJobs = &table_drawJobs;
					pSS->pSubjsSet = &tableParts;
					pSS->buildModelMatrix();
					pSS->chordaCollision.chordType = -1;
					pSS->chordaScreen.chordType = -1;
					pMB->useSubjN(pMB, pSS->nInSubjsSet);
					if ((zN + xN + startTileColorN) % 2 == 0)
						mt.uColor.setRGBA(0.4f, 0.4f, 0.4f);
					else
						mt.uColor.setRGBA(0.3f, 0.3f, 0.3f);

					pMB->usingMaterialN = pMB->getMaterialN(pMB, &mt);

					float z0 = tileSize * zN - worldBox.bbRad[2] + tileSize / 2;
					float x0 = tileSize * xN - worldBox.bbRad[0] + tileSize / 2;

					pMB->lockGroup(pMB);
					pMB->buildBoxFace(pMB, "top", &vs);
					pMB->shiftGroup(pMB, x0, 0, z0);
					pMB->releaseGroup(pMB);
					
				}
			/*
			TexCoords tc;
			tc.set_GL(&tc, 0,0,10,10,"");
			pMB->groupApplyTexture(pMB, "top", &tc);//, TexCoords * pTC = NULL, TexCoords * pTC2nm = NULL);
			*/
		}
		pMB->releaseGroup(pMB); //table top

		pMB->lockGroup(pMB); {//table walls
			v3set(vs.whl, tileSize, seaDepth, 0);
			for (int yaw = 0; yaw < 360; yaw += 90) {
				int tilesN;
				int colorN = startTileColorN;
				float shiftXZ[2] = { 0,0 };
				bool withBorder = false;
				float wallRad = worldBox.bbRad[0];
				switch (yaw) {
				case 0://front
					tilesN = tableTiles[0];
					colorN = (startTileColorN + tableTiles[1]-1) % 2; //startTileColorN;
					shiftXZ[1] = worldBox.bbRad[2];
					if (tableTiles[1] >= maxTilesSize)
						withBorder = true;
					wallRad = worldBox.bbRad[0];
					break;
				case 180://back
					tilesN = tableTiles[0];
					colorN = (startTileColorN + tableTiles[0] -1) % 2;//startTileColorN; //
					shiftXZ[1] = -worldBox.bbRad[2];
					if (tableTiles[1] >= maxTilesSize)
						withBorder = true;
					wallRad = worldBox.bbRad[0];
					break;
				case 90://right
					tilesN = tableTiles[1];
					colorN = (startTileColorN + tableTiles[0] + tableTiles[1] ) % 2;
					shiftXZ[0] = worldBox.bbRad[0];
					if (tableTiles[0] >= maxTilesSize)
						withBorder = true;
					wallRad = worldBox.bbRad[2];
					break;
				case 270://left
					tilesN = tableTiles[1];
					colorN = startTileColorN;
					shiftXZ[0] = -worldBox.bbRad[0];
					if (tableTiles[0] >= maxTilesSize)
						withBorder = true;
					wallRad = worldBox.bbRad[2];
					break;
				}
				pMB->lockGroup(pMB); //whole wall
				mt.takesShadow = 0;
				for (int tileN = 0; tileN < tilesN; tileN++) {
					pMB->lockGroup(pMB); { //1 tile
						SceneSubj* pSS = new SceneSubj();
						pSS->nInSubjsSet = tableParts.size();
						tableParts.push_back(pSS);
						pSS->pDrawJobs = &table_drawJobs;
						pSS->pSubjsSet = &tableParts;
						pSS->buildModelMatrix();
						pSS->chordaCollision.chordType = -1;
						pSS->chordaScreen.chordType = -1;
						pMB->useSubjN(pMB, pSS->nInSubjsSet);
						if ((colorN+ tileN) % 2 == 0)
							mt.uColor.setRGBA(0.4f, 0.4f, 0.4f);
						else
							mt.uColor.setRGBA(0.3f, 0.3f, 0.3f);
						pMB->usingMaterialN = pMB->getMaterialN(pMB, &mt);
						pMB->buildBoxFace(pMB, "front", &vs);
						if (withBorder) {
							float borderWidth = 4;
							VirtualShape vsB;
							v3set(vsB.whl, tileSize, groundLevel0, borderWidth);
							pMB->lockGroup(pMB); {
								mt.uColor.setRGBA(1.0f, 0.0f, 0.0f);
								pMB->usingMaterialN = pMB->getMaterialN(pMB, &mt);
								pMB->buildBoxFace(pMB, "top", &vsB);
							}
							pMB->shiftGroup(pMB, 0, seaDepth / 2+0.5, -borderWidth/2);
							pMB->releaseGroup(pMB); //1 tile
						}
					}
					float x0 = tileSize * tileN - wallRad + tileSize / 2;
					pMB->shiftGroup(pMB, x0, 0, 0);
					pMB->releaseGroup(pMB); //1 tile

				}
				pMB->moveGroupDg(pMB, 0,yaw,0, shiftXZ[0], 0, shiftXZ[1]);
				pMB->releaseGroup(pMB); //whole wall
			}
		}
		pMB->shiftGroup(pMB, 0, -seaDepth/2, 0);
		pMB->releaseGroup(pMB); //table walls
	}
	//pMB->shiftGroup(pMB, 0, 0, 0);
	pMB->releaseGroup(pMB); //entire table
	pMB->buildDrawJobs(pMB, &tableParts, &table_drawJobs, &table_buffersIds);
	delete pMB;
	/*
	//debug
	int subjN = ModelLoader::loadModelStandard(&tableParts, &table_drawJobs, &table_buffersIds,
		"/dt/md/cross3.txt", "", "", "", NULL);
	SceneSubj* pSS = tableParts.at(subjN);
	pSS->scaleMe(50);
	v3set(pSS->ownCoords.pos,-worldBox.bbRad[0]+50,groundLevel0+1, -worldBox.bbRad[2]);
	///////////
	*/

	for (int tN = tableParts.size() - 1; tN >= 0; tN--) {
		SceneSubj* pTP = tableParts.at(tN);
		pTP->buildModelMatrix();
		SceneSubj::initGabarites(pTP);
	}

	return 1;
}
int TheTable::buildTableParts() {
	ModelBuilder* pMB = new ModelBuilder();
	Material mt;
	strcpy_s(mt.shaderType32, 32, "phong");
	strcpy_s(mt.materialName32, 32, "table");
	mt.dropsShadow = 0;
	mt.uSpecularIntencity = 0;
	mt.uAmbient = 0.6;
	/*
	if (getRandom(0, 1) == 0)
		mt.uTex0 = Texture::loadTexture("/dt/mt/wood00.jpg");
	else
	*/
		mt.uTex0 = Texture::loadTexture("/dt/mt/wood00.jpg");
	VirtualShape vs;


	pMB->lockGroup(pMB); {//entire table		
		pMB->lockGroup(pMB); {//table top
			v3set(vs.whl, tileSize, 0, tileSize);
			for (int zN = 0; zN < tableTiles[1]; zN++)
				for (int xN = 0; xN < tableTiles[0]; xN++) {

					SceneSubj* pSS = new SceneSubj();
					pSS->nInSubjsSet = tableParts.size();
					tableParts.push_back(pSS);
					pSS->pDrawJobs = &table_drawJobs;
					pSS->pSubjsSet = &tableParts;
					pSS->buildModelMatrix();
					pSS->chordaCollision.chordType = -1;
					pSS->chordaScreen.chordType = -1;
					pMB->useSubjN(pMB, pSS->nInSubjsSet);
					/*
					if ((zN + xN + startTileColorN) % 2 == 0)
						mt.uColor.setRGBA(0.2f, 0.2f, 0.2f);
					else
						mt.uColor.setRGBA(0.3f, 0.3f, 0.3f);
						*/
					pMB->usingMaterialN = pMB->getMaterialN(pMB, &mt);

					float z0 = tileSize * zN - worldBox.bbRad[2] + tileSize / 2;
					float x0 = tileSize * xN - worldBox.bbRad[0] + tileSize / 2;

					pMB->lockGroup(pMB);
					pMB->buildBoxFace(pMB, "top", &vs);
					pMB->shiftGroup(pMB, x0, 0, z0);
					pMB->releaseGroup(pMB);

				}
			TexCoords tc;
			tc.set_GL(&tc, 0,0,1,1,"");
			pMB->groupApplyTexture(pMB, "top", &tc);//, TexCoords * pTC = NULL, TexCoords * pTC2nm = NULL);
		}
		pMB->releaseGroup(pMB); //table top

		pMB->lockGroup(pMB); {//table walls
			v3set(vs.whl, tileSize, seaDepth, 0);
			for (int yaw = 0; yaw < 360; yaw += 90) {
				int tilesN;
				int colorN = startTileColorN;
				float shiftXZ[2] = { 0,0 };
				bool withBorder = false;
				float wallRad = worldBox.bbRad[0];
				switch (yaw) {
				case 0://front
					tilesN = tableTiles[0];
					colorN = (startTileColorN + tableTiles[1] - 1) % 2; //startTileColorN;
					shiftXZ[1] = worldBox.bbRad[2];
					if (tableTiles[1] >= maxTilesSize)
						withBorder = true;
					wallRad = worldBox.bbRad[0];
					break;
				case 180://back
					tilesN = tableTiles[0];
					colorN = (startTileColorN + tableTiles[0] - 1) % 2;//startTileColorN; //
					shiftXZ[1] = -worldBox.bbRad[2];
					if (tableTiles[1] >= maxTilesSize)
						withBorder = true;
					wallRad = worldBox.bbRad[0];
					break;
				case 90://right
					tilesN = tableTiles[1];
					colorN = (startTileColorN + tableTiles[0] + tableTiles[1]) % 2;
					shiftXZ[0] = worldBox.bbRad[0];
					if (tableTiles[0] >= maxTilesSize)
						withBorder = true;
					wallRad = worldBox.bbRad[2];
					break;
				case 270://left
					tilesN = tableTiles[1];
					colorN = startTileColorN;
					shiftXZ[0] = -worldBox.bbRad[0];
					if (tableTiles[0] >= maxTilesSize)
						withBorder = true;
					wallRad = worldBox.bbRad[2];
					break;
				}
				pMB->lockGroup(pMB); //whole wall
				mt.takesShadow = 0;
				for (int tileN = 0; tileN < tilesN; tileN++) {
					pMB->lockGroup(pMB); { //1 tile
						SceneSubj* pSS = new SceneSubj();
						pSS->nInSubjsSet = tableParts.size();
						tableParts.push_back(pSS);
						pSS->pDrawJobs = &table_drawJobs;
						pSS->pSubjsSet = &tableParts;
						pSS->buildModelMatrix();
						pSS->chordaCollision.chordType = -1;
						pSS->chordaScreen.chordType = -1;
						pMB->useSubjN(pMB, pSS->nInSubjsSet);

						pMB->usingMaterialN = pMB->getMaterialN(pMB, &mt);
						pMB->buildBoxFace(pMB, "front", &vs);
					}
					float x0 = tileSize * tileN - wallRad + tileSize / 2;
					pMB->shiftGroup(pMB, x0, 0, 0);
					pMB->releaseGroup(pMB); //1 tile

				}
				TexCoords tc;
				tc.set_GL(&tc, 0, 0, 1, 0.2, "90");
				pMB->groupApplyTexture(pMB, "front", &tc);//, TexCoords * pTC = NULL, TexCoords * pTC2nm = NULL);

				pMB->moveGroupDg(pMB, 0, yaw, 0, shiftXZ[0], 0, shiftXZ[1]);
				pMB->releaseGroup(pMB); //whole wall
			}
		}
		pMB->shiftGroup(pMB, 0, -seaDepth / 2, 0);
		pMB->releaseGroup(pMB); //table walls
	}
	//pMB->shiftGroup(pMB, 0, 0, 0);
	pMB->releaseGroup(pMB); //entire table
	pMB->buildDrawJobs(pMB, &tableParts, &table_drawJobs, &table_buffersIds);
	delete pMB;
	/*
	//debug
	int subjN = ModelLoader::loadModelStandard(&tableParts, &table_drawJobs, &table_buffersIds,
		"/dt/md/cross3.txt", "", "", "", NULL);
	SceneSubj* pSS = tableParts.at(subjN);
	pSS->scaleMe(50);
	v3set(pSS->ownCoords.pos,-worldBox.bbRad[0]+50,groundLevel0+1, -worldBox.bbRad[2]);
	///////////
	*/

	for (int tN = tableParts.size() - 1; tN >= 0; tN--) {
		SceneSubj* pTP = tableParts.at(tN);
		pTP->buildModelMatrix();
		SceneSubj::initGabarites(pTP);
	}

	return 1;
}

int TheTable::onClick() { 
	if (theApp.cameraMan.zoom != 0)
		cameraMan_zoomInOut(this);
	else
		;// MySound::playSound(MySound::soundNwhat01, NULL, false);
	return 1;
}