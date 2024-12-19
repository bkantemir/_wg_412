#pragma once
#include "TheTable.h"
#include "Rail.h"
#include "ModelLoader.h"
#include "RailCoord.h"
#include "Coupler.h"

class RailMap45 : public TheTable
{
public:
	std::vector<SceneSubj*> railsMap;
	int primeLocoN = 0;

public:
	virtual ~RailMap45();
	virtual void cleanUpLayout();
	void addVirtualRails();
	void removeVirtualRails();
	void addSnowflakes();
	int addVirtualRailsToEnd(Rail* pR, int endN);
	int getRailN(float idxX, float idxZ, float yawIn, float yawOut);
	Rail*  getRail(float idxX, float idxZ, float yawIn, float yawOut);
	int setRail(float idxX, float idxZ, float yawIn, float yawOut, int railStatus);
	int addNewRail(float idxX, float idxZ, float yawIn, float yawOut, int railStatus);
	void initRailModelsVirtual();
	void initRailModels();
	static int reInitLayout(RailMap45* pMap, std::vector<SceneSubj*>* pSubjs);
	static int reInitLayout(RailMap45* pMap, float tileSize, bool tilesNchanged);
	int indexes2coords(Rail* pR);
	void sleeperGroup(ModelLoader* pML, int sizeMode); //sizeMode 0-normal, -1-small, -2-tiny
	int buildModelRailStraight(ModelLoader* pML, float railLenght, int sizeMode);
	int buildModelRailCurved(ModelLoader* pML, int sizeMode);
	int buildModelRailGuideStraight(ModelLoader* pML, float length, bool withRails);
	int buildModelRailGuideCurved(ModelLoader* pML, float angle, bool withRails);
	void buildModelRailPairSection(ModelLoader* pML, float railLenght, int sizeMode);
	int findClosestRail(RailCoord* pRC, Coords* pCoords) { return findClosestRail(pRC, this, pCoords); };
	static int findClosestRail(RailCoord* pRC, RailMap45* pMap, Coords* pCoords);
	int addDiagonalShortTo(float idxX, float idxZ, float yaw2extend, int railStatus);
	int fillRailEnd(Rail* pR, int whichEnd);
	static int shiftRailCoord(RailCoord* pRCout, RailCoord* pRCin, RailMap45* pMap, float shiftZ, bool defacto);
	static int shiftRailCoord00(RailCoord* pRC, RailMap45* pMap, float shiftZ, bool defacto);
	static void fillRailCoordsFromPercent(RailCoord* pRC, Rail* pR);
	int checkRailCrossing(Rail* pR);
	static float offset2railCoord(RailCoord* pRC1, RailCoord* pRC2, RailMap45* pMap, float distLimit=1000000);
	int check2extendTable(int railN);
	int trimTable();
	static float checkClearanceAhead(RailCoord* pRC, RailMap45* pMap, float dist2check);
	bool approveRail(float idxX, float idxZ, float yawIn, float yawOut, int railStatus);
	void freeRails();

	virtual int onClick();//stop trains, zoom
	static int stopAllTrains();
	int countRailsInTile(float idxX, float idxZ, bool realRailsOnly);
	int placeAtEmptySlot(Rail* pR);
	static int fillCrossing(Rail* pR, Rail* pR2);
	static void fillRailCrossPercentAndYaw(Rail* pR, float* vPoint);
	int map2log();
	void basicOval5x5();
	void basicOval6x6();
};


