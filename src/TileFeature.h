#ifndef TILE_FEATURE_H
#define TILE_FEATURE_H

#include <array>
#include <memory>
#include <string>
#include <map>
#include <SFML/Graphics.hpp>
#include "rng.h"
#include "ResourceLoader.h"

class RandomRect
{
	std::unique_ptr<std::discrete_distribution<int>> rectChance;
	std::vector<sf::FloatRect> rects;
	std::vector<sf::Vector2f> pos;
	int probTotal;
	bool active;
public:
	RandomRect();
	int RandomRect::randomize(std::mt19937& urng) const;
	const sf::FloatRect& getRect(int index) const;
	const sf::Vector2f& getPos(int index) const;
	bool operator!();
	void setToDefaultRect();
	void loadJson(Json::Value& rData, SpriteSheet* sheet, const sf::Vector2i& hexSize);
	bool empty();
};

class TileFeatureS
{
public:
	enum {
		NONE, MOUNTAIN, FOREST_S, FOREST_M, FOREST_L, TAIGA_S, TAIGA_M, TAIGA_L, FEATURE_NUM
	};
private:
	static const sf::Texture* tex;
	static std::array<std::unique_ptr<TileFeatureS>, FEATURE_NUM> feature;
	TileFeatureS(std::string id);
public:
	const static sf::Color fade;
	static void loadJson(std::string filename);
	static const TileFeatureS& get(int t);
	static const sf::Texture& getTexture();
	// Finds a TileFeature based on its id; will return f_null
	// if it can't find one! Just scans the array, so worst
	// case is O(n)
	static const TileFeatureS& get(std::string t);
	sf::VertexArray vert_;
	std::string id_;
	std::string name_;
	std::array<RandomRect, 3> rects_;
	int moveCost;
};

#endif