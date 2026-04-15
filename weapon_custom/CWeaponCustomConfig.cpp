#include "extdll.h"
#include "util.h"
#include "CBasePlayerWeapon.h"
#include "weapon_custom.h"
#include "CWeaponCustomConfig.h"
#include "CWeaponCustomShoot.h"
#include "CWeaponCustomSound.h"
#include "CWeaponCustomUserEffect.h"
#include "CWeaponCustomAmmo.h"

void CWeaponCustomConfig::KeyValue(KeyValueData* pkvd)
{
	bool relink = false;
	// Only custom keyvalues get sent here
	if (HandleKv(pkvd, "weapon_name")) weapon_classname = ALLOC_STRING(pkvd->szValue);

	else if (HandleKv(pkvd, "movespeed")) { movespeed = atof(pkvd->szValue); update_active_weapons(pkvd->szKeyName, pkvd->szValue); }
	else if (HandleKv(pkvd, "default_ammo")) default_ammo = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "default_ammo2")) default_ammo2 = atoi(pkvd->szValue);

	else if (HandleKv(pkvd, "primary_fire")) { primary_fire = ALLOC_STRING(pkvd->szValue); relink = true; }
	else if (HandleKv(pkvd, "primary_alt_fire")) { primary_alt_fire = ALLOC_STRING(pkvd->szValue); relink = true; }
	else if (HandleKv(pkvd, "primary_empty_snd")) primary_empty_snd.file = ALLOC_STRING(pkvd->szValue);
	else if (HandleKv(pkvd, "primary_ammo")) primary_ammo_type = ALLOC_STRING(pkvd->szValue);
	else if (HandleKv(pkvd, "primary_ammo_drop")) primary_ammo_drop_class = ALLOC_STRING(pkvd->szValue);
	else if (HandleKv(pkvd, "primary_regen_time")) primary_regen_time = atof(pkvd->szValue);
	else if (HandleKv(pkvd, "primary_regen_amt")) primary_regen_amt = atoi(pkvd->szValue);

	else if (HandleKv(pkvd, "secondary_action")) secondary_action = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "secondary_fire")) { secondary_fire = ALLOC_STRING(pkvd->szValue); relink = true; }
	else if (HandleKv(pkvd, "secondary_empty_snd")) secondary_empty_snd.file = ALLOC_STRING(pkvd->szValue);
	else if (HandleKv(pkvd, "secondary_ammo")) secondary_ammo_type = ALLOC_STRING(pkvd->szValue);
	else if (HandleKv(pkvd, "secondary_ammo_drop")) secondary_ammo_drop_class = ALLOC_STRING(pkvd->szValue);
	else if (HandleKv(pkvd, "secondary_regen_time")) secondary_regen_time = atof(pkvd->szValue);
	else if (HandleKv(pkvd, "secondary_regen_amt")) secondary_regen_amt = atoi(pkvd->szValue);

	else if (HandleKv(pkvd, "tertiary_action")) tertiary_action = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "tertiary_fire")) { tertiary_fire = ALLOC_STRING(pkvd->szValue); relink = true; }
	else if (HandleKv(pkvd, "tertiary_empty_snd")) tertiary_empty_snd.file = ALLOC_STRING(pkvd->szValue);
	else if (HandleKv(pkvd, "tertiary_ammo")) tertiary_ammo_type = atoi(pkvd->szValue);

	else if (HandleKv(pkvd, "deploy_snd")) deploy_snd.file = ALLOC_STRING(pkvd->szValue);
	else if (HandleKv(pkvd, "reload_snd")) reload_snd.file = ALLOC_STRING(pkvd->szValue);
	else if (HandleKv(pkvd, "reload_snd2")) reload_snd2.file = ALLOC_STRING(pkvd->szValue);
	else if (HandleKv(pkvd, "reload_start_snd")) reload_start_snd.file = ALLOC_STRING(pkvd->szValue);
	else if (HandleKv(pkvd, "reload_end_snd")) reload_end_snd.file = ALLOC_STRING(pkvd->szValue);
	else if (HandleKv(pkvd, "reload_cancel_snd")) reload_cancel_snd.file = ALLOC_STRING(pkvd->szValue);
	else if (HandleKv(pkvd, "reload_start_time")) reload_start_time = atof(pkvd->szValue);
	else if (HandleKv(pkvd, "reload_end_time")) reload_end_time = atof(pkvd->szValue);
	else if (HandleKv(pkvd, "reload_cancel_time")) reload_cancel_time = atof(pkvd->szValue);
	else if (HandleKv(pkvd, "reload_start_anim")) reload_start_anim = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "reload_end_anim")) reload_end_anim = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "reload_cancel_anim")) reload_cancel_anim = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "reload_ammo_amt")) reload_ammo_amt = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "reload_mode")) reload_mode = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "reload_mode2")) reload_mode2 = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "reload_anim")) reload_anim = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "reload_anim2")) reload_anim2 = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "reload_empty_anim")) reload_empty_anim = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "reload_time")) reload_time = atof(pkvd->szValue);
	else if (HandleKv(pkvd, "reload_time2")) reload_time2 = atof(pkvd->szValue);
	else if (HandleKv(pkvd, "secondary_clip")) clip_size2 = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "user_effect1")) user_effect1_str = ALLOC_STRING(pkvd->szValue);
	else if (HandleKv(pkvd, "user_effect2")) user_effect2_str = ALLOC_STRING(pkvd->szValue);
	else if (HandleKv(pkvd, "user_effect3")) user_effect3_str = ALLOC_STRING(pkvd->szValue);
	else if (HandleKv(pkvd, "user_effect_r2")) user_effect_r2_str = ALLOC_STRING(pkvd->szValue);

	else if (HandleKv(pkvd, "weapon_slot")) slot = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "weapon_slot_pos")) slotPosition = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "wpn_v_model")) wpn_v_model = ALLOC_STRING(pkvd->szValue);
	else if (HandleKv(pkvd, "wpn_w_model")) wpn_w_model = ALLOC_STRING(pkvd->szValue);
	else if (HandleKv(pkvd, "wpn_p_model")) wpn_p_model = ALLOC_STRING(pkvd->szValue);
	else if (HandleKv(pkvd, "wpn_w_model_body")) wpn_w_model_body = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "deploy_anim")) deploy_anim = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "idle_anims")) parseStrings(pkvd, idle_anims);
	else if (HandleKv(pkvd, "idle_time")) idle_time = atof(pkvd->szValue);
	else if (HandleKv(pkvd, "deploy_time")) deploy_time = atof(pkvd->szValue);
	else if (HandleKv(pkvd, "zoom_fov")) zoom_fov = atoi(pkvd->szValue);

	else if (HandleKv(pkvd, "laser_sprite")) laser_sprite = ALLOC_STRING(pkvd->szValue);
	else if (HandleKv(pkvd, "laser_sprite_scale")) laser_sprite_scale = atof(pkvd->szValue);
	else if (HandleKv(pkvd, "laser_sprite_color")) laser_sprite_color = UTIL_ParseRGBA(pkvd->szValue);
	else if (HandleKv(pkvd, "hud_sprite")) hud_sprite = ALLOC_STRING(pkvd->szValue);
	else if (HandleKv(pkvd, "sprite_directory")) hud_sprite_folder = ALLOC_STRING(pkvd->szValue);
	else if (HandleKv(pkvd, "weapon_priority")) priority = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "player_anims")) player_anims = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "hl_client_weapon")) hl_client_weapon = ALLOC_STRING(pkvd->szValue);
	else if (HandleKv(pkvd, "display_name")) display_name = ALLOC_STRING(pkvd->szValue);
	else CBaseEntity::KeyValue(pkvd);

	// dead code in the original scripts
	//if (relink && g_map_activated)
	//	link_shoot_settings();
}

