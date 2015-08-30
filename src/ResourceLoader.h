#ifndef RESOURCE_LOADER_H
#define RESOURCE_LOADER_H

#include <map>
#include <SFML/Graphics/Texture.hpp>
#include "AnimHandler.h"

using namespace std;

class ResourceLoader {
	string root;
	map<string, sf::Texture> textures;
	map<string, SpriteSheet> sheets;
	map<string, AnimationData> animations;
	ResourceLoader();
	void getSpriteRecursive(SpriteSheet& sheet, pugi::xml_node& node, string name);
	bool loadAnimData(AnimationData& aData, const pugi::xml_document &ani, const SpriteSheet* sheet);
public:
	static ResourceLoader& instance();
	// Release every stored resource
	void releaseAll();
	// Try to retrieve or load the image file at this location
	const sf::Texture* tex(string name);
	// Try to retrieve or load the spritesheet file at this location
	SpriteSheet* sh(string name);
	// Try to retrieve or load the animation file at this location
	const AnimationData* anim(string name);
	// Set the root folder to start all file searches from; relative to working directory
	void setRoot(string r);
	string getRoot();
};

#define RESOURCE ResourceLoader::instance()

#endif
