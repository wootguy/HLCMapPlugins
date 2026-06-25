#pragma once
#include <unordered_map>
#include "sound_nodes.h"

enum key_types
{
	KEY_BLUE = 1,
	KEY_YELLOW = 2,
	KEY_RED = 4,
	SKULL_BLUE = 8,
	SKULL_YELLOW = 16,
	SKULL_RED = 32,
};

struct PlayerState
{
	float lastAttack; // time this player last attacked with a weapon (used to temporarily disable invisibility)
	float lastSuit = 0; // last time suit was picked up
	float lastGoggles = 0; // last time suit was picked up
	float lastGod = 0;
	float lastInvis = 0;
	float lastHudKeys = 0; // last time key hud was updated
	int hudKeys = 0; // last key set displayed
	//PlayerViewMode viewMode = ViewMode_FirstPerson;
	SoundNode* soundNode = NULL;
	bool acceptedStrafeBug = false;

	float suitTimeLeft() { return lastSuit > 0 ? 60.0f - (gpGlobals->time - lastSuit) : 0; }
	float goggleTimeLeft() { return lastGoggles > 0 ? 120.0f - (gpGlobals->time - lastGoggles) : 0; }
	float godTimeLeft() { return lastGod > 0 ? 30.0f - (gpGlobals->time - lastGod) : 0; }
	float invisTimeLeft() { return lastInvis > 0 ? 60.0f - (gpGlobals->time - lastInvis) : 0; }
};

extern cvar_t* g_dmgScale;
extern std::unordered_map<uint64_t, PlayerState> g_player_states;

extern int g_kills;
extern int g_keys;
extern bool debug_mode;
extern int g_item_gets;
extern bool g_strict_keys;
extern bool g_map_init_done;

void DoomBlood(Vector vecSpot);