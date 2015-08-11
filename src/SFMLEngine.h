#ifndef SFML_ENGINE_H
#define SFML_ENGINE_H

#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <stack>
#include "UI.h"



class SFMLEngine;

class GameState
{
protected:
	GameState* prev;
	SFMLEngine* engine;
	//
	virtual void init(){}
	virtual void end(){}
public:
	virtual void update(){}
	virtual void render(sf::RenderWindow& window){}
	virtual void input(sf::Event& e){}
	//
	friend class SFMLEngine;
};//GameState

class SFMLEngine
{
	SFMLEngine() :lastFrame(sf::Time::Zero){}
public:
	sf::RenderWindow* window;
	sf::Event event;
	std::stack<std::shared_ptr<GameState>> states;
	sf::Color clearColor;
	sf::Time lastFrame;
	sfg::SFGUI sfgui;
	sfg::Desktop desktop;
	//
	static SFMLEngine& instance();
	~SFMLEngine(){ popAllStates(); }
	void init(sf::RenderWindow* windowset);
	void start();
	void pushState(std::shared_ptr<GameState> newState);
	void popState();
	void popAllStates();
	int getFPS() const;
	const sf::Time& getLastTick() const;
	void setClearColor(sf::Color& set);
};//SFMLEngine

#endif