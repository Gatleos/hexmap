#ifndef STATES_H
#define STATES_H

#include "SFMLEngine.h"
#include "HexMap.h"
#include "pugixml.hpp"
#include "AnimHandler.h"
#include "UIdef.h"

class EngineState: public GameState
{
public:
	shared_ptr<UIdef::MapGenDebug> mapGenDebug;
	//
	pugi::xml_document ani;
	pugi::xml_document spr;
	sf::Texture tex;
	AnimHandler sh[16384];
	sf::Sprite sprite;
	//
	mt19937 customSeed;
	HexMap hg;
	sf::Font font;
	sf::Text text;
	sf::Texture hex;
	sf::Sprite hexspr;
	sf::Shader shader;
	void init();
	void end();
	void update();
	void render(sf::RenderWindow &window);
	void input(sf::Event &e);
	void generate();
	void loadResourcesInPlace();
}; // EngineState

#endif