#include <iostream>
#include "Species.h"
#include "json.h"
#include "ResourceLoader.h"
#include "MapEntity.h"



unordered_map<string, Species> Species::map;
const Species& Species::null = Species::map["s_null"];
const Species& Species::get(std::string s)
{
	const auto& spc = map.find(s);
	if (spc == map.end()) {
		return null;
	}
	return spc->second;
}

Species::Species() :anims(nullptr)
{

}

void Species::loadJson(string filename)
{
	Species::map.clear();
	Json::Value root = config::openJson("data/species.json");
	if (root.begin() == root.end()) {
		cerr << "ERROR: couldn't open file \"" << filename << "\"\n";
		return;
	}
	string element = "";
	auto names = root.getMemberNames();
	for (auto& idSet : names) {
		Json::Value sdata = root.get(idSet, Json::Value::null);
		try {
			Species& spc = Species::map[idSet];
			spc.id = idSet;
			element = "name";
			auto& names = sdata["name"];
			spc.name = names[0].asString();
			spc.plural = names[1].asString();
			spc.adjective = names[2].asString();
			element = "animFile";
			spc.anims = ResourceLoader::instance().anim(sdata["animFile"].asString());
			element = "idleAnim";
			spc.animKeys[MapEntityS::IDLE] = sdata["idleAnim"].asString();
		}
		catch (runtime_error e) {
			cerr << "[data/species.json, " << idSet << ", " << element << "] " << e.what() << "\n";
		}
	}
}
