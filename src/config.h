#ifndef JSON_H
#define JSON_H

#include <array>
#ifdef _WIN32
#include <json/json.h>
#else
#include <jsoncpp/json/json.h>
#endif
#include "HexTile.h"

#define ZOOM_LEVELS 3

namespace config {
	extern std::array<const char*, 6> roadTypes;
	extern const array<const char*, ZOOM_LEVELS> rectNames;
	extern const array<const char*, ZOOM_LEVELS> featureNames;
	// keys
	bool key(sf::Event& e, std::string binding);
	// map generation
	namespace gen {
		extern float heightParams[3];
		extern float tempParams[3];
		extern float moistParams[3];
		extern float drainParams[3];
		extern float forest[3];
		extern float desert;
		extern float swamp;
		extern float sand;
		extern float cold;
		extern float hot;
		extern float mountNum;
		extern float mountDensity;
	}
	// load functions
	void load();
	void save(); // TODO
	Json::Value openJson(std::string file);
	void loadAllJson();
}

#endif
