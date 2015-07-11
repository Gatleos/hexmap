#ifndef TILE_FEATURE_H
#define TILE_FEATURE_H

#include <array>
#include <memory>
#include <SFML/Graphics.hpp>



namespace featuretypes
{
	enum {
		NONE, MOUNTAIN, SIZE
	};
};

class TileFeatureS
{
	static array<unique_ptr<TileFeatureS>, featuretypes::SIZE> feature;
	TileFeatureS(string id);
public:
	const static sf::Color fade;
	static void loadJson(string filename);
	static const TileFeatureS& get(int t);
	sf::VertexArray vert_;
	string id_;
	string name_;
	std::array<sf::Vector2f, 3> pos_;
	std::array<map<int, sf::FloatRect>, 3> rect_;
	std::array<int, 3> probTotal_;
	std::array<bool, 3> randomRect_;
	const sf::FloatRect* getRect(int rectNum, mt19937& urng = rng::r) const;
};

#endif