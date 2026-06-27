#pragma once
#include "CBaseEntity.h"
#include <vector>
#include <unordered_set>

struct NodeTarget
{
	int id = -1;
	std::vector<EHANDLE> checkEntities;
	float lastCheck = 0;
	bool isClear = true;
};

struct NodeReach
{
	bool hasPath;
	std::vector<EHANDLE> checkEnts; // these parts of the path need to be traced
	float lastCheck;
	bool isClear;

	bool isReachable();
};

struct SoundNode
{
	int id;
	Vector pos;
	bool hitsEntity;
	std::vector<NodeTarget> targets;
	std::unordered_map<int, NodeReach> reachability; // cached results of A* algorithm.
};

class CSoundNode : public CBaseEntity
{
	void Spawn() override;
};

SoundNode* getSoundNode(Vector pos);
void createSoundGraph();
bool canHearSound(SoundNode* start, SoundNode* end, Vector b = Vector(0, 0, 0), CBaseEntity* listener = NULL);