void CWeaponCustomConfig::update_active_weapons(const char* changedKey, const char* newValue)
{
	if (!g_map_activated)
		return;

	CBaseEntity* ent = NULL;
	do {
		ent = UTIL_FindEntityByClassname(ent, STRING(weapon_classname));
		if (ent)
		{
			if (changedKey == "movespeed") {
				EALERT(at_error, "movespeed adjustments not implemented\n");
				//WeaponCustomBase* c_wep = (WeaponCustomBase*)ent;
				//c_wep->applyPlayerSpeedMult();
			}
		}
	} while (ent);
}

void CWeaponCustomConfig::link_shoot_settings()
{
	loadExternalSoundSettings();
	loadExternalEffectSettings();

	if (!primary_fire && !secondary_fire)
	{
		EALERT(at_warning, "no primary || secondary fire function set");
		return;
	}

	bool foundPrimary = false;
	bool foundAltPrimary = false;
	bool foundSecondary = false;
	bool foundTertiary = false;

	HashMap<EHANDLE>::iterator_t iter;
	while (custom_weapon_shoots.iterate(iter)) {
		CWeaponCustomShoot* shoot = (CWeaponCustomShoot*)iter.value->GetEntity();
		if (!strcmp(STRING(shoot->pev->targetname), STRING(primary_fire)) && primary_fire) {
			fire_settings[0] = shoot;
			shoot->weapon = EHANDLE(edict());
			foundPrimary = true;
		}
		if (!strcmp(STRING(shoot->pev->targetname), STRING(secondary_fire)) && secondary_fire) {
			fire_settings[1] = shoot;
			shoot->weapon = EHANDLE(edict());
			foundSecondary = true;
		}
		if (!strcmp(STRING(shoot->pev->targetname), STRING(tertiary_fire)) && tertiary_fire) {
			fire_settings[2] = shoot;
			shoot->weapon = EHANDLE(edict());
			foundTertiary = true;
		}
		if (!strcmp(STRING(shoot->pev->targetname), STRING(primary_alt_fire)) && primary_alt_fire) {
			alt_fire_settings[0] = shoot;
			shoot->weapon = EHANDLE(edict());
			foundAltPrimary = true;
		}
	}

	if (!foundPrimary && primary_fire) {
		EALERT(at_error, "Couldn't find primary fire entity %s for %s\n", STRING(primary_fire), STRING(weapon_classname));
	}
	if (!foundSecondary && secondary_fire) {
		EALERT(at_error, "Couldn't find secondary fire entity %s for %s\n", STRING(secondary_fire), STRING(weapon_classname));
	}
	if (!foundTertiary && tertiary_fire) {
		EALERT(at_error, "Couldn't find tertiary fire entity %s for %s\n", STRING(tertiary_fire), STRING(weapon_classname));
	}
	if (!foundAltPrimary && primary_alt_fire) {
		EALERT(at_error, "Couldn't find alternate primary fire entity %s for %s\n", STRING(primary_alt_fire), STRING(weapon_classname));
	}
}

