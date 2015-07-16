#ifndef SPECIES_H
#define SPECIES_H

#include <array>
#include <string>
#include <unordered_map>
#include "AnimHandler.h"

using namespace std;

class Species
{
public:
	static unordered_map<string, Species> map;
	static const Species& null;
	static const Species& get(string s);
	static void loadJson(string filename);
	//
	string id;
	string name;
	string plural;
	string adjective;
	const AnimationData* anims;
	array<string, 3> animKeys;
	Species();
};

#endif
