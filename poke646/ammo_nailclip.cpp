#include "CBasePlayerAmmo.h"

const int AMMO_NAIL_GIVE = 25;
const int AMMO_NAIL_MAX_CARRY = 200;

class CNailClip : public CBasePlayerAmmo
{
	void Spawn(void)
	{
		Precache();
		SET_MODEL(ENT(pev), "models/poke646/items/w_nailclip.mdl");
		CBasePlayerAmmo::Spawn();
	}
	void Precache(void)
	{
		PRECACHE_MODEL("models/poke646/items/w_nailclip.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	BOOL AddAmmo(CBaseEntity* pOther)
	{
		int bResult = (pOther->GiveAmmo(AMMO_NAIL_GIVE, "9mm", AMMO_NAIL_MAX_CARRY) != -1);
		if (bResult)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
		}
		return bResult;
	}
};

LINK_ENTITY_TO_CLASS(ammo_nailclip, CNailClip)
