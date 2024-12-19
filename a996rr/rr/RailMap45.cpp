#include "RailMap45.h"
#include "ModelLoader.h"
#include "TheApp.h"
#include "RollingStock.h"
#include "MeltingSign.h"
#include "MySound.h"

extern float PI;
extern float degrees2radians;
extern TheApp theApp;

RailMap45::~RailMap45() {
	cleanUpLayout();
}

void RailMap45::cleanUpLayout() {
	for (int i = railsMap.size() - 1; i >= 0; i--) {
		SceneSubj* pR = railsMap.at(i);
		if(pR != NULL)
			delete pR;
	}
	railsMap.clear();

	TheTable::cleanUpTable();
}




void RailMap45::initRailModelsVirtual() {

	ModelBuilder* pMB = new ModelBuilder();
	Material mt;
	strcpy_s(mt.shaderType32, 32, "flat");
	strcpy_s(mt.materialName32, 32, "virtual rail");
	mt.uColor.setRGBA(56, 200, 56);
	mt.lineWidth = 3;
	mt.lineWidthFixed = 2;
	mt.dropsShadow = false;
	mt.uAlphaBlending = true;
	mt.uAlphaFactor = 0.5;

	//railModelStraightVirtualN
	mt.primitiveType = GL_LINES;
	pMB->usingMaterialN = pMB->getMaterialN(pMB, &mt);
	Rail::railModelStraightVirtualN = Rail::railModels.size();
	Rail* pSS = new Rail();
	strcpy_s(pSS->className, 32, "Rail");

	Rail::railModels.push_back(pSS);
	pSS->railStatus = 1; //virtual
	pSS->railType = 0;//straight
	pSS->railLength = Rail::railLenghtLong;

	//pSS->ownCoords.pos[1] = Rail::railsLevel;
	pSS->pDrawJobs = &table_drawJobs;
	pSS->pSubjsSet = &Rail::railModels;
	pSS->nInSubjsSet = Rail::railModels.size() - 1;
	pSS->buildModelMatrix();
	pMB->useSubjN(pMB, pSS->nInSubjsSet);
	pMB->lockGroup(pMB);
	pMB->addVertex(pMB, -tileSize / 2, 0, 0);
	pMB->addVertex(pMB, tileSize / 2, 0, 0);
	pMB->releaseGroup(pMB);
	pSS->totalNativeElements = Rail::railModels.size() - pSS->nInSubjsSet;
	pSS->totalElements = pSS->totalNativeElements;

	//railModelShortVirtualN
	mt.primitiveType = GL_LINES;
	pMB->usingMaterialN = pMB->getMaterialN(pMB, &mt);
	Rail::railModelShortVirtualN = Rail::railModels.size();
	pSS = new Rail();
	strcpy_s(pSS->className, 32, "Rail");

	Rail::railModels.push_back(pSS);
	pSS->railStatus = 1; //virtual
	pSS->railType = 0;//straight
	pSS->railLength = Rail::railLenghtShort;
	pSS->pDrawJobs = &table_drawJobs;
	pSS->pSubjsSet = &Rail::railModels;
	pSS->nInSubjsSet = Rail::railModels.size() - 1;
	pSS->buildModelMatrix();
	pMB->useSubjN(pMB, pSS->nInSubjsSet);
	pMB->lockGroup(pMB);
	pMB->addVertex(pMB, -Rail::railLenghtShort / 2, 0, 0);
	pMB->addVertex(pMB, Rail::railLenghtShort / 2, 0, 0);
	pMB->releaseGroup(pMB);
	pSS->totalNativeElements = Rail::railModels.size() - pSS->nInSubjsSet;
	pSS->totalElements = pSS->totalNativeElements;

	//railModelCurvedVirtualN
	mt.primitiveType = GL_LINE_STRIP;
	pMB->usingMaterialN = pMB->getMaterialN(pMB, &mt);
	Rail::railModelCurvedVirtualN = Rail::railModels.size();
	pSS = new Rail();
	strcpy_s(pSS->className, 32, "Rail");

	Rail::railModels.push_back(pSS);
	pSS->railStatus = 1; //virtual
	pSS->railType = 1;//curved
	pSS->railLength = Rail::railLenghtCurved;
	pSS->pDrawJobs = &table_drawJobs;
	pSS->pSubjsSet = &Rail::railModels;
	pSS->nInSubjsSet = Rail::railModels.size() - 1;
	pSS->buildModelMatrix();
	pSS->chordaCollision.chordType = -1;
	pSS->chordaScreen.chordType = -1;

	pMB->useSubjN(pMB, pSS->nInSubjsSet);
	int lineStartsAt = pMB->vertices.size();
	pMB->lockGroup(pMB);
	for (float aY = -22.5; aY <= 22.5; aY += 5) {
		float angleRd = (aY - 90.0) * degrees2radians;
		float kx = cosf(angleRd) * Rail::curveRadius;
		float kz = sinf(angleRd) * Rail::curveRadius;
		pMB->addVertex(pMB, kx, 0, kz);
	}
	//mark first and last verts
	Vertex01* pV = pMB->vertices.back();
	pV->endOfSequence = 1;
	pV = pMB->vertices.at(lineStartsAt);
	pV->endOfSequence = -1;
	pMB->moveGroupDg(pMB, 0, 0, 0, 0, 0, Rail::tileCenter2curveCenter);
	pMB->releaseGroup(pMB);
	pSS->totalNativeElements = Rail::railModels.size() - pSS->nInSubjsSet;
	pSS->totalElements = pSS->totalNativeElements;

	
	mt.uAlphaBlending = true;
	mt.uAlphaFactor = 0.2;
	mt.lineWidth = 1;
	mt.lineWidthFixed = 1;
	mt.uColor.setRGBA(255, 255, 255);

	//railModelSbowFlakeN
	mt.primitiveType = GL_LINES;
	pMB->usingMaterialN = pMB->getMaterialN(pMB, &mt);
	pMB->lockGroup(pMB);
	Rail::railModelSnowFlakeN = Rail::railModels.size();
	pSS = new Rail();
	strcpy_s(pSS->className, 32, "Rail");

	Rail::railModels.push_back(pSS);
	pSS->railStatus = -1; //virtual
	//pSS->ownCoords.pos[1] = Rail::railsLevel-2.5;
	pSS->pDrawJobs = &table_drawJobs;
	pSS->pSubjsSet = &Rail::railModels;
	pSS->nInSubjsSet = Rail::railModels.size() - 1;
	pSS->buildModelMatrix();
	pMB->useSubjN(pMB, pSS->nInSubjsSet);
	for (float yaw = 0; yaw < 180; yaw += 45) {
		pMB->lockGroup(pMB);
		pMB->addVertex(pMB, 0, 0, -tileSize / 2);
		pMB->addVertex(pMB, 0, 0, tileSize / 2);
		pMB->rotateGroupDg(pMB, 0, yaw, 0);
		pMB->releaseGroup(pMB);
	}
	pMB->releaseGroup(pMB);
	//curved rails
	mt.primitiveType = GL_LINE_STRIP;
	pMB->usingMaterialN = pMB->getMaterialN(pMB, &mt);
	for (float yaw = 0; yaw <= 360; yaw += 45) {
		int lineStartsAt = pMB->vertices.size();
		pMB->lockGroup(pMB);
		for (float aY = -22.5; aY <= 22.5; aY += 5) {
			float angleRd = (aY - 90.0) * degrees2radians;
			float kx = cosf(angleRd) * Rail::curveRadius;
			float kz = sinf(angleRd) * Rail::curveRadius;
			pMB->addVertex(pMB, kx, 0, kz);
		}
		//mark first and last verts
		Vertex01* pV = pMB->vertices.back();
		pV->endOfSequence = 1;
		pV = pMB->vertices.at(lineStartsAt);
		pV->endOfSequence = -1;
		pMB->shiftGroup(pMB, 0, 0, Rail::tileCenter2curveCenter);
		pMB->rotateGroupDg(pMB, 0, yaw - 22.5, 0);
		pMB->releaseGroup(pMB);
	}
	pSS->totalNativeElements = Rail::railModels.size() - pSS->nInSubjsSet;
	pSS->totalElements = pSS->totalNativeElements;

	//railModelSnowFlakeSmallN (railModelCrossN)
	mt.primitiveType = GL_LINES;
	mt.lineWidth = 3;
	pMB->usingMaterialN = pMB->getMaterialN(pMB, &mt);
	pMB->lockGroup(pMB);
	Rail::railModelSnowFlakeSmallN = Rail::railModels.size();
	pSS = new Rail();
	strcpy_s(pSS->className, 32, "Rail");

	Rail::railModels.push_back(pSS);
	pSS->railStatus = -1; //virtual
	//pSS->ownCoords.pos[1] = Rail::railsLevel-2.5;
	pSS->pDrawJobs = &table_drawJobs;
	pSS->pSubjsSet = &Rail::railModels;
	pSS->nInSubjsSet = Rail::railModels.size() - 1;
	pSS->buildModelMatrix();
	pMB->useSubjN(pMB, pSS->nInSubjsSet);
	for (float yaw = 45; yaw < 180; yaw += 90) {
		pMB->lockGroup(pMB);
		pMB->addVertex(pMB, 0, 0, -Rail::railLenghtShort / 2);
		pMB->addVertex(pMB, 0, 0, Rail::railLenghtShort / 2);
		pMB->moveGroupDg(pMB, 0, yaw, 0, 0, 0, 0);
		pMB->releaseGroup(pMB);
	}
	pMB->releaseGroup(pMB);
	pMB->buildDrawJobs(pMB, &Rail::railModels, &table_drawJobs, &table_buffersIds);
	pSS->totalNativeElements = Rail::railModels.size() - pSS->nInSubjsSet;
	pSS->totalElements = pSS->totalNativeElements;
	/*
	//debug
	{
		Rail* pR = (Rail * )Rail::railModels.at(Rail::railModelSnowFlakeN);
		int a = 0;
	}
	*/
	delete pMB;
}

