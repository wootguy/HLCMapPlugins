#include "extdll.h"
#include "util.h"
#include "CItem.h"
#include "CBasePlayer.h"
#include "user_messages.h"
#include "aomdc.h"

#define HUD_CHANNEL_BATTERY 0
#define HUD_CHANNEL_BATTERY_METER 1
#define HUD_CHANNEL_BATTERY_COUNT 2
#define HUD_CHANNEL_FLASH 3

void UpdateFlashlightHud(CBasePlayer* plr, bool flashOn) {
	if (!plr->IsAlive())
		return;

	// flash on/off buttons
	if (plr->m_hasFlashlight && plr->HasSuit()) {
		HUDSpriteParams params;
		memset(&params, 0, sizeof(HUDSpriteParams));
		params.hud.x = 1;
		params.hud.xPixels = -(32 + 20);
		params.hud.yPixels = 20;
		params.hud.channel = HUD_CHANNEL_FLASH;
		params.hud.color1 = RGB(255, 255, 255);
		params.hud.flags = HUD_SPR_USE_CONFIG;
		UTIL_HudCustomSprite(plr, params, flashOn ? "flash_on" : "flash_off");
	}
	else {
		UTIL_HudToggleElement(plr, HUD_CHANNEL_FLASH, false);
	}
}

void UpdateFlashlightBatteryHud(CBasePlayer* plr, bool fullRefresh) {
	PlayerState& state = getPlayerState(plr);

	if (!plr->IsAlive())
		return;

	if (!plr->m_hasFlashlight || !plr->HasSuit()) {
		UTIL_HudToggleElement(plr, HUD_CHANNEL_BATTERY, false);
		UTIL_HudToggleElement(plr, HUD_CHANNEL_BATTERY_METER, false);
		UTIL_HudToggleElement(plr, HUD_CHANNEL_BATTERY_COUNT, false);
		UTIL_HudToggleElement(plr, HUD_CHANNEL_FLASH, false);
		return;
	}

	if (plr->m_iFlashBattery == state.lastFlashBattery && !fullRefresh)
		return;

	RGB batColor = plr->m_iFlashBattery >= 6 ? RGB(130, 130, 130) : RGB(180, 0, 0);
	RGB chargeColor = plr->m_iFlashBattery >= 6 ? RGB(70, 70, 70) : RGB(70, 0, 0);
	RGB textColor = plr->m_iFlashBattery >= 6 ? RGB(255, 255, 255) : RGB(255, 0, 0);

	bool colorChanged = (plr->m_iFlashBattery < 6 && state.lastFlashBattery >= 6)
		|| (plr->m_iFlashBattery >= 6 && state.lastFlashBattery < 6) || fullRefresh;

	// battery
	if (colorChanged) {
		HUDSpriteParams params;
		memset(&params, 0, sizeof(HUDSpriteParams));
		params.hud.x = 1.0f;
		params.hud.xPixels = -(20 * 4 + 40);
		params.hud.yPixels = 32 * 2;
		params.hud.channel = HUD_CHANNEL_BATTERY;
		params.hud.color1 = batColor;
		params.hud.flags = HUD_SPR_USE_CONFIG;
		UTIL_HudCustomSprite(plr, params, "suit_full");
	}

	// battery meter
	{
		int sprHeight = 40;
		float chargeAmt = plr->m_iFlashBattery / 100.0f;
		int chargeHeight = 5 + 35 * chargeAmt;
		HUDSpriteParams params;
		memset(&params, 0, sizeof(HUDSpriteParams));
		params.hud.x = 1.0f;
		params.hud.xPixels = -(20 * 4 + 40);
		params.hud.yPixels = 32 * 2 + (sprHeight - chargeHeight);
		params.hud.channel = HUD_CHANNEL_BATTERY_METER;
		params.hud.color1 = chargeColor;
		params.hud.flags = HUD_SPR_USE_CONFIG;
		params.spr.width = sprHeight;
		params.spr.top = sprHeight - chargeHeight;
		params.spr.height = chargeHeight;
		UTIL_HudCustomSprite(plr, params, "suit_full");
	}

	// battery counter
	if (colorChanged) {
		HUDNumDisplayParams params;
		memset(&params, 0, sizeof(HUDNumDisplayParams));
		params.hud.x = 1.0f;
		params.hud.xPixels = -20 * 4;
		params.hud.yPixels = 32 * 2;
		params.hud.channel = HUD_CHANNEL_BATTERY_COUNT;
		params.hud.color1 = textColor;
		params.hud.flags = HUD_NUM_RIGHT_ALIGN;
		params.num.value = plr->m_iFlashBattery;
		params.num.defdigits = 3;
		UTIL_HudNumDisplay(plr, params);
	}
	else {
		UTIL_HudUpdateNum(plr, HUD_CHANNEL_BATTERY_COUNT, plr->m_iFlashBattery, true);
	}

	state.lastFlashBattery = plr->m_iFlashBattery;
}

class CItemFlashBattery : public CItem
{
	void Spawn(void)
	{
		Precache();
		SET_MODEL(ENT(pev), GetModel());
		//pev->spawnflags |= SF_ITEM_USE_ONLY | SF_NORESPAWN;
		pev->spawnflags |= SF_ITEM_USE_ONLY;
		//m_flCustomRespawnTime = 0.1f;
		CItem::Spawn();
	}

	void Precache(void)
	{
		m_defaultModel = "models/aomdc/battery.mdl";
		PRECACHE_MODEL(GetModel());
	}

	virtual BOOL OnePickupLimit() { return TRUE; }

	BOOL MyTouch(CBasePlayer* pPlayer)
	{
		if (pPlayer->m_iFlashBattery >= 100)
			return FALSE;

		pPlayer->m_iFlashBattery = V_min(100, pPlayer->m_iFlashBattery + 8);

		UpdateFlashlightBatteryHud(pPlayer, false);

		EMIT_SOUND(pPlayer->edict(), CHAN_ITEM, "items/gunpickup2.wav", 1, ATTN_NORM);

		MESSAGE_BEGIN(MSG_ONE, gmsgItemPickup, NULL, pPlayer->pev);
		WRITE_STRING(STRING(pev->classname));
		MESSAGE_END();

		return TRUE;
	}
};

class CItemFlashlight : public CItem
{
	void Spawn(void)
	{
		Precache();
		SET_MODEL(ENT(pev), GetModel());
		pev->spawnflags |= SF_ITEM_USE_ONLY;
		m_flCustomRespawnTime = 0.1f;
		CItem::Spawn();
	}

	void Precache(void)
	{
		m_defaultModel = "models/aomdc/w_flashlight.mdl";
		PRECACHE_MODEL(GetModel());
	}

	virtual BOOL OnePickupLimit() { return TRUE; }

	BOOL MyTouch(CBasePlayer* pPlayer)
	{
		if (pPlayer->m_hasFlashlight)
			return FALSE;

		pPlayer->m_hasFlashlight = true;

		EMIT_SOUND(pPlayer->edict(), CHAN_ITEM, "items/gunpickup2.wav", 1, ATTN_NORM);

		UpdateFlashlightHud(pPlayer, false);
		UpdateFlashlightBatteryHud(pPlayer, true);

		return TRUE;
	}
};

LINK_ENTITY_TO_CLASS(item_flashlight, CItemFlashlight)
LINK_ENTITY_TO_CLASS(item_battery, CItemFlashBattery)