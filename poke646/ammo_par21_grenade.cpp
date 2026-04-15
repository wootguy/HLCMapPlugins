#include "CBasePlayerAmmo.h"

const int AMMO_PAR21GL_GIVE = 2;
const int AMMO_PAR21GL_MAX_CARRY = 10;

class CPar21Grenade : public CBasePlayerAmmo
{
	void Precache(void)
	{
		m_defaultModel = "models/vendetta/items/w_par21_grenades.mdl";
		CBasePlayerAmmo::Precache();
		PRECACHE_SOUND("items/9mmclip1.wav");
	}

	BOOL AddAmmo(CBaseEntity* pOther)
	{
		int bResult = (pOther->GiveAmmo(AMMO_PAR21GL_GIVE, "ARgrenades") != -1);

		if (bResult)
		{
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
		}
		return bResult;
	}
};
LINK_ENTITY_TO_CLASS(ammo_par21_grenade, CPar21Grenade)