void RailMap45::initRailModels() {
	Rail::railsLevel = groundLevel0 + 3;
	Rail::curveRadius = tileSize * 1.207;
	Rail::tileCenter2curveCenter = tileSize * 1.306;
	Rail::railLenghtLong = tileSize;
	Rail::railLenghtShort = tileSize * 0.414;// 0.207;
	Rail::railLenghtCurved = Rail::curveRadius * PI * 2 / 8;
	//guides
	Rail::railGuideLength = Rail::railLenghtLong * 0.5;
	Rail::railGuideAngle = Rail::railGuideLength * 360 / (PI * 2 * Rail::curveRadius);
	Rail::railGuideShortLength = Rail::railLenghtLong * 0.25;
	Rail::railGuideShortAngle = Rail::railGuideShortLength * 360 / (PI * 2 * Rail::curveRadius);


	ModelLoader* pML = new ModelLoader(&Rail::railModels, &table_drawJobs, &table_buffersIds,-1,NULL);
	ModelBuilder* pMB = pML->pModelBuilder;
	pML->doTag("<include='/dt/mt/materials.txt' />");
	pML->doTag("<texture_as=tx1 src='/dt/md/rr/rail01.bmp'/>");

	pML->doTag("<mt_save_as=guide-bed mt_use=satin uColor=#332222 uSpecularIntencity=0.3 noShadow />");
	//pML->doTag("<mt_save_as=guide-bed mt_use=teflon uColor=#665544 uSpecularIntencity=0.3 noShadow />");
	pML->doTag("<mt_save_as=guide-wire mt_use=metal-wire uColor=#bbbbbb lineWidth=0.5 lineWidthFixed=2 />");

	int year = getRandom(1860, 2023); //1990; //
	pML->doTag("<mt_save_as=rail_wire mt_use=metal-wire uColor=#aaaaaa uSpecularIntencity=0.2 lineWidth=0.8 lineWidthFixed=2 />");
	pML->doTag("<mt_save_as=rail_top mt_use=silver />"); //metal uColor=#cccccc />");
	if (year < 1920) { //old time

		pML->doTag("<mt_save_as=sleeper_wall mt_use=teflon uColor=#665544 / >");
		pML->doTag("<mt_save_as=sleeper_top mt_use=sleeper_wall />");
		pML->doTag("<mt_save_as=fastener mt_use=metal uColor=#442211 uTex2nm_use=tx1 />");

		pML->doTag("<mt_save_as=rail_wall mt_use=sleeper_wall uTex2nm_use=tx1 / >");// 
	}
	else if (year > 1980) { //modern/concrete
		//pML->doTag("<mt_save_as=rail_wall mt_use=metal uColor=#555555 uTex2nm_use=tx1 / >");// 
		pML->doTag("<mt_save_as=rail_wall mt_use=metal uColor=#443333 uTex2nm_use=tx1 uSpecularIntencity=0.4/ >");// 

		pML->doTag("<mt_save_as=sleeper_wall mt_use=matte uColor=#8f8f80 / >");
		pML->doTag("<mt_save_as=sleeper_top mt_use=sleeper_wall uTex2nm_use=tx1 />");
		//pML->doTag("<mt_save_as=fastener mt_use=rail_wall />");// uColor = #888888 / > "); //
		pML->doTag("<mt_save_as=fastener mt_use=rail_wall uColor = #888898 / > "); //
	}
	else { //default
		pML->doTag("<mt_save_as=rail_wall mt_use=rail_top uColor=#ddeedd uTex2nm_use=tx1 / >");// 

		pML->doTag("<mt_save_as=sleeper_wall mt_use=satin uColor=#332222 / >");
		pML->doTag("<mt_save_as=sleeper_top mt_use=sleeper_wall />");
		pML->doTag("<mt_save_as=fastener mt_use=sleeper_wall uColor=#555566 uTex2nm_use=tx1 />");
	}
	Rail::railModelStraightN = buildModelRailStraight(pML, Rail::railLenghtLong, 0);
	Rail::railModelShortN = buildModelRailStraight(pML, Rail::railLenghtShort, 0);
	Rail::railModelCurvedN = buildModelRailCurved(pML, 0);

	Rail::railModelStraightNsmall = buildModelRailStraight(pML, Rail::railLenghtLong, -1);
	Rail::railModelShortNsmall = buildModelRailStraight(pML, Rail::railLenghtShort, -1);
	Rail::railModelCurvedNsmall = buildModelRailCurved(pML, -1);

	Rail::railModelStraightNtiny = buildModelRailStraight(pML, Rail::railLenghtLong, -2);
	Rail::railModelShortNtiny = buildModelRailStraight(pML, Rail::railLenghtShort, -2);
	Rail::railModelCurvedNtiny = buildModelRailCurved(pML, -2);

	Rail::railModelGuideStraightN = buildModelRailGuideStraight(pML, Rail::railGuideLength, true);
	Rail::railModelGuideCurvedN = buildModelRailGuideCurved(pML, Rail::railGuideAngle, true);
	Rail::railModelGuideBedStraightN = buildModelRailGuideStraight(pML, Rail::railGuideLength, false);
	Rail::railModelGuideBedCurvedN = buildModelRailGuideCurved(pML, Rail::railGuideAngle, false);
	Rail::railModelGuideShortStraightN = buildModelRailGuideStraight(pML, Rail::railGuideShortLength, false);
	Rail::railModelGuideShortCurvedN = buildModelRailGuideCurved(pML, Rail::railGuideShortAngle, false);

	delete pML;


	Rail::couplerModelN = ModelLoader::loadModelStandard(&Rail::railModels, &table_drawJobs, &table_buffersIds, "/dt/md/rr/coupler/couplerlock01.txt");
	Rail::couplerShaftModelN = ModelLoader::loadModelStandard(&Rail::railModels, &table_drawJobs, &table_buffersIds, "/dt/md/rr/coupler/couplershaft01.txt");
	Rail::couplersCoupleModelN = ModelLoader::loadModelStandard(&Rail::railModels, &table_drawJobs, &table_buffersIds, "/dt/md/rr/coupler/couplerspair01.txt");

	//Rail::gangwayModelN = ModelLoader::loadModelStandard(&Rail::railModels, &table_drawJobs, &table_buffersIds, "/dt/md/rr/coupler/gangway01.txt");
	Rail::gangwaysCoupleModelN = ModelLoader::loadModelStandard(&Rail::railModels, &table_drawJobs, &table_buffersIds, "/dt/md/rr/coupler/gangwayscouple01.txt");
}
int RailMap45::buildModelRailStraight(ModelLoader* pML, float railLenght, int sizeMode) {
	ModelBuilder* pMB = pML->pModelBuilder;

	Rail* pR = new Rail();
	strcpy_s(pR->className, 32, "Rail");
	pR->ownCoords.pos[1] = Rail::railsLevel;
	pR->railType = 0;
	pR->railLength = railLenght;
	pR->chordaCollision.chordType = -1;
	pR->chordaScreen.chordType = -1;

	int railModelN = pML->addSubj(pR);

	pML->doTag("<group >"); {//entire section with sleepers
		buildModelRailPairSection(pML, railLenght,sizeMode);
		
		if (sizeMode<0) {
			//rail top wire
			if (sizeMode ==-2)
				pML->doTag("<mt_type=flat uColor=#cccccc lineWidth=1 lineWidthFixed=1 noShadow/>");
			else
				pML->doTag("<mt_use=rail_wire />");
			pML->doTag("<group >"); {
				pML->doTag("<short p0='0,0,-0.5' dz=1 />");
			}
			pMB->scaleGroup(pMB, 1, 1, railLenght);
			pMB->shiftGroup(pMB, Rail::railroadGauge / 2, 0, 0);
			pML->doTag("</group >");
			pML->doTag("<clone=reflect xyz='1,0,0'/>");
		}
		pML->doTag("<do='calc merge normals'/>");
		
		//sleepers
		int intervalsN = round(railLenght / Rail::railSleepersInterval);
		float sleepersInterval = railLenght / intervalsN;
		float sleeperZ = -railLenght / 2 + sleepersInterval / 2;
		pML->doTag("<group >"); {
			sleeperGroup(pML, sizeMode);
			pMB->shiftGroup(pMB, 0, 0, sleeperZ);
		}
		pML->doTag("</group >");
		while (1) {
			sleeperZ += sleepersInterval;
			if (sleeperZ >= railLenght / 2)
				break;
			pML->doTag("<clone>");
			pMB->shiftGroup(pMB, 0, 0, sleepersInterval);
			pML->doTag("</clone>");
		}
		
	}
	//whole rail section with sleepers
	pML->doTag("</group ay=90 >");
	pR->totalElements = pR->pSubjsSet->size() - railModelN;
	pR->totalNativeElements=pR->totalElements;
	return railModelN;
}
void RailMap45::buildModelRailPairSection(ModelLoader* pML, float railLenght, int sizeMode) {
	ModelBuilder* pMB = pML->pModelBuilder;
	pML->doTag("<vs=box-tank whl='20,6,2' />");
	float ext = Rail::railWidth * 0.3f;
	VirtualShape* pVS = pMB->pCurrentVShape;
	v3set(pVS->whl, Rail::railWidth-ext*2, Rail::railHeight-ext, railLenght);
	pVS->extU = ext;
	pVS->extL = ext;
	pVS->extR = ext;

	pML->doTag("<group >"); { //1 rails
		if (sizeMode >-1) {
			pML->doTag("<mt_use='rail_top' />");
			pML->doTag("<a='top all' />");
		}
		if (sizeMode > -2) {
			pML->doTag("<mt_use='rail_wall' />");
			pML->doTag("<a='left,right' xywh2nm='0.5,0,0,8' />");
			pMB->shiftGroup(pMB, 0, -(Rail::railHeight-ext)/2-ext, 0);
			pML->doTag("<do='calculate normals' />");
			pML->doTag("<do='merge normals' />");
		}
	}
	pMB->shiftGroup(pMB, Rail::railroadGauge/2, 0, 0);
	pML->doTag("</group >");
	pML->doTag("<clone >");
	pMB->shiftGroup(pMB, -Rail::railroadGauge, 0, 0);
	pML->doTag("</clone >");
}
int RailMap45::buildModelRailCurved(ModelLoader* pML, int sizeMode) {
	ModelBuilder* pMB = pML->pModelBuilder;

	Rail* pR = new Rail();
	strcpy_s(pR->className, 32, "Rail");
	pR->ownCoords.pos[1] = Rail::railsLevel;
	pR->railType = 1;
	pR->railLength = Rail::railLenghtCurved;
	pR->chordaCollision.chordType = -1;
	pR->chordaScreen.chordType = -1;
	int railModelN = pML->addSubj(pR);

	pMB->lockGroup(pMB); { //entire rail section
		float stepDg = 5;
		if (sizeMode<0) {
			//wire rails
			if (sizeMode ==-2)
				pML->doTag("<mt_type=flat uColor=#cccccc lineWidth=1 lineWidthFixed=1 noShadow/>");
			else
				pML->doTag("<mt_use=rail_wire />");
			//outer rail
			float r = Rail::curveRadius + Rail::railroadGauge / 2;
			pML->doTag("<line>"); //
			for (float aY = -22.5; aY <= 22.5; aY += stepDg) {
				float angleRd = (aY - 90.0) * degrees2radians;
				float kx = cosf(angleRd) * r;
				float kz = sinf(angleRd) * r;
				pMB->addVertex(pMB, kx, 0, kz);
			}
			pML->doTag("</line>");
			//inner rail
			r = Rail::curveRadius - Rail::railroadGauge / 2;
			pML->doTag("<line>"); //
			for (float aY = -22.5; aY <= 22.5; aY += stepDg) {
				float angleRd = (aY - 90.0) * degrees2radians;
				float kx = cosf(angleRd) * r;
				float kz = sinf(angleRd) * r;
				pMB->addVertex(pMB, kx, 0, kz);
			}
			pML->doTag("</line>");
		}
		if (sizeMode > -2) {
			//guideline
			pML->doTag("<mt_type=flat uColor=#00ff00 lineWidth=1 />");
			pML->doTag("<line mark='guideline1' dontRender >"); //
			for (float aY = -22.5; aY <= 22.5; aY += stepDg) {
				float angleRd = (aY - 90.0) * degrees2radians;
				float kx = cosf(angleRd) * Rail::curveRadius;
				float kz = sinf(angleRd) * Rail::curveRadius;
				pMB->addVertex(pMB, kx, 0, kz);
			}
			pML->doTag("</line>");
			//rails to clone
			pML->doTag("<group >"); {//rails pair section
				buildModelRailPairSection(pML, 1,sizeMode);
			}
			pML->doTag("<group2line='guideline1' />");
			//pML->doTag("<do='calculate normals'>");
			//pML->doTag("<do='merge normals'>");
			pML->doTag("</group >");
		}
		pML->doTag("<do='calc merge normals'/>");

		//sleepers
		int intervalsN = round(Rail::railLenghtCurved / Rail::railSleepersInterval);
		stepDg = 45.0f / intervalsN;
		pMB->lockGroup(pMB);
		sleeperGroup(pML,sizeMode);
		pMB->moveGroupDg(pMB, 0, 90, 0, 0, 0, -Rail::curveRadius);
		float nextDg = -22.5 + stepDg / 2;
		pMB->moveGroupDg(pMB, 0, nextDg, 0, 0, 0, 0);
		pMB->releaseGroup(pMB);

		while (1) {
			nextDg += stepDg;
			if ((nextDg >= 22.5))
				break;
			pML->doTag("<clone >");
			pMB->moveGroupDg(pMB, 0, stepDg, 0, 0, 0, 0);
			pML->doTag("</clone >");
		}

	}

	// move entire rail section
	pMB->moveGroupDg(pMB, 0, 0, 0, 0, 0, Rail::tileCenter2curveCenter);
	//pMB->moveGroupDg(pMB, 0, 90, 0, 0, 10, 0);
	//pML->doTag("<do scale='4,4,4'/>");
	pMB->releaseGroup(pMB);

	pR->totalElements = pR->pSubjsSet->size() - railModelN;
	pR->totalNativeElements = pR->totalElements;
	return railModelN;
}


