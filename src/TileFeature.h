#ifndef TILE_FEATURE_H
#define TILE_FEATURE_H

#include <array>
#include <memory>
#include <string>
#include <map>
#include <SFML/Graphics.hpp>
#include "rng.h"

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
	std::array<std::map<int, sf::FloatRect>, 3> rect_;
	std::array<int, 3> probTotal_;
	std::array<bool, 3> randomRect_;
	const sf::FloatRect* getRect(int rectNum, std::mt19937& urng = rng::r) const;
};

#endif