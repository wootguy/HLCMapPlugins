#pragma once
#include "weapon_custom.h"

class CWeaponCustomSound : public CBaseEntity
{
public:
	WeaponSound next_snd;

	void Spawn();
	void loadExternalSoundSettings();
	void PrecacheSound(string_t sound);
	void Precache();
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);

	SoundOpts getOpts();
	WeaponSound getWeaponSound();
};

void loadSoundSettings(WeaponSound& snd);
void loadSoundSettings(PodArray<WeaponSound, MAX_KV_ARRAY>& sounds);