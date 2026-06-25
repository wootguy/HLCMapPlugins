#pragma once
#include "CBaseEntity.h"

#define SF_DOOR_START_OPEN 1
#define SF_DOOR_USE_ONLY 256
#define SF_DOOR_NO_AUTO_RETURN 32
#define FL_DOOR_BUTTON_DONT_MOVE 2

class CDoomDoor : public CBaseEntity
{
public:
	Vector m_vecPosition1;
	Vector m_vecPosition2;
	Vector m_vecFinalDest;
	int m_toggle_state;
	int dir;
	float m_flLip;
	float m_flWait;
	float lastCrush;
	bool m_bIsReopening;
	bool isButton;
	bool always_use;
	int lock = 0; // keys required to open (bitfield)
	int sounds = 0;
	bool touch_opens = false;
	bool isCrusher = false;
	bool shootable = false;
	Vector useDir;
	string_t sync;
	float attn = 0.6f;

	const char* switchSnd;
	const char* openSnd;
	const char* closeSnd;

	std::vector<EHANDLE> sync_buttons; // buttons to move with the door
	EHANDLE parent;

	int	ObjectCaps(void) override { return FCAP_IMPULSE_USE; }

	void KeyValue(KeyValueData* pkvd) override;

	void Precache() override;

	void Spawn() override;

	void Touch(CBaseEntity* other);

	int TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) override;

	void ButtonReset();

	void Useit(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value, bool wasShot = false);

	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);

	int DoorActivate(int useType = USE_TOGGLE);

	void DoorGoUp();

	void DoorHitTop();

	void DoorGoDown();

	void DoorHitBottom();
	void Blocked(CBaseEntity* pOther);

	void LinearMove(Vector vecDest, float flSpeed);

	void LinearMoveDone();
	void SetToggleState(int state);
};