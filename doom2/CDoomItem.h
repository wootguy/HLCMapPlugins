#pragma once
#include "CItem.h"

class CDoomItem : public CItem
{
public:
	int itemFrame;
	int itemFrameMax = -1;
	float giveHealth;
	float giveHealthMax = 100;
	float giveArmor;
	float giveArmorMax = 200;
	bool giveGod;
	bool giveBerserk;
	bool giveInvis;
	bool giveSuit;
	bool giveGoggles;
	bool intermission;
	bool giveBackpack;
	const char* pickupSnd = "doom/dsitemup.wav";
	int modelIndexSw;
	msprite_sv_t* headerHw;
	msprite_sv_t* headerSw;

	int animDir = 1;

	virtual void ItemSpawn();

	void Precache();

	virtual bool CustomPickup();

	virtual void ItemThink() override;

	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);

	void Touch(CBaseEntity* pOther);

	int AddToFullPack(struct entity_state_s* state, CBasePlayer* player) override;
};