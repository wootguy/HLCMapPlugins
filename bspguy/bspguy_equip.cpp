#include "extdll.h"
#include "util.h"
#include "CBasePlayerWeapon.h"

namespace bspguy {

	const int FL_EQUIP_ALL_ON_USE = 1;
	const int FL_FORCE_WEAPON_SWITCH = 2; // force switch to most powerful weapon on trigger
	const int FL_REQUIP_ON_USE = 4;
	const int FL_ALLOW_MULTI_USE_PER_LIFE = 8;
	const int FL_REFILL_CLIPS = 16;
	const int MAX_EQUIP_ITEMS = 64;

	enum respawn_equip_modes {
		RESPAWN_EQUIP_IF_ON,
		RESPAWN_EQUIP_ALWAYS
	};
	
	enum ammo_equip_modes {
		AMMO_EQUIP_RESTOCK,
		AMMO_EQUIP_ADD
	};

	class EquipItem {
	public:
		string_t classname;
		int16_t primaryAmmo = 0;
		int16_t secondaryAmmo = 0;
		bool isWeapon = true;

		EquipItem() {}

		EquipItem(string_t classname) {
			this->classname = classname;
		}
	};
	
	std::vector<EHANDLE> g_equip_ents;

	class CBspguyEquip : public CBaseEntity
	{
	public:
		EquipItem items[MAX_EQUIP_ITEMS];
		int itemCount;

		bool playerUsedAlready[33];
		bool oneUsePerLife = true;
		bool applyToSpawners;
		int respawn_equip_mode;
		int ammo_equip_mode;
		
		float newMaxHealth;
		float newMaxArmor;
		float setHealth;
		float setArmor;
		
		string_t best_weapon;
		
		bool stripSuit = false;
		
		void addItem(EquipItem& item) {
			if (itemCount >= MAX_EQUIP_ITEMS) {
				ALERT(at_error, "bspguy_equip max items exceeded!\n");
				return;
			}

			items[itemCount++] = item;
		}

		void KeyValue(KeyValueData* pkvd) override
		{
			if (FStrEq(pkvd->szKeyName, "set_max_health")) {
				newMaxHealth = atof(pkvd->szValue);
			}
			else if (FStrEq(pkvd->szKeyName, "set_max_armor")) {
				newMaxArmor = atof(pkvd->szValue);
			}
			else if (FStrEq(pkvd->szKeyName, "set_armor")) {
				setArmor = atof(pkvd->szValue);
			}
			else if (FStrEq(pkvd->szKeyName, "set_health")) {
				setHealth = atof(pkvd->szValue);
			}
			else if (FStrEq(pkvd->szKeyName, "nosuit")) {
				stripSuit = true;
			}
			else if (FStrEq(pkvd->szKeyName, "respawn_equip_mode")) {
				respawn_equip_mode = atoi(pkvd->szValue);
				if (respawn_equip_mode == RESPAWN_EQUIP_ALWAYS) {
					applyToSpawners = true;
				}
			}
			else if (FStrEq(pkvd->szKeyName, "best_weapon")) {
				best_weapon = ALLOC_STRING(pkvd->szValue);
			}
			else if (strstr(pkvd->szKeyName, "weapon_") == pkvd->szKeyName) {
				EquipItem item = EquipItem(ALLOC_STRING(pkvd->szKeyName));
				item.isWeapon = true;
				
				std::string val = pkvd->szValue;
				const char* primaryAmmo = "0";
				const char* secondaryAmmo = "0";
				
				int ammoSep = val.find("+");
				if (ammoSep != -1) {
					item.primaryAmmo = atoi(val.substr(0, ammoSep).c_str());
					item.secondaryAmmo = atoi(val.substr(ammoSep+1).c_str());
				} else {
					item.primaryAmmo = atoi(pkvd->szValue);
				}
				
				addItem(item);
			}
			else if (strstr(pkvd->szKeyName, "item_") == pkvd->szKeyName) {
				EquipItem item = EquipItem(ALLOC_STRING(pkvd->szKeyName));
				item.isWeapon = false;
				addItem(item);
			}
			return CBaseEntity::KeyValue(pkvd);
		}
		
		void Spawn() override
		{
			Precache();
			g_equip_ents.push_back(EHANDLE(edict()));
		}

