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
