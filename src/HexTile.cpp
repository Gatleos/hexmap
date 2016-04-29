#include <sstream>
#include <iostream>
#include <queue>
#include "config.h"
#include "HexTile.h"
#include "HexMap.h"
#include "lerp.h"
#include "clamp.h"

const int SEA_LEVEL = 100;
const int HEIGHT_LIMIT = 256;

#define terptr(x) unique_ptr<HexTileS>(new HexTileS(x))

array<unique_ptr<HexTileS>, HexTileS::TERRAIN_NUM> HexTileS::terrain = { {
	terptr("t_null"), terptr("t_ocean"), terptr("t_tundra"), terptr("t_tundra_snow"),
	terptr("t_taiga_s"), terptr("t_taiga_m"), terptr("t_taiga_l"), terptr("t_taiga_snow_s"),
	terptr("t_taiga_snow_m"), terptr("t_taiga_snow_l"), terptr("t_forest_s"), terptr("t_forest_m"),
	terptr("t_forest_l"), terptr("t_grassland"), terptr("t_semiarid"), terptr("t_jungle_s"),
	terptr("t_jungle_m"), terptr("t_jungle_l"), terptr("t_savanna"), terptr("t_desert"), terptr("t_swamp")
	} };
const sf::Texture* HexTileS::tex = nullptr;
const std::map<std::string, unsigned char> HexTileS::gradientTypes = {
	{ "height", HexTileS::P_HEIGHT }, { "temperature", HexTileS::P_TEMPERATURE },
	{ "moisture", HexTileS::P_MOISTURE }, { "drainage", HexTileS::P_DRAINAGE }
};

HexTileS::HexTileS(string idSet) :id(idSet), moveCost(10U), gradientType(P_NONE) {
}

void HexTileS::loadJson(string filename) {
	Json::Value root = config::openJson(filename);
	if (root.begin() == root.end()) {
		cerr << "ERROR: couldn't open file \"" << filename << "\"\n";
		return;
	}
	string spriteSheet = root.get("spriteSheet", "").asString();
	SpriteSheet* sheet = RESOURCE.sh(spriteSheet);
	if (sheet == nullptr) {
		cerr << "\t(requested by \"" << filename << "\")\n";
		return;
	}
	tex = RESOURCE.tex(sheet->getImageName());
	if (tex == nullptr) {
		cerr << "\t(requested by \"" << filename << "\")\n";
		return;
	}
	// Cycle through defined terrain types; make sure to skip t_null!
	for (int a = 1; a < HexTileS::TERRAIN_NUM; a++) {
		auto& hex = HexTileS::terrain[a];
		Json::Value tdata = root.get(hex->id, Json::Value::null);
		if (tdata.isNull()) { // terrain not found
			cerr << "Warning: terrain type " << hex->id << " was not found in \"" << filename << "\"\n";
			continue;
		}
		string element;
		if (hex->id == "t_ocean") {
			hex->FLAGS[HexTileS::WALKABLE] = false;
		}
		else {
			hex->FLAGS[HexTileS::WALKABLE] = true;
		}
		try {
			//name
			element = "name";
			hex->name = tdata.get(element, "").asString();
			// rect
			Json::Value rectData;
			// First, look for a whole feature link
			element = "feature";
			rectData = tdata.get(element, Json::Value::null);
			if (rectData.isString()) {
				auto featureAll = &TileFeatureS::get(rectData.asString());
				if (featureAll != nullptr) {
					for (int i = 0; i < 3; i++) {
						hex->features[i] = featureAll;
					}
				}
			}
			// Now we iterate through zoom levels
			for (int i = 0; i < 3; i++) {
				element = config::rectNames[i];
				rectData = tdata.get(element, Json::Value::null);
				hex->tiles[i].loadJson(rectData, sheet, HexMap::getHexSize(i));
				element = config::featureNames[i];
				rectData = tdata.get(element, Json::Value::null);
				// Features are optional
				if (rectData.isString()) {
					hex->features[i] = &TileFeatureS::get(rectData.asString());
				}
			}
			// gradient - optional
			Json::Value gradient;
			gradient = tdata.get("gradient", Json::Value::null);
			int gradientMax = 0;
			if (!gradient.isNull()) {
				if (hex->id == "t_ocean") {
					gradientMax = SEA_LEVEL;
				}
				else {
					gradientMax = HEIGHT_LIMIT - SEA_LEVEL;
				}
				element = "gradient";
				hex->gradientKeys.clear();
				auto members = gradient.getMemberNames();
				char gradientType = P_NONE;
				for (auto& s : members) {
					auto e = gradient[s];
					if (s == "type") { // gradient type
						auto it = gradientTypes.find(e.asString());
						if (it == gradientTypes.end()) {
							stringstream ss;
							ss << "invalid gradient type \"" << e.asString() << "\"";
							throw runtime_error(ss.str());
						}
						hex->gradientType = it->second;
						continue;
					}
					if (e.size() != 3) {
						throw runtime_error("wrong amount of elements for color definition (needs 3)");
					}
					unsigned char indexValue = stoi(s);
					if (indexValue < 0 || indexValue > 255) {
						hex->FLAGS[GRADIENT] = false;
						stringstream ss;
						ss << "invalid index \"" << s << "\"";
						throw runtime_error(ss.str());
					}
					// gradient key values
					int index = stof(s) * gradientMax;
					hex->gradientKeys[indexValue] = sf::Color(e[0].asInt(), e[1].asInt(), e[2].asInt());
				}
			}
			// moveCost
			element = "moveCost";
			hex->moveCost = max(1U, tdata.get(element, 10U).asUInt());
		}
		catch (runtime_error e) { // report the error with the name of the object and member
			// make sure we have placeholder drawing rects!
			for (int r = 0; r < 3; r++) {
				if (hex->tiles[r].empty()) {
					hex->tiles[r].setToDefaultRect();
				}
			}
			cerr << "[" << filename << ", " << hex->id << ", " << element << "] " << e.what() << "\n";
		}
	}
}

const HexTileS& HexTileS::get(int t) {
	return *terrain[t];
}

const sf::Texture& HexTileS::getTexture() {
	return *tex;
}

HexTile::HexTile() :
ent(nullptr),
hts(nullptr),
tfs(nullptr) {
}

sf::Color HexTile::getGradientValue() {
	if (hts->gradientType == HexTileS::P_NONE || hts->gradientKeys.empty()) {
		return sf::Color::White;
	}
	unsigned char value = properties[hts->gradientType];
	unsigned char prevKey = 0;
	bool found = false;
	sf::Color prevColor = sf::Color::Transparent;
	sf::Color finalColor;
	for (auto k : hts->gradientKeys) {
		if (value <= k.first) {
			found = true;
			if (prevColor.a == 0) {
				finalColor = k.second;
			}
			else {
				float fraction = float(value - prevKey) / float(k.first - prevKey);
				finalColor = lerp::interpolateColor(prevColor, k.second, fraction);
			}
		}
		prevKey = k.first;
		prevColor = k.second;
	}
	if (!found) {
		finalColor = hts->gradientKeys.rbegin()->second;
	}
	return finalColor;
}