void RailMap45::sleeperGroup(ModelLoader* pML,int sizeMode) {

	ModelBuilder* pMB = pML->pModelBuilder;
	pML->doTag("<vs=box whl='20,6,2' />");
	VirtualShape* pVS = pMB->pCurrentVShape;
	pML->doTag("<group >"); {
		v3set(pVS->whl, Rail::railSleeperLength, Rail::railSleeperHeight, Rail::railSleeperWidth);
		if (sizeMode > -2) {
			pML->doTag("<mt_use=sleeper_wall />");
			pML->doTag("<a='left,right,front,back' />");
		}
		pML->doTag("<mt_use=sleeper_top />");
		pML->doTag("<a=top xywh2nm='1,0.5,15,0' />");
		if (sizeMode > -2) {
			//fasteners
			pML->doTag("<group >"); {
				pML->doTag("<mt_use=fastener />");
				v3set(pVS->whl, Rail::fastenerWidth, 0, Rail::fastenerWidth);
				//fastener
				pML->doTag("<group >"); {
					pML->doTag("<a='top' xywh2nm='1,1,7,7' flip2nm=-90 />");
					pML->doTag("<do py=0.5 applyTo=left>");
					pML->doTag("<do='calculate normals'>");
					pMB->moveGroupDg(pMB, 0, 0, 0, Rail::fastenerWidth / 2 + Rail::railWidth / 2, 0.6, 0);
				}
				pML->doTag("</group>");
				pML->doTag("<clone=reflect xyz='1, 0, 0' />");
				pMB->moveGroupDg(pMB, 0, 0, 0, Rail::railroadGauge / 2, 0, 0);
			}
			pML->doTag("</group >");
			pML->doTag("<clone=reflect xyz='1, 0, 0' />");
		}
	}
	pMB->shiftGroup(pMB, 0, -Rail::railHeight - Rail::railSleeperHeight / 2, 0);
	pML->doTag("</group >");
}
int RailMap45::findClosestRail(RailCoord* pRC, RailMap45* pTable, Coords* pCoords) {
	std::vector<SceneSubj*>* pMap = &pTable->railsMap;
	float bestDistance = 1000000;
	int bestRailN = -1;
	Rail* pR = NULL;
	int railsN = pMap->size();
	for (int rN = 0; rN < railsN; rN++) {
		pR = (Rail*)pMap->at(rN);
		if (pR == NULL)
			continue;
		if (pR->railStatus != 0)
			continue;

		//check gabarites
		bool tooFar = false;
		for (int i = 0; i < 3; i += 2) {
			if (abs(pR->ownCoords.pos[i] - pCoords->pos[i]) >= bestDistance) {
				tooFar = true;
				break;
			}
		}
		if (tooFar)
			continue;
		//get precise distance
		float dist=pR->dist2point3d(pCoords->pos);
		if (bestDistance > dist) {
			bestDistance = dist;
			bestRailN = rN;
			if (bestDistance == 0)
				break;
		}
	}
	if (bestRailN < 0)
		return -1;
	if (pRC == NULL)
		return bestRailN;
	pRC->railN = bestRailN;
	pR = (Rail*)pMap->at(bestRailN);
	pRC->percentOnRail = 0.5;
	//aligned?
	if(abs(angleDgFromTo(pCoords->getEulerDg(1),pR->yawInOut[1])) <= 90)
		pRC->alignedWithRail = 1;
	else
		pRC->alignedWithRail = -1;
	fillRailCoordsFromPercent(pRC, pR);
	/*
	//dismiss previously selected rails
	for (int rN = 0; rN < railsN; rN++) {
		Rail* pR0 = (Rail*)pMap->at(rN);
		if (pR0 == NULL)
			continue;
		pR0->uFogLevel = 0;
	}
	pR->setFog(0.7, MyColor::getUint32(1.0f,1.0f,1.0f));

	mylog("bestRailN=%d at %dx%d, chorde %dx%d=>%dx%d bestDistance=%d\n", bestRailN, (int)pR->ownCoords.pos[0], (int)pR->ownCoords.pos[2], 
		(int)pR->line2d.p0[0], (int)pR->line2d.p0[1],
		(int)pR->line2d.p1[0], (int)pR->line2d.p1[1],
		(int)bestDistance);
		*/
	return bestRailN;
}


int RailMap45::addNewRail(float idxX, float idxZ, float yawIn, float yawOut, int railStatus) {
	yawIn = angleDgNorm360(yawIn);
	yawOut = angleDgNorm360(yawOut);
	if (yawIn > yawOut)
		std::swap(yawIn, yawOut);
	Rail* pR = getRail(idxX, idxZ, yawIn, yawOut);
	if (pR != NULL) {
		if (pR->railStatus == railStatus)
			return pR->nInSubjsSet;
		if (pR->railStatus == 0)
			return pR->nInSubjsSet;
		//it's a virtual rail
		if (railStatus == 0) {
			//free slot
			railsMap.at(pR->nInSubjsSet) = NULL;
			delete pR;
		}
	}
	if (!approveRail(idxX, idxZ, yawIn, yawOut, railStatus))
		return 0;
	if (railStatus == 0) { //normal real rail
		if (abs(angleDgFromTo(yawIn, yawOut)) == 135) //curve
			pR = new Rail((Rail*)Rail::railModels.at(Rail::railModelCurvedN));
		else {//straight
			if (idxX == (int)idxX) //short straight
				pR = new Rail((Rail*)Rail::railModels.at(Rail::railModelShortN));
			else //long straight
				pR = new Rail((Rail*)Rail::railModels.at(Rail::railModelStraightN));
		}
	}
	else if (railStatus == 1) { //virtual rail
		if (abs(angleDgFromTo(yawIn, yawOut)) == 135) //curve
			pR = new Rail((Rail*)Rail::railModels.at(Rail::railModelCurvedVirtualN));
		else {//straight
			if (idxX == (int)idxX) //short straight
				pR = new Rail((Rail*)Rail::railModels.at(Rail::railModelShortVirtualN));
			else //long straight
				pR = new Rail((Rail*)Rail::railModels.at(Rail::railModelStraightVirtualN));
		}
		pR->ownCoords.pos[1] = theApp.gameTable.groundLevel0 + 1;// 0.5;
	}
	pR->railStatus = railStatus;
	pR->refModelN = pR->nInSubjsSet;
	pR->pSubjsSet = &railsMap;
	pR->yawInOut[0] = yawIn;
	pR->yawInOut[1] = yawOut;
	pR->tileIndexXZ[0] = idxX;
	pR->tileIndexXZ[1] = idxZ;
	if (pR->railType == 1) //curve
		pR->CCW = signOf(angleDgFromTo(pR->yawInOut[1], pR->yawInOut[0]));

	placeAtEmptySlot(pR);
	pR->rootN = pR->nInSubjsSet;

	indexes2coords(pR);

	if (pR->railStatus == 0) {
		fillRailEnd(pR, 0);
		fillRailEnd(pR, 1);
		checkRailCrossing(pR);
	}
	else {//virtual rail
		fillRailEnd(pR, 0);
		fillRailEnd(pR, 1);
		if (pR->railEnd[0].toRailN >= 0 && pR->railEnd[1].toRailN >= 0)
			pR->priority = 1;
		else
			pR->priority = 0;
	}

	pR->initGabarites(pR);

	return pR->nInSubjsSet;
}
int RailMap45::indexes2coords(Rail* pR){

	float modelYaw = (pR->yawInOut[1] + pR->yawInOut[0]) / 2;
	if (angleDgFromTo(pR->yawInOut[0], modelYaw) > 90)
		modelYaw = angleDgNorm360(modelYaw - 180);
	pR->ownCoords.setEulerDg(0, modelYaw, 0);

	pR->ownCoords.pos[0] = tileSize * pR->tileIndexXZ[0] - worldBox.bbRad[0];
	pR->ownCoords.pos[2] = tileSize * pR->tileIndexXZ[1] - worldBox.bbRad[2];

	pR->buildModelMatrix();
	pR->initGabarites(pR);

	//set in and out points
	if (pR->railType == 0) {//straight
		float vIn[4] = { 0,0,0,0 };
		vIn[0] = -pR->railLength / 2;
		mat4x4_mul_vec4plus(pR->p0, pR->ownModelMatrixUnscaled, vIn, 1, true);
		vIn[0] = pR->railLength / 2;
		mat4x4_mul_vec4plus(pR->p1, pR->ownModelMatrixUnscaled, vIn, 1, true);
		LineXY::initLineXZ(&pR->line2d, pR->p0, pR->p1);
	}
	else{//curve
		float angle00, a0, a1;
		float vIn[4] = { 0,0,0,0 };
		vIn[2] = Rail::tileCenter2curveCenter;
		mat4x4_mul_vec4plus(pR->curveCenter, pR->ownModelMatrixUnscaled, vIn, 1, true);
		angle00 = angleDgNorm180(modelYaw + 180.0);
		a0 = angle00 - 22.5 * pR->CCW;
		float angleRd = a0 * degrees2radians;
		pR->p0[0] = pR->curveCenter[0] + sin(angleRd) * Rail::curveRadius;
		pR->p0[1] = pR->ownCoords.pos[1];
		pR->p0[2] = pR->curveCenter[2] + cos(angleRd) * Rail::curveRadius;
		a1 = angle00 + 22.5 * pR->CCW;
		angleRd = a1 * degrees2radians;
		pR->p1[0] = pR->curveCenter[0] + sin(angleRd) * Rail::curveRadius;
		pR->p1[1] = pR->ownCoords.pos[1];
		pR->p1[2] = pR->curveCenter[2] + cos(angleRd) * Rail::curveRadius;
		//radiusAngles
		pR->radiusAngle[0] = a0;
		pR->radiusAngle[1] = a1;
		ArcXY::initArcXY(&pR->arc2d, pR->curveCenter[0], pR->curveCenter[2], Rail::curveRadius, pR->radiusAngle[0], pR->radiusAngle[1], pR->CCW);
		LineXY::initLineXZ(&pR->line2d, pR->p0, pR->p1);
	}
	if (pR->crossingRailN >= 0) {
		//has crossing point - re-check coords
		RailCoord rc;
		rc.railN = pR->nInSubjsSet;
		rc.percentOnRail = pR->crossingPercent;
		rc.alignedWithRail = -pR->CCW;
		fillRailCoordsFromPercent(&rc, pR);
		pR->crossingYaw = rc.currentYaw;
		v3copy(pR->crossingPoint, rc.xyz);
	}
	return 1;
}