const char* CWeaponCustomConfig::getPlayerAnimExt()
{
	if (player_anims < 0 || player_anims >= int(g_panim_refs.size()))
		return g_panim_refs[ANIM_REF_ONEHANDED];
	return g_panim_refs[player_anims];
}

int CWeaponCustomConfig::getRandomIdleAnim()
{
	if (idle_anims.size == 0)
		return 0;
	int randIdx = RANDOM_LONG(0, idle_anims.size - 1);
	return atoi(STRING(idle_anims.data[randIdx]));
}

bool CWeaponCustomConfig::validateSettings()
{
	// clamp values
	if (slot < 0 || slot > MAX_WEAPON_SLOT)
		slot = 0;
	if (slotPosition < MIN_WEAPON_SLOT_POSITION || slotPosition > MAX_WEAPON_SLOT_POSITION)
		slotPosition = -1;

	static StringMap ammo_remap = {
		{"m40a1", "762"},
		{"sporeclip", "spores"},
	};

	// remap sven ammo types
	const char* remapPrimary = ammo_remap.get(STRING(primary_ammo_type));
	const char* remapSecondary = ammo_remap.get(STRING(secondary_ammo_type));
	if (remapPrimary) {
		primary_ammo_type = ALLOC_STRING(remapPrimary);
	}
	if (remapSecondary) {
		secondary_ammo_type = ALLOC_STRING(remapSecondary);
	}

	/*
	// check that slot isn't filled
	if (slotPosition == -1) // user chose "Auto"
	{
		slotPosition = getFreeWeaponSlotPosition(slot);
		if (slotPosition == -1 || !isFreeWeaponSlot(slot, slotPosition))
			EALERT(weapon_classname + " Can't fit in weapon slot " + slot + ". Move this weapon to another slot && try again.");
	}
	else if (!isFreeWeaponSlot(slot, slotPosition))
	{
		println(logPrefix + "The weapon slot you chose for " + weapon_classname + " is filled. Choose another slot || slot position && try again.");
	}
	*/

	return true;
}

