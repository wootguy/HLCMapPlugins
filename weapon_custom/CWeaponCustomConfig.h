
class CWeaponCustomShoot;

class CWeaponCustomConfig : public CBaseEntity
{
public:
	string_t weapon_classname;

	string_t primary_fire; // targetname of weapon_custom_shoot
	string_t primary_alt_fire;
	WeaponSound primary_empty_snd;
	string_t primary_ammo_type;
	string_t primary_ammo_drop_class;
	int primary_ammo_drop_amt;
	float primary_regen_time;
	int primary_regen_amt;
	int default_ammo;
	int default_ammo2 = -1;

	int secondary_action;
	string_t secondary_fire;
	WeaponSound secondary_empty_snd;
	string_t secondary_ammo_type;
	string_t secondary_ammo_drop_class;
	int secondary_ammo_drop_amt;
	float secondary_regen_time;
	int secondary_regen_amt;

	int tertiary_action;
	string_t tertiary_fire;
	WeaponSound tertiary_empty_snd;
	int tertiary_ammo_type;

	string_t wpn_v_model;
	string_t wpn_w_model;
	string_t wpn_p_model;
	string_t hud_sprite;
	string_t hud_sprite_folder;
	string_t laser_sprite;
	float laser_sprite_scale;
	RGBA laser_sprite_color;
	int wpn_w_model_body;
	int zoom_fov;
	int max_live_projectiles = 0;

	PodArray<string_t, MAX_KV_ARRAY> idle_anims;

	float idle_time;
	float deploy_time;
	WeaponSound deploy_snd;

	float movespeed;

	WeaponSound reload_snd;
	WeaponSound reload_start_snd;
	WeaponSound reload_end_snd;
	WeaponSound reload_cancel_snd;
	float reload_time;
	float reload_start_time;
	float reload_end_time;
	float reload_cancel_time;
	int reload_start_anim;
	int reload_end_anim;
	int reload_cancel_anim;
	int reload_anim;
	int reload_empty_anim;
	int reload_mode;
	int reload_ammo_amt;

	int clip_size2 = 0;
	int reload_mode2 = 0;
	float reload_time2 = 1;
	int reload_anim2 = 1;
	WeaponSound reload_snd2;
	string_t user_effect_r2_str;

	EHANDLE user_effect1; // reload (weapon_custom_user_effect)
	EHANDLE user_effect2; // empty reload
	EHANDLE user_effect3; // 
	EHANDLE user_effect_r2; // secondary reload
	string_t user_effect1_str;
	string_t user_effect2_str;
	string_t user_effect3_str;

	int deploy_anim;
	int player_anims;
	int slot;
	int slotPosition;
	int priority; // auto switch priority

	bool matchingAmmoTypes = false;

	// primary and secondary fire settings
	EHANDLE fire_settings[3]; // weapon_custom_shoot

	EHANDLE alt_fire_settings[3]; // weapon_custom_shoot

	//weapon_custom_ammo@ primary_custom_ammo;
	//weapon_custom_ammo@ secondary_custom_ammo;

	void KeyValue(KeyValueData* pkvd);

	void update_active_weapons(const char* changedKey, const char* newValue);

	void link_shoot_settings();

	int clip_size() { return pev->skin; }
	CWeaponCustomShoot* get_shoot_settings(int fmode) { return (CWeaponCustomShoot*)(fire_settings[fmode].GetEntity()); }
	CWeaponCustomShoot* get_alt_shoot_settings(int fmode) { return (CWeaponCustomShoot*)(alt_fire_settings[fmode].GetEntity()); }

	const char* getPlayerAnimExt();

	int getRandomIdleAnim();

	bool validateSettings();

	bool isFreeWeaponSlot(int slot, int position);

	int getFreeWeaponSlotPosition(int slot);

	void loadExternalSoundSettings();

	void loadExternalEffectSettings();

	float getReloadTime(bool emptyReload = false, bool secondary = false);

	AmmoDrop getSmallestAmmoDropType(const char* ammoType);

	int getAmmoDropAmt(const char* ammoClass);

	void Spawn();

	void PrecacheModel(string_t model);

	void PrecacheSound(string_t sound);

	void Precache();
};