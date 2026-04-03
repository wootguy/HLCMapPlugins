#include "extdll.h"
#include "util.h"
#include "CBasePlayerWeapon.h"
#include "weapon_custom.h"
#include "CWeaponCustomAmmo.h"
#include "CWeaponCustomSound.h"

void CWeaponCustomAmmo::KeyValue(KeyValueData* pkvd)
{
	if (HandleKv(pkvd, "ammo_name")) ammo_classname = ALLOC_STRING(pkvd->szValue);

	else if (HandleKv(pkvd, "w_model")) w_model = ALLOC_STRING(pkvd->szValue);
	else if (HandleKv(pkvd, "pickup_snd")) pickup_snd.file = ALLOC_STRING(pkvd->szValue);
	else if (HandleKv(pkvd, "give_ammo")) give_ammo = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "max_ammo")) max_ammo = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "ammo_type")) ammo_type = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "custom_ammo_type")) custom_ammo_type = ALLOC_STRING(pkvd->szValue);

	else CBaseEntity::KeyValue(pkvd);
}

void CWeaponCustomAmmo::loadExternalSoundSettings()
{
	loadSoundSettings(pickup_snd);
}

void CWeaponCustomAmmo::Spawn()
{
	if (g_mapinit_finished && !g_map_activated) {
		UTIL_Remove(this);
		return; // already spawned in MapInit, don't spawn again
	}

	if (ammo_classname)
	{
		custom_ammos.put(STRING(ammo_classname), EHANDLE(edict()));
		EALERT(at_error, "Ammo registering not implemented\n");
		//g_CustomEntityFuncs.RegisterCustomEntity("WeaponCustom::AmmoCustomBase", ammo_classname);
		Precache();
	}
	else
		EALERT(at_error, "creation failed. No ammo_classname specified");
}

void CWeaponCustomAmmo::PrecacheModel(string_t model)
{
	if (model) {
		EALERT(at_aiconsole, "Precaching model: %s\n", STRING(model));
		PRECACHE_MODEL(STRING(model));
	}
}

void CWeaponCustomAmmo::PrecacheSound(string_t sound)
{
	if (sound) {
		EALERT(at_aiconsole, "Precaching sound: %s\n", STRING(sound));
		PRECACHE_SOUND(STRING(sound));
	}
}

void CWeaponCustomAmmo::Precache()
{
	PrecacheSound(pickup_snd.file);
	PrecacheModel(w_model);
}

LINK_ENTITY_TO_CLASS(weapon_custom_ammo, CWeaponCustomAmmo)