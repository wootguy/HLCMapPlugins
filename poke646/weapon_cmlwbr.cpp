#include "extdll.h"
#include "util.h"
#include "CWeaponCustom.h"
#include "te_effects.h"
#include "skill.h"

const int CMLWBR_MOD_PROJ_SPEED = 2000;
const int CMLWBR_MOD_PROJ_SPEED_UNDERWATER = 1000;

const int CMLWBR_DEFAULT_AMMO = 5;
const int CMLWBR_MAX_CARRY = 20;
const int CMLWBR_MAX_CLIP = 5;
const int CMLWBR_WEIGHT = 5;

enum CMLWBRAnimation
{
	CMLWBR_IDLE1 = 0,
	CMLWBR_IDLE2,
	CMLWBR_FIDGET1,
	CMLWBR_FIDGET2,
	CMLWBR_FIRE,
	CMLWBR_FIRE_LAST,
	CMLWBR_RELOAD,
	CMLWBR_RELOAD_EMPTY,
	CMLWBR_DRAW
};

ItemInfo g_cmlwbr_info = {
	3,								// iSlot
	-1,								// iPosition (-1 = automatic)
	"bolts",						// pszAmmo1
	CMLWBR_MAX_CARRY,				// iMaxAmmo1
	NULL,							// pszAmmo2
	-1,								// iMaxAmmo2
	"poke646/weapon_cmlwbr",// pszName (path to HUD config)
	CMLWBR_MAX_CLIP,				// iMaxClip
	-1,								// iId (-1 = automatic)
	0,								// iFlags
	CMLWBR_WEIGHT,					// iWeight
	WEP_FLAG_USE_ZOOM_CROSSHAIR,	// iFlagsEx
	0								// accuracy degrees
};

class CCmlwbr : public CWeaponCustom {

	float m_lastText;

	void Spawn()
	{
		m_iDefaultAmmo = CMLWBR_DEFAULT_AMMO;
		m_iId = g_cmlwbr_info.iId;
		CWeaponCustom::Spawn();
	}

	void Precache() {
		m_defaultModelV = "models/poke646/weapons/cmlwbr/v_cmlwbr.mdl";
		m_defaultModelP = "models/poke646/weapons/cmlwbr/p_cmlwbr.mdl";
		m_defaultModelW = "models/poke646/weapons/cmlwbr/w_cmlwbr.mdl";
		CBasePlayerWeapon::Precache();

		int shootSnd = PRECACHE_SOUND("poke646/weapons/cmlwbr/cmlwbr_fire.wav");
		int zoomSnd = PRECACHE_SOUND("poke646/weapons/cmlwbr/cmlwbr_zoom.wav");
		int reloadSnd = PRECACHE_SOUND("poke646/weapons/cmlwbr/cmlwbr_reload.wav");
		int reloadSnd2 = PRECACHE_SOUND("poke646/weapons/cmlwbr/cmlwbr_reload_empty.wav");
		int drawbackSnd = PRECACHE_SOUND("poke646/weapons/cmlwbr/cmlwbr_drawback.wav");
		
		int boltMdl = PRECACHE_MODEL("models/poke646/items/crossbow_bolt.mdl");

		PRECACHE_HUD_FILES("sprites/poke646/weapon_cmlwbr.txt");

		animExt = "bow";
		animExtZoom = "bowscope";
		wrongClientWeapon = "weapon_crossbow";

		params.flags = FL_WC_WEP_HAS_PRIMARY | FL_WC_WEP_HAS_SECONDARY | FL_WC_WEP_UNLINK_COOLDOWNS
			| FL_WC_WEP_ZOOM_SPR_STRETCH | FL_WC_WEP_ZOOM_SPR_ASPECT | FL_WC_WEP_EMPTY_IDLES;
		params.vmodel = MODEL_INDEX(GetModelV());
		params.deployAnim = CMLWBR_DRAW;
		params.deployAnimTime = 600;
		params.maxClip = CMLWBR_MAX_CLIP;
		params.reloadStage[0] = { CMLWBR_RELOAD, 7530 };
		params.reloadStage[1] = { CMLWBR_RELOAD_EMPTY, 6100 };
		params.idles[0] = { CMLWBR_IDLE1, 80, 3100 };
		params.idles[1] = { CMLWBR_FIDGET1, 20, 2770 };
		params.idles[2] = { CMLWBR_IDLE2, 80, 3100 };
		params.idles[3] = { CMLWBR_FIDGET2, 20, 2770 };

		CustomWeaponShootOpts& primary = params.shootOpts[0];
		primary.ammoCost = 1;
		primary.cooldown = 1500;
		primary.accuracyX = 0 * 100;
		primary.accuracyY = 0 * 100;
		primary.flags = FL_WC_SHOOT_UNDERWATER;

		CustomWeaponShootOpts& secondary = params.shootOpts[1];
		secondary.ammoCost = 0;
		secondary.cooldown = 500;
		secondary.flags = FL_WC_SHOOT_NO_ATTACK;

		AddEvent(WepEvt().Primary().WepAnim(CMLWBR_FIRE));
		AddEvent(WepEvt().PrimaryEmpty().WepAnim(CMLWBR_FIRE_LAST));
		AddEvent(WepEvt().PrimaryNotEmpty().Delay(280).IdleSound(drawbackSnd));
		AddEvent(WepEvt().Primary().PlaySound(shootSnd, CHAN_WEAPON, 1.0f, ATTN_NORM, 94, 109, DISTANT_NONE, WC_AIVOL_QUIET));
		AddEvent(WepEvt().Primary()
			.Projectile(WC_PROJECTILE_CUSTOM, 2000)
			.ProjClass(ALLOC_STRING("cmlwbr_bolt"))
			.ProjModel(boltMdl));

		AddEvent(WepEvt().Secondary().IdleSound(zoomSnd));
		AddEvent(WepEvt().Secondary().ToggleZoom(40, 20));

		AddEvent(WepEvt().ReloadNotEmpty().Delay(150).IdleSound(reloadSnd));
		AddEvent(WepEvt().ReloadEmpty().Delay(150).IdleSound(reloadSnd2));

		PrecacheEvents();
	}

