#include <iostream>
#include <fstream>
#include "config.h"
#include "HexMap.h"
#include "lerp.h"
#include "Species.h"
#include "ResourceLoader.h"

array<const char*, 6> config::roadTypes = { "r_e", "r_ne", "r_nw", "r_w", "r_sw", "r_se" };
const array<const char*, ZOOM_LEVELS> config::rectNames = { "full", "half", "quarter" };
const array<const char*, ZOOM_LEVELS> config::featureNames = { "featureFull", "featureHalf", "featureQuarter" };
const char* config::resourceRoot = "data/sprites/";

Json::Value config::openJson(string file) {
	Json::Value root;
	Json::Reader reader;
	filebuf fb;
	fb.open(file, ios::in);
	istream config_file(&fb);
	if (!reader.parse(config_file, root))
	{
		cerr << "Failed to parse " << file << "\n"
			<< reader.getFormattedErrorMessages();
	}
	return root;
}

void config::loadAllJson() {
	cerr << "\nBegin Json Parsing\n------------------\n";
	TileFeatureS::loadJson("data/feature.json");
	HexTileS::loadJson("data/terrain.json");
	Species::loadJson("data/species.json");
	SiteS::loadJson("data/sites.json");
	config::loadKeyJson("data/keybindings.json");
	cerr << "------------\nParsing Done\n\n";
}

// config.json

#define loadInt(prop,dest,limitL,limitH) if(root[prop].isInt())dest=std::max(limitL, std::min(limitH, root.get(prop, dest).asInt()));
#define saveInt(prop,source) value=source;root[prop]=value;
#define loadFloat(prop,dest,limitL,limitH) if(root[prop].isFloat())dest=std::max(limitL, std::min(limitH, root.get(prop, dest).asFloat()));
#define saveFloat(prop,source) value=source;root[prop]=value;

float config::gen::heightParams[3] = { 0.0, 0.0, 0.0 };
float config::gen::tempParams[3] = { 0.0, 0.0, 0.0 };
float config::gen::moistParams[3] = { 0.0, 0.0, 0.0 };
float config::gen::drainParams[3] = { 0.0, 0.0, 0.0 };
float config::gen::forest[3] = { 0.0, 0.0, 0.0 };
float config::gen::desert = 0.0f;
float config::gen::swamp = 0.0f;
float config::gen::sand = 0.0f;
float config::gen::cold = 0.0f;
float config::gen::hot = 0.0f;
float config::gen::mountNum = 10.0f;
float config::gen::mountDensity = 100.0f;

void config::load() {
	Json::Value root = openJson("data/config.json");
	if (root.begin() == root.end()) {
		return;
	}
	array<string, 5U> paramNames = { "height", "temperature", "moisture", "drainage", "forest" };
	array<float*, 5U> params = { gen::heightParams, gen::tempParams, gen::moistParams, gen::drainParams, gen::forest };
	try {
		for (int p = 0; p < 5; p++) {
			auto& element = root[paramNames[p]];
			int size = min(3, (int)element.size());
			for (int e = 0; e < size; e++) {
				params[p][e] = element[e].asFloat();
			}
		}
		gen::desert = root["desert"].asFloat();
		gen::swamp = root["swamp"].asFloat();
		gen::sand = root["sand"].asFloat();
		gen::cold = root["cold"].asFloat();
		gen::hot = root["hot"].asFloat();
		gen::mountNum = root["mountNum"].asFloat();
		gen::mountDensity = root["mountDensity"].asFloat();
	}
	catch (runtime_error e) {
		std::cout << e.what() << "\n";
	}
}
void config::save() {
	Json::Value root;
	Json::Value value;
	std::ofstream out;
	out.open("data/config.json");
	//saveInt("window_width", WINDOW_W);
	//saveInt("window_height", WINDOW_H);
	//saveInt("renders_per_frame", RENDERS_PER_FRAME);
	//saveInt("map_width", MAP_W);
	//saveInt("map_height", MAP_H);
	//saveInt("map_depth", MAP_D);
	out << root;
	out.close();
}