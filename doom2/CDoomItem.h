#pragma once
#include "CItem.h"

class CDoomItem : public CItem
{
public:
	int itemFrame = 0;
	int itemFrameMax = -1;
	float giveHealth = 0;
	float giveHealthMax = 100;
	float giveArmor = 0;
	float giveArmorMax = 200;
	bool giveGod = false;
	bool giveBerserk = false;
	bool giveInvis = false;
	bool giveSuit = false;
	bool giveGoggles = false;
	bool intermission = false;
	bool giveBackpack = false;
	const char* pickupSnd = "doom/dsitemup.wav";

	int animDir = 1;

	virtual void ItemSpawn();

	void Precache();

	virtual bool CustomPickup();

	virtual void ItemThink() override;

	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);

	void Touch(CBaseEntity* pOther);
};