		void Precache() override
		{
			for (int i = 0; i < itemCount; i++) {
				UTIL_PrecacheOther(STRING(items[i].classname));
			}
		}

		void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float flValue = 0.0f) override
		{
			oneUsePerLife = !(pev->spawnflags & FL_ALLOW_MULTI_USE_PER_LIFE);
			
			if (!applyToSpawners && useType == USE_ON) {
				memset(playerUsedAlready, 0, sizeof(playerUsedAlready));
			}
			
			if (useType == USE_ON) {
				applyToSpawners = true;
			} else if (useType == USE_OFF && respawn_equip_mode != RESPAWN_EQUIP_ALWAYS) {
				applyToSpawners = false;
				return;
			}
			
			if (pev->spawnflags & FL_EQUIP_ALL_ON_USE) {
				for ( int i = 1; i <= gpGlobals->maxClients; i++ )
				{
					CBasePlayer* plr = UTIL_PlayerByIndex(i);
					if (!plr)
						continue;
					
					if (pev->spawnflags & FL_REQUIP_ON_USE) {
						plr->RemoveAllItems(stripSuit);
						//plr->SetItemPickupTimes(0);
					}
					
					if (oneUsePerLife && playerUsedAlready[plr->entindex()]) {
						return;
					}
					
					if (equip_player(plr, pev->spawnflags & FL_FORCE_WEAPON_SWITCH)) {
						EMIT_SOUND_DYN(plr->edict(), CHAN_ITEM, "items/gunpickup2.wav", 1.0f, 1.0f, 0, 100);
					}
				}
				
				return;
			}
			
			if (!pActivator || !pActivator->IsPlayer()) {
				return;
			}
			
			CBasePlayer* plr = pActivator->MyPlayerPointer();
			
			if (oneUsePerLife && playerUsedAlready[plr->entindex()]) {
				return;
			}
			
			if (pev->spawnflags & FL_REQUIP_ON_USE) {
				plr->RemoveAllItems(stripSuit);
				//plr->SetItemPickupTimes(0);
			}
			
			if (equip_player(plr, pev->spawnflags & FL_FORCE_WEAPON_SWITCH)) {
				EMIT_SOUND_DYN(plr->edict(), CHAN_ITEM, "items/gunpickup2.wav", 1.0f, 1.0f, 0, 100);
			}
		}
		
