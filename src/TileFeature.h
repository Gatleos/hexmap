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
	sf::Vector2f pos_;
	map<int, sf::FloatRect> rect_[3];
	int probTotal_[3];
	bool randomRect[3];
	const sf::FloatRect* getRect(int rectNum, mt19937& urng = rng::r) const;
};

#endif