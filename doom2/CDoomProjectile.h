#pragma once
#include "CDoomSprite.h"

class CDoomProjectile : public CDoomSprite
{
public:
	const char* spawnSound = "doom/dsfirsht.wav";
	const char* deathSound = "doom/dsfirxpl.wav";
	const char* trailSprite;
	bool dead;
	int moveFrameStart;
	int moveFrameEnd;
	int deathFrameStart;
	int deathFrameEnd;
	int frameCounter;
	int damageMin;
	int damageMax;
	float radiusDamage;
	float size;
	Vector lastVelocity;
	bool is_bfg;
	bool is_vile_fire;
	int fire_state;
	bool trailFrame;
	float deathTime;
	Vector flash_color;
	EHANDLE h_followEnt;
	EHANDLE h_aimEnt;
	
	void Spawn() override;
	void Precache() override;
	void Remove();
	void Touch(CBaseEntity* pOther);
	bool FireThink();
	void Think();
};