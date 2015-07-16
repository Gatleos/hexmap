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
		NONE, MOUNTAIN, FEATURE_NUM
	};
private:
	static std::array<std::unique_ptr<TileFeatureS>, FEATURE_NUM> feature;
	TileFeatureS(std::string id);
public:
	const static sf::Color fade;
	static void loadJson(std::string filename);
	static const TileFeatureS& get(int t);
	sf::VertexArray vert_;
	std::string id_;
	std::string name_;
	std::array<sf::Vector2f, 3> pos_;
	std::array<RandomRect, 3> rects_;
};

#endif