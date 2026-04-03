#include "extdll.h"
#include "util.h"
#include "CBaseEntity.h"
#include "WeaponSound.h"
#include "CWeaponCustomSound.h"
#include "weapon_custom.h"
#include "Scheduler.h"

void WaterSoundEffects(Vector pos, SoundArgs& args)
{
	if (!args.underwaterEffects)
		return;
	if (UTIL_PointContents(pos) == CONTENTS_WATER)
	{
		args.pitch = max(0, args.pitch - RANDOM_LONG(20, 30));
		args.volume *= 0.7f;
	}
}

void playSoundDelay(SoundArgs args)
{
	uint32_t targets = args.target_ent ? PLRBIT(args.target_ent) : 0xffffffff;

	if (args.attachToEnt)
	{
		CBaseEntity* ent = args.ent;

		if (!ent)
			return;

		WaterSoundEffects(ent->pev->origin + ent->pev->view_ofs, args);		
		StartSound(ent->edict(), args.channel, STRING(args.file), args.volume, args.attn, args.flags,
			args.pitch, args.pos, targets);
	}
	else
	{
		WaterSoundEffects(args.pos, args);
		StartSound(NULL, args.channel, STRING(args.file), args.volume, args.attn, args.flags,
			args.pitch, args.pos, targets);
	}

}

SoundArgs WeaponSound::getSoundArgs(Vector pos, CBaseEntity* ent, int channel, float volMult,
	int pitchOverride, int additionalFlags, bool attachToEnt, bool userOnly)
{
	if (!file)
		return SoundArgs();
	float volume = 1.0f;
	float attn = ATTN_NORM;
	int flags = additionalFlags;
	int pitch = getPitch();
	float delay = 0.0f;
	int chan = channel;
	CWeaponCustomSound* nextSnd;
	bool underwaterEffects = true;
	if (h_options.GetEntity())
	{
		nextSnd = (CWeaponCustomSound*)h_options.GetEntity();
		underwaterEffects = (h_options->pev->spawnflags & FL_SOUND_NO_WATER_EFFECT) == 0;
		delay = h_options->pev->friction;
		volume = h_options->pev->health / 100.0f;
		if (h_options->pev->sequence > -1)
			chan = h_options->pev->sequence;

		switch (h_options->pev->body)
		{
		case 1: attn = ATTN_IDLE; break;
		case 2: attn = ATTN_STATIC; break;
		case 3: attn = ATTN_NORM; break;
		case 4: attn = 0.3f; break;
		case 5: attn = ATTN_NONE; break;
		}

		if (h_options->pev->skin == 2) {
			//flags |= SND_FORCE_SINGLE;
			ALERT(at_error, "WeaponSound: SND_FORCE_SINGLE flag not implemented\n");
		}
		if (h_options->pev->skin == 3) {
			//flags |= SND_FORCE_LOOP;
			ALERT(at_error, "WeaponSound: SND_FORCE_LOOP flag not implemented\n");
		}
	}
	if (pitchOverride != -1)
		pitch = pitchOverride;


	SoundArgs args;
	args.valid = true;
	args.attachToEnt = attachToEnt;
	args.pos = pos;
	args.delay = delay;
	args.ent = ent;
	args.channel = chan;
	args.file = file;
	args.volume = volume * volMult;
	args.attn = attn;
	args.flags = flags;
	args.pitch = pitch;
	args.target_ent = (ent && userOnly) ? ent->entindex() : 0;
	args.underwaterEffects = underwaterEffects;
	args.next_snd = EHANDLE(nextSnd->edict());

	return args;
}

bool WeaponSound::play(CBaseEntity* ent, int channel, float volMult, int pitchOverride,
	int additionalFlags, bool userOnly)
{
	SoundArgs args = getSoundArgs(Vector(0, 0, 0), ent, channel, volMult, pitchOverride, additionalFlags,
		true, userOnly);
	if (!args.valid)
		return false;

	//println("PLAY: " + args.file);
	if (args.delay > 0)
		g_Scheduler.SetTimeout(playSoundDelay, args.delay, args);
	else
	{
		uint32_t targets = args.target_ent ? PLRBIT(args.target_ent) : 0xffffffff;
		WaterSoundEffects(ent->pev->origin + ent->pev->view_ofs, args);		
		StartSound(ent->edict(), args.channel, STRING(args.file), args.volume, args.attn, args.flags,
			args.pitch, args.pos, targets);
	}

	if (args.next_snd) {
		CWeaponCustomSound* nxt = (CWeaponCustomSound*)args.next_snd.GetEntity();
		nxt->next_snd.play(ent, channel, volMult, pitchOverride, additionalFlags, userOnly);
	}

	return true;
}

bool WeaponSound::play(Vector pos, int channel, float volMult, int pitchOverride, int additionalFlags)
{
	SoundArgs args = getSoundArgs(pos, NULL, channel, volMult, pitchOverride, additionalFlags, false);
	if (!args.valid)
		return false;

	if (args.delay > 0)
		g_Scheduler.SetTimeout(playSoundDelay, args.delay, args);
	else
	{
		uint32_t targets = args.target_ent ? PLRBIT(args.target_ent) : 0xffffffff;
		WaterSoundEffects(args.pos, args);
		StartSound(NULL, args.channel, STRING(args.file), args.volume, args.attn, args.flags,
			args.pitch, args.pos, targets);
	}

	if (args.next_snd) {
		CWeaponCustomSound* nxt = (CWeaponCustomSound*)args.next_snd.GetEntity();
		nxt->next_snd.play(pos, channel, volMult, pitchOverride, additionalFlags);
	}

	return true;
}

int WeaponSound::getPitch()
{
	if (h_options)
	{
		int pitch_rand = h_options->pev->renderfx;
		return h_options->pev->rendermode + RANDOM_LONG(-pitch_rand, pitch_rand);
	}
	return 100;
}

SoundOpts WeaponSound::getOpts() {
	if (h_options) {
		return ((CWeaponCustomSound*)h_options.GetEntity())->getOpts();
	}

	static SoundOpts defaultOpts = {
		0,				// delay
		0,				// file
		CHAN_STATIC,	// channel
		1,				// play mode
		ATTN_IDLE,		// radius
		1.0f,			// volume
		100,			// pitch
		0,				// pitch rand
		false,			// has next
	};

	defaultOpts.file = file;
	return defaultOpts;
}

float WeaponSound::getVolume()
{
	if (h_options)
	{
		return h_options->pev->health / 100.0f;
	}
	return 1.0f;
}

void WeaponSound::stop(CBaseEntity* ent, int channel)
{
	if (!file || !ent)
		return;
	if (h_options && h_options->pev->sequence > -1)
		channel = h_options->pev->sequence;
	STOP_SOUND(ent->edict(), channel, STRING(file));
}
