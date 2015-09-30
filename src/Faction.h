#ifndef FACTION_H
#define FACTION_H

#include <vector>
#include <set>
#include <deque>

using namespace std;

class Faction {
public:
	int capitol;
	set<int> sites;
	set<int> leaders;
	set<int> units;
};

#endif