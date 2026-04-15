
class CWeaponCustomAmmo : public CBaseEntity
{
public:
	string_t ammo_classname;
	string_t w_model;
	WeaponSound pickup_snd;
	int give_ammo;
	int max_ammo;
	int ammo_type;
	int ammo_type_hl; // ammo type given to HL players who don't have the client installed
	string_t custom_ammo_type;

	void KeyValue(KeyValueData* pkvd);
	void loadExternalSoundSettings();
	const char* GetAmmoType(bool isHlClient);
	void Spawn() ;
	void PrecacheModel(string_t model);
	void PrecacheSound(string_t sound);
	void Precache();
};