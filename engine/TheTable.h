#pragma once
#include "DrawJob.h"
#include "SceneSubj.h"
#include "TouchScreen.h"

class TheTable : public SceneSubj
{
public:
	int tableTiles[2] = { 5,5 };
	float tileSize = 100;
	float groundLevel0 = 0;
	float skyHeight = 100;
	float seaDepth = 20;
	Gabarites worldBox;
	int maxTilesSize = 12;
	bool cursorOnTableGood = true;
	int dragMode = 0;//xz,1-spin

	std::vector<DrawJob*> table_drawJobs;
	std::vector<unsigned int> table_buffersIds;
	std::vector<SceneSubj*> tableParts;
	int startTileColorN=0;
public:
	virtual ~TheTable();
	int initTable(float tileSizeXZ, float tileSizeUp, float tileSizeDown, int tilesNx, int tilesNz);
	void cleanUpTable();
	void placeAt(float* pos, float x, float y,float z);

	virtual bool isDraggable() { return true; };
	virtual int onDrag();
	virtual int onFocusOut() { return 1; };
	virtual int onFocus() { return 1; };
	virtual int onLeftButtonUp() { return onFocusOut(); };
	virtual int onLeftButtonDown();
	static int getCursorAncorPointTable(float* ancorPoint, float* cursorPos, TheTable* pTable);
	virtual int onClick();
	float suggestedStageSize();
	int buildTablePartsGrid();
	int buildTableParts();
};


