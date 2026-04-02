
class CWeaponCustomEffect : public CBaseEntity
{
public:
	string_t name;
	bool valid = false;

	float delay = 0;

	int explosion_style;
	float explode_radius;
	float explode_damage;
	float explode_offset;
	float explode_spr_scale;
	float explode_spr_fps;
	string_t explode_water_spr;
	string_t explode_spr;

	int damage_type;
	int damage_type2;
	int gib_type;

	int blood_stream;

	string_t explode_smoke_spr;
	float explode_smoke_spr_scale;
	float explode_smoke_spr_fps;
	float explode_smoke_delay;

	float explode_beam_radius;
	int explode_beam_width;
	int explode_beam_life;
	int explode_beam_noise;
	RGBA explode_beam_color;
	int explode_beam_frame;
	int explode_beam_fps;
	int explode_beam_scroll;

	int explode_bubbles;
	Vector explode_bubble_mins;
	Vector explode_bubble_maxs;
	float explode_bubble_delay;
	float explode_bubble_speed;
	string_t explode_bubble_spr;

	RGBA explode_light_color;
	RGBA explode_light_color2;
	Vector explode_light_adv;
	Vector explode_light_adv2;

	int explode_gibs;
	string_t explode_gib_mdl;
	int explode_gib_mat;
	int explode_gib_speed;
	int explode_gib_rand;
	int explode_gib_effects;

	PodArray<WeaponSound, MAX_KV_ARRAY> sounds;
	int rico_decal;
	string_t rico_part_spr;
	int rico_part_count;
	int rico_part_scale;
	int rico_part_speed;

	int rico_trace_count;
	int rico_trace_color;
	int rico_trace_speed;
	int rico_trace_rand;

	string_t glow_spr;
	int glow_spr_scale;
	int glow_spr_life;
	int glow_spr_opacity;

	int spray_count;
	string_t spray_sprite;
	int spray_speed;
	int spray_rand;

	int burst_life;
	int burst_radius;
	int burst_color;

	int implode_count;
	int implode_radius;
	int implode_life;

	float shake_radius;
	float shake_amp;
	float shake_freq;
	float shake_time;

	int rico_scale;

	string_t next_effect_str;

	EHANDLE next_effect; // weapon_custom_effect
	bool next_effect_loaded = false;

	void KeyValue(KeyValueData* pkvd);

	void Spawn();

	void loadExternalSoundSettings();

	void loadExternalEffectSettings();

	WeaponSound* getRandomSound();

	void PrecacheSound(string_t sound);

	int PrecacheModel(string_t model);

	int damageType();

	void Precache();

	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
};


void custom_explosion(Vector pos, Vector vel, CWeaponCustomEffect* effect, Vector decalPos,
	CBaseEntity* decalEnt, EHANDLE owner, bool inWater, bool friendlyFire);

void custom_effect(Vector pos, EHANDLE h_effect, EHANDLE creator, EHANDLE target,
	EHANDLE owner, Vector vDir, int flags, DecalTarget* dt = NULL);

CWeaponCustomEffect* loadEffectSettings(CWeaponCustomEffect* effect, string_t name = 0);