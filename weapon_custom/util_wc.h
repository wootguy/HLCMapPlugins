#pragma once
#include "const_wc.h"
#include "WeaponSound.h"
#include <vector>

class CWeaponCustomShoot;

template<typename T, uint16_t Capacity>
struct PodArray
{
	T data[Capacity];
	uint16_t size;

	bool add(const T& value)
	{
		if (size >= Capacity)
			return false;

		data[size++] = value;
		return true;
	}
};

struct DecalTarget
{
	Vector pos;
	TraceResult tr;
	const char* texture;
	EHANDLE ent; // for when the target is a brush entity, not the world (0 = world)
};

enum impact_decal_type
{
	DECAL_NONE = -2,
	DECAL_BIGBLOOD = 0,
	DECAL_BIGSHOT,
	DECAL_BLOOD,
	DECAL_BLOODHAND,
	DECAL_BULLETPROOF,
	DECAL_GLASSBREAK,
	DECAL_LETTERS,
	DECAL_SMALLCRACK,
	DECAL_LARGECRACK,
	DECAL_LARGEDENT,
	DECAL_SMALLDENT,
	DECAL_DING,
	DECAL_RUST,
	DECAL_FEET,
	DECAL_GARGSTOMP,
	DECAL_GUASS,
	DECAL_GRAFITTI,
	DECAL_HANDICAP,
	DECAL_MOMMABLOB,
	DECAL_SMALLSCORTCH,
	DECAL_MEDIUMSCORTCH,
	DECAL_TINYSCORTCH,
	DECAL_OIL,
	DECAL_LARGESCORTCH,
	DECAL_SMALLSHOT,
	DECAL_NUMBERS,
	DECAL_TINYSCORTCH2,
	DECAL_SMALLSCORTCH2,
	DECAL_SPIT,
	DECAL_BIGABLOOD,
	DECAL_TARGET,
	DECAL_TIRE,
	DECAL_ABLOOD,
	DECAL_TYPES,
};

bool HandleKv(KeyValueData* pkvd, const char* matchKey);

void parseSounds(KeyValueData* pkvd, PodArray<WeaponSound, MAX_KV_ARRAY>& sounds);

void parseStrings(KeyValueData* pkvd, PodArray<string_t, MAX_KV_ARRAY>& arr);

DecalTarget getProjectileDecalTarget(CBaseEntity* ent, Vector pos, float searchDist);

const char* getDecal(int decalType);

const char* getBulletDecalOverride(CBaseEntity* ent, const char* currentDecal);

Vector resizeVector(Vector v, float length);

int getMonsterClass(CBaseMonster* mon);

bool isHuman(CBaseEntity* ent);

bool isAlien(CBaseEntity* ent);

bool shouldHealTarget(CBaseEntity* target, CBaseEntity* healer, CWeaponCustomShoot* shoot_opts);

float applyDamageModifiers(float damage, CBaseEntity* target, CBaseEntity* plr, CWeaponCustomShoot* shoot_opts);

void knockBack(CBaseEntity* target, Vector vel);

const char* cstr2(string_t s); // because cstr doesn't work in the debugger