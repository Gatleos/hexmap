#ifndef HEX_TILE_H
#define HEX_TILE_H

#include <string>
#include <bitset>
#include <memory>
#include "TileFeature.h"

class MapEntity;

extern const int SEA_LEVEL;
extern const int HEIGHT_LIMIT;

class HexTileS {
public:
	enum {
		NONE, OCEAN, TUNDRA, TUNDRA_SNOW, TAIGA_S, TAIGA_M, TAIGA_L, TAIGA_SNOW_S, TAIGA_SNOW_M, TAIGA_SNOW_L, FOREST_S,
		FOREST_M, FOREST_L, GRASSLAND, SEMIARID, JUNGLE_S, JUNGLE_M, JUNGLE_L, SAVANNA, DESERT, SWAMP, TERRAIN_NUM
	};
private:
	static const sf::Texture* tex;
	static std::array<unique_ptr<HexTileS>, TERRAIN_NUM> terrain;
public:
	enum {
		GRADIENT, WALKABLE, FLAG_MAX
	};
	static void loadJson(string filename);
	static const HexTileS& get(int t);
	static const sf::Texture& getTexture();
	std::string id;
	std::string name;
	std::array<RandomRect, 3> tiles;
	std::array<const TileFeatureS*, 3> features;
	std::vector<sf::Color> colors;
	unsigned int moveCost;
	bitset<FLAG_MAX> FLAGS;
	HexTileS(std::string idSet);
};

class HexTile {
public:
	enum {
		MOUNTAINS, OCCUPIED_NEXT_TURN, FLAG_MAX
	};
	const HexTileS* hts;
	const TileFeatureS* tfs;
	MapEntity* ent;
	unsigned char height;
	char riverType;
	bitset<6> roads;
	bitset<FLAG_MAX> FLAGS;
	sf::Sprite spr[3];
	stack<sf::Color> color;
	HexTile();
};

#endif