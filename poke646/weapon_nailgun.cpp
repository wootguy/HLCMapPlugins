#include "extdll.h"
#include "util.h"
#include "CWeaponCustom.h"
#include "te_effects.h"
#include "skill.h"

const float NAILGUN_MOD_DAMAGE = 15.0;
const float NAILGUN_MOD_FIRERATE = 0.10;

enum NAILGUNAnimation
{
	NAILGUN_LONGIDLE = 0,
	NAILGUN_IDLE1,
	NAILGUN_LAUNCH,
	NAILGUN_RELOAD,
	NAILGUN_DEPLOY,
	NAILGUN_FIRE1,
	NAILGUN_FIRE2,
	NAILGUN_FIRE3,
};

const int NAILGUN_DEFAULT_GIVE = 50;
const int NAILGUN_MAX_AMMO = 200;
const int NAILGUN_MAX_CLIP = 50;
const int NAILGUN_WEIGHT = 10;

ItemInfo g_nailgun_info = {
	1,								// iSlot
	-1,								// iPosition (-1 = automatic)
	"9mm",							// pszAmmo1
	NAILGUN_MAX_AMMO,				// iMaxAmmo1
	NULL,							// pszAmmo2
	-1,								// iMaxAmmo2
	"poke646/weapon_nailgun",		// pszName (path to HUD config)
	NAILGUN_MAX_CLIP,				// iMaxClip
	-1,								// iId (-1 = automatic)
	0,								// iFlags
	NAILGUN_WEIGHT,						// iWeight
	0,								// iFlagsEx
	3,								// accuracy degrees
};

class CNailgun : public CWeaponCustom {

	void Spawn()
	{
		m_iDefaultAmmo = NAILGUN_DEFAULT_GIVE;
		m_iId = g_nailgun_info.iId;
		CWeaponCustom::Spawn();
	}

	void Precache() {
		m_defaultModelV = "models/poke646/weapons/nailgun/v_nailgun.mdl";
		m_defaultModelP = "models/poke646/weapons/nailgun/p_nailgun.mdl";
		m_defaultModelW = "models/poke646/weapons/nailgun/w_nailgun.mdl";
		CBasePlayerWeapon::Precache();

		int shootSnd = PRECACHE_SOUND("poke646/weapons/nailgun/nailgun_fire.wav");
		int nailMdl = PRECACHE_MODEL("models/poke646/weapons/nail.mdl");
		int reloadSnd1 = PRECACHE_SOUND("items/cliprelease1.wav");
		int reloadSnd2 = PRECACHE_SOUND("items/clipinsert1.wav");

		PRECACHE_HUD_FILES("sprites/poke646/weapon_nailgun.txt");

		animExt = "mp5";
		//wrongClientWeapon = "weapon_mp5";

		params.flags = FL_WC_WEP_HAS_PRIMARY;
		params.vmodel = MODEL_INDEX(GetModelV());
		params.deployAnim = NAILGUN_DEPLOY;
		params.deployAnimTime = 1000;
		params.maxClip = NAILGUN_MAX_CLIP;
		params.reloadStage[0] = { NAILGUN_RELOAD, 1630 };
		params.idles[0] = { NAILGUN_LONGIDLE, 80, 5380 };
		params.idles[1] = { NAILGUN_IDLE1, 20, 3230 };

		CustomWeaponShootOpts& primary = params.shootOpts[0];
		primary.ammoCost = 1;
		primary.cooldown = 100;
		primary.accuracyX = 3 * 100;
		primary.accuracyY = 3 * 100;
		primary.flags = FL_WC_SHOOT_UNDERWATER;

		float spread = VECTOR_CONE_3DEGREES.x;
		int bulletf = FL_WC_BULLETS_NO_DECAL;

		AddEvent(WepEvt().Primary().WepAnim(NAILGUN_FIRE1).AddAnim(NAILGUN_FIRE2).AddAnim(NAILGUN_FIRE3));
		AddEvent(WepEvt().Primary().PlaySound(shootSnd, CHAN_WEAPON, 1.0f, ATTN_NORM, 94, 109, DISTANT_NONE, WC_AIVOL_NORMAL));
		AddEvent(WepEvt().Primary().PunchSet(-1.5f, 0));
		AddEvent(WepEvt().Primary().Projectile(WC_PROJECTILE_CUSTOM, 4000, spread, spread, Vector(8, 16, -6))
			.ProjClass(ALLOC_STRING("nail"))
			.ProjModel(nailMdl));

		AddEvent(WepEvt().Reload().Delay(170).IdleSound(reloadSnd1));
		AddEvent(WepEvt().Reload().Delay(800).IdleSound(reloadSnd2));

		PrecacheEvents();
	}

	int GetItemInfo(ItemInfo* info) {
		*info = g_nailgun_info;
		return true;
	}
};

class CNail : public CProjectileCustom {
	void Spawn() {
		CProjectileCustom::Spawn();

		Vector vecDir = pev->velocity.Normalize();

		pev->velocity = vecDir * 4000;
		pev->avelocity.z = 10;
		ParametricInterpolation(0.1f);

		UTIL_BeamFollow(entindex(), MODEL_INDEX("sprites/laserbeam.spr"), 1, 1, RGB(64, 64, 64));
	}

	bool CustomTouch(CBaseEntity* pOther) override {
		if (pOther->pev->takedamage)
		{
			TraceResult tr = UTIL_GetGlobalTrace();
			entvars_t* pevOwner;

			pevOwner = VARS(pev->owner);

			ClearMultiDamage();
			pOther->TraceAttack(pevOwner, NAILGUN_MOD_DAMAGE, pev->velocity.Normalize(), &tr, DMG_BULLET | DMG_NEVERGIB);
			ApplyMultiDamage(pev, pevOwner);

			pev->velocity = Vector(0, 0, 0);

			const char* snd = RANDOM_LONG(0, 1) ? "weapons/xbow_hitbod1.wav" : "weapons/xbow_hitbod2.wav";
			EMIT_SOUND(ENT(pev), CHAN_BODY, snd, 1, ATTN_NORM);
		}
		else
		{
			SetThink(&CBaseEntity::SUB_Remove);
			pev->nextthink = gpGlobals->time;// this will get changed below if the bolt is allowed to stick in what it hit.

			if (FClassnameIs(pOther->pev, "worldspawn"))
			{
				// if what we hit is static architecture, can stay around for a while.
				Vector vecDir = pev->velocity.Normalize();
				UTIL_SetOrigin(pev, pev->origin - vecDir * 8);
				pev->angles = UTIL_VecToAngles(vecDir);
				pev->solid = SOLID_NOT;
				pev->movetype = MOVETYPE_FLY;
				pev->velocity = Vector(0, 0, 0);
				pev->avelocity.z = 0;
				pev->angles.z = RANDOM_LONG(0, 360);
				pev->nextthink = gpGlobals->time + 3.0;

				pev->starttime = 0;
				pev->impacttime = 0;
			}

			if (UTIL_PointContents(pev->origin) != CONTENTS_WATER) {
				if (RANDOM_LONG(0,2) == 0)
					UTIL_Sparks(pev->origin);
			}

			return true;
		}

		return false;
	}

	bool CustomThink(void) override
	{
		pev->nextthink = gpGlobals->time + 0.1;
		ParametricInterpolation(0.1f);

		if (pev->waterlevel == 0)
			return true;

		UTIL_BubbleTrail(pev->origin - pev->velocity * 0.1, pev->origin, 1);

		return true;
	}
};

LINK_ENTITY_TO_CLASS(weapon_nailgun, CNailgun);
LINK_ENTITY_TO_CLASS(nail, CNail);