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
	if (g_mapinit_finished && !g_map_activated) {
		UTIL_Remove(this);
		return; // already spawned in MapInit, don't spawn again
	}

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

SoundOpts CWeaponCustomSound::getOpts() {
	SoundOpts opts;
	opts.delay = pev->friction;
	opts.file = pev->message;
	opts.channel = pev->sequence;
	opts.playMode = pev->skin;
	opts.volume = pev->health;
	opts.pitch = pev->rendermode;
	opts.pitchRand = pev->renderfx;
	opts.hasNext = pev->noise != 0;

	switch (pev->body) {
	default:
	case 1:
		opts.attn = ATTN_IDLE;
		break;
	case 2:
		opts.attn = ATTN_STATIC;
		break;
	case 3:
		opts.attn = ATTN_NORM;
		break;
	case 4:
		opts.attn = 0.3f;
		break;
	case 5:
		opts.attn = ATTN_NONE;
		break;
	}

	if (opts.channel == -1) {
		opts.channel = CHAN_STATIC;
	}

	return opts;
}

LINK_ENTITY_TO_CLASS(weapon_custom_sound, CWeaponCustomSound)