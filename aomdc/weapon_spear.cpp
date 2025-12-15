#include "extdll.h"
#include "util.h"
#include "CWeaponCustom.h"
#include "te_effects.h"
#include "skill.h"
#include "shake.h"

enum SpearAnim
{
	SPEAR_IDLE,
	SPEAR_STAB_START,
	SPEAR_STAB_MISS,
	SPEAR_STAB,
	SPEAR_DRAW,
	SPEAR_ELECTROCUTED,
};

ItemInfo g_spear_info = {
	0,								// iSlot
	0,								// iPosition (-1 = automatic)
	NULL,							// pszAmmo1
	-1,								// iMaxAmmo1
	NULL,							// pszAmmo2
	-1,								// iMaxAmmo2
	"aomdc/weapon_spear",			// pszName (path to HUD config)
	-1,								// iMaxClip
	-1,								// iId (-1 = automatic)
	0,								// iFlags
	5,								// iWeight
};

class CSpear : public CWeaponCustom {
	const char* electroSnd = "aomdc/weapons/spear_electrocute.wav";
	static const char* thunderSounds[3];

	int m_electrocuteSnd;
	int m_thunderSpr;
	float m_nextElectro;
	float resetSpeedTime;

	void Spawn()
	{
		pev->classname = ALLOC_STRING("weapon_spear");
		m_iId = g_spear_info.iId;
		CWeaponCustom::Spawn();
	}

	int GetItemInfo(ItemInfo* info) {
		*info = g_spear_info;
		return true;
	}

	virtual const char* GetDeathNoticeWeapon() { return "weapon_crowbar"; }

	void Precache() {
		m_defaultModelV = "models/aomdc/v_spear.mdl";
		m_defaultModelP = "models/aomdc/p_spear.mdl";
		m_defaultModelW = "models/aomdc/w_spear.mdl";
		CBasePlayerWeapon::Precache();

		PRECACHE_SOUND(electroSnd);
		PRECACHE_SOUND_ARRAY(thunderSounds);
		m_thunderSpr = PRECACHE_MODEL("sprites/lgtning.spr");

		PRECACHE_HUD_FILES("sprites/aomdc/weapon_spear.txt");

		animExt = "gauss";

		params.flags = FL_WC_WEP_HAS_PRIMARY | FL_WC_WEP_USE_ONLY | FL_WC_WEP_NO_PREDICTION;
		params.vmodel = MODEL_INDEX(GetModelV());
		params.deployAnim = SPEAR_DRAW;
		params.deployAnimTime = 910;
		params.idles[0] = { SPEAR_IDLE, 100, 1000 };

		CustomWeaponShootOpts& primary = params.shootOpts[0];
		primary.flags = FL_WC_SHOOT_UNDERWATER | FL_WC_SHOOT_IS_MELEE | FL_WC_SHOOT_CHARGEUP_ONCE;
		primary.melee.damage = 50;
		primary.melee.range = 48;
		primary.melee.attackOffset = Vector(0, -5, 5);
		primary.chargeTime = 300;
		primary.chargeCancelTime = 300;
		primary.melee.missCooldown = 700;
		primary.melee.hitCooldown = 700;
		primary.melee.decalDelay = 0;
		primary.melee.damageBits = DMG_SLASH | DMG_SHOCK | DMG_NEVERGIB; // shock used for unique david boss damage
		primary.melee.missSounds[0] = PRECACHE_SOUND("aomdc/weapons/spear_swing.wav");
		primary.melee.hitWallSounds[0] = PRECACHE_SOUND("aomdc/weapons/spear_hitwall.wav");
		primary.melee.hitFleshSounds[0] = PRECACHE_SOUND("aomdc/weapons/spear_stab.wav");

		params.shootOpts[1] = params.shootOpts[0];

		AddEvent(WepEvt().PrimaryCharge().WepAnim(SPEAR_STAB_START));

		PrecacheEvents();
	}

