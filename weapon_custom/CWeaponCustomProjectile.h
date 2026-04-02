
class CWeaponCustomShootOpts;

class WeaponCustomProjectile : public CBaseAnimating
{
public:
	float thinkDelay = 0.05;
	EHANDLE h_shoot_opts; // weapon_custom_shoot
	ProjectileOptions options;
	EHANDLE spriteAttachment; // we'll need to kill this before we die (lol murder)
	bool attached;
	EHANDLE target; // entity attached to
	Vector attachStartOri; // Our initial position when attaching to the entity
	Vector targetStartOri; // initial position of the entity we attached to
	Vector attachStartDir; // our initial direction when attaching to the entity
	int attachTime = 0;

	bool move_snd_playing = false;
	string_t pickup_classname;
	bool weaponPickup = false;
	float pickupRadius = 64.0f;
	float nextBubbleTime = 0;
	float bubbleDelay = 0.07;
	float nextTrailEffectTime = 0;
	float nextBounceEffect = 0;

	void Spawn();

	void MoveThink();

	void uninstall_steam_and_kill_yourself();

	void DamageTarget(CBaseEntity* ent, bool friendlyFire);

	void ConvertToWeapon();

	bool isValidHookSurface(CBaseEntity* pOther);

	void Touch(CBaseEntity* pOther);

	CWeaponCustomShootOpts* getShootOpts();
};