/*
bool CWeaponCustomConfig::isFreeWeaponSlot(int slot, int position)
{
	if (slot < 0 || slot > MAX_WEAPON_SLOT)
		return false;
	if (position < MIN_WEAPON_SLOT_POSITION || position > MAX_WEAPON_SLOT_POSITION)
		return false;

	// there aren't actually 6 weapons in this slot, but pos 5 && 6 don't work for some reason
	if (slot == 1 && position < 7)
		return false;

	array<string>@ stateKeys = custom_weapons.getKeys();
	for (uint i = 0; i < stateKeys.length(); i++)
	{
		weapon_custom@ settings = cast<weapon_custom@>(custom_weapons[stateKeys[i]]);
		if (settings.slot == slot && settings.slotPosition == position)
			return false;
	}

	// TODO: What if another weapon script registered a weapon here?
	return true;
}

int CWeaponCustomConfig::getFreeWeaponSlotPosition(int slot)
{
	for (int i = MIN_WEAPON_SLOT_POSITION; i < MAX_WEAPON_SLOT_POSITION; i++)
	{
		if (isFreeWeaponSlot(slot, i))
			return i;
	}
	return MAX_WEAPON_SLOT_POSITION;
}
*/

void CWeaponCustomConfig::loadExternalSoundSettings()
{
	loadSoundSettings(primary_empty_snd);
	loadSoundSettings(secondary_empty_snd);
	loadSoundSettings(reload_snd);
	loadSoundSettings(reload_snd2);
	loadSoundSettings(reload_start_snd);
	loadSoundSettings(reload_end_snd);
}

void CWeaponCustomConfig::loadExternalEffectSettings()
{
	user_effect1 = loadUserEffectSettings((CWeaponCustomUserEffect*)user_effect1.GetEntity(), user_effect1_str);
	user_effect2 = loadUserEffectSettings((CWeaponCustomUserEffect*)user_effect2.GetEntity(), user_effect2_str);
	user_effect3 = loadUserEffectSettings((CWeaponCustomUserEffect*)user_effect3.GetEntity(), user_effect3_str);
	user_effect_r2 = loadUserEffectSettings((CWeaponCustomUserEffect*)user_effect_r2.GetEntity(), user_effect_r2_str);
}

float CWeaponCustomConfig::getReloadTime(bool emptyReload, bool secondary)
{
	if (reload_mode == RELOAD_SIMPLE)
		return secondary ? reload_time2 : reload_time;

	float time = 0;
	CWeaponCustomUserEffect* ef = emptyReload ? (CWeaponCustomUserEffect*)user_effect2.GetEntity() : (CWeaponCustomUserEffect*)user_effect1.GetEntity();
	if (secondary)
		ef = (CWeaponCustomUserEffect*)user_effect_r2.GetEntity();

	for (int i = 0; i < 128; i++) // ...just in case someone tries an endless reload loop
	{
		if (!ef)
			break;
		time += ef->delay;
		ef = (CWeaponCustomUserEffect*)ef->next_effect.GetEntity();
	}
	return time;
}

AmmoDrop CWeaponCustomConfig::getSmallestAmmoDropType(const char* ammoType)
{
	AmmoDrop best;

	if (!strcmp(ammoType, "buckshot")) {
		best.cname = ALLOC_STRING("ammo_buckshot");
	}
	else if (!strcmp(ammoType, "556")) {
		best.cname = ALLOC_STRING("ammo_556");
	}
	else if (!strcmp(ammoType, "m40a1")) {
		best.cname = ALLOC_STRING("ammo_762");
	}
	else if (!strcmp(ammoType, "argrenades")) {
		best.cname = ALLOC_STRING("ammo_ARgrenades");
	}
	else if (!strcmp(ammoType, "357")) {
		best.cname = ALLOC_STRING("ammo_357");
	}
	else if (!strcmp(ammoType, "9mm")) {
		best.cname = ALLOC_STRING("ammo_9mmclip");
	}
	else if (!strcmp(ammoType, "sporeclip")) {
		best.cname = ALLOC_STRING("ammo_sporeclip");
	}
	else if (!strcmp(ammoType, "uranium")) {
		best.cname = ALLOC_STRING("ammo_gaussclip");
	}
	else if (!strcmp(ammoType, "rockets")) {
		best.cname = ALLOC_STRING("ammo_rpgclip");
	}
	else if (!strcmp(ammoType, "bolts")) {
		best.cname = ALLOC_STRING("ammo_crossbow");
	}
	else {
		best.cname = 0;
	}

	best.dropAmt = getAmmoDropAmt(STRING(best.cname));

	// check custom ammos
	HashMap<EHANDLE>::iterator_t iter;
	while (custom_ammos.iterate(iter)) {
		CWeaponCustomAmmo* ammo = (CWeaponCustomAmmo*)iter.value->GetEntity();
		if (!strcmp(STRING(ammo->custom_ammo_type), ammoType) && (ammo->give_ammo < best.dropAmt || best.dropAmt == -1))
		{
			best.cname = ammo->ammo_classname;
			best.dropAmt = ammo->give_ammo;
		}
	}

	return best;
}

