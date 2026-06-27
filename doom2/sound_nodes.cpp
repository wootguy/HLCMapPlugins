#include "extdll.h"
#include "util.h"
#include "sound_nodes.h"
#include "doom2.h"
#include "CDoomDoor.h"

using namespace std;

bool NodeReach::isReachable()
{
	if (!hasPath)
		return false;

	//println("CHECK " + checkEnts.length() + " DOORS");
	for (int i = 0; i < checkEnts.size(); i++)
	{
		CDoomDoor* door = (CDoomDoor*)checkEnts[i].GetEntity();
		if (door && door->m_toggle_state == TS_AT_BOTTOM)
		{
			//println("" + door->pev->targetname + " is blocking sound");
			return false;
		}
	}

	return true;
}

vector<SoundNode> g_sound_nodes;

void CSoundNode::Spawn()
{
	SoundNode node;
	node.pos = pev->origin;
	node.id = g_sound_nodes.size();
	g_sound_nodes.push_back(node);
	UTIL_Remove(this);
}

LINK_ENTITY_TO_CLASS(info_node_sound, CSoundNode)

SoundNode* getSoundNode(Vector pos)
{
	TraceResult tr;
	for (int i = 0; i < g_sound_nodes.size(); i++)
	{
		UTIL_TraceLine( g_sound_nodes[i].pos, pos, ignore_monsters, NULL, &tr );
		if (tr.flFraction >= 1.0f)
			return &g_sound_nodes[i];
	}
	return NULL;
}

SoundNode* getNodeByID(int id)
{
	if (id < 0 || id > int(g_sound_nodes.size()))
		return NULL;
	return &g_sound_nodes[id];
}

// cost to move to between these waypoints
float path_cost(SoundNode& a, SoundNode& b)
{
	Vector delta = a.pos - b.pos;
	if (abs(delta.Normalize().z) < 0.4)
		delta.z = 0; // vertical movement doesn't decrease speed unless its a ladder || something
	return delta.Length();
}

vector<SoundNode*> reconstruct_path(unordered_map<int, int>& cameFrom, int current)
{
    vector<SoundNode*> path;
	path.push_back(getNodeByID(current));
	
    while (cameFrom.count(current))
	{
		current = cameFrom[current];
        path.push_back(getNodeByID(current));
	}
	
    return path;
}

bool isPathClear(SoundNode& currentNode, NodeTarget& target)
{
	//println("TRACE " + currentNode.id + " " + target.id);
	SoundNode& targetNode = g_sound_nodes[target.id];
	//te_beampoints(currentNode.pos, targetNode.pos, "sprites/laserbeam.spr", 0, 100, 40, 5, 0, PURPLE);
	if (target.lastCheck + 1.0f > gpGlobals->time)
	{	
		// data is fresh
		if (!target.isClear)
			return false;
	}
	else
	{
		// check if a door is in the way
		TraceResult tr;
		//SoundNode* targetNode = g_sound_nodes[target.id];
		UTIL_TraceLine( currentNode.pos, targetNode.pos, ignore_monsters, NULL, &tr );
		target.isClear = tr.flFraction >= 1.0f;
		target.lastCheck = gpGlobals->time;
		
		for (int k = 0; k < targetNode.targets.size(); k++)
		{
			if (targetNode.targets[k].id == currentNode.id)
			{
				targetNode.targets[k].isClear = target.isClear;
				targetNode.targets[k].lastCheck = target.lastCheck;
				break;
			}
		}
		
		//println("DOIN A TRACE");
		if (!target.isClear)
			return false;
	}
	return true;
}

// finds the shortest path between two waypoints
vector<SoundNode*> AStarRouteWaypoint(SoundNode* start, SoundNode* goal, bool ignoreSolids)
{	
	unordered_set<int> closedSet;
	unordered_set<int> openSet;
	unordered_map<int, float> gScore;
	unordered_map<int, float> fScore;
	unordered_map<int, int> cameFrom;
	
	if (start == NULL  || goal  == NULL )
		return vector<SoundNode*>();
	
	//println("ROUTE FROM " + start.id + " TO " + goal.id);
	
	if (debug_mode)
	{
		if (start == NULL )
			ALERT(at_console, "FAILED TO START");
		if (goal == NULL )
			ALERT(at_console, "FAILED TO GOAL");
	}
	
	if (start->id == goal->id)
	{
		vector<SoundNode*> route;
		route.push_back(goal);
		return route;
	}
	
	openSet.insert(start->id);
	gScore[start->id] = 0;
	fScore[start->id] = path_cost(*start, *goal);
	
	while (!openSet.empty())
	{
		// get node in openset with lowest cost
		int current = -1;
		float bestScore = 9e99;
		for (int i : openSet)
		{
			float score = fScore[i];
			//println("CHECK SCORE FOR " + openKeys[i] + " " + score);
			if (score < bestScore)
			{
				bestScore = score;
				current = i;
			}
		}
		
		//println("Current is " + current);
		
		if (current == goal->id)
		{
			//println("MAde it to the goal");			
			return reconstruct_path(cameFrom, current);
		}
		
		openSet.erase(current);
		closedSet.insert(current);
		
		SoundNode& currentNode = g_sound_nodes[current];
		
		for (int i = 0; i < currentNode.targets.size(); i++)
		{
			NodeTarget& target = currentNode.targets[i];
			if (!ignoreSolids)
			{
				//if (target.checkEntity)
				//	continue;
				if (!isPathClear(currentNode, target))
					continue;
			}
			
			int neighbor = target.id;
			
			if (closedSet.count(neighbor))
				continue;
				
			// discover a new node
			openSet.insert(neighbor);
			//println("DISCOVERED " + neighbor);
				
			// The distance from start to a neighbor
			SoundNode& neighborNode = g_sound_nodes[neighbor];
			
			float tentative_gScore = gScore[current];
			tentative_gScore += path_cost(currentNode, neighborNode);
			
			float neighbor_gScore = 9e99;
			if (gScore.count(neighbor_gScore))
				neighbor_gScore = gScore[neighbor];
			
			if (tentative_gScore >= neighbor_gScore)
				continue; // not a better path
				
			//println("Route to neighbor " + neighbor);
				
			// This path is the best until now. Record it!
            cameFrom[neighbor] = current;
            gScore[neighbor] = tentative_gScore;
            fScore[neighbor] = tentative_gScore + path_cost(neighborNode, *goal);
		}
	}
	return vector<SoundNode*>();
}

