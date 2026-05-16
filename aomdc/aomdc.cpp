#include "extdll.h"
#include "util.h"
#include "weapons.h"
#include "PluginHooks.h"
#include "CBasePlayer.h"
#include "Scheduler.h"
#include "aomdc.h"
#include "te_effects.h"
#include "CBreakable.h"
#include "CWeaponCustom.h"

#define BATTERY_RECHARGE_CAP 5

PlayerState g_playerStates[33];

PlayerState& getPlayerState(CBasePlayer* plr) {
	return g_playerStates[plr->entindex()];
}

void FlashlightThink() {
	for (int i = 1; i <= gpGlobals->maxClients; i++) {
		CBasePlayer* plr = UTIL_PlayerByIndex(i);
		if (!plr)
			continue;

		PlayerState& state = getPlayerState(plr);

		if (state.lastFlashBattery == BATTERY_RECHARGE_CAP && plr->m_iFlashBattery > BATTERY_RECHARGE_CAP) {
			plr->m_iFlashBattery = BATTERY_RECHARGE_CAP;
		}

		if (plr->IsSevenKewpClient()) {
			if (plr->m_hasFlashlight) {
				UpdateFlashlightBatteryHud(plr, false);
			}
			if (plr->pev->armorvalue <= 0) {
				plr->m_iHideHUD |= HIDEHUD_ARMOR;
			}
			else {
				plr->m_iHideHUD &= ~HIDEHUD_ARMOR;
			}
		}
		else {
			if (plr->m_hasFlashlight && state.nextFlashlightText < gpGlobals->time) {
				state.nextFlashlightText = gpGlobals->time + 1;

				hudtextparms_t params;
				memset(&params, 0, sizeof(params));
				params.x = 1;
				params.y = 0.05f;
				params.channel = 3;

				if (plr->m_iFlashBattery >= 6) {
					params.r1 = 200;
					params.g1 = 200;
					params.b1 = 200;
				}
				else {
					params.r1 = 255;
					params.g1 = 0;
					params.b1 = 0;
				}
				params.holdTime = 1.5f;
				
				UTIL_HudMessage(plr, params, UTIL_VarArgs("%d", plr->m_iFlashBattery), MSG_ONE_UNRELIABLE);
			}
		}
	}
}

HOOK_RETURN_DATA PlayerSpawn(CBasePlayer* plr) {
	UpdateFlashlightHud(plr, false);
	UpdateFlashlightBatteryHud(plr, true);
	return HOOK_CONTINUE;
}

HOOK_RETURN_DATA ClientPutInServer(CBasePlayer* plr) {
	memset(&g_playerStates[plr->entindex()], 0, sizeof(PlayerState));
	g_playerStates[plr->entindex()].lastFlashBattery = -1; // force an update
	return HOOK_CONTINUE;
}

HOOK_RETURN_DATA PlayerJoin(CBasePlayer* plr) {
	UpdateFlashlightHud(plr, false);
	UpdateFlashlightBatteryHud(plr, true);
	return HOOK_CONTINUE;
}

HOOK_RETURN_DATA PlayerFlashlightToggle(CBasePlayer* plr, bool onNotOff) {
	if (plr->m_hasFlashlight) {
		UpdateFlashlightHud(plr, onNotOff);
	}

	return HOOK_CONTINUE;
}

HLCOOP_PLUGIN_HOOKS g_hooks;

HOOK_RETURN_DATA MapInit() {
	UTIL_RegisterEquipmentEntity("item_flashlight");
	UTIL_RegisterEquipmentEntity("ammo_p228");
	UTIL_RegisterEquipmentEntity("ammo_glock");
	UTIL_RegisterEquipmentEntity("ammo_beretta");
	UTIL_RegisterEquipmentEntity("ammo_deagle");
	UTIL_RegisterEquipmentEntity("ammo_mp5k");
	UTIL_RegisterEquipmentEntity("ammo_uzi");

	// found with ctrl+f "ammo_glock" in the server.dll B)
	static const char* breakableSpawns[22] = {
		"item_battery",
		"item_healthkit",
		"item_flashlight",
		"weapon_axe",
		"weapon_beretta",
		"weapon_deagle",
		"weapon_glock",
		"weapon_hammer",
		"weapon_knife",
		"weapon_mp5k",
		"weapon_P228",
		"weapon_revolver",
		"weapon_shotgun",
		"weapon_uzi",
		"ammo_beretta",
		"ammo_deagle",
		"ammo_glock",
		"ammo_mp5k",
		"ammo_P228",
		"ammo_revolver",
		"ammo_shotgun",
		"ammo_uzi",
	};

	for (int i = 0; i < 22; i++) {
		g_breakableSpawnRemap[i+1] = ALLOC_STRING(breakableSpawns[i]);
	}

	return HOOK_CONTINUE;
}

extern "C" int DLLEXPORT PluginInit() {
	g_hooks.pfnMapInit = MapInit;
	g_hooks.pfnPlayerSpawn = PlayerSpawn;
	g_hooks.pfnClientJoin = PlayerJoin;
	g_hooks.pfnClientPutInServer = ClientPutInServer;
	g_hooks.pfnPlayerFlashlightToggle = PlayerFlashlightToggle;

	g_Scheduler.SetInterval(FlashlightThink, 0.05f, -1);

	return RegisterPlugin(&g_hooks);
}

extern "C" void DLLEXPORT PluginExit() {
	// nothing to clean up
}