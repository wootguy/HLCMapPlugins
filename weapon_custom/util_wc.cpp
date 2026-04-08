#include "extdll.h"
#include "util.h"
#include "CBasePlayer.h"
#include "WeaponSound.h"
#include "CWeaponCustomSound.h"
#include "CWeaponCustomShoot.h"
#include "weapon_custom.h"
#include "Scheduler.h"
#include "util_wc.h"

bool HandleKv(KeyValueData* pkvd, const char* matchKey) {
	if (FStrEq(pkvd->szKeyName, matchKey)) {
		pkvd->fHandled = true;
		return true;
	}

	return false;
}

void parseSounds(KeyValueData* pkvd, PodArray<WeaponSound, MAX_KV_ARRAY>& sounds)
{
	vector<string> strings = splitString(pkvd->szValue, ";");
	for (int i = 0; i < strings.size(); i++)
	{
		WeaponSound s;
		s.file = ALLOC_STRING(strings[i].c_str());
		sounds.add(s);
	}
}

void parseStrings(KeyValueData* pkvd, PodArray<string_t, MAX_KV_ARRAY>& arr)
{
	vector<string> strings = splitString(pkvd->szValue, ";");
	for (int i = 0; i < strings.size(); i++) {
		arr.add(ALLOC_STRING(strings[i].c_str()));
	}
}

// Traces out in every direction in hopes of finding a surface
DecalTarget getProjectileDecalTarget(CBaseEntity* ent, Vector pos, float searchDist)
{
	DecalTarget decalTarget = DecalTarget();
	decalTarget.pos = ent ? ent->pev->origin : pos;

	TraceResult tr;
	Vector src = decalTarget.pos;

	float bboxSize = 0;
	if (ent)
		bboxSize = fabs(ent->pev->maxs.x - ent->pev->mins.x);

	static vector<Vector> dirs = {
		// Box sides
		Vector(1, 0, 0),
		Vector(-1, 0, 0),
		Vector(0, 1, 0),
		Vector(0, -1, 0),
		Vector(0, 0, 1),
		Vector(0, 0, -1),
		// Box corners
		Vector(-0.577350, -0.577350, -0.577350),
		Vector(0.577350, -0.577350, -0.577350),
		Vector(-0.577350, 0.577350, -0.577350),
		Vector(-0.577350, -0.577350, 0.577350),
		Vector(0.577350, -0.577350, 0.577350),
		Vector(0.577350, 0.577350, -0.577350),
		Vector(0.577350, 0.577350, 0.577350),
		Vector(-0.577350, 0.577350, 0.577350)
	};

	for (int i = 0; i < dirs.size(); i++)
	{
		Vector end = src + dirs[i] * (bboxSize + searchDist);
		edict_t* edict = ent ? ent->edict() : NULL;
		UTIL_TraceLine(src, end, ignore_monsters, edict, &tr);

		//te_beampoints(src, end);

		if (tr.flFraction < 1.0)
		{
			decalTarget.pos = tr.vecEndPos;
			decalTarget.tr = tr;
			if (tr.pHit)
			{
				decalTarget.ent = EHANDLE(tr.pHit);
				// get the texture too, we might need that for something
				decalTarget.texture = TRACE_TEXTURE(tr.pHit, src, end);
			}

			return decalTarget;
		}
	}

	return decalTarget;
}

vector< vector<const char*> > g_decals = {
	{"{bigblood1", "{bigblood2"},
	{"{bigshot1", "{bigshot2", "{bigshot3", "{bigshot4", "{bigshot5"},
	{"{blood1", "{blood2", "{blood3", "{blood4", "{blood5", "{blood6", "{blood7", "{blood8"},
	{"{bloodhand1", "{bloodhand2", "{bloodhand3", "{bloodhand4", "{bloodhand5", "{bloodhand6"},
	{"{bproof1"},
	{"{break1", "{break2", "{break3"},
	{"{capsa", "{capsb", "{capsc", "{capsd", "{capse", "{capsf", "{capsg", "{capsh", "{capsi", "{capsj",
		"{capsk", "{capsl", "{capsm", "{capsn", "{capso", "{capsp", "{capsq", "{capsr", "{capss", "{capst",
		"{capsu", "{capsv", "{capsw", "{capsx", "{capsy", "{capsz"},
	{"{crack1", "{crack2"},
	{"{crack3", "{crack4"},
	{"{dent1", "{dent2"},
	{"{dent3", "{dent4", "{dent5", "{dent6"},
	{"{ding3", "{ding4", "{ding5", "{ding6", "{ding7", "{ding8", "{ding9"},
	{"{ding10", "{ding11"},
	{"{foot_l", "{foot_r"},
	{"{gargstomp"},
	{"{gaussshot1"},
	{"{graf001", "{graf002", "{graf003", "{graf004", "{graf005"},
	{"{handi"},
	{"{mommablob"},
	{"{ofscorch1", "{ofscorch2", "{ofscorch3"},
	{"{ofscorch4", "{ofscorch5", "{ofscorch6"},
	{"{ofsmscorch1", "{ofsmscorch2", "{ofsmscorch3"},
	{"{oil1", "{oil2"},
	{"{scorch1", "{scorch2", "{scorch3"},
	{"{shot1", "{shot2", "{shot3", "{shot4", "{shot5"},
	{"{small#s0", "{small#s1", "{small#s2", "{small#s3", "{small#s4",
		"{small#s5", "{small#s6", "{small#s7", "{small#s8", "{small#s9"},
	{"{smscorch1", "{smscorch2"},
	{"{smscorch3"},
	{"{spit1", "{spit2"},
	{"{spr_splt1", "{spr_splt2", "{spr_splt3"},
	{"{target", "{target2"},
	{"{tire1", "{tire2"},
	{"{yblood1", "{yblood2", "{yblood3", "{yblood4", "{yblood5", "{yblood6"},
};

