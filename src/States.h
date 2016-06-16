#ifndef STATES_H
#define STATES_H

#include "SFMLEngine.h"
#include "HexMap.h"
#include "pugixml.hpp"
#include "AnimHandler.h"
#include "UIdef.h"

class EngineState: public GameState {
	EngineState();
public:
	static std::shared_ptr<EngineState> instance();
	VectorSet vs;
	shared_ptr<UIdef::SiteMenu> siteMenu;
	int mtMilli = 0;
	bool timeDisplay;
	//
	sf::Clock frames;
	mt19937 customSeed;
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

class MapControlState : public GameState {
	const sf::Vector2f& viewSize_;
	const sf::Vector2f& viewCenter_;
	sf::Vector2i camDelta_;
	MapControlState();
public:
	sf::Vector2f mouseMapPos;
	sf::Vector2i tilePos;
	static std::shared_ptr<MapControlState> instance();
	void init();
	void end();
	void update();
	void render(sf::RenderWindow &window);
	void input(sf::Event &e);
}; // MapControlState

class SelectState : public GameState {
	shared_ptr<VectorSet> selectableCoords_;
	sf::Vector2f tilePos_;
	std::vector<sf::Sprite> selectable_;
	sf::Sprite selected_;
	bool inBounds_;
	std::function<void(const sf::Vector2i&)> selectCallback_;
public:
	static const sf::Color selectCol;
	static const sf::Color validCol;
	static const sf::Color invalidCol;
	SelectState(std::function<void(const sf::Vector2i&)> selectCallback);
	SelectState(shared_ptr<VectorSet> selectable, std::function<void(const sf::Vector2i&)> selectCallback);
	void init();
	void end();
	void update();
	void render(sf::RenderWindow &window);
	void input(sf::Event &e);
}; // SelectState

class DungeonState : public GameState {
	sf::VertexArray vArray;
	sf::Vector2f vOffset;
	sf::Vector2f vSize;
	sf::RectangleShape rectShape;
	std::vector<sf::Color> vColors;
	DungeonState();
public:
	static std::shared_ptr<DungeonState> instance();
	void init();
	void end();
	void update();
	void render(sf::RenderWindow &window);
	void input(sf::Event &e);
	void chart();
}; // DungeonState

class UIState : public GameState {
	UIState();
public:
	sf::Vector2f mouseMapPos;
	sf::Vector2i tilePos;
	static std::shared_ptr<UIState> instance();
	void init();
	void end();
	void update();
	void render(sf::RenderWindow &window);
	void input(sf::Event &e);
}; // UIState

#endif