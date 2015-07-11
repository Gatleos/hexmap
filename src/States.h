#ifndef STATES_H
#define STATES_H

#include "SFMLEngine.h"
#include "HexMap.h"
#include "pugixml.hpp"
#include "AnimHandler.h"
#include "UI.h"

class EngineState: public GameState
{
public:
	shared_ptr<UILayout> mapGenDebug;
	array<shared_ptr<sfg::Label>, 6U> debugInfo;
	shared_ptr<sfg::Entry> seedBox;
	shared_ptr<sfg::CheckButton> randomSeed;
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
	void init();
	void end();
	void update();
	void render(sf::RenderWindow &window);
	void input(sf::Event &e);
	void generate();
	void loadResourcesInPlace();
}; // EngineState

#endif