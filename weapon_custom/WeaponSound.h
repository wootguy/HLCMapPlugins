#pragma once

struct SoundArgs
{
	bool valid = false;
	bool attachToEnt = true;
	Vector pos;
	EHANDLE ent;
	int target_ent = 0; // target_ent_unreliable param
	int channel = CHAN_STATIC;
	string_t file;
	float volume = 1.0f;
	float attn = ATTN_NORM;
	int flags = 0;
	int pitch = 100;
	float delay = 0;
	EHANDLE next_snd; // CWeaponCustomSound->next_snd
	bool underwaterEffects = true;
};

class WeaponSound
{
public:
	string_t file;
	EHANDLE h_options; // weapon_custom_sound

	SoundArgs getSoundArgs(Vector pos, CBaseEntity* ent, int channel = CHAN_STATIC, float volMult = 1.0f,
		int pitchOverride = -1, int additionalFlags = 0, bool attachToEnt = false, bool userOnly = false);

	bool play(CBaseEntity* ent, int channel = CHAN_STATIC, float volMult = 1.0f, int pitchOverride = -1,
		int additionalFlags = 0, bool userOnly = false);

	bool play(Vector pos, int channel = CHAN_STATIC, float volMult = 1.0f, int pitchOverride = -1,
		int additionalFlags = 0);

	int getPitch();

	float getVolume();

	void stop(CBaseEntity* ent, int channel = CHAN_STATIC);
};