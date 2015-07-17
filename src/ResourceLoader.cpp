#include <iostream>
#include "ResourceLoader.h"

ResourceLoader::ResourceLoader(){}

ResourceLoader& ResourceLoader::instance()
{
	static ResourceLoader instance;
	return instance;
}

void ResourceLoader::getSpriteRecursive(SpriteSheet& sheet, pugi::xml_node& node, string name)
{
	string newPath = name + node.attribute("name").as_string();
	string curNodeType = node.name();
	if (curNodeType == "spr") {
		auto& newRect = sheet.sprites.insert(make_pair(newPath, sf::FloatRect())).first->second;
		newRect.left = node.attribute("x").as_float();
		newRect.top = node.attribute("y").as_float();
		newRect.width = node.attribute("w").as_float();
		newRect.height = node.attribute("h").as_float();
	}
	else {
		for (auto& n : node) {
			getSpriteRecursive(sheet, n, newPath + "/");
		}
	}
}

const sf::Texture* ResourceLoader::tex(string name)
{
	auto texIt = textures.find(name);
	if (texIt == textures.end()) {
		string path = root + name;
		sf::Texture* tex = &textures[name];
		if (!tex->loadFromFile(path)) {
			// Couldn't open image!
			textures.erase(textures.find(name));
			std::cerr << "ERROR: couldn't load texture \"" << path << "\"\n";
			return nullptr;
		}
		return tex;
	}
	return &texIt->second;
}

SpriteSheet* ResourceLoader::sh(string name)
{
	const auto& sh = sheets.find(name);
	if (sh == sheets.end()) {
		string path = root + name;
		pugi::xml_document sprXml;
		sprXml.load_file(path.c_str());
		if (sprXml.begin() == sprXml.end()) {
			// Couldn't open spritesheet!
			std::cerr << "ERROR: couldn't load spritesheet \"" << path << "\"\n";
			return nullptr;
		}
		SpriteSheet& newSheet = sheets[name];
		for (auto& node : sprXml.first_child().first_child().first_child().children()) {
			getSpriteRecursive(newSheet, node, "/");
		}
		newSheet.imageName = sprXml.first_child().attribute("name").as_string();
		newSheet.name = name;
		return &newSheet;
	}
	return &sh->second;
}

const AnimationData* ResourceLoader::anim(string name)
{
	auto anim = animations.find(name);
	if (anim == animations.end()) {
		pugi::xml_document animXml;
		string path = root + name;
		animXml.load_file(path.c_str());
		if (animXml.begin() == animXml.end()) {
			// Couldn't open .anim file!
			std::cerr << "ERROR: couldn't load animation \"" << path << "\"\n";
			return nullptr;
		}
		pugi::xml_node node = animXml.child("animations");
		string sheetName = animXml.first_child().first_attribute().as_string();
		const SpriteSheet* sheet = sh(sheetName);
		if (sheet == nullptr) {
			std::cerr << "\t(requested by \"" << path << "\")\n";
			return nullptr;
		}
		AnimationData& aData = animations[name];
		if (!loadAnimData(aData, animXml, sheet)) {
			animations.erase(animations.find(name));
			std::cerr << "\t(requested by \"" << path << "\")\n";
			return nullptr;
		}
		const sf::Texture* texture = tex(sheet->imageName);
		if (texture == nullptr) {
			std::cerr << "\t(requested by \"" << path << "\")\n";
			return nullptr;
		}
		aData.tx = texture;
		return &aData;
	}
	return &anim->second;
}

void ResourceLoader::setRoot(string r)
{
	root = r;
}

string ResourceLoader::getRoot()
{
	return root;
}

bool ResourceLoader::loadAnimData(AnimationData& aData, const pugi::xml_document &animXmlDoc, const SpriteSheet* sheet)
{
	pugi::xml_node animationsXml = animXmlDoc.first_child();
	aData.sheetName = animationsXml.attribute("spriteSheet").as_string();
	// Iterate through all animations
	for (auto& animXml : animationsXml.children()) {
		string name = animXml.attribute("name").value();
		int frameNum = (int)std::distance(animXml.children().begin(), animXml.children().end());
		AnimationData::anim& a = aData.animations.emplace(make_pair(name, AnimationData::anim(frameNum))).first->second;
		a.name = name;
		a.loops = animXml.attribute("loops").as_uint();
		// Iterate through cells in the current animation
		int cellIndex = 0;
		for (auto& cellXml : animXml.children()) {
			AnimationData::frame& f = a.frames[cellIndex];
			f.delay = max(1, cellXml.attribute("delay").as_int() * 30900);
			std::multimap<int, AnimationData::sprite> zList;
			// Iterate through sprites in the current cell
			for (auto& spriteXml : cellXml.children()) {
				int z = spriteXml.attribute("z").as_int();
				std::pair<int, AnimationData::sprite> smap(z, {});
				auto& s = smap.second;
				string spriteName = spriteXml.attribute("name").as_string();
				const auto& it = sheet->sprites.find(spriteName);
				if (it == sheet->sprites.end()) {
					// Couldn't find the requested sprite!
					std::cerr << "ERROR: couldn't find sprite \"" << spriteName << "\" in sheet \"" << sheet->imageName << "\"\n";
					return false;
				}
				// Get draw rect from sprite object, and offset data from anim file
				s.draw = { it->second.left, it->second.top, it->second.width, it->second.height };
				s.offset.x = spriteXml.attribute("x").as_float() - (int)(s.draw.width / 2.0f);
				s.offset.y = spriteXml.attribute("y").as_float() - (int)(s.draw.height / 2.0f);
				// Does it need to be flipped?
				if (spriteXml.attribute("flipH").as_bool())
				{
					s.flipH = true;
				}
				if (spriteXml.attribute("flipV").as_bool())
				{
					s.flipV = true;
				}
				// Use an associative container to keep the sprites in this frame in z-order
				zList.insert(smap);
			}
			// Create our vertex array from the collected rect data
			f.sprites.resize(zList.size() * 4);
			int i = 0;
			for (auto z : zList) {
				auto& s = z.second;
				f.sprites[i].texCoords = { s.draw.left, s.draw.top };
				f.sprites[i].position = { s.offset.x, s.offset.y };
				f.sprites[i + 1].texCoords = { s.draw.left + s.draw.width, s.draw.top };
				f.sprites[i + 1].position = { s.draw.width + s.offset.x, s.offset.y };
				f.sprites[i + 2].texCoords = { s.draw.left + s.draw.width, s.draw.top + s.draw.height };
				f.sprites[i + 2].position = { s.draw.width + s.offset.x, s.draw.height + s.offset.y };
				f.sprites[i + 3].texCoords = { s.draw.left, s.draw.top + s.draw.height };
				f.sprites[i + 3].position = { s.offset.x, s.draw.height + s.offset.y };
				if (s.flipH)
				{
					std::swap(f.sprites[i].position, f.sprites[i + 1].position);
					std::swap(f.sprites[i + 2].position, f.sprites[i + 3].position);
				}
				if (s.flipV)
				{
					std::swap(f.sprites[i].position, f.sprites[i + 3].position);
					std::swap(f.sprites[i + 1].position, f.sprites[i + 2].position);
				}
				i += 4;
			}
			cellIndex++;
		}
	}
	return true;
}