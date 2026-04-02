#include "extdll.h"
#include "util.h"
#include "CBaseEntity.h"
#include "CWeaponCustomSound.h"
#include "weapon_custom.h"

void loadSoundSettings(WeaponSound& snd)
{
	if (snd.h_options.GetEntity())
		return;

	CBaseEntity* ent = UTIL_FindEntityByTargetname(NULL, STRING(snd.file));
	if (ent && FClassnameIs(ent->pev, "weapon_custom_sound"))
	{
		snd.file = ent->pev->message;
		snd.h_options = EHANDLE(ent->edict());

		((CWeaponCustomSound*)ent)->loadExternalSoundSettings();
	}
}

void loadSoundSettings(PodArray<WeaponSound, MAX_KV_ARRAY> sounds) {
	for (int i = 0; i < sounds.size; i++) {
		loadSoundSettings(sounds.data[i]);
	}
}

void CWeaponCustomSound::Spawn()
{
	next_snd.file = pev->noise;
	Precache();
}

void CWeaponCustomSound::loadExternalSoundSettings()
{
	loadSoundSettings(next_snd);
}

void CWeaponCustomSound::PrecacheSound(string_t sound)
{
	if (sound) {
		ALERT(at_console, "Precaching sound for %s: %s", STRING(pev->targetname), STRING(sound));
		PRECACHE_SOUND(STRING(sound));
	}
}

void CWeaponCustomSound::Precache()
{
	PrecacheSound(pev->message);
}

void CWeaponCustomSound::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	WeaponSound snd;
	snd.file = pev->message;
	snd.h_options = this;

	snd.play(pev->origin);
}

LINK_ENTITY_TO_CLASS(weapon_custom_sound, CWeaponCustomSound)