int CWeaponCustomConfig::getAmmoDropAmt(const char* ammoClass)
{
	if (!ammoClass || !ammoClass[0])
		return -1;

	if (!strcmp(ammoClass, "ammo_357")) {
		return 6;
	}
	else if (!strcmp(ammoClass, "ammo_556")) {
		return 100;
	}
	else if (!strcmp(ammoClass, "ammo_762")) {
		return 5;
	}
	else if (!strcmp(ammoClass, "ammo_9mmAR")) {
		return 50;
	}
	else if (!strcmp(ammoClass, "ammo_9mmbox")) {
		return 200;
	}
	else if (!strcmp(ammoClass, "ammo_9mmclip")) {
		return 17;
	}
	else if (!strcmp(ammoClass, "ammo_ARgrenades")) {
		return 2;
	}
	else if (!strcmp(ammoClass, "ammo_buckshot")) {
		return 12;
	}
	else if (!strcmp(ammoClass, "ammo_crossbow")) {
		return 5;
	}
	else if (!strcmp(ammoClass, "ammo_gaussclip")) {
		return 20;
	}
	else if (!strcmp(ammoClass, "ammo_rpgclip")) {
		return 2;
	}
	else if (!strcmp(ammoClass, "ammo_sporeclip")) {
		return 1;
	}
	else if (!strcmp(ammoClass, "ammo_uziclip")) {
		return 32;
	}

	HashMap<EHANDLE>::iterator_t iter;
	while (custom_ammos.iterate(iter)) {
		CWeaponCustomAmmo* ammo = (CWeaponCustomAmmo*)iter.value->GetEntity();
		if (!strcmp(STRING(ammo->ammo_classname), ammoClass))
			return ammo->give_ammo;
	}

	return -1;
}

