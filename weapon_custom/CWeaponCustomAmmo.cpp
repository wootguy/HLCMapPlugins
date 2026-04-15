#include "extdll.h"
#include "util.h"
#include "CBasePlayerWeapon.h"
#include "weapon_custom.h"
#include "CWeaponCustomAmmo.h"
#include "CWeaponCustomSound.h"
#include "CBasePlayerAmmo.h"

class CWeaponCustomAmmoBase : public CBasePlayerAmmo {
	EHANDLE h_settings;

	void Spawn() override {
		h_settings = *custom_ammos.get(STRING(pev->classname));
		CBasePlayerAmmo::Spawn();
	}

	void Precache(void) {
		CWeaponCustomAmmo* settings = (CWeaponCustomAmmo*)h_settings.GetEntity();
		if (!settings)
			return;

		m_defaultModel = settings->w_model ? STRING(settings->w_model) : NULL;
		CBasePlayerAmmo::Precache();

		if (settings->pickup_snd.file)
			PRECACHE_SOUND(STRING(settings->pickup_snd.file));
	}

	BOOL AddAmmo(CBaseEntity* pOther) {
		CWeaponCustomAmmo* settings = (CWeaponCustomAmmo*)h_settings.GetEntity();
		if (!settings)
			return 0;

		bool isHlClient = pOther->IsPlayer() && !pOther->MyPlayerPointer()->UseSevenKewpGuns();
		int bResult = pOther->GiveAmmo(settings->give_ammo, settings->GetAmmoType(isHlClient), settings->max_ammo) != -1;

		if (bResult && settings->pickup_snd.file) {
			settings->pickup_snd.play(pev->origin, CHAN_ITEM);
		}

		return bResult;
	}
};

LINK_ENTITY_TO_CLASS(ammo_custom_base, CWeaponCustomAmmoBase)

void CWeaponCustomAmmo::KeyValue(KeyValueData* pkvd)
{
	if (HandleKv(pkvd, "ammo_name")) ammo_classname = ALLOC_STRING(pkvd->szValue);
	else if (HandleKv(pkvd, "w_model")) w_model = ALLOC_STRING(pkvd->szValue);
	else if (HandleKv(pkvd, "pickup_snd")) pickup_snd.file = ALLOC_STRING(pkvd->szValue);
	else if (HandleKv(pkvd, "give_ammo")) give_ammo = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "max_ammo")) max_ammo = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "ammo_type")) ammo_type = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "ammo_type_hl")) ammo_type_hl = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "custom_ammo_type")) custom_ammo_type = ALLOC_STRING(pkvd->szValue);

	else CBaseEntity::KeyValue(pkvd);
}

void CWeaponCustomAmmo::loadExternalSoundSettings()
{
	loadSoundSettings(pickup_snd);
}

const char* CWeaponCustomAmmo::GetAmmoType(bool isHlClient) {
	switch (isHlClient ? ammo_type_hl : ammo_type) {
	case AMMO_BUCKSHOT: return "buckshot";
	case AMMO_HEALTH: return "health";
	case AMMO_556: return "556";
	case AMMO_M40A1: return "762";
	case AMMO_ARGRENADES: return "ARgrenades";
	case AMMO_357: return "357";
	case AMMO_9MM: return "9mm";
	case AMMO_SHOCKCHARGE: return "shock";
	case AMMO_SPORECLIP: return "spores";
	case AMMO_URANIUM: return "uranium";
	case AMMO_ROCKETS: return "rockets";
	case AMMO_BOLTS: return "bolts";
	case AMMO_TRIPMINE: return "Trip Mine";
	case AMMO_SATCHELCHARGE: return "Satchel Charge";
	case AMMO_HANDGRENADE: return "Hand Grenade";
	case AMMO_SNARKS: return "Snarks";
	case AMMO_HORNETS: return "Hornets";
	default: return STRING(custom_ammo_type);
	}
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

		UTIL_RegisterEquipmentEntity(STRING(ammo_classname));

		if (ammo_type == -1 && custom_ammo_type)
			UTIL_RegisterAmmoCapacity(STRING(custom_ammo_type), max_ammo);

		g_entityRemap.put(STRING(ammo_classname), ammo_custom_base);

		Precache();
	}
	else {
		EALERT(at_error, "creation failed. No ammo_classname specified\n");
	}
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
	if (sound && strstr(STRING(sound), ".")) {
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