#pragma once
#include "extdll.h"
#include "util.h"
#include "EHandle.h"
#include "doom2.h"

using namespace std;

// Will create a new state if the requested one does not exit
PlayerState& getPlayerState(CBasePlayer* plr);

TraceResult TraceLook(CBasePlayer* plr, float dist = 128, bool bigHull = false);

void delay_remove(EHANDLE ent);

vector<float> rotationMatrix(Vector axis, float angle);

// multiply a matrix with a vector (assumes w component of vector is 1.0f) 
Vector matMultVector(vector<float> rotMat, Vector v);

void knockBack(CBaseEntity* target, Vector vel);

int getDecal(int decalType);

enum spread_func
{
	SPREAD_GAUSSIAN,
	SPREAD_UNIFORM,
};

void HitScan(CBaseEntity* attacker, Vector vecSrc, Vector dir, float spread, float damage);

// Randomize the direction of a vector by some amount
// Max degrees = 360, which makes a full sphere
Vector spreadDir(Vector dir, float degrees, int spreadFunc = SPREAD_UNIFORM);

CBasePlayer* getAnyPlayer();