#include "extdll.h"
#include "util.h"
#include "CWeaponCustom.h"
#include "te_effects.h"
#include "skill.h"

class CUsp : public CWeaponCustom {
	bool m_silencer;

	CustomWeaponParams unsilenced_params;
	CustomWeaponParams silenced_params;

	void Precache() {
		UTIL_ParseCustomWeaponConfig("io/weapon_usp_sil.txt", silenced_params);
		params = silenced_params;
		PrecacheEvents();

		UTIL_ParseCustomWeaponConfig("io/weapon_usp.txt", unsilenced_params);
		params = unsilenced_params;
		PrecacheEvents();

		// default without silencer (keep in sync with cfg)
		params = unsilenced_params;
		m_silencer = false;

		CWeaponCustom::Precache();
	}

	void SecondaryAttackCustom() {
		CBasePlayer* m_pPlayer = GetPlayer();

		m_silencer = !m_silencer;
		params = m_silencer ? silenced_params : unsilenced_params;

		UTIL_SendCustomWeaponPredictionData(m_pPlayer->edict(), this, WC_PRED_SEND_BOTH);
		m_pPlayer->SetAnimation(PLAYER_RELOAD, 3.0f);
	}
};


LINK_ENTITY_TO_CLASS(weapon_usp, CUsp);