int RailMap45::getRailN(float idxX, float idxZ, float yawIn, float yawOut) {
	float idxXZ[2];
	idxXZ[0] = idxX;
	idxXZ[1] = idxZ;
	float yawInOut[2];
	yawIn = angleDgNorm360(yawIn);
	yawOut = angleDgNorm360(yawOut);
	if (yawIn > yawOut)
		std::swap(yawIn, yawOut);
	yawInOut[0] = yawIn;
	yawInOut[1] = yawOut;
	//find
	int railsN = railsMap.size();
	int railsInTile = 0;
	for (int rN = 0; rN < railsN; rN++) {
		Rail* pR0 = (Rail*)railsMap.at(rN);
		if (pR0 == NULL)
			continue;
		if (!v2match(pR0->tileIndexXZ, idxXZ))
			continue;
		if (!v2match(pR0->yawInOut, yawInOut))
			continue;
		return rN;
	}
	return -1;
}

Rail* RailMap45::getRail(float idxX, float idxZ, float yawIn, float yawOut) {
	int railN = getRailN(idxX, idxZ, yawIn, yawOut);
	if (railN < 0)
		return NULL;
	return (Rail*)railsMap.at(railN);
}
int RailMap45::setRail(float idxX, float idxZ, float yawIn, float yawOut, int railStatus) {
	int railN00 = addNewRail(idxX, idxZ, yawIn, yawOut, railStatus);
	if (railStatus == 0) {
		addDiagonalShortTo(idxX, idxZ, yawIn, railStatus);
		addDiagonalShortTo(idxX, idxZ, yawOut, railStatus);
	}

	check2extendTable(railN00);

	return railN00;
}
int RailMap45::addDiagonalShortTo(float idxX, float idxZ, float yaw2extend, int railStatus) {
	yaw2extend = angleDgNorm360(yaw2extend);
	if (idxX == (int)idxX) //short itself
		return -1;
	if (yaw2extend == 45) {
		idxX += 0.5;
		idxZ += 0.5;
	}
	else if (yaw2extend == 135) {
		idxX += 0.5;
		idxZ -= 0.5;
	}
	else if (yaw2extend == 225) {
		idxX -= 0.5;
		idxZ -= 0.5;
	}
	else if (yaw2extend == 315) {
		idxX -= 0.5;
		idxZ += 0.5;
	}
	else
		return -1; //not diagonal

	return addNewRail(idxX, idxZ, yaw2extend, yaw2extend - 180, railStatus);
}
int RailMap45::checkRailCrossing(Rail* pR) {
	//check if switch or crossing
	pR->switchEndN = -1;
	pR->railEnd[0].isSwitch = 0;
	pR->railEnd[1].isSwitch = 0;
	pR->crossingRailN = -1;
	int railsN = railsMap.size();
	for (int rN = 0; rN < railsN; rN++) {
		Rail* pR2 = (Rail*)railsMap.at(rN);
		if (pR2 == NULL)
			continue;
		if (rN == pR->nInSubjsSet)
			continue;
		if (pR2->railStatus != 0)
			continue;
		if (!v2match(pR2->tileIndexXZ, pR->tileIndexXZ))
			continue;
		//same tile rail found
		int endN = -1;
		int endN2 = -1;
		for (int i0 = 0; i0 < 2; i0++)
			for (int i2 = 0; i2 < 2; i2++)
				if (pR->railEnd[i0].endYaw == pR2->railEnd[i2].endYaw) {
					//switch
					RailEnd* pRE = &pR->railEnd[i0];
					pRE->isSwitch = true;
					pRE->isOn = false;
					pRE->altRailN = rN;
					pRE->altRailEndN = i2;
					pR->switchEndN = i0;
					//alt rail
					pRE = &pR2->railEnd[i2];
					pRE->isSwitch = true;
					pRE->isOn = true;
					pRE->altRailN = pR->nInSubjsSet;
					pRE->altRailEndN = i0;
					pR2->switchEndN = i2;
					return 1;
				}
		//crossing?
		fillCrossing(pR, pR2);
		return 1;
	}
	return 0;
}
void RailMap45::fillRailCoordsFromPercent(RailCoord* pRC, Rail* pR) {
	//find position and yaw on the rail
	pRC->xyz[1] = pR->ownCoords.pos[1];
	if (pR->railType == 0) {
		//straight
		pRC->currentYaw = pR->yawInOut[1];
		for (int i = 0; i < 3; i += 2)
			pRC->xyz[i] = pR->p0[i] * (1.0 - pRC->percentOnRail) + pR->p1[i] * pRC->percentOnRail;
	}
	else {//curve
		pRC->currentYaw = angleDgNorm180(pR->yawInOut[0] + 180 + 45.0 * pRC->percentOnRail * pR->CCW);
		float radAngleDg = pR->radiusAngle[0] + 45.0 * pRC->percentOnRail * pR->CCW;
		float radAngleRd = radAngleDg * degrees2radians;
		pRC->xyz[0] = sin(radAngleRd) * pR->curveRadius;
		pRC->xyz[2] = cos(radAngleRd) * pR->curveRadius;
		for (int i = 0; i < 3; i += 2)
			pRC->xyz[i] += pR->curveCenter[i];
	}
	if (pRC->alignedWithRail < 0)
		pRC->currentYaw = angleDgNorm180(pRC->currentYaw + 180.0);
}

int RailMap45::shiftRailCoord(RailCoord* pRCout, RailCoord* pRCin, RailMap45* pMap, float shiftZ,bool defacto) {
	memcpy(pRCout, pRCin, sizeof(RailCoord));
	return shiftRailCoord00(pRCout, pMap, shiftZ, defacto);
}
float RailMap45::offset2railCoord(RailCoord* pRC1, RailCoord* pRC2, RailMap45* pMap, float distLimit) {
	RailCoord rc1;
	memcpy(&rc1, pRC1, sizeof(RailCoord));
	float distChecked = 0;
	while (distChecked < distLimit) {
		Rail* pR = (Rail*)pMap->railsMap.at(rc1.railN);
		if (rc1.railN == pRC2->railN) {
			//same rail
			float percentD = pRC2->percentOnRail - rc1.percentOnRail;
			if (rc1.alignedWithRail<0)
				percentD = -percentD;
			if (percentD < 0)
				return 1000000;//overlap
			float dist = pR->railLength* percentD;
			distChecked += dist;

			if (rc1.alignedWithRail == pRC2->alignedWithRail)
				return 1000000; //wrong direction

			return distChecked;
		}
		//if here-move to next rail
		float railLeft = 0;
		if (rc1.alignedWithRail > 0) //moving to rail end
			railLeft = pR->railLength * (1.0 - rc1.percentOnRail);
		else //moving to rail start
			railLeft = pR->railLength * rc1.percentOnRail;
		distChecked += railLeft;
		int whichEnd0 = 0;
		if (rc1.alignedWithRail > 0) //moved to end
			whichEnd0 = 1;
		RailEnd* pRE0 = &pR->railEnd[whichEnd0];
		if (pRE0->toRailN < 0)
			return 1000000;//no rail ahead
		rc1.railN = pRE0->toRailN;
		pR = (Rail*)pMap->railsMap.at(rc1.railN);
		int whichEnd2 = pRE0->toRailEndN;
		RailEnd* pRE2 = &pR->railEnd[whichEnd2];
		if (pRE2->isSwitch && !pRE2->isOn) {
			rc1.railN = pRE2->altRailN;
			pR = (Rail*)pMap->railsMap.at(rc1.railN);
			whichEnd2 = pRE2->altRailEndN;
			RailEnd* pRE2 = &pR->railEnd[whichEnd2];
		}
		if (whichEnd0 == whichEnd2) {
			rc1.alignedWithRail *= -1.0;
		}
		if (whichEnd2 == 0)
			rc1.percentOnRail = 0.0;
		else
			rc1.percentOnRail = 1.0;
	}
	return distChecked;
}


