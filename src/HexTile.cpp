#include <sstream>
#include <iostream>
#include <queue>
#include "json.h"
#include "HexTile.h"
#include "ResourceLoader.h"
#include "lerp.h"
#include "clamp.h"

const int SEA_LEVEL = 100;
const int HEIGHT_LIMIT = 256;

#define terptr(x) unique_ptr<HexTileS>(new HexTileS(x))

array<unique_ptr<HexTileS>, HexTileS::TERRAIN_NUM> HexTileS::terrain = { {
	terptr("t_null"), terptr("t_ocean"), terptr("t_mountain"), terptr("t_tundra"), terptr("t_taiga_s"),
	terptr("t_taiga_m"), terptr("t_taiga_l"), terptr("t_forest_s"), terptr("t_forest_m"),
	terptr("t_forest_l"), terptr("t_grassland"), terptr("t_semiarid"), terptr("t_jungle_s"),
	terptr("t_jungle_m"), terptr("t_jungle_l"), terptr("t_savanna"), terptr("t_desert"), terptr("t_swamp")
} };

HexTileS::HexTileS(string idSet) :id(idSet), townChance(1.0f)
{
}

const sf::FloatRect* HexTileS::getRect(int rectNum, mt19937& urng) const
{
	if (!randomRect[rectNum]) {
		return &rect[rectNum].begin()->second;
	}
	int r = rng::getInt(probTotal[rectNum] - 1, urng);
	for (auto f = rect[rectNum].rbegin(); f != rect[rectNum].rend(); f++) {
		if (r < f->first) {
			return &f->second;
		}
		r -= f->first;
	}
	return nullptr;
}

void HexTileS::loadJson(string filename)
{
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
	const char* rectNames[] = { "rect", "rectHalf", "rectQuart" };
	const char* featureNames[] = { "feature", "featureHalf", "featureQuart" };
	// Cycle through defined terrain types; make sure to skip t_null!
	for (int a = 1; a < HexTileS::TERRAIN_NUM; a++) {
		auto& hex = HexTileS::terrain[a];
		Json::Value tdata = root.get(hex->id, Json::Value::null);
		if (tdata.isNull()) { // terrain not found
			cerr << "Warning: terrain type " << hex->id << " was not found in \"" << filename << "\"\n";
			continue;
		}
		string element;
		if (hex->id == "t_ocean" || hex->id == "t_mountain") {
			hex->walkable = false;
		}
		else {
			hex->walkable = true;
		}
		try {
			//name
			element = "name";
			hex->name = tdata.get(element, "").asString();
			// rect
			const sf::FloatRect* rectData = nullptr;
			for (int i = 0; i < 3; i++) {
				element = rectNames[i];
				Json::Value j = tdata.get(element, Json::Value::null);
				hex->probTotal[i] = 0;
				if (j.isArray()) {
					hex->randomRect[i] = true;
					if (j.size() & 1) {
						throw runtime_error("incorrect number of arguments");
					}
					for (int f = 0; f < j.size(); f += 2) {
						rectData = sheet->spr(j[f].asString());
						if (rectData == nullptr) {
							throw runtime_error("couldn't find tile sprite");
						}
						int prob = clamp(j[f + 1].asInt(), 0, 100);
						hex->probTotal[i] += prob;
						hex->rect[i].emplace(make_pair(clamp(j[f + 1].asInt(), 0, 100), *rectData));
					}
				}
				else {
					hex->randomRect[i] = false;
					rectData = sheet->spr(j.asString());
					if (rectData == nullptr) {
						throw runtime_error("couldn't find tile sprite");
					}
					hex->rect[i].emplace(make_pair(100, *rectData));
					hex->probTotal[i] = 100;
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
				hex->FLAGS[GRADIENT] = true;
				element = "gradient";
				hex->colors.clear();
				auto members = gradient.getMemberNames();
				vector<sf::Color> keyColors;
				vector<int> keyIndices;
				for (auto& s : members) {
					auto e = gradient[s];
					if (e.size() != 3) {
						hex->FLAGS[GRADIENT] = false;
						throw runtime_error("wrong amount of elements for color definition (needs 3)");
					}
					float indexFraction = stof(s);
					if (indexFraction < 0.0f || indexFraction > 1.0f) {
						hex->FLAGS[GRADIENT] = false;
						stringstream ss;
						ss << "invalid index \"" << s << "\"";
						throw runtime_error(ss.str());
					}
					int index = stof(s) * gradientMax;
					if (!keyIndices.empty() && index <= keyIndices.back()) {
						hex->FLAGS[GRADIENT] = false;
						throw runtime_error("gradient indices must be in ascending order");
					}
					if (index != 0) {
						keyIndices.push_back(index);
					}
					keyColors.push_back(sf::Color(e[0].asInt(), e[1].asInt(), e[2].asInt()));
				}
				lerpColorRange(hex->colors, keyColors, keyIndices);
			}
			// townChance
			element = "townChance";
			hex->townChance = tdata.get("townChance", Json::Value::null).asFloat();
		}
		catch (runtime_error e) { // report the error with the name of the object and member
			// make sure we have placeholder drawing rects!
			for (int r = 0; r < 3; r++) {
				auto& rect = hex->rect[r];
				if (rect.empty()) {
					rect.emplace(make_pair(0, sf::FloatRect()));
					hex->randomRect[r] = false;
				}
			}
			cerr << "[" << filename << ", " << hex->id << ", " << element << "] " << e.what() << "\n";
		}
	}
}

const HexTileS& HexTileS::get(int t)
{
	return *terrain[t];
}

HexTile::HexTile() :ent(nullptr)
{
}
