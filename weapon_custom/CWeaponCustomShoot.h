
class CWeaponCustomShoot : public CBaseEntity
{
public:
	EHANDLE weapon;
	int shoot_type;
	PodArray<WeaponSound, MAX_KV_ARRAY> sounds; // shoot sounds
	PodArray<string_t, MAX_KV_ARRAY> shoot_anims; // shoot or melee swing
	PodArray<string_t, MAX_KV_ARRAY> melee_anims; // melee hit anims
	PodArray<WeaponSound, MAX_KV_ARRAY> melee_hit_sounds;
	PodArray<WeaponSound, MAX_KV_ARRAY> melee_flesh_sounds;
	PodArray<WeaponSound, MAX_KV_ARRAY> shoot_fail_snds;
	WeaponSound shoot_empty_snd;
	int shoot_empty_anim;
	int ammo_cost;
	float cooldown = 0.5;
	float cooldown_fail = 0.5;
	Vector recoil;
	Vector kickback;
	Vector knockback;
	float max_range;
	int heal_mode;
	int heal_targets;

	float damage;
	int damage_type;
	int damage_type2;
	int gib_type;
	bool friendly_fire;

	int bullets;
	int bullet_type; // see docs for "Bullet"
	int bullet_color = -1;
	int bullet_spread_func;
	int bullet_decal;
	float bullet_spread;
	float bullet_delay; // burst fire delay

	float melee_miss_cooldown;

	ProjectileOptions projectile = ProjectileOptions();

	float beam_impact_speed;
	string_t beam_impact_spr;
	float beam_impact_spr_scale;
	int beam_impact_spr_fps;
	RGBA beam_impact_spr_color;
	int beam_ricochet_limit; // maximum number of ricochets
	float beam_ammo_cooldown;

	EHANDLE effect1; // weapon_custom_effect
	EHANDLE effect2;
	EHANDLE effect3;
	EHANDLE effect4;

	// for effect linking by name
	string_t effect1_name;
	string_t effect2_name;
	string_t effect3_name;
	string_t effect4_name;

	EHANDLE user_effect1; // shoot (weapon_custom_user_effect)
	EHANDLE user_effect2; // overcharge
	EHANDLE user_effect3; // cooldown
	EHANDLE user_effect4; // windup
	EHANDLE user_effect5; // toggle to
	EHANDLE user_effect6; // victim effect (monster attacking player)
	string_t user_effect1_str;
	string_t user_effect2_str;
	string_t user_effect3_str;
	string_t user_effect4_str;
	string_t user_effect5_str;
	string_t user_effect6_str;

	float rico_angle;
	Vector muzzle_flash_color;
	Vector muzzle_flash_adv;
	float toggle_cooldown;

	float windup_time;
	float windup_min_time;
	float wind_down_time;
	float wind_down_cancel_time;
	float windup_mult;
	float windup_kick_mult;
	float windup_anim_time;
	WeaponSound windup_snd;
	WeaponSound wind_down_snd;
	WeaponSound windup_loop_snd;
	int windup_pitch_start;
	int windup_pitch_end;
	int windup_easing;
	int windup_action;
	int windup_cost;
	int windup_anim;
	int wind_down_anim;
	int windup_anim_loop;
	float windup_overcharge_time;
	float windup_overcharge_cooldown;
	float windup_movespeed;
	float windup_shoot_movespeed;
	int windup_overcharge_action;
	int windup_overcharge_anim;

	int hook_type;
	int hook_pull_mode;
	int hook_targets;
	int hook_anim;
	int hook_anim2;
	float hook_force;
	float hook_speed;
	float hook_max_speed;
	float hook_delay;
	float hook_delay2;
	string_t hook_texture_filter;
	WeaponSound hook_snd;
	WeaponSound hook_snd2;

	int shell_type = 0;
	string_t shell_model;
	Vector shell_offset;
	Vector shell_vel;
	float shell_delay;
	WeaponSound shell_delay_snd;
	float shell_spread;
	int shell_idx;

	BeamOptions beams[2];

	void KeyValue(KeyValueData* pkvd);

	bool isPrimary();
	bool isSecondary();
	bool isTertiary();

	void loadExternalSoundSettings();
	void loadExternalEffectSettings();

	int damageType(int defaultType);

	WeaponSound* getRandomShootSound();
	WeaponSound* getRandomMeleeHitSound();
	WeaponSound* getRandomMeleeFleshSound();
	WeaponSound* getRandomShootFailSound();

	bool validateSettings() { return true; }
	void Spawn();
	int PrecacheModel(string_t model);
	void PrecacheSound(string_t sound);
	bool can_fire_underwater() { return pev->spawnflags & FL_SHOOT_IN_WATER != 0; }
	void update_shell_type();
	void Precache();
};

class CWeaponCustomBullet : public CWeaponCustomShoot
{
	void Spawn();
};

class CWeaponCustomMelee : public CWeaponCustomShoot
{
	void Spawn();
};

class CWeaponCustomProjectile : public CWeaponCustomShoot
{
	void Spawn();
};

class CWeaponCustomBeam : public CWeaponCustomShoot
{
	void Spawn();
};