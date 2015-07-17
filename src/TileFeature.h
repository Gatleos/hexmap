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
	std::map<int, sf::FloatRect> rect;
	int probTotal;
	bool active;
public:
	RandomRect();
	const sf::FloatRect* getRect(std::mt19937& urng) const;
	bool operator!();
	void setToDefaultRect();
	void loadJson(Json::Value& rdata, SpriteSheet* sheet);
	bool empty();
};

class TileFeatureS
{
public:
	enum {
		NONE, MOUNTAIN, FOREST_L, FEATURE_NUM
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
	std::array<sf::Vector2f, 3> pos_;
	std::array<RandomRect, 3> rects_;
};

#endif