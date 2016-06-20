#ifndef UI2_H
#define UI2_H

#include "imgui.h"
#include "imgui-rendering-SFML.h"
#include "imgui-events-SFML.h"
#include "HexMap.h"

namespace ui2 {
	void mapInfoMenu(sf::Vector2i& mouseMapPos, sf::Vector2i& tilePos, sf::Vector2i& viewCenter, std::string tileName, int& genTime, bool& randomSeed, AnimHandler& anim);
}

#endif