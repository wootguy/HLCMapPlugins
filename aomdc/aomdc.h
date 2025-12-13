#pragma once
#include "CBasePlayer.h"

struct PlayerState {
	int lastFlashBattery;
	float nextFlashlightText;
	bool ghostDamage; // player is experiencing ghost damage
};

extern PlayerState g_playerStates[33];

PlayerState& getPlayerState(CBasePlayer* plr);

void UpdateFlashlightHud(CBasePlayer* plr, bool flashOn);
void UpdateFlashlightBatteryHud(CBasePlayer* plr, bool fullRefresh);

extern "C" DLLEXPORT void weapon_aom_knife(entvars_t* pev);
extern "C" DLLEXPORT void weapon_aom_uzi(entvars_t* pev);
extern "C" DLLEXPORT void ammo_aom_uzi(entvars_t* pev);
extern "C" DLLEXPORT void item_battery(entvars_t* pev);
extern "C" DLLEXPORT void monster_zombie(entvars_t* pev);
extern "C" DLLEXPORT void monster_bullchicken(entvars_t* pev);
extern "C" DLLEXPORT void monster_headcrab(entvars_t* pev);
extern "C" DLLEXPORT void monster_houndeye(entvars_t* pev);
extern "C" DLLEXPORT void monster_alien_controller(entvars_t* pev);
extern "C" DLLEXPORT void monster_alien_grunt(entvars_t* pev);