int RailMap45::reInitLayout(RailMap45* pMap, std::vector<SceneSubj*>* pSubjs) {
	float maxSectionLength = 0;
	if (pSubjs != NULL)
		for (int i = pSubjs->size() - 1; i >= 0; i--) {
			SceneSubj* pSS = pSubjs->at(i);
			if (pSS == NULL)
				continue;
			if (pSS->d2parent != 0)
				continue;
			if (strstr(pSS->className, "RollingStock") != pSS->className)
				continue;
			RollingStock* pRS = (RollingStock*)pSS;
			if (maxSectionLength < pRS->maxSectionLength)
				maxSectionLength = pRS->maxSectionLength;
		}
	float tileSize = maxSectionLength;// *0.9; //36 minimum, 100-max
		tileSize = fmax(tileSize, 36);
		tileSize = fmin(tileSize, 130);
	return reInitLayout(pMap, tileSize,false);
}

int RailMap45::reInitLayout(RailMap45* pMap, float tileSize, bool tilesNchanged) {
	tileSize = (float)fmax(tileSize, 36); //36 minimum
	tileSize = round(tileSize / 5) * 5;

	if (tileSize == pMap->tileSize && !tilesNchanged)
		return 0;

	/*
	RollingStock::maxTrainSpeed = tileSize * 0.01;
	RollingStock::maxDragSpeed = RollingStock::maxTrainSpeed *3;
	RollingStock::accelerationLinActive = RollingStock::maxTrainSpeed * 0.015;
	RollingStock::accelerationLinPassive = RollingStock::accelerationLinActive / 3;
	RollingStock::accelerationOnDrag = RollingStock::accelerationLinActive *3;
	*/
	RollingStock::resetMaxSpeed();

	mylog("%d tileSize=%.2f maxTrainSpeed=%.2f\n", theApp.frameN, tileSize, RollingStock::maxTrainSpeed);

	MySound::soundClose = tileSize * 2;
	MySound::soundFar = tileSize * 12;
	MySound::soundRange = MySound::soundFar-MySound::soundClose;

	Rail::cleanUp();
	pMap->cleanUpTable();
	pMap->initTable(tileSize, 100, 20, pMap->tableTiles[0], pMap->tableTiles[1]);
	pMap->initRailModels();
	pMap->initRailModelsVirtual();


	std::vector<SceneSubj*>* pSubjs = &pMap->railsMap;
	for (int i = pSubjs->size() - 1; i >= 0; i--) {
		SceneSubj* pSS = pSubjs->at(i);
		if (pSS == NULL)
			continue;
		if (strcmp(pSS->className, "Rail") != 0)
			continue;
		Rail* pR = (Rail*)pSS;

		Rail* pR0 = (Rail*)Rail::railModels.at(pR->refModelN);
		pR->railLength = pR0->railLength;
		pR->djStartN = pR0->djStartN;
		pR->djTotalN = pR0->djTotalN;
		memcpy(&pR->gabaritesOnLoad, &pR0->gabaritesOnLoad, sizeof(Gabarites));
		pMap->indexes2coords(pR);

		pR->initGabarites(pR);
	}
	
	return 1;
}
int RailMap45::onClick() {
	if (theApp.cameraMan.zoom != 0)
		theApp.cameraMan.setView(false);
	else
		stopAllTrains();
	return 1;
}
int RailMap45::stopAllTrains(){
	theApp.trainIsReady = true;
	//stop all trains
	int hadMoving = 0;
	std::vector<SceneSubj*>* pSubjs=&theApp.sceneSubjs;
	for (int sN = pSubjs->size() - 1; sN >= 0; sN--) {
		SceneSubj* pSS = pSubjs->at(sN);
		if (pSS == NULL)
			continue;
		if (pSS->d2parent != 0)
			continue;
		if (strstr(pSS->className, "RollingStock") == NULL)
			continue;
		if (pSS->desirableZdir != 0)
			hadMoving = 1;
		pSS->desirableZdir = 0;
		pSS->desirableZspeed = 0;
	}
	if (hadMoving) {
		MeltingSign::addMeltingSign("/dt/ui/signs/stop.png", TouchScreen::cursorPos[0], TouchScreen::cursorPos[1]);
	}
	return hadMoving;
}
void RailMap45::removeVirtualRails() {
	for (int rN = railsMap.size() - 1; rN >= 0; rN--) {
		Rail* pR = (Rail*)railsMap.at(rN);
		if (pR == NULL)
			continue;
		if (pR->railStatus == 0)
			continue; //normal real rail
		delete pR;
		railsMap.at(rN) = NULL;
	}
}

void RailMap45::addVirtualRails() {
	removeVirtualRails();

	if (theApp.bEditMode == false)
		return;

	for (int rN = railsMap.size() - 1; rN >= 0; rN--) {
		Rail* pR = (Rail*)railsMap.at(rN);
		if (pR == NULL)
			continue;
		if (pR->railStatus != 0)
			continue; //not a real rail
		addVirtualRailsToEnd(pR,0);
		addVirtualRailsToEnd(pR,1);
	}

	addSnowflakes();
}
void RailMap45::addSnowflakes() {
	unsigned int color32 = MyColor::getUint32(1.0f, 1.0f, 1.0f);
	float y0 = theApp.gameTable.groundLevel0 + 0.5;
	for (int zN = 0; zN < tableTiles[1]; zN++) {
		float z0 = tileSize * zN - worldBox.bbRad[2] + tileSize / 2;// +dz;
		if (abs(z0) > worldBox.bbRad[2] - tileSize / 2)
			continue;
		for (int xN = 0; xN < tableTiles[0]; xN++) {
			float x0 = tileSize * xN - worldBox.bbRad[0] + tileSize / 2;// +dx;
			if (abs(x0) > worldBox.bbRad[0] - tileSize / 2)
				continue;
			Rail* pSS = new Rail((Rail*)Rail::railModels.at(Rail::railModelSnowFlakeN));
			pSS->refModelN = pSS->nInSubjsSet;

			pSS->tileIndexXZ[0] = xN + 0.5;
			pSS->tileIndexXZ[1] = zN + 0.5;
			pSS->pSubjsSet = &railsMap; //which vector/set this subj belongs to
			placeAtEmptySlot(pSS);
			v3set(pSS->ownCoords.pos, x0, y0, z0);
			pSS->buildModelMatrix();
			pSS->initGabarites(pSS);

			//pSS->uHighLightLevel = 1.0;
			//pSS->uHighLightColor.setUint32(color32);
		}
	}
	//crossings
	for (int zN = 0; zN < tableTiles[1]; zN++) {
		float z0 = tileSize * zN - worldBox.bbRad[2];
		if (abs(z0) >= worldBox.bbRad[2] - tileSize / 2)
			continue;
		for (int xN = 0; xN < tableTiles[0]; xN++) {
			float x0 = tileSize * xN - worldBox.bbRad[0];
			if (abs(x0) >= worldBox.bbRad[0] - tileSize / 2)
				continue;
			//initVirtualTile(x0, z0);
			Rail* pSS = new Rail((Rail*)Rail::railModels.at(Rail::railModelSnowFlakeSmallN));
			pSS->refModelN = pSS->nInSubjsSet;

			pSS->tileIndexXZ[0] = xN;// +0.5;
			pSS->tileIndexXZ[1] = zN;// +0.5;
			railsMap.push_back(pSS);
			pSS->nInSubjsSet = railsMap.size() - 1;
			pSS->pSubjsSet = &railsMap; //which vector/set this subj belongs to
			v3set(pSS->ownCoords.pos, x0, y0, z0);
			pSS->buildModelMatrix();
			pSS->initGabarites(pSS);
		}
	}
}
int RailMap45::addVirtualRailsToEnd(Rail* pR,int endN) {
	RailEnd* pEnd = &pR->railEnd[endN];
	//check if pR->toTileIndexXZ[] -s out of limits
	float yaw0 = angleDgNorm360(pEnd->endYaw + 180);
	float yaw2 = pEnd->endYaw;
	bool toShortDigonal = false;
	if (pR->railLength != Rail::railLenghtShort) {
		//normal rail (not short diagonal
		if ((int)pEnd->endYaw % 90 != 0)
			toShortDigonal = true;
	}
	if (toShortDigonal) {
		addNewRail(pEnd->toTileIndexXZ[0], pEnd->toTileIndexXZ[1], yaw0, yaw2, 1);
		return 1;
	}
	//normal case
	if (countRailsInTile(pEnd->toTileIndexXZ[0], pEnd->toTileIndexXZ[1], true) > 1)
		return 0;
	for(float yawD=-45;yawD<=45;yawD+=45)
		addNewRail(pEnd->toTileIndexXZ[0], pEnd->toTileIndexXZ[1], yaw0, yaw2+yawD, 1);

	return 1;
}
int RailMap45::countRailsInTile(float idxX, float idxZ, bool realRailsOnly) {
	int railsN = 0;
	for (int rN = railsMap.size() - 1; rN >= 0; rN--) {
		Rail* pR = (Rail*)railsMap.at(rN);
		if (pR == NULL)
			continue;
		if (pR->railStatus < 0)
			continue;
		if (pR->tileIndexXZ[0] != idxX)
			continue;
		if (pR->tileIndexXZ[1] != idxZ)
			continue;
		if (realRailsOnly && pR->railStatus != 0)
			continue;
		railsN++;
	}
	/*
	//debug
	if (railsN > 1)
		int a = 0;
	*/
	return railsN;
}
int RailMap45::placeAtEmptySlot(Rail* pR) {
	//find empty slot
	int railsN = railsMap.size();
	for (int rN = 0; rN <= railsN; rN++) {
		if (rN == railsN) {
			railsMap.push_back(pR);
			pR->nInSubjsSet = railsN;
			break;
		}
		Rail* pR0 = (Rail*)railsMap.at(rN);
		if (pR0 == NULL) {
			pR->nInSubjsSet = rN;
			railsMap.at(rN) = pR;
			break;
		}
	}
	return pR->nInSubjsSet;
}
int RailMap45::fillCrossing(Rail* pR, Rail* pR2) {
	pR->crossingRailN = pR2->nInSubjsSet;
	pR2->crossingRailN = pR->nInSubjsSet;
	//crossing coords
	float vCross[2];
	if (pR->railType == 0) { //pR - straight
		if (pR2->railType == 0) { //pR2 - straight
			//both straight
			if (LineXY::lineSegmentsIntersectionXY(vCross, &pR->line2d, &pR2->line2d) == 0)
				return 0;
		}
		else { //pR2-curved
			//straight to arc
			float vCrosses[2][2];
			int pointsN=ArcXY::arcCrossLineSegment(vCrosses, &pR2->arc2d, &pR->line2d);
			if (pointsN == 1)
				v2copy(vCross, vCrosses[0]);
			else
				return 0;

			//debug
			ArcXY* pA = &pR2->arc2d;
			mylog("%d RailMap45::fillCrossing: arc2Center %d x %d, p0 %d x %d, p1 %d x %d, pointsN=%d, vCross %d x %d\n",
				theApp.frameN, (int)pA->centerPos[0], (int)pA->centerPos[1],
				(int)pA->p0[0], (int)pA->p0[1], (int)pA->p1[0], (int)pA->p1[1],
				pointsN, (int)vCross[0], (int)vCross[1]);

		}
		int a = 0;
	}
	else { //pR-curved
		if (pR2->railType == 0) { //pR2 - straight
			//arc to straight
			float vCrosses[2][2];
			int pointsN = ArcXY::arcCrossLineSegment(vCrosses, &pR->arc2d, &pR2->line2d);
			if (pointsN == 1)
				v2copy(vCross, vCrosses[0]);
			else
				return 0;

			//debug
			ArcXY* pA = &pR->arc2d;
			mylog("%d RailMap45::fillCrossing: arcCenter %d x %d, p0 %d x %d, p1 %d x %d, pointsN=%d, vCross %d x %d\n",
				theApp.frameN, (int)pA->centerPos[0], (int)pA->centerPos[1],
				(int)pA->p0[0], (int)pA->p0[1], (int)pA->p1[0], (int)pA->p1[1],
				pointsN, (int)vCross[0], (int)vCross[1]);
		}
		else { //pR2-curved
			//both curved
			float vCrosses[2][2];
			int pointsN = ArcXY::arcCrossArc(vCrosses, &pR->arc2d, &pR2->arc2d);
			if (pointsN == 1)
				v2copy(vCross, vCrosses[0]);
			else
				return 0;
		}

	}
	pR->crossingPoint[0] = vCross[0];
	pR->crossingPoint[1] = pR->ownCoords.pos[1];
	pR->crossingPoint[2] = vCross[1];
	v3copy(pR2->crossingPoint,pR->crossingPoint);
	fillRailCrossPercentAndYaw(pR, pR->crossingPoint);
	fillRailCrossPercentAndYaw(pR2, pR->crossingPoint);

	return 1;
}
void RailMap45::fillRailCrossPercentAndYaw(Rail* pR, float* vCross) {
	float vIn[2];
	vIn[0] = vCross[0];
	vIn[1] = vCross[2];
	//find percent
	if (pR->railType == 0) //straignt
		pR->crossingPercent = LineXY::segmentPercent(&pR->line2d, vIn);
	else //curved
		pR->crossingPercent = ArcXY::arcPercent(&pR->arc2d, vIn);
	//find yaw
	RailCoord rc;
	rc.railN = pR->nInSubjsSet;
	rc.percentOnRail = pR->crossingPercent;
	rc.alignedWithRail = -pR->CCW;
	fillRailCoordsFromPercent(&rc, pR);
	pR->crossingYaw = rc.currentYaw;
	v3copy(pR->crossingPoint, rc.xyz);
	/*
	//debug
	if (pR->railType == 1) {//curved
		ArcXY* pA = &pR->arc2d;
		mylog("%d RailMap45::fillRailCrossPercentAndYaw: arcCenter %d x %d, p0 %d x %d, p1 %d x %d, vCross %d x %d\n",
			theApp.frameN, (int)pA->centerPos[0], (int)pA->centerPos[1], 
			(int)pA->p0[0], (int)pA->p0[1], (int)pA->p1[0], (int)pA->p1[1],
			(int)vIn[0], (int)vIn[1]);
	}
	*/
}
int RailMap45::buildModelRailGuideStraight(ModelLoader* pML, float length, bool withRails) {
	ModelBuilder* pMB = pML->pModelBuilder;
	SceneSubj* pR = new SceneSubj();
	pR->ownCoords.pos[1] = Rail::railsLevel;
	int modelN = pML->addSubj(pR);

	pML->doTag("<group >"); {//entire section
		if (withRails) {
			pML->doTag("<group >"); { //1 rails
				//pML->doTag("<mt_type=flat uColor=#eeeeee lineWidth=0.5 lineWidthFixed=2 />");
				pML->doTag("<mt_use=guide-wire />");
				pML->doTag("<short p0='0,0,-0.5' dz=1 />");
				Vertex01* pV = pMB->vertices.at(pMB->vertices.size() - 2);
				pV->aPos[2] = -length / 2;
				pV = pMB->vertices.at(pMB->vertices.size() - 1);
				pV->aPos[2] = length / 2;
			}
			float shiftX = Rail::railGuideGaugeWidth / 2;
			pMB->shiftGroup(pMB, shiftX, 0, 0);
			pML->doTag("</group >");
			pML->doTag("<clone=reflectX />");
		}
		//bed
		pML->doTag("<mt_use=guide-bed />");// sleeper_wall uSpecularIntencity = 0.2 noShadow / > ");
		pML->doTag("<group >"); {
			pML->doTag("<box='1,1,3' />");
			VirtualShape* pVS = pMB->pCurrentVShape;
			v3set(pVS->whl, Rail::railroadGauge - Rail::railWidth, 0, length);
			pML->doTag("<a=top />");
			pMB->shiftGroup(pMB, 0, Rail::railGuideBedDy, 0);
		}
		pML->doTag("</group >");

	}
	//whole section
	pML->doTag("</group >");
	pR->totalElements = 1;
	pR->totalNativeElements = pR->totalElements;
	return modelN;
}