void CWeaponCustomConfig::Spawn()
{
	if (g_mapinit_finished && !g_map_activated) {
		UTIL_Remove(this);
		return; // already spawned in MapInit, don't spawn again
	}

	if (weapon_classname)
	{
		validateSettings();

		// load ammo drop amounts if a specific drop class was set
		primary_ammo_drop_amt = getAmmoDropAmt(STRING(primary_ammo_drop_class));
		secondary_ammo_drop_amt = getAmmoDropAmt(STRING(secondary_ammo_drop_class));
		if (primary_ammo_drop_amt == -1 && primary_ammo_drop_class) {
			EALERT(at_error, "%s uses an invalid primary ammo drop class: %s\n", STRING(weapon_classname), STRING(primary_ammo_drop_class));
		}
		if (secondary_ammo_drop_amt == -1 && secondary_ammo_drop_class) {
			EALERT(at_error, "%s uses an invalid primary ammo drop class: %s\n", STRING(weapon_classname), STRING(secondary_ammo_drop_class));
		}

		// automatically determine ammo drop classes if none was set
		if (!primary_ammo_drop_class && primary_ammo_type)
		{
			AmmoDrop bestMatch = getSmallestAmmoDropType(STRING(primary_ammo_type));
			primary_ammo_drop_class = bestMatch.cname;
			primary_ammo_drop_amt = bestMatch.dropAmt;
		}
		if (!secondary_ammo_drop_class && secondary_ammo_type)
		{
			AmmoDrop bestMatch = getSmallestAmmoDropType(STRING(secondary_ammo_type));
			secondary_ammo_drop_class = bestMatch.cname;
			secondary_ammo_drop_amt = bestMatch.dropAmt;
		}

		if (debug_mode) {
			EALERT(at_console, "Assigning %s to slot %d at position %d\n", STRING(weapon_classname), slot, slotPosition);
		}

		custom_weapons.put(STRING(weapon_classname), EHANDLE(edict()));
		UTIL_RegisterEquipmentEntity(STRING(weapon_classname));
		g_entityRemap.put(STRING(weapon_classname), weapon_custom_base);

		if (g_wep_info_count < MAX_WEAPONS) {
			string_t hud_path;
			if (hud_sprite_folder)
				hud_path = ALLOC_STRING(UTIL_VarArgs("%s/%s", STRING(hud_sprite_folder), STRING(weapon_classname)));
			else
				hud_path = ALLOC_STRING(STRING(weapon_classname));

			g_wep_name_info_idx.put(STRING(weapon_classname), g_wep_info_count);

			const char* ammoType1 = primary_ammo_type ? STRING(primary_ammo_type) : NULL;
			const char* ammoType2 = secondary_ammo_type ? STRING(secondary_ammo_type) : NULL;
			bool hideAmmo2 = pev->spawnflags & FL_WEP_HIDE_SECONDARY_AMMO;
			int zoomFlag = zoom_fov != 0 ? WEP_FLAG_USE_ZOOM_CROSSHAIR : 0;

			ItemInfo& info = g_wep_info[g_wep_info_count++];
			info = {
				slot,							// iSlot
				slotPosition,					// iPosition (-1 = auto)
				ammoType1,						// pszAmmo1
				hideAmmo2 ? NULL : ammoType2,	// pszAmmo2
				STRING(hud_path),				// pszName (path to HUD config)
				clip_size(),					// iMaxClip
				-1,								// iId (-1 = automatic)
				0,								// iFlags
				priority,						// iWeight
				zoomFlag,						// iFlagsEx
				0								// accuracy degrees
			};

			info = UTIL_RegisterWeapon(STRING(weapon_classname));
		}
		else {
			ALERT(at_error, "Exceeded max registered weapons!\n");
		}

		matchingAmmoTypes = toLowerCase(STRING(primary_ammo_type)) == toLowerCase(STRING(secondary_ammo_type));
		Precache();
	}
	else {
		EALERT(at_error, "creation failed. No weapon_class specified\n");
	}
}

void CWeaponCustomConfig::PrecacheModel(string_t model)
{
	if (model) {
		EALERT(at_aiconsole, "Precaching model: %s\n", STRING(model));
		PRECACHE_MODEL(STRING(model));
	}
}

void CWeaponCustomConfig::PrecacheSound(string_t sound)
{
	if (sound && strstr(STRING(sound), ".")) {
		EALERT(at_aiconsole, "Precaching sound: %s\n", STRING(sound));
		PRECACHE_SOUND(STRING(sound));
	}
}

void CWeaponCustomConfig::Precache()
{
	PrecacheSound(primary_empty_snd.file);
	PrecacheSound(secondary_empty_snd.file);
	PrecacheSound(deploy_snd.file);
	PrecacheSound(reload_snd.file);
	PrecacheSound(reload_snd2.file);
	PrecacheSound(reload_start_snd.file);
	PrecacheSound(reload_end_snd.file);
	PrecacheModel(wpn_v_model);
	PrecacheModel(wpn_w_model);
	PrecacheModel(wpn_p_model);
	PrecacheModel(hud_sprite);
	PrecacheModel(laser_sprite);

	if (hud_sprite_folder)
		PRECACHE_HUD_FILES(UTIL_VarArgs("sprites/%s/%s.txt", STRING(hud_sprite_folder), STRING(weapon_classname)));
	else
		PRECACHE_HUD_FILES(UTIL_VarArgs("sprites/%s.txt", STRING(weapon_classname)));

	if (primary_ammo_drop_class)
		UTIL_PrecacheOther(STRING(primary_ammo_drop_class));

	if (secondary_ammo_drop_class)
		UTIL_PrecacheOther(STRING(secondary_ammo_drop_class));
}

LINK_ENTITY_TO_CLASS(weapon_custom, CWeaponCustomConfig)