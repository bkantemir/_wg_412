#include "ExhaustLoco.h"
#include "subjs/Smoke.h"
#include "rr/RollingStock.h"
#include "WheelPair.h"
#include "TheApp.h"

extern TheApp theApp;

int ExhaustLoco::moveExhaustLoco(ExhaustLoco* pEx) {
	if (theApp.bEditMode)
		return 1;

	RollingStock* pLoco = (RollingStock*)pEx->pSubjsSet->at(pEx->rootN);
	if (strcmp(pLoco->type16, "loco") == 0) {
		if (strcmp(pLoco->subtype16, "diesel") == 0)
			return moveExhaustLocoDiesel(pEx, pLoco);
		else if (strcmp(pLoco->subtype16, "steam") == 0)
				return moveExhaustLocoSteam(pEx, pLoco);
		else {
			mylog("ERROR in ExhaustLoco::moveExhaustLoco: Unhandled loco subtype %s/%s\n", pLoco->type16, pLoco->subtype16);
			return -1;
		}
	}
	else {
		mylog("ERROR in ExhaustLoco::moveExhaustLoco: Unhandled loco type %s/%s\n", pLoco->type16, pLoco->subtype16);
		return -1;
	}
	return 1;
}
int ExhaustLoco::moveExhaustLocoDiesel(ExhaustLoco* pEx, RollingStock* pLoco) {
	if (!pLoco->powerOn)
		return 1;
	float size0 = pEx->scale[0];
	float linSpeed = size0 * 0.8;
	if (abs(pLoco->ownZspeed) < abs(pLoco->desirableZspeed))//speeding up
		return startCloud(pEx, linSpeed, size0, 1, MyColor::getUint32(20, 10, 20), 0.1, 0.4);//torq
	else //steady speed or idle
		return startCloud(pEx, linSpeed, size0, 1, MyColor::getUint32(200, 180, 250), 0.02, 0.4);//idle
	return 1;
}

int ExhaustLoco::moveExhaustLocoSteam(ExhaustLoco* pEx, RollingStock* pLoco) {
	if (!pLoco->powerOn && pLoco->engineHeat == 0)
		return 1;

	bool running = (pLoco->desirableZspeed != 0);
	bool speedUp = (abs(pLoco->ownZspeed) < abs(pLoco->desirableZspeed));

	float size0 = pEx->scale[0];
	float linSpeed = size0 * 0.8;

	if (strcmp(pEx->type16, "smoke") == 0) {
		uint32_t smokeColor = MyColor::getUint32(55, 50, 50);
		float fullDencity = 1;
		float fullPressure = 0.25;
		if (pLoco->engineHeat == 1)
			return startCloud(pEx, linSpeed, size0, 2, uTex3N4black, fullPressure, fullDencity, 1.5);
			//return startCloud(pEx, linSpeed, size0, 1, smokeColor, fullPressure, fullDencity, 1.5);

		if (pLoco->engineHeat < 0.8) {//smoke rings
			int minInterval = 15;
			if (running) {
				if (pEx->countdown > minInterval)
					pEx->countdown = minInterval;
			}
			if (pEx->countdown >= 0) {
				pEx->countdown--;
				if (pEx->countdown <= 0) {
					float intervalMax = 33 * 4;
					pEx->countdown = minInterval;
					if (!speedUp) {
						pEx->countdown = fmax(30, intervalMax * (1.0 - sqrt(pLoco->engineHeat)));
					}
					float percent = 1.0 - pLoco->engineHeat;
					float dencity = minimax( fullDencity * percent, 0.2, 0.6);
					return startCloudRing(pEx, linSpeed*0.7, size0, 2, uTex3N4black, fullPressure, dencity,1.5);
					//return startCloud(pEx, linSpeed, size0, 1, smokeColor, fullPressure, dencity, 1.5);
				}
			}
		}
		if (pLoco->engineHeat > 0) {
			float percent=pLoco->engineHeat;
			float dencity = fullDencity * percent;
			return startCloud(pEx, linSpeed,size0, 2, uTex3N4black, fullPressure, dencity);
		}
	}
	else { //steam
		uint32_t steamColor = MyColor::getUint32(220, 220, 250);
		if (strcmp(pEx->subtype16, "leak") == 0) {
			if (pLoco->engineHeat > 0.95)
				return 1;
			float fullDencity = 0.5;
			float percent = 1.0-pLoco->engineHeat;
			float dencity = fullDencity * percent * percent;
			return startCloud(pEx, linSpeed, size0, 1, steamColor, 0.05, dencity);
		}
		else if (strcmp(pEx->subtype16, "valve") == 0) {
			if (running)
				return 1;
			if (pLoco->engineHeat > 0.5)
				return 1;
			pEx->countdown--;
			if (pEx->countdown <= 0) {
				pEx->phase = ((int)pEx->phase + 1) % 2;
				if (pEx->phase == 0)
					pEx->countdown = 33 * 10; //idle
				else
					pEx->countdown = 33 * 3; //run
			}
			if (pEx->phase == 0)
				return 1;
			return startCloud(pEx, linSpeed, size0, 1, steamColor, 0.3, 0.8);
		}
		else if (strcmp(pEx->subtype16, "piston") == 0) {
			if (!running)
				return 1;
			//find leading wheel
			int elementsN = pLoco->totalElements;
			for (int i = 1; i < elementsN; i++) {
				SceneSubj* pS = pLoco->pSubjsSet->at(pLoco->nInSubjsSet + i);
				if (pS == NULL)
					continue;
				if (pS->rootN != pLoco->rootN)
					continue;
				if (strcmp(pS->className, "WheelPair") != 0)
					continue;
				WheelPair* pW = (WheelPair*)pS;
				if (pLoco->maxWheelRad > pW->wheelRadius)
					continue;
				float spinAngle = pW->spinAngle;
				if (pEx->ownCoords.pos[0] < 0) //left piston
					spinAngle += 90;
				spinAngle = angleDgNorm180(spinAngle);
				if (spinAngle < 0)
					return 1;
			}
			float dencity = minimax(1.0 - pLoco->engineHeat, 0.2, 0.8);
			if (speedUp)
				dencity = 1;
			return startCloud(pEx, linSpeed, size0, 1, steamColor, 0.15, dencity);
		}
	}
	return 1;
}