const char* getDecal(int decalType)
{
	if (decalType < 0 || decalType >= int(g_decals.size()))
		decalType = DECAL_SMALLSHOT;

	vector<const char*> decals = g_decals[decalType];
	return decals[RANDOM_LONG(0, decals.size() - 1)];
}

const char* getBulletDecalOverride(CBaseEntity* ent, const char* currentDecal)
{
	TraceResult tr;
	if (ent && ent->IsBreakable())
	{
		if (ent->pev->playerclass == 1) // learned this from HLSDK func_break.cpp line 158
			return getDecal(DECAL_GLASSBREAK);
		if (ent->TakeDamage(ent->pev, ent->pev, 0, DMG_GENERIC) == 0) // TODO: Don't do this, it makes sound
			return getDecal(DECAL_BULLETPROOF); // only unbreakable glass can't take damage
	}

	return currentDecal;
}

Vector resizeVector(Vector v, float length)
{
	float d = length / sqrt((v.x * v.x) + (v.y * v.y) + (v.z * v.z));
	v.x *= d;
	v.y *= d;
	v.z *= d;
	return v;
}

int getMonsterClass(CBaseMonster* mon)
{
	if (mon->Classify() == CLASS_PLAYER_ALLY)
	{
		// PLAYER_ALLY masks the actual monster class, so remove that for a sec
		mon->m_IsPlayerAlly = false;
		int c = mon->Classify();
		mon->m_IsPlayerAlly = true;
		return c;
	}
	return mon->Classify();
}

bool isHuman(CBaseEntity* ent)
{
	if (ent->IsMonster()) {
		CBaseMonster* mon = ent->MyMonsterPointer();
		int c = getMonsterClass(mon);
		switch (c)
		{
		case CLASS_PLAYER:
		case CLASS_HUMAN_PASSIVE:
		case CLASS_HUMAN_MILITARY:
			return true;
		}
	}
	return false;
}

bool isAlien(CBaseEntity* ent)
{
	if (ent->IsMonster()) {
		CBaseMonster* mon = ent->MyMonsterPointer();
		int c = getMonsterClass(mon);

		switch (c)
		{
		case CLASS_ALIEN_MILITARY:
		case CLASS_ALIEN_PASSIVE:
		case CLASS_ALIEN_MONSTER:
		case CLASS_ALIEN_PREY:
		case CLASS_ALIEN_PREDATOR:
		case CLASS_PLAYER_BIOWEAPON:
		case CLASS_ALIEN_BIOWEAPON:
		case CLASS_ALIEN_RACE_X:
		case CLASS_ALIEN_RACE_X_PITDRONE:
		case CLASS_BARNACLE:
			return true;
		}
	}
	return false;
}


bool shouldHealTarget(CBaseEntity* target, CBaseEntity* healer, CWeaponCustomShoot* shoot_opts)
{
	if (shoot_opts->heal_mode == HEAL_OFF)
		return false;

	int mode = shoot_opts->heal_mode;

	int heals = shoot_opts->heal_targets;
	bool healAll = heals == HEALT_EVERYTHING;
	bool healMachines = heals == HEALT_MACHINES || heals == HEALT_MACHINES_AND_BREAKABLES;
	bool healHumans = heals == HEALT_HUMANS || heals == HEALT_HUMANS_AND_ALIENS;
	bool healAliens = heals == HEALT_ALIENS || heals == HEALT_HUMANS_AND_ALIENS;
	bool healBreakables = heals == HEALT_BREAKABLES || heals == HEALT_MACHINES_AND_BREAKABLES;

	// breakables ignore friendly status for whatever reason
	if (target->IsBSPModel() && target->IsRepairable() && (healBreakables || healAll))
		return true;

	int rel = healer->IRelationship(target);
	bool isFriendly = rel == R_AL || rel == R_NO;
	bool healFriend = mode == HEAL_FRIENDS || mode == HEAL_REVIVE_FRIENDS || mode == HEAL_ALL;
	bool healFoe = mode == HEAL_FOES || mode == HEAL_REVIVE_FOES || mode == HEAL_ALL;

	if ((isFriendly && healFriend) || (!isFriendly && healFoe))
	{
		if (healAll) return true;
		if (target->IsMachine() && healMachines) return true;
		if (isHuman(target) && healHumans) return true;
		if (isAlien(target) && healAliens) return true;
	}
	return false;
}


float applyDamageModifiers(float damage, CBaseEntity* target, CBaseEntity* plr, CWeaponCustomShoot* shoot_opts)
{
	// don't do any damage if target is friendly && npc_kill is set to 0 || 2
	bool ignoreDmg = false;
	if (target->IsMonster()) {
		CBaseMonster* mon = target->MyMonsterPointer();
		if (mon->IsImmune(plr->pev, damage)) {
			damage = 0;
		}
	}

	bool didHeal = false;
	if (shouldHealTarget(target, plr, shoot_opts))
	{
		didHeal = true;
		damage = -damage;
	}

	// Award player with poitns (TODO: Account for hitgroup multipliers)
	target->GiveScorePoints(plr->pev, didHeal ? -damage : damage);

	return damage;
}


void knockBack(CBaseEntity* target, Vector vel)
{
	if (target->IsMonster() && !target->IsMachine())
		target->pev->velocity = target->pev->velocity + vel;
}

const char* cstr2(string_t s) {
	return STRING(s);
}