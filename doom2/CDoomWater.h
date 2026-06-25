#pragma once
#include "CBaseEntity.h"

class CDoomWater : public CBaseEntity
{
public:
	float damage;
	int maxFrame;
	float lastTouches[33];
	float lastPains[33];

	void KeyValue(KeyValueData* pkvd);

	void Spawn();

	void WaterThink();

	void Touch(CBaseEntity* other);
};