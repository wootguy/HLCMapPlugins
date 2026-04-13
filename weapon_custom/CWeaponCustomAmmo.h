
class CWeaponCustomAmmo : public CBaseEntity
{
public:
	string_t ammo_classname;
	string_t w_model;
	WeaponSound pickup_snd;
	int give_ammo;
	int max_ammo;
	int ammo_type;
	string_t custom_ammo_type;

	void KeyValue(KeyValueData* pkvd);
	void loadExternalSoundSettings();
	const char* GetAmmoType();
	void Spawn() ;
	void PrecacheModel(string_t model);
	void PrecacheSound(string_t sound);
	void Precache();
};