int RailMap45::buildModelRailGuideCurved(ModelLoader* pML, float angle, bool withRails) {
	ModelBuilder* pMB = pML->pModelBuilder;
	SceneSubj* pR = new SceneSubj();
	pR->ownCoords.pos[1] = Rail::railsLevel;
	int modelN = pML->addSubj(pR);

	float stepDg = angle / 4;
	pMB->lockGroup(pMB); { //entire section
		if (withRails) {
			//pML->doTag("<mt_type=flat uColor=#eeeeee lineWidth=0.5 lineWidthFixed=2 />");
			pML->doTag("<mt_use=guide-wire />");
			//outer line
			pML->doTag("<line >");
			float r = Rail::curveRadius + Rail::railGuideGaugeWidth / 2;
			for (float aY = -angle / 2; aY <= angle / 2; aY += stepDg) {
				float angleRd = (aY - 90.0) * degrees2radians;
				float kx = cosf(angleRd) * r;
				float kz = sinf(angleRd) * r;
				pMB->addVertex(pMB, kx, 0, kz);
			}
			pML->doTag("</line>");
			//inner line
			pML->doTag("<line >");
			r = Rail::curveRadius - Rail::railGuideGaugeWidth / 2;
			for (float aY = -angle / 2; aY <= angle / 2; aY += stepDg) {
				float angleRd = (aY - 90.0) * degrees2radians;
				float kx = cosf(angleRd) * r;
				float kz = sinf(angleRd) * r;
				pMB->addVertex(pMB, kx, 0, kz);
			}
			pML->doTag("</line>");
		}
		//bed
		pML->doTag("<mt_type=flat uColor=#00ff00 lineWidth=1 />");
		pML->doTag("<line mark='guideline1' dontRender >"); //
		for (float aY = -angle/2; aY <= angle/2; aY += stepDg) {
			float angleRd = (aY - 90.0) * degrees2radians;
			float kx = cosf(angleRd) * Rail::curveRadius;
			float kz = sinf(angleRd) * Rail::curveRadius;
			pMB->addVertex(pMB, kx, 0, kz);
		}
		pML->doTag("</line>");
		//section to clone
		pML->doTag("<group >"); {//bed section
			//pML->doTag("<mt_type=phong uColor=#333333 uSpecularIntencity=0.2 noShadow/>");
			pML->doTag("<mt_use=guide-bed />");// sleeper_wall uSpecularIntencity = 0.2 noShadow / > ");
			pML->doTag("<group >"); {
				pML->doTag("<box='1,1,3' />");
				VirtualShape* pVS = pMB->pCurrentVShape;
				v3set(pVS->whl, Rail::railroadGauge-Rail::railWidth, 0, 1);
				pML->doTag("<a=top />");
				pMB->shiftGroup(pMB, 0, Rail::railGuideBedDy, 0);
			}
			pML->doTag("</group >");
		}
		pML->doTag("<group2line='guideline1' />");
		pML->doTag("</group >");
	}

	// move entire rail section
	pMB->shiftGroup(pMB, 0, 0, Rail::curveRadius);
	pMB->rotateGroupDg(pMB, 0, -90, 0);
	pMB->releaseGroup(pMB);

	pR->totalElements = 1;
	pR->totalNativeElements = pR->totalElements;
	return modelN;
}
int RailMap45::check2extendTable(int railN) {
	Rail* pR = (Rail*)railsMap.at(railN);
	if (pR->railStatus != 0)
		return 0;
	bool frontier = false;
	for (int i = 0; i < 2; i++) {
		if (pR->tileIndexXZ[i] < 1) {
			frontier = true;
			break;
		}
		if (pR->tileIndexXZ[i] > tableTiles[i]-1) {
			frontier = true;
			break;
		}
	}
	if (!frontier)
		return 0;
	//frontier tile
	int extendTo[2] = { 0,0 };
	for (int eN = 0; eN < 2; eN++) {
		RailEnd* pRE = &pR->railEnd[eN];
		for (int i = 0; i < 2; i++) {
			if (pRE->toTileIndexXZ[i] <= 0)
				extendTo[i] = -1;
			else if (pRE->toTileIndexXZ[i] >= tableTiles[i])
				extendTo[i] = 1;
		}
	}
	if (v2equals(extendTo, 0))
		return 0;
	//if here-need to extend
	bool need2reindex = false;
	for (int i = 0; i < 2; i++) {
		if (extendTo[i] != 0) {
			tableTiles[i]++;
			if (extendTo[i] < 0) {
				need2reindex = true;
				startTileColorN = (startTileColorN + 1) % 2;
			}
		}
	}
	//reindex rails?
	removeVirtualRails();
	if (need2reindex) {
		//reindex rails
		for (int rN = railsMap.size() - 1; rN >= 0; rN--) {
			pR = (Rail*)railsMap.at(rN);
			if (pR == NULL)
				continue;
			for (int i = 0; i < 2; i++)
				if (extendTo[i] < 0)
					pR->tileIndexXZ[i]++;
			for (int eN = 0; eN < 2; eN++) {
				RailEnd* pRE = &pR->railEnd[eN];
				for (int i = 0; i < 2; i++)
					if (extendTo[i] < 0)
						pRE->toTileIndexXZ[i]++;				
			}
		}	
	}

	reInitLayout(this, tileSize, true);

	//recheck rails gabarites
	for (int rN = railsMap.size() - 1; rN >= 0; rN--) {
		pR = (Rail*)railsMap.at(rN);
		if (pR == NULL)
			continue;
		indexes2coords(pR);
	}
	//adjust static objs and camera
	float shift[3] = { 0,0,0 };
	if (extendTo[0] != 0)
		shift[0] = -(tileSize / 2 * extendTo[0]);
	if (extendTo[1] != 0)
		shift[2] = -(tileSize / 2 * extendTo[1]);

	std::vector<SceneSubj*>* pSubjs = &theApp.staticSubjs;
	int totalSubjs = pSubjs->size();
	for (int sN = 0; sN < totalSubjs; sN++) {
		SceneSubj* pS = pSubjs->at(sN);
		if (pS == NULL)
			continue;
		if (pS->d2parent == 0) //root
			for (int i = 0; i < 3; i += 2)
				pS->ownCoords.pos[i] += shift[i];
		pS->buildModelMatrix();
		pS->initGabarites(pS);
	}
	//to keep camera in old position
	Camera* pCam = &theApp.mainCamera;
	for (int i = 0; i < 3; i += 2)
		pCam->lookAtPoint[i] += shift[i];
	pCam->reset(pCam, &worldBox);

	float camPitch = theApp.mainCamera.ownCoords.getEulerDg(0);
	bool mapMode = (camPitch > 70);
	if(mapMode)
		CameraMan::setView(true);
	return 1;
}
int RailMap45::trimTable() {
	bool tableSizeChanged = false;
	//check for empty rows
	for (int ang = 0; ang < 360; ang += 90) {
		float tileIndexXZ[2];
		float stepXZ[2];
		int stepsN;
		while (1) {
			addVirtualRails();
			switch (ang) {
			case 0: //front row
				stepsN = tableTiles[0];
				v2set(tileIndexXZ, 0.5, tableTiles[1] - 0.5);
				v2set(stepXZ, 1, 0);
				break;
			case 180: //back row
				stepsN = tableTiles[0];
				v2set(tileIndexXZ, 0.5, 0.5);
				v2set(stepXZ, 1, 0);
				break;
			case 90: //right column
				stepsN = tableTiles[1];
				v2set(tileIndexXZ, tableTiles[0] - 0.5, 0.5);
				v2set(stepXZ, 0, 1);
				break;
			case 270: //left column
				stepsN = tableTiles[1];
				v2set(tileIndexXZ, 0.5, 0.5);
				v2set(stepXZ, 0, 1);
				break;
			}
			bool isBusy = false;
			for (int stepN = 0; stepN < stepsN; stepN++) {
				if (countRailsInTile(tileIndexXZ[0], tileIndexXZ[1], false) > 0) {
					isBusy = true;
					break;
				}
				//check corners
				for (float dz = -0.5; dz <= 0.5; dz += 1) {
					for (float dx = -0.5; dx <= 0.5; dx += 1) {
						if (countRailsInTile(tileIndexXZ[0] + dx, tileIndexXZ[1] + dz, false) > 0) {
							isBusy = true;
							break;
						}
					}
					if (isBusy)
						break;
				}
				if (isBusy)
					break;

				for (int i = 0; i < 2; i++)
					tileIndexXZ[i] += stepXZ[i];
			}
			if (isBusy)//row/col not empty - move to next ang
				break;
			//if here - delete row/col
			if (ang % 180 == 0)//delete row
				tableTiles[1]--;
			else//delete col
				tableTiles[0]--;
			if (ang >= 180) {
				//need to reindex rails
				float indexShift[2] = { 0,0 };
				if (ang == 180)//top row removed
					indexShift[1] = -1;
				if (ang == 270)//left col removed
					indexShift[0] = -1;
				//reindex rails
				for (int rN = railsMap.size() - 1; rN >= 0; rN--) {
					Rail* pR = (Rail*)railsMap.at(rN);
					if (pR == NULL)
						continue;
					for (int i = 0; i < 2; i++)
						if (indexShift[i] < 0)
							pR->tileIndexXZ[i]--;
					for (int eN = 0; eN < 2; eN++) {
						RailEnd* pRE = &pR->railEnd[eN];
						for (int i = 0; i < 2; i++)
							if (indexShift[i] < 0)
								pRE->toTileIndexXZ[i]--;
					}
				}
			}
			tableSizeChanged = true;

			reInitLayout(this, tileSize, true);

			//recheck rails gabarites
			for (int rN = railsMap.size() - 1; rN >= 0; rN--) {
				Rail* pR = (Rail*)railsMap.at(rN);
				if (pR == NULL)
					continue;
				indexes2coords(pR);
			}
			//adjust static objs and camera
			float shift[3] = { 0,0,0 };
			float d = tileSize / 2;
			switch (ang) {
			case 0://front row
				shift[2] = d;
				break;
			case 180://back row
				shift[2] = -d;
				break;
			case 90://right col
				shift[0] = d;
				break;
			case 270://left col
				shift[0] = -d;
				break;
			}
			std::vector<SceneSubj*>* pSubjs = &theApp.staticSubjs;
			int totalSubjs = pSubjs->size();
			for (int sN = 0; sN < totalSubjs; sN++) {
				SceneSubj* pS = pSubjs->at(sN);
				if (pS == NULL)
					continue;
				if (pS->d2parent == 0) //root
					for (int i = 0; i < 3; i += 2)
						pS->ownCoords.pos[i] += shift[i];
				pS->buildModelMatrix();
				pS->initGabarites(pS);
			}
			//to keep camera in old position
			Camera* pCam = &theApp.mainCamera;
			for (int i = 0; i < 3; i += 2)
				pCam->lookAtPoint[i] += shift[i];
			pCam->reset(pCam, &worldBox);

		}
	}
	if (tableSizeChanged) {
		float camPitch = theApp.mainCamera.ownCoords.getEulerDg(0);
		bool mapMode = (camPitch > 70);
		if (mapMode) {
			TheTable* pT = &theApp.gameTable;
			CameraMan* pCM = &theApp.cameraMan;
			pCM->task = 0;//change position
			Camera::copyParams(&pCM->startCamera, &theApp.mainCamera);
			Camera::copyParams(&pCM->endCamera, &theApp.mainCamera);
			Camera* pCam = &pCM->endCamera;
			v3set(pCam->lookAtPoint, 0, pT->groundLevel0, 0);

			//update Map view
			pCM->zoom = -1;
			pCam->focusDistance = Camera::getDistance4stage(pCam,
				fmax(theApp.gameTable.worldBox.bbRad[0], theApp.gameTable.worldBox.bbRad[2]) * 2,
				fmin(theApp.gameTable.worldBox.bbRad[0], theApp.gameTable.worldBox.bbRad[2]) * 2);
			pCam->ownCoords.setPitch(85);

			float yaw00 = pCam->ownCoords.getEulerDg(1);
			float targetYaw = 0;
			float bestD = 1000000;
			for (int y = 0; y < 360; y += 90) {
				if (theApp.gameTable.worldBox.bbRad[0] < theApp.gameTable.worldBox.bbRad[2])
					if (y % 180 == 0)
						continue;
				if (theApp.gameTable.worldBox.bbRad[0] > theApp.gameTable.worldBox.bbRad[2])
					if (y % 180 != 0)
						continue;
				float dy = angleDgFromTo(yaw00, y);
				if (abs(bestD) > abs(dy)) {
					targetYaw = y;
					bestD = dy;
				}
			}
			pCam->ownCoords.setYaw(targetYaw);
			Camera::setCameraPosition(pCam);
			Camera::setRs4distance(pCam);
			pCM->progress = 0;
			pCM->progressSpeed = 0;
		}
	}
	return 1;
}