		bool equip_player(CBasePlayer* plr, bool switchWeapon) {
			bool anyWeaponGiven = false;
			
			for (int i = 0; i < itemCount; i++) {
				if (!items[i].isWeapon) {
					if (!strcmp(STRING(items[i].classname), "item_longjump") && plr->m_fLongJump) {
						continue;
					}
					if (plr->HasNamedPlayerItem(STRING(items[i].classname)) ) {
						continue;
					}

					Vector wow;
					
					StringMap keys;
					keys.put("origin", UTIL_VectorToString(plr->pev->origin));
					keys.put("spawnflags", "1024");
					CBaseEntity* item = CBaseEntity::Create(STRING(items[i].classname), g_vecZero, g_vecZero, true, NULL, keys);
					item->Use(plr, plr, USE_TOGGLE);
					
					//plr.GiveNamedItem(items[i].classname);
					continue;
				}
				
				CBasePlayerItem* it = plr->GetNamedPlayerItem(STRING(items[i].classname));
				CBasePlayerWeapon* wep = it ? it->GetWeaponPtr() : NULL;
				
				if (wep == NULL) {
					StringMap keys;
					keys.put("origin", UTIL_VectorToString(plr->pev->origin));
					keys.put("spawnflags", "1024");
					CBaseEntity* cent = CBaseEntity::Create(STRING(items[i].classname), g_vecZero, g_vecZero, true, NULL, keys);
					wep = cent ? cent->GetWeaponPtr() : NULL;

					if (wep == NULL) {
						ALERT(at_console, "bspguy_equip: Invalid weapon class %s\n", STRING(items[i].classname));
						continue;
					}
					
					// using wrong classname works with CreateEntity but not HasNamedPlayerItem
					// delete the new weapon if the player actually has the same item by a different name
					bool alreadyHadWeapon = false;
					if (wep->pev->classname != items[i].classname) {
						CBasePlayerItem* it = plr->GetNamedPlayerItem(STRING(wep->pev->classname));
						CBasePlayerWeapon* oldWep = it ? it->GetWeaponPtr() : NULL;
						if (oldWep) {
							alreadyHadWeapon = true;
							UTIL_Remove(wep);
							wep = oldWep;
						}
					}
					
					if (!alreadyHadWeapon) {
						wep->m_iDefaultAmmo = 0;
						//plr->SetItemPickupTimes(0);
						wep->Touch(plr);
						
						if (wep->m_iClip != -1) {
							wep->m_iClip = wep->iMaxClip();
						}
						
						anyWeaponGiven = true;
					}
				} else if (pev->spawnflags & FL_REFILL_CLIPS) {
					wep->m_iClip = wep->iMaxClip();
				}

				ItemInfo itemInfo;
				wep->GetItemInfo(&itemInfo);
				
				int primaryAmmoIdx = wep->PrimaryAmmoIndex();
				if (primaryAmmoIdx != -1) {
					int newAmmo = plr->m_rgAmmo[primaryAmmoIdx] + items[i].primaryAmmo;
					
					if (ammo_equip_mode == AMMO_EQUIP_RESTOCK) {
						newAmmo = V_max(items[i].primaryAmmo, plr->m_rgAmmo[primaryAmmoIdx]);
					}
					
					int maxAmmo = itemInfo.iMaxAmmo1;
					plr->m_rgAmmo[primaryAmmoIdx] = V_min(newAmmo, maxAmmo);
				}
				
				int secondaryAmmoIdx = wep->SecondaryAmmoIndex();
				if (secondaryAmmoIdx != -1) {
					int newAmmo = plr->m_rgAmmo[secondaryAmmoIdx] + items[i].secondaryAmmo;
					
					if (ammo_equip_mode == AMMO_EQUIP_RESTOCK) {
						newAmmo = V_max(items[i].secondaryAmmo, plr->m_rgAmmo[secondaryAmmoIdx]);
					}
					
					int maxAmmo = itemInfo.iMaxAmmo2;
					plr->m_rgAmmo[secondaryAmmoIdx] = V_min(newAmmo, maxAmmo);
				}
			}
			
			// select the best weapon
			bool forceWeaponSwitch = pev->spawnflags & FL_FORCE_WEAPON_SWITCH;
			if (switchWeapon && (anyWeaponGiven || forceWeaponSwitch)) {
				CBasePlayerWeapon* bestWeapon = NULL;
				int bestWeight = -1;
			
				for (int i = 0; i < MAX_ITEM_TYPES; i++) {
					CBaseEntity* it = plr->m_rgpPlayerItems[i];
					CBasePlayerWeapon* wep = it ? it->GetWeaponPtr() : NULL;
					
					if (wep ) {
						if (!strcmp(STRING(best_weapon), STRING(wep->pev->classname))) {
							bestWeapon = wep;
							break;
						}
					
						ItemInfo itemInfo;
						wep->GetItemInfo(&itemInfo);
						if (itemInfo.iWeight > bestWeight) {
							bestWeight = itemInfo.iWeight;
							bestWeapon = wep;
						}
					}
				}
				
				if (bestWeapon) {
					plr->SwitchWeapon(bestWeapon);
				}
			}
			
			playerUsedAlready[plr->entindex()] = true;
			
			if (newMaxArmor > 0) {
				plr->pev->armortype = newMaxArmor;
			}
			if (newMaxHealth > 0) {
				plr->pev->max_health = newMaxHealth;
			}
			if (setHealth > 0) {
				plr->pev->health = setHealth;
			}
			if (setArmor > 0) {
				plr->pev->armorvalue = setArmor;
			}
			
			return anyWeaponGiven;
		}
	};

	HOOK_RETURN_DATA EquipPlayerSpawn(CBasePlayer* plr) {
		for (int i = 0; i < g_equip_ents.size(); i++) {
			if (g_equip_ents[i]) {
				CBspguyEquip* equip = (CBspguyEquip*)(g_equip_ents[i].GetEntity());
				equip->playerUsedAlready[plr->entindex()] = false;
				if (equip->applyToSpawners)
					equip->equip_player(plr, true); // bpyass one-use-per-life limit
			}
		}
		return HOOK_CONTINUE;
	}
}

LINK_ENTITY_TO_CLASS(bspguy_equip, bspguy::CBspguyEquip)