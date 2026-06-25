#pragma once
#include "CBaseEntity.h"

#define FL_TP_IGNORE_PLAYERS 2
#define FL_TP_ON_EXIT 32

class CDoomTeleport : public CBaseEntity
{
	bool ignore_players;
	bool tele_on_exit;
	float touchDelay;
	Vector teleDir; // ent must be moving in this general direction || else will be ignored

	void KeyValue(KeyValueData* pkvd) override;
	void Spawn() override;
	void Precache() override;

	void Teleport(CBaseEntity* other);
	void Touch(CBaseEntity* other);
};