int RailMap45::shiftRailCoord00(RailCoord* pRC, RailMap45* pMap, float shiftZ,bool defacto) {
	float alignedWithRail = signOf(shiftZ) * pRC->alignedWithRail; //1-moving to rail end, -1-to start
	float remaningShiftZ = abs(shiftZ);
	while (1) {
		Rail* pR = (Rail*)pMap->railsMap.at(pRC->railN);
		if (defacto)
			pR->isBusy = true;
		float railLeft = 0;
		if (alignedWithRail > 0) //moving to rail end
			railLeft = pR->railLength * (1.0 - pRC->percentOnRail);
		else //moving to rail start
			railLeft = pR->railLength * pRC->percentOnRail;
		if (railLeft >= remaningShiftZ) {
			//adjust percent on rail
			float remainingPercent = remaningShiftZ / pR->railLength;
			pRC->percentOnRail = pRC->percentOnRail + remainingPercent * alignedWithRail;
			fillRailCoordsFromPercent(pRC, pR);
			return 1;
		}
		//if here-move to next rail
		remaningShiftZ -= railLeft;
		int whichEnd0 = 0;
		if (alignedWithRail > 0) //moved to end
			whichEnd0 = 1;
		RailEnd* pRE0 = &pR->railEnd[whichEnd0];
		if (pRE0->toRailN < 0)
			return -1;//no rail ahead
		if (defacto) {
			if(pRE0->isSwitch)
				if(pRE0->isOn==false)
					Rail::switchRail(pR);
			pRE0->isBusy = true;
		}
		pRC->railN = pRE0->toRailN;
		pR = (Rail*)pMap->railsMap.at(pRC->railN);
		int whichEnd2 = pRE0->toRailEndN;
		RailEnd* pRE2 = &pR->railEnd[whichEnd2];
		if (pRE2->isSwitch && !pRE2->isOn) {
			pRC->railN = pRE2->altRailN;
			pR = (Rail*)pMap->railsMap.at(pRC->railN);
			whichEnd2 = pRE2->altRailEndN;
			pRE2 = &pR->railEnd[whichEnd2];
			if (defacto)
				pRE2->isBusy = true;
		}
		if (whichEnd0 == whichEnd2) {
			alignedWithRail *= -1.0;
			pRC->alignedWithRail *= -1.0;
		}
		if (whichEnd2 == 0)
			pRC->percentOnRail = 0.0;
		else
			pRC->percentOnRail = 1.0;
	}
	//return 1;
}
float RailMap45::checkClearanceAhead(RailCoord* pRC0, RailMap45* pMap, float shiftZ) {
	RailCoord rc;
	memcpy(&rc, pRC0, sizeof(RailCoord));
	RailCoord* pRC = &rc;

	float alignedWithRail = signOf(shiftZ) * pRC->alignedWithRail; //1-moving to rail end, -1-to start
	float remaningShiftZ = abs(shiftZ);
	float confirnedClearance = 0;
	pRC0->pSwitchAhead = NULL;
	while (1) {
		if (pRC->railN < 0) {//no rail ahead
			return confirnedClearance;
		}
		Rail* pR = (Rail*)pMap->railsMap.at(pRC->railN);
		float percentLimit2head = 0;
		float percentLimit2end = 1;
		//if switch
		if (!theApp.bEditMode && pR->switchEndN >= 0) {
			RailEnd* pRE = &pR->railEnd[pR->switchEndN];
			if (pRE->isOn == false) {
				//switch not yours
				if (pR->switchEndN == 0)
					percentLimit2head = 0.5;
				else
					percentLimit2end = 0.5;
				pRC0->pSwitchAhead = pRE;
			}
		}
		float percentLeft = pRC->percentOnRail- percentLimit2head;//moving to head
		if (alignedWithRail > 0) //moving to rail end
			percentLeft = percentLimit2end - pRC->percentOnRail;

		float railLeft = pR->railLength * percentLeft;
		remaningShiftZ -= railLeft;
		confirnedClearance += railLeft;

		if (confirnedClearance >= abs(shiftZ))
			return confirnedClearance;//have enough room

		//check if faced limit
		if (alignedWithRail > 0 && percentLimit2end < 1) //moved to rail end limit
			return confirnedClearance;
		if (alignedWithRail < 0 && percentLimit2head>0) //moved to rail head  limit
			return confirnedClearance;

		//if here-move to next rail
		int whichEnd0 = 0;
		if (alignedWithRail > 0) //moved to end
			whichEnd0 = 1;
		RailEnd* pRE0 = &pR->railEnd[whichEnd0];
		if (pRE0->toRailN < 0)
			return confirnedClearance;//no rail ahead

		pRC->railN = pRE0->toRailN;
		pR = (Rail*)pMap->railsMap.at(pRC->railN);
		int whichEnd2 = pRE0->toRailEndN;
		RailEnd* pRE2 = &pR->railEnd[whichEnd2];
		if (pRE2->isSwitch && !pRE2->isOn) {
			pRC->railN = pRE2->altRailN;
			pR = (Rail*)pMap->railsMap.at(pRC->railN);
			whichEnd2 = pRE2->altRailEndN;
			RailEnd* pRE2 = &pR->railEnd[whichEnd2];
		}
		if (whichEnd0 == whichEnd2) {
			alignedWithRail *= -1.0;
			pRC->alignedWithRail *= -1.0;
		}
		if (whichEnd2 == 0)
			pRC->percentOnRail = 0.0;
		else
			pRC->percentOnRail = 1.0;
	}
	//return confirnedClearance;
}
bool RailMap45::approveRail(float idxX, float idxZ, float yawIn, float yawOut, int railStatus) {
	yawIn = angleDgNorm360(yawIn);
	yawOut = angleDgNorm360(yawOut);
	if (yawIn > yawOut)
		std::swap(yawIn, yawOut);

	if (abs(angleDgFromTo(yawIn, yawOut)) != 180) { //curve
		//try to find opposite curve rail
		Rail* pR2 = getRail(idxX, idxZ, yawIn + 180, yawOut + 180);
		if (pR2 != NULL)
			if (pR2->railStatus == 0)
				return false;
	}

	Rail* pMd;//model
	if (abs(angleDgFromTo(yawIn, yawOut)) == 135) //curve
		pMd = (Rail*)Rail::railModels.at(Rail::railModelCurvedVirtualN);
	else {//straight
		if (idxX == (int)idxX) //short straight
			pMd = (Rail*)Rail::railModels.at(Rail::railModelShortVirtualN);
		else //long straight
			pMd = (Rail*)Rail::railModels.at(Rail::railModelStraightVirtualN);
	}

	Rail rail;
	memcpy(&rail, pMd, sizeof(Rail));
	Rail* pR = &rail;

	pR->railStatus = railStatus;
	pR->refModelN = pR->nInSubjsSet;
	pR->pSubjsSet = NULL;
	pR->yawInOut[0] = yawIn;
	pR->yawInOut[1] = yawOut;
	pR->tileIndexXZ[0] = idxX;
	pR->tileIndexXZ[1] = idxZ;
	if (pR->railType == 1) //curve
		pR->CCW = signOf(angleDgFromTo(pR->yawInOut[1], pR->yawInOut[0]));
	///////////////////
	//check if border tile
	bool borderTile = false;
	for (int i = 0; i < 2; i++) {
		if (tableTiles[i] >= maxTilesSize) {
			//this dimension is limited
			if (pR->tileIndexXZ[i] <= 0.5) {
				borderTile = true;
				break;
			}
			if (pR->tileIndexXZ[i] >= tableTiles[i]-0.5) {
				borderTile = true;
				break;
			}
		}
	}
	if (!borderTile)
		return true;
	for (int endN = 0; endN < 2; endN++) {
		fillRailEnd(pR, endN);
		RailEnd* pRE = &pR->railEnd[endN];
		for (int i = 0; i < 2; i++) {
			if (tableTiles[i] >= maxTilesSize) {
				//this dimension is limited
				if (pRE->toTileIndexXZ[i] <= 0) {
					return false;
				}
				if (pRE->toTileIndexXZ[i] >= tableTiles[i]) {
					return false;
				}
			}
		}
	}
	return true;
}
void RailMap45::freeRails() {
	for (int rN = railsMap.size() - 1; rN >= 0; rN--) {
		Rail* pR = (Rail*)railsMap.at(rN);
		if (pR == NULL)
			continue;
		pR->isBusy = false;
		for (int endN = 0; endN < 2; endN++) {
			RailEnd* pRE = &pR->railEnd[endN];
			pRE->isBusy = false;
		}
	}
}
int RailMap45::fillRailEnd(Rail* pR, int endN) {
	RailEnd* pRE = &pR->railEnd[endN];
	pRE->endYaw = pR->yawInOut[endN];
	pRE->ownRailN = pR->nInSubjsSet;
	//define connected tile index
	v2copy(pRE->toTileIndexXZ, pR->tileIndexXZ);
	if (pRE->endYaw == 0)
		pRE->toTileIndexXZ[1] += 1.0;
	else if (pRE->endYaw == 180)
		pRE->toTileIndexXZ[1] -= 1.0;
	else if (pRE->endYaw == 90)
		pRE->toTileIndexXZ[0] += 1.0;
	else if (pRE->endYaw == 270)
		pRE->toTileIndexXZ[0] -= 1.0;
	else if (pRE->endYaw == 45) {
		pRE->toTileIndexXZ[0] += 0.5;
		pRE->toTileIndexXZ[1] += 0.5;
	}
	else if (pRE->endYaw == 135) {
		pRE->toTileIndexXZ[0] += 0.5;
		pRE->toTileIndexXZ[1] -= 0.5;
	}
	else if (pRE->endYaw == 225) {
		pRE->toTileIndexXZ[0] -= 0.5;
		pRE->toTileIndexXZ[1] -= 0.5;
	}
	else if (pRE->endYaw == 315) {
		pRE->toTileIndexXZ[0] -= 0.5;
		pRE->toTileIndexXZ[1] += 0.5;
	}
	//if (pR->railStatus != 0)
	//	return 1;
	//find connected rail end
	pRE->toRailN = -1;
	float look4yaw = angleDgNorm360(pRE->endYaw + 180);
	int railsN = railsMap.size();
	for (int rN = 0; rN < railsN; rN++) {
		Rail* pR2 = (Rail*)railsMap.at(rN);
		if (pR2 == NULL)
			continue;
		if (rN == pR->nInSubjsSet)
			continue;
		if (pR2->railStatus != 0)
			continue;
		if (!v2match(pR2->tileIndexXZ, pRE->toTileIndexXZ))
			continue;
		//rail is in connected tile
		int endN2 = -1;
		if (look4yaw == pR2->yawInOut[0])
			endN2 = 0;
		else if (look4yaw == pR2->yawInOut[1])
			endN2 = 1;
		else
			continue;
		//connected rail found
		pRE->toRailN = rN;
		pRE->toRailEndN = endN2;
		if (pR->railStatus == 0) {//for real rails only
			RailEnd* pRE2 = &pR2->railEnd[endN2];
			pRE2->toRailN = pRE->ownRailN;
			pRE2->toRailEndN = endN;
			//if pRE2 is a switch - populate to alt rail too
			if (pRE2->isSwitch) {
				Rail* pR3 = (Rail*)railsMap.at(pRE2->altRailN);
				RailEnd* pRE3 = &pR3->railEnd[pRE2->altRailEndN];
				pRE3->toRailN = pRE2->toRailN;
				pRE3->toRailEndN = pRE2->toRailEndN;
			}
		}
		return 1;
	}
	return 0;
}
int RailMap45::map2log() {

	mylog("\n\n");
	mylog("tableTiles[0]=%d;\n", tableTiles[0]);
	mylog("tableTiles[1]=%d;\n", tableTiles[1]);

	int railsN = railsMap.size();
	for (int rN = 0; rN < railsN; rN++) {
		Rail* pR = (Rail*)railsMap.at(rN);
		if (pR == NULL)
			continue;
		if (pR->railStatus != 0)
			continue;
		if (pR->railLength == Rail::railLenghtShort)
			continue;
		mylog("setRail(%.1f,%.1f,%d,%d,0);\n", pR->tileIndexXZ[0], pR->tileIndexXZ[1], (int)pR->yawInOut[0], (int)pR->yawInOut[1]);
	}
	mylog("\n\n");
	return 1;
}

