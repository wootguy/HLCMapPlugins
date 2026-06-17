#pragma once
#include "CWeaponCustom.h"
#include "te_effects.h"

#define FUSELOOP "ins2/wpn/tnt/fuse.wav"
#define SPR_EXPLOSION "sprites/ins2/faexplode1.spr"

class ExplosiveBase : public CProjectileCustom {
public:
	void SelfExplode(CBasePlayer* pPlayer);

	void ExplodeMsg01(const Vector& origin, float scale, int framerate);

	void ExplodeMsg02(const Vector& origin, float scale, int framerate);

	void SmokeMsg(const Vector& origin, float scale, int framerate, const char* spr_path = "sprites/steam1.spr");

	void Smoke();

protected:
	// For Grenade Explosion Sounds
	static const char* GrenadeExplode[3];
	static const char* GrenadeWaterExplode[3];
	// For Grenade Bounce Sounds
	static const char* GrenadeBounce[4];
	// For Rocket Explosion Sounds
	static const char* RocketExplode[3];
	static const char* RocketWaterExplode[3];
	// For C4s, Shaped Charges
	static const char* ChargeExplode[3];
	static const char* ChargeWaterExplode[3];
	// For C4/TNT Bounce Sounds
	static const char* C4Bounce[3];
	static const char* TNTBounce[3];
};