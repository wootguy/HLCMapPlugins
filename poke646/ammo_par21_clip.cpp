#include "CBasePlayerAmmo.h"

const int AMMO_PAR21_GIVE = 30;
const int AMMO_PAR21_MAX_CARRY = 150;

class CPar21Clip : public CBasePlayerAmmo
{
	void Spawn(void)
	{
		Precache();
		SET_MODEL(ENT(pev), "models/vendetta/items/w_par21_clip.mdl");
		CBasePlayerAmmo::Spawn();
	}
	void Precache(void)
	{
		PRECACHE_MODEL("models/vendetta/items/w_par21_clip.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	BOOL AddAmmo(CBaseEntity* pOther)
	{
		int bResult = (pOther->GiveAmmo(AMMO_PAR21_GIVE, "9mm", AMMO_PAR21_MAX_CARRY) != -1);
		if (bResult)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
		}
		return bResult;
	}
};

LINK_ENTITY_TO_CLASS(ammo_par21_clip, CPar21Clip)