void RailMap45::basicOval5x5() {
	tableTiles[0] = 5;
	tableTiles[1] = 5;
	setRail(1.5, 0.5, 315, 90, 0);
	setRail(2.5, 0.5, 270, 45, 0);
	setRail(3.5, 1.5, 225, 45, 0);
	setRail(4.5, 2.5, 225, 0, 0);
	setRail(4.5, 3.5, 180, 315, 0);
	setRail(3.5, 4.5, 135, 270, 0);
	setRail(2.5, 4.5, 90, 225, 0);
	setRail(1.5, 3.5, 45, 225, 0);
	setRail(0.5, 2.5, 45, 180, 0);
	setRail(0.5, 1.5, 0, 135, 0);
}

void RailMap45::basicOval6x6() {
	tableTiles[0] = 6;
	tableTiles[1] = 6;
	setRail(3.5, 1.5, 0, 225, 0);
	setRail(4.5, 2.5, 45, 225, 0);
	setRail(5.5, 3.5, 0, 225, 0);
	setRail(5.5, 4.5, 180, 315, 0);
	setRail(4.5, 5.5, 135, 270, 0);
	setRail(3.5, 5.5, 90, 225, 0);
	setRail(2.5, 4.5, 45, 225, 0);
	setRail(0.5, 2.5, 45, 180, 0);
	setRail(1.5, 0.5, 90, 315, 0);
	setRail(3.5, 2.5, 90, 315, 0);
	setRail(2.5, 3.5, 135, 270, 0);
	setRail(3.5, 1.5, 45, 225, 0);
	setRail(1.5, 3.5, 90, 225, 0);
	setRail(1.5, 3.5, 45, 225, 0);
	setRail(2.5, 3.5, 0, 135, 0);
	setRail(3.5, 2.5, 180, 315, 0);
	setRail(0.5, 1.5, 0, 135, 0);
	setRail(2.5, 0.5, 45, 270, 0);
	setRail(4.5, 2.5, 45, 270, 0);
	setRail(2.5, 4.5, 45, 180, 0);
}
