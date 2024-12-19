#pragma once
#include "subjs/Exhaust.h"
#include "rr/RollingStock.h"

class ExhaustLoco : public Exhaust
{
public:

public:
	ExhaustLoco() {};
	ExhaustLoco(ExhaustLoco* pSS0) { memcpy((void*)this, (void*)pSS0, sizeof(ExhaustLoco)); };
	virtual ExhaustLoco* clone() {
		if (strcmp(this->className, "ExhaustLoco") != 0)
			return NULL;
		return new ExhaustLoco(this);
	};
	virtual int moveSubj() { return moveExhaustLoco(this); };
	static int moveExhaustLoco(ExhaustLoco* pGS);
	static int moveExhaustLocoDiesel(ExhaustLoco* pXH, RollingStock* pLoco);
	static int moveExhaustLocoSteam(ExhaustLoco* pXH, RollingStock* pLoco);
};
