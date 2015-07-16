#ifndef HEX_TILE_H
#define HEX_TILE_H

#include <map>
#include <string>
#include <bitset>
#include <memory>
#include "rng.h"
#include "TileFeature.h"

using namespace std;

class MapEntity;

extern const int SEA_LEVEL;
extern const int HEIGHT_LIMIT;

class HexTileS
{
public:
	enum {
		NONE, OCEAN, MOUNTAIN, TUNDRA, TAIGA_S, TAIGA_M, TAIGA_L, FOREST_S, FOREST_M, FOREST_L,
		GRASSLAND, SEMIARID, JUNGLE_S, JUNGLE_M, JUNGLE_L, SAVANNA, DESERT, SWAMP, TERRAIN_NUM
	};
private:
	static array<unique_ptr<HexTileS>, TERRAIN_NUM> terrain;
public:
	enum {
		GRADIENT, FLAG_MAX
	};
	static void loadJson(string filename);
	static const HexTileS& get(int t);
	string id;
	string name;
	map<int, sf::FloatRect> rect[3];
	int probTotal[3];
	bool randomRect[3];
	vector<sf::Color> colors;
	bool walkable;
	float townChance;
	bitset<FLAG_MAX> FLAGS;
	HexTileS(string idSet);
	const sf::FloatRect* getRect(int rectNum, mt19937& urng = rng::r) const;
};

class HexTile
{
public:
	const HexTileS* hts;
	const TileFeatureS* tfs;
	unsigned char height;
	char riverType;
	bitset<6> roads;
	MapEntity* ent;
	sf::Sprite spr[3];
	HexTile();
};

#endif