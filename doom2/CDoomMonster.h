#pragma once
#include "CBaseMonster.h"
#include "CDoomSprite.h"
#include "sound_nodes.h"

enum activities {
	ANIM_IDLE,
	ANIM_MOVE,
	ANIM_ATTACK,
	ANIM_ATTACK2, // range attack
	ANIM_PAIN,
	ANIM_DEAD,
	ANIM_GIB,
};

extern float g_monster_scale;

extern float g_monster_scale;
extern uint32_t g_monster_idx;

class AnimInfo
{
public:
	float framerate;
	bool looped;
	std::vector<int> frameIndices;
	std::vector<int> attackFrames; // frame index where attack is called (-1 = every frame)

	AnimInfo() {}
	AnimInfo(int min, int max, float rate, bool loop);

	int getFrameIdx(int frameCounter, float oldCounter, bool& looped);
	bool isAttackFrame(int idx);
	int lastFrame();
};

class CDoomMonster : public CBaseMonster
{
public:
	const char* bodySprite;
	std::vector<AnimInfo> animInfo;
	AnimInfo currentAnim;

	int activity;
	bool hasMelee = true;
	bool hasRanged = true;
	bool isSpectre;
	bool canFly;
	bool fullBright;
	bool canRevive; // can revive other monsters
	bool constantAttack; // attack until target is obscured
	int constantAttackMax; // maximum attacks played in sequence
	int constantAttackLoopFrame; // restart at this frame when attacking again
	bool rangeWhenMeleeFails = true; // do a range attack if the melee attack fails
	float deathBoom;
	bool didDeathBoom;
	const char* dropItem; // item spawned on death
	const char* hullModel = "models/doom/null.mdl"; // model used for hitboxes
	bool isDeaf; // doesn't target player when heard unless in line of sight
	bool killPoints = true;

	int frameCounter;
	int oldFrameCounter;
	int animLoops;
	int oldFrameIdx;
	int dmgImmunity;

	float walkSpeed = 8.0f;
	float painChance = 1.0f;
	float meleeRange = 64.0f * g_monster_scale;
	float minRangeAttackDelay = 1.0f;
	float maxRangeAttackDelay = 3.0f;
	float walkSoundFreq = 0.6f;
	float reviveDelay = 1.0f; // time between revive checks
	float lastReviveCheck;
	float revive_range = 256;

	float nextWalkSound;
	float nextRangeAttack;
	float lastWallReflect;
	float lastDirChange;
	float lastEnemy;
	float deathRemoveDelay = 60.0f; // time before entity is destroyed after death
	float nextIdleSound;
	bool dormant = true;
	bool superDormant = true; // not even loaded yet
	bool spritesCreated; // client sprites for rendering
	bool isCorpse;
	bool isBeingRevived;
	bool entsCreated;
	int brighten; // if > 0 then draw full-bright. Decremented each frame
	bool dashing;
	Vector dashVel;
	float dashDamage;
	float dashTimeout;
	bool largeHull;
	Vector gunPos = Vector(0, 0, 8); // offset relative to body position

	EHANDLE h_enemy;
	EHANDLE oldEnemy; // remember last enemy

	std::vector<const char*> idleSounds;
	std::vector<const char*> deathSounds;
	std::vector<const char*> alertSounds;
	const char* painSound;
	const char* meleeSound;
	const char* meleeWindupSound;
	const char* shootSound;
	const char* walkSound;

	SoundNode* dormantNode; // save this to reduce CPU usage
	Vector lastDormantPos;

	EHANDLE m_hSprite; // sprite used for rendering (can't set self as a sprite as that breaks collision client-side)

	void KeyValue(KeyValueData* pkvd) override;

	void Precache() override;

	void DelayAttack();
	void DoomSpawn();
	void CreateRenderSprites();
	void DoomTouched(CBaseEntity* pOther) {}
	void SetupHull();
	void Setup();
	int SetActivity(int act);
	void Wakeup();
	BOOL ShouldGibMonster(int iGib) override { return FALSE; }
	void Killed(entvars_t* pevAttacker, int iGib) override;
	int TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) override;
	void PainSound(void) override;

	void SetEnemy(CBaseEntity* ent);
	void ClearEnemy();
	void Sleep();
	Vector BodyPos();
	virtual void RangeAttack(Vector aimDir);
	virtual void MeleeAttackStart() {}
	virtual void RangeAttackStart() {}
	virtual void MeleeAttack(Vector aimDir);
	void ShootBullet(Vector dir, float spread, float damage, bool flash = true);
	bool Slash(Vector dir, float damage);
	void Dash(Vector velocity, float damage, float timeout);
	bool isAttacking();
	virtual void DeathBoom() {}
	void Revive();
	bool ReviveNearbyDemon();
	void DoomThink();
	void UpdateOnRemove(void) override;
};
