

class CWeaponCustomUserEffect : public CBaseEntity
{
public:
	bool valid;
	float delay;
	PodArray<WeaponSound, MAX_KV_ARRAY> sounds;

	float self_damage;
	int damage_type;
	int damage_type2;
	int gib_type;

	int primary_mode;

	Vector add_angle;
	Vector add_angle_rand;
	float add_angle_time;
	Vector punch_angle;
	Vector push_vel;

	string_t action_sprite;
	float action_sprite_height;
	float action_sprite_time;

	int fade_mode;
	RGBA fade_color;
	float fade_hold;
	float fade_time;

	int wep_anim;
	int anim; // thirdperson anim
	float anim_speed; // thirdperson anim
	int anim_frame; // thirdperson anim

	int player_sprite_count;
	string_t player_sprite;
	float player_sprite_freq;
	float player_sprite_time;

	float glow_time;
	int glow_amt;
	Vector glow_color;

	string_t next_effect_str;
	EHANDLE next_effect; // weapon_custom_user_effect
	bool next_effect_loaded;

	// TODO: There should really just be another entity that holds beam settings
	int beam_mode;
	int beam_type;
	int beam_width;
	int beam_noise;
	int beam_scroll;
	float beam_time;
	Vector beam_start;
	Vector beam_end;
	RGBA beam_color;
	string_t beam_spr;

	string_t v_model;
	string_t p_model;
	string_t w_model;
	int w_model_body;

	string_t hud_text;

	int triggerstate;

	void KeyValue(KeyValueData* pkvd);
	void Spawn();

	void loadExternalSoundSettings();
	void loadExternalUserEffectSettings();

	WeaponSound* getRandomSound();
	void PrecacheSound(string_t sound);
	int PrecacheModel(string_t model);
	int damageType();
	void Precache();
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
};

CWeaponCustomUserEffect* loadUserEffectSettings(CWeaponCustomUserEffect* effect, string_t name = 0);
