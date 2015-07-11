#include <iostream>
#include <algorithm>
#include <fstream>
#include "json.h"



#define loadInt(prop,dest,limitL,limitH) if(root[prop].isInt())dest=std::max(limitL, std::min(limitH, root.get(prop, dest).asInt()));
#define saveInt(prop,source) value=source;root[prop]=value;
#define loadFloat(prop,dest,limitL,limitH) if(root[prop].isFloat())dest=std::max(limitL, std::min(limitH, root.get(prop, dest).asFloat()));
#define saveFloat(prop,source) value=source;root[prop]=value;

float config::heightParams[3] = { 0.0, 0.0, 0.0 };
float config::tempParams[3] = { 0.0, 0.0, 0.0 };
float config::moistParams[3] = { 0.0, 0.0, 0.0 };
float config::drainParams[3] = { 0.0, 0.0, 0.0 };
float config::forest[3] = { 0.0, 0.0, 0.0 };
float config::desert = 0.0f;
float config::swamp = 0.0f;
float config::sand = 0.0f;
float config::cold = 0.0f;
float config::hot = 0.0f;
float config::mountNum = 10.0f;
float config::mountDensity = 100.0f;

void config::load() {
	Json::Value root = openJson("data/config.json");
	if (root.begin() == root.end()) {
		return;
	}
	array<string, 5U> paramNames = {"height", "temperature", "moisture", "drainage", "forest"};
	array<float*, 5U> params = { heightParams, tempParams, moistParams, drainParams, forest };
	try {
		for (int p = 0; p < 5; p++) {
			auto& element = root[paramNames[p]];
			int size = min(3, (int)element.size());
			for (int e = 0; e < size; e++) {
				params[p][e] = element[e].asFloat();
			}
		}
		desert = root["desert"].asFloat();
		swamp = root["swamp"].asFloat();
		sand = root["sand"].asFloat();
		cold = root["cold"].asFloat();
		hot = root["hot"].asFloat();
		mountNum = root["mountNum"].asFloat();
		mountDensity = root["mountDensity"].asFloat();
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