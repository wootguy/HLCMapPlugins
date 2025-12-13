#include "extdll.h"
#include "util.h"
#include "CPointEntity.h"
#include "CBasePlayer.h"

class CEnvHideHud : public CPointEntity
{
	void Spawn(void)
	{
		Precache();
	}

	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) {
		for (int i = 1; i < gpGlobals->maxClients; i++) {
			CBasePlayer* plr = UTIL_PlayerByIndex(i);

			if (plr) {
				plr->m_iHideHUD |= HIDEHUD_ALL;
			}
		}
	}
};

// doesn't work right, so just remove the weapons in the cfg
//LINK_ENTITY_TO_CLASS(env_hidehud, CEnvHideHud)