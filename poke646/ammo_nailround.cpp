#include "CBasePlayerAmmo.h"

const int AMMO_NAILR_GIVE = 50;
const int AMMO_NAILR_MAX_CARRY = 200;

class CNailRound : public CBasePlayerAmmo
{
	void Spawn(void)
	{
		Precache();
		SET_MODEL(ENT(pev), "models/poke646/items/w_nailround.mdl");
		CBasePlayerAmmo::Spawn();
	}
	void Precache(void)
	{
		PRECACHE_MODEL("models/poke646/items/w_nailround.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	BOOL AddAmmo(CBaseEntity* pOther)
	{
		int bResult = (pOther->GiveAmmo(AMMO_NAILR_GIVE, "9mm", AMMO_NAILR_MAX_CARRY) != -1);
		if (bResult)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
		}
		return bResult;
	}
};

LINK_ENTITY_TO_CLASS(ammo_nailround, CNailRound)
