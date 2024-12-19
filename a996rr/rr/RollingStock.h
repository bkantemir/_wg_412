#pragma once
#include "OnRails.h"
#include "Train.h"
#include "Gangway.h"
#include "RailMap45.h"

class RollingStock : public OnRails
{
public:
	Train tr;
	int trainRootN = -1;
	int alignedWithTrainHead = 1;
	float zOffsetFromHead = 0;
	int nInTrain = 0;
	Coupler* pCouplerFront = NULL;
	Coupler* pCouplerBack = NULL;
	Gangway* pGangwayFront = NULL;
	Gangway* pGangwayBack = NULL;
	float unitLength = 0;
	float maxSectionLength = 0;
	bool powered = false; //1-locomotive
	bool activeLoco = false;
	bool powerOn = true;
	float engineHeat = 0;//0 to 1
	float maxWheelRad = 0;
	bool dragging = false;

	static float accelerationLinPassive;
	static float accelerationLinActive;
	static float accelerationOnDrag;
	static float divorceSpeed;
	static float maxTrainSpeed;
	static float maxDragSpeed;

public:
	RollingStock() {};
	RollingStock(RollingStock* pSS0) { memcpy((void*)this, (void*)pSS0, sizeof(RollingStock)); };
	virtual RollingStock* clone() {
		if (strcmp(this->className, "RollingStock") != 0)
			return NULL;
		return new RollingStock(this);
	};
	virtual int onDeploy(std::string tagStr) { return onDeployRS(this, tagStr); };
	static int onDeployRS(RollingStock* pSS, std::string tagStr);
	virtual int onLoad(std::string tagStr) { return onLoadRS(this, tagStr); };
	static int onLoadRS(RollingStock* pSS, std::string tagStr);
	virtual int moveSubj() { return moveRS(this); };
	static int moveRS(RollingStock* pRS);
	virtual bool ignoreCollision(SceneSubj* pS2) { return ignoreCollisionRollingStock(this, pS2); };
	static bool ignoreCollisionRollingStock(RollingStock* pRS1, SceneSubj* pS2);
	virtual int processCollision(float penetrationDepth,SceneSubj* pS2, float* collisionPointWorld, float* hitPointNormal, float* hitSpeed) {
		return processCollisionRollingStock(penetrationDepth,this, pS2, collisionPointWorld, hitPointNormal, hitSpeed);
	};
	static int processCollisionRollingStock(float penetrationDepth, RollingStock* pS1, SceneSubj* pS2, float* collisionPointWorld, float* hitPointNormal, float* hitSpeed);
	virtual int readClassProps(std::string tagStr) { return readClassPropsRS(this,  tagStr); };
	static int readClassPropsRS(RollingStock* pRS,std::string tagStr);

	static int checkTrains(std::vector<SceneSubj*>* pSceneSubjs);
	static float couplerClearance(Coupler* pCoupler);
	static int reinspectTrain(Coupler* pCoupler);
	static int emergencyTrainShift(RollingStock* pRoot1, float dist, RollingStock* pRoot2);
	static int divorceTrains(RollingStock* pW1, RollingStock* pW2);
	static int trainToLog(RollingStock* pW);
	static int trainToLog(Coupler* pCp);
	static std::string path2title(std::string path);
	static bool trainRootIsWrong(RollingStock* pW0);
	static Coupler* findHeadCoupler(Coupler* pCp);
	static int assignNewTrainHead(Coupler* pCp);
	static int decouple(Coupler* pCp);
	static bool obstacleAhead(RollingStock* pRS);
	static int resetMaxSpeed();
	virtual int deleteMe(bool withChilds = true);
	static bool approveMoveIntent(RollingStock* pRoot, RailMap45* pMap);

	virtual bool isResponsive() { return true; };
	virtual bool isDraggable() { return true; };

	virtual int onDrag() {return onDragRS(this);};
	static int onDragRS(RollingStock* pRS);
	virtual int onClick();
	virtual int onLeftButtonUp();

};