	void RestoreMoveSpeed() {
		CBasePlayer* m_pPlayer = GetPlayer();
		if (!m_pPlayer)
			return;

		if (resetSpeedTime && resetSpeedTime < gpGlobals->time) {
			m_pPlayer->m_speed_modifier = 0;
			resetSpeedTime = 0;
			m_pPlayer->ApplyEffects();
		}
	}

	void ItemPostFrame() override {
		CWeaponCustom::ItemPostFrame();
		RestoreMoveSpeed();

		CBasePlayer* m_pPlayer = GetPlayer();
		if (!m_pPlayer)
			return;

		if (!m_nextElectro) {
			m_nextElectro = gpGlobals->time + RANDOM_FLOAT(8, 12);
		}

		// in the original game, the shock effect comes randomly from the walls and behind you and
		// stuff. Here I'm making it look more like the spear is the reason you get electroctued.
		if (m_nextElectro < gpGlobals->time) {
			Vector randomDir = Vector(RANDOM_FLOAT(-1, 1), RANDOM_FLOAT(-1, 1), RANDOM_FLOAT(-0.7f, 0.7f)).Normalize();

			TraceResult tr;
			Vector vecSrc = m_pPlayer->pev->origin;
			UTIL_TraceLine(vecSrc, vecSrc + randomDir * 4096, ignore_monsters, NULL, &tr);

			EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(thunderSounds), 1.0, ATTN_NORM, 0, RANDOM_LONG(90, 110));

			UTIL_BeamEntPoint(m_pPlayer->entindex(), 1, tr.vecEndPos, m_thunderSpr,
				0, 0, 2, 50, 96, RGBA(64, 180, 255, 255), 0, MSG_PVS, pev->origin);

			m_pPlayer->TakeDamage(pev, pev, 10, DMG_SHOCK);

			// more spaced out for co-op so there's less spam with lots of players.
			// the original hits every 10 seconds.
			m_nextElectro = gpGlobals->time + RANDOM_FLOAT(8, 12);
		}
	}
	
	void Holster(int skiplocal) override {
		CWeaponCustom::Holster(skiplocal);
		RestoreMoveSpeed();
	}

	bool Chargeup(int attackIdx, bool leftHand, bool akimboFire) override {
		CBasePlayer* m_pPlayer = GetPlayer();
		if (!m_pPlayer)
			return false;

		if (m_pPlayer->pev->waterlevel > WATERLEVEL_DRY) {
			EMIT_SOUND(m_pPlayer->edict(), CHAN_WEAPON, electroSnd, 1, ATTN_NORM);
			UTIL_ScreenFade(m_pPlayer, Vector(255, 255, 255), 1.0f, 0, 255, FFADE_IN);
			m_pPlayer->TakeDamage(m_pPlayer->pev, m_pPlayer->pev, 10, DMG_SHOCK);
			SendWeaponAnimSpec(SPEAR_ELECTROCUTED);
			Cooldown(attackIdx, 2340);
			return false;
		}

		return CWeaponCustom::Chargeup(attackIdx, leftHand, akimboFire);
	}

	bool MeleeHit(CBasePlayer* plr, CBaseEntity* target) override {
		UTIL_ScreenShake(plr, 6, 5, 0.7f);
		SendWeaponAnimSpec(SPEAR_STAB);
		plr->m_speed_modifier = 0.001f;
		plr->pev->velocity = g_vecZero;
		resetSpeedTime = gpGlobals->time + 0.4f;
		plr->ApplyEffects();
		return false;
	}

	void MeleeMiss(CBasePlayer* plr) override {
		SendWeaponAnimSpec(SPEAR_STAB_MISS);
	}
};

const char* CSpear::thunderSounds[3] = {
	"aomdc/davidbad/thunder_attack1.wav",
	"aomdc/davidbad/thunder_attack2.wav",
	"aomdc/davidbad/thunder_attack3.wav",
};

LINK_ENTITY_TO_CLASS(weapon_spear, CSpear)
LINK_ENTITY_TO_CLASS(weapon_Spear, CSpear)