	int GetItemInfo(ItemInfo* info) {
		*info = g_cmlwbr_info;
		return true;
	}

	void ItemPostFrame() override {
		CWeaponCustom::ItemPostFrame();

		CBasePlayer* m_pPlayer = GetPlayer();
		if (!m_pPlayer)
			return;

		if (gpGlobals->time - m_lastText < 0.05f) {
			return;
		}

		m_lastText = gpGlobals->time;

		if (m_pPlayer->m_iFOV) {
			hudconparms_t params;
			memset(&params, 0, sizeof(hudconparms_t));
			params.r = 255;
			params.xEm = 20;
			params.yEm = -2;
			params.yPercent = 0.5f;
			params.xPercent = 0.5f;
			params.holdTime = 100;
			params.id = 1;

			TraceResult tr;
			Vector vecSrc = m_pPlayer->GetGunPosition();
			Vector vecEnd = vecSrc + m_pPlayer->GetLookDirection() * 8192;
			UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, m_pPlayer->edict(), &tr);
			float dist = (tr.vecEndPos - vecSrc).Length() * 0.025f;

			const char* msg = UTIL_VarArgs("%.2f m", dist);

			UTIL_ClientHudConPrint(m_pPlayer, params, msg, false);
		}
	}
};

class CCmlwbrBolt : public CProjectileCustom {
	void Spawn() {
		CProjectileCustom::Spawn();

		Vector vecDir = pev->velocity.Normalize();

		pev->velocity = vecDir * (pev->waterlevel == 3 ?
			CMLWBR_MOD_PROJ_SPEED_UNDERWATER : CMLWBR_MOD_PROJ_SPEED);
		pev->avelocity.z = 10;
		ParametricInterpolation(0.1f);
	}

	bool CustomTouch(CBaseEntity* pOther) override {
		if (pOther->pev->takedamage)
		{
			TraceResult tr = UTIL_GetGlobalTrace();
			entvars_t* pevOwner;

			pevOwner = VARS(pev->owner);

			ClearMultiDamage();
			pOther->TraceAttack(pevOwner, GetDamage(gSkillData.sk_plr_xbow_bolt_monster), pev->velocity.Normalize(), &tr, DMG_BULLET | DMG_NEVERGIB);
			ApplyMultiDamage(pev, pevOwner);

			pev->velocity = Vector(0, 0, 0);

			const char* snd = RANDOM_LONG(0, 1) ? "weapons/xbow_hitbod1.wav" : "weapons/xbow_hitbod2.wav";
			EMIT_SOUND(ENT(pev), CHAN_BODY, snd, 1, ATTN_NORM);
		}
		else
		{
			EMIT_SOUND_DYN(ENT(pev), CHAN_BODY, "weapons/xbow_hit1.wav", RANDOM_FLOAT(0.95, 1.0), ATTN_NORM, 0, 98 + RANDOM_LONG(0, 7));

			SetThink(&CBaseEntity::SUB_Remove);
			pev->nextthink = gpGlobals->time;// this will get changed below if the bolt is allowed to stick in what it hit.

			if (FClassnameIs(pOther->pev, "worldspawn"))
			{
				// if what we hit is static architecture, can stay around for a while.
				Vector vecDir = pev->velocity.Normalize();
				UTIL_SetOrigin(pev, pev->origin - vecDir * 12);
				pev->angles = UTIL_VecToAngles(vecDir);
				pev->solid = SOLID_NOT;
				pev->movetype = MOVETYPE_FLY;
				pev->velocity = Vector(0, 0, 0);
				pev->avelocity.z = 0;
				pev->angles.z = RANDOM_LONG(0, 360);
				pev->nextthink = gpGlobals->time + 10.0;

				pev->starttime = 0;
				pev->impacttime = 0;
			}

			if (UTIL_PointContents(pev->origin) != CONTENTS_WATER) {
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

LINK_ENTITY_TO_CLASS(weapon_cmlwbr, CCmlwbr);
LINK_ENTITY_TO_CLASS(cmlwbr_bolt, CCmlwbrBolt);