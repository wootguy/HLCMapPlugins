#include "CBasePlayerAmmo.h"

const int AMMO_PAR21GL_GIVE = 2;
const int AMMO_PAR21GL_MAX_CARRY = 10;

class CPar21Grenade : public CBasePlayerAmmo
{
	void Spawn(void)
	{
		Precache();
		SET_MODEL(ENT(pev), "models/vendetta/items/w_par21_grenades.mdl");
		CBasePlayerAmmo::Spawn();
	}
	void Precache(void)
	{
		PRECACHE_MODEL("models/vendetta/items/w_par21_grenades.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}
	BOOL AddAmmo(CBaseEntity* pOther)
	{
		int bResult = (pOther->GiveAmmo(AMMO_PAR21GL_GIVE, "ARgrenades", AMMO_PAR21GL_MAX_CARRY) != -1);

		if (bResult)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
		}
		return bResult;
	}
};
LINK_ENTITY_TO_CLASS(ammo_par21_grenade, CPar21Grenade)