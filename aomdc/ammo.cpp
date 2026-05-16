#include "extdll.h"
#include "util.h"
#include "CWeaponCustom.h"
#include "te_effects.h"
#include "skill.h"
#include "CBasePlayerAmmo.h"

#define BERETTA_MAX_CLIP 15
#define DEAGLE_MAX_CLIP 7
#define GLOCK_MAX_CLIP 20
#define MP5K_MAX_CLIP 30
#define P228_MAX_CLIP 13
#define REVOLVER_MAX_CLIP 6
#define UZI_MAX_CLIP 25

class CAmmoBeretta : public CBasePlayerAmmo
{
	void Precache(void) {
		m_defaultModel = "models/aomdc/w_weaponclips/w_berettaclip.mdl";
		CBasePlayerAmmo::Precache();
		PRECACHE_SOUND("items/9mmclip1.wav");
	}

	BOOL AddAmmo(CBaseEntity* pOther) {
		int bResult = pOther->GiveAmmo(BERETTA_MAX_CLIP, "9mm") != -1;
		if (bResult) {
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
		}
		return bResult;
	}
};

LINK_ENTITY_TO_CLASS(ammo_beretta, CAmmoBeretta)

class CAmmoDeagle : public CBasePlayerAmmo
{
	void Precache(void) {
		m_defaultModel = "models/aomdc/w_weaponclips/w_deagleclip.mdl";
		CBasePlayerAmmo::Precache();
		PRECACHE_SOUND("items/9mmclip1.wav");
	}

	BOOL AddAmmo(CBaseEntity* pOther) {
		int bResult = pOther->GiveAmmo(DEAGLE_MAX_CLIP, "357") != -1;
		if (bResult) {
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
		}
		return bResult;
	}
};

LINK_ENTITY_TO_CLASS(ammo_deagle, CAmmoDeagle)

class CAmmoGlock : public CBasePlayerAmmo
{
	void Precache(void) {
		m_defaultModel = "models/aomdc/w_weaponclips/w_glockclip.mdl";
		CBasePlayerAmmo::Precache();
		PRECACHE_SOUND("items/9mmclip1.wav");
	}

	BOOL AddAmmo(CBaseEntity* pOther) {
		int bResult = pOther->GiveAmmo(GLOCK_MAX_CLIP, "9mm") != -1;
		if (bResult) {
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
		}
		return bResult;
	}
};

LINK_ENTITY_TO_CLASS(ammo_glock, CAmmoGlock)

class CAmmoMp5k : public CBasePlayerAmmo
{
	void Precache(void) {
		m_defaultModel = "models/aomdc/w_weaponclips/w_mp5kclip.mdl";
		CBasePlayerAmmo::Precache();
		PRECACHE_SOUND("items/9mmclip1.wav");
	}

	BOOL AddAmmo(CBaseEntity* pOther) {
		int bResult = pOther->GiveAmmo(MP5K_MAX_CLIP, "9mm") != -1;
		if (bResult) {
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
		}
		return bResult;
	}
};

LINK_ENTITY_TO_CLASS(ammo_mp5k, CAmmoMp5k)

class CAmmoP228 : public CBasePlayerAmmo
{
	void Precache(void) {
		m_defaultModel = "models/aomdc/w_weaponclips/w_p228clip.mdl";
		CBasePlayerAmmo::Precache();
		PRECACHE_SOUND("items/9mmclip1.wav");
	}

	BOOL AddAmmo(CBaseEntity* pOther) {
		int bResult = pOther->GiveAmmo(P228_MAX_CLIP, "9mm") != -1;
		if (bResult) {
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
		}
		return bResult;
	}
};

LINK_ENTITY_TO_CLASS(ammo_p228, CAmmoP228)
LINK_ENTITY_TO_CLASS(ammo_P228, CAmmoP228)

class CAmmoRevolver : public CBasePlayerAmmo
{
	void Precache(void) {
		m_defaultModel = "models/aomdc/w_weaponclips/w_revolverrounds.mdl";
		CBasePlayerAmmo::Precache();
		PRECACHE_SOUND("items/9mmclip1.wav");
	}

	BOOL AddAmmo(CBaseEntity* pOther) {
		int bResult = pOther->GiveAmmo(REVOLVER_MAX_CLIP, "357") != -1;
		if (bResult) {
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
		}
		return bResult;
	}
};

LINK_ENTITY_TO_CLASS(ammo_revolver, CAmmoRevolver)

class CAmmoAomUzi : public CBasePlayerAmmo
{
	void Precache(void) {
		m_defaultModel = "models/aomdc/w_weaponclips/w_uziclip.mdl";
		CBasePlayerAmmo::Precache();
		PRECACHE_SOUND("items/9mmclip1.wav");
	}

	BOOL AddAmmo(CBaseEntity* pOther) {
		int bResult = pOther->GiveAmmo(UZI_MAX_CLIP, "9mm") != -1;
		if (bResult) {
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
		}
		return bResult;
	}
};

LINK_ENTITY_TO_CLASS(ammo_uzi, CAmmoAomUzi)
LINK_ENTITY_TO_CLASS(ammo_aom_uzi, CAmmoAomUzi)