bool canHearSound(SoundNode* start, SoundNode* end, Vector b, CBaseEntity* listener)
{
	if (start == NULL || end == NULL)
		return false;

	if (start->reachability.count(end->id))
	{
		// check reachability cache
		//println("CHECK REACH " + reach.hasPath + " " + start->id + " " + end.id);
		bool reachable = start->reachability[end->id].isReachable();
		/*
		if (debug_mode && reachable)
		{
			vector<SoundNode*> route = AStarRouteWaypoint(start, end, true);
			if (route.length() > 0 && listener )
			{
				te_beampoints(route[0].pos, b, "sprites/laserbeam.spr", 0, 100, 40, 5, 0, PURPLE);
				te_beampoints(route[route.length()-1].pos, listener->pev->origin, "sprites/laserbeam.spr", 0, 100, 40, 5, 0, PURPLE);
				for (uint i = 0; i < route.length()-1; i++)
				{
					te_beampoints(route[i].pos, route[i+1].pos, "sprites/laserbeam.spr", 0, 100, 40, 5, 0, PURPLE);
				}
			}
		}
		*/
		return reachable;
	}

	vector<SoundNode*> route = AStarRouteWaypoint(start, end, true);

	NodeReach reach;
	reach.hasPath = route.size() > 0;
	if (reach.hasPath)
	{
		std::reverse(route.begin(), route.end());
		for (int i = 0; i < route.size() - 1; i++)
		{
			SoundNode* node = route[i];
			for (int k = 0; k < node->targets.size(); k++)
			{
				if (node->targets[k].id == route[i + 1]->id)
				{
					for (int c = 0; c < node->targets[k].checkEntities.size(); c++)
						reach.checkEnts.push_back(node->targets[k].checkEntities[c]);
					//println("HAS A PATH " + start.id + " " + end.id + " with " + reach.checkEnts.length() + " blockers" );
				}
			}
		}
	}
	//else
	//	println("NO PATH " + start.id + " " + end.id);
	start->reachability[end->id] = reach;

	return route.size() > 0;
}

void createSoundGraph()
{	
	for (int i = 0; i < g_sound_nodes.size(); i++) {
		g_sound_nodes[i].reachability.clear();
		g_sound_nodes[i].targets.clear();
		g_sound_nodes[i].hitsEntity = false;
	}

	for (int i = 0; i < g_sound_nodes.size(); i++)
	{
		vector<EHANDLE> checkEntities;
		SoundNode& n1 = g_sound_nodes[i];
		for (int k = 0; k < g_sound_nodes.size(); k++)
		{
			SoundNode& n2 = g_sound_nodes[k];
			
			TraceResult tr;
			UTIL_TraceLine( n1.pos, n2.pos, ignore_monsters, NULL, &tr );
			CBaseEntity* phit = CBaseEntity::Instance( tr.pHit );
			if (tr.flFraction >= 1.0f)
			{
				NodeTarget target;
				target.id = k;
				target.checkEntities = checkEntities;
				n1.targets.push_back(target);
			}
			else if (string(STRING(phit->pev->classname)) != "worldspawn" && phit->pev->solid == SOLID_BSP)
			{
				phit->pev->solid = SOLID_NOT;
				checkEntities.push_back(EHANDLE(phit->edict()));
				k--;
				continue;
			}
			
			for (int c = 0; c < checkEntities.size(); c++)
				checkEntities[c].GetEntity()->pev->solid = SOLID_BSP;
			checkEntities.resize(0);
		}
	}
	
	// calculate reachability
	//println("Calc reachability");
	for (int i = 0; i < g_sound_nodes.size(); i++)
	{
		SoundNode& n1 = g_sound_nodes[i];
		for (int k = 0; k < g_sound_nodes.size(); k++)
		{
			if (k != i)
				canHearSound(&n1, &g_sound_nodes[k]);
		}
	}
}