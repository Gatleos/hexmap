#include <iostream>
#include <SFGUI/Renderers.hpp>
#include "SFMLEngine.h"
#include "HexMap.h"



SFMLEngine& SFMLEngine::instance() {
	static SFMLEngine instance;
	return instance;
}
void SFMLEngine::init(sf::RenderWindow* windowset) {
	window = windowset;
	window->setFramerateLimit(60);
	clearColor = sf::Color::Black;
	UI::init(&desktop);
	UI::setAppSize((sf::Vector2f)window->getSize());
}
void SFMLEngine::start() {
	try {
		sf::Clock fr;
		while (window->isOpen()) {
			while (window->pollEvent(event)) {
				desktop.HandleEvent(event);
				if (event.type == sf::Event::Resized) {
					UI::setAppSize({ (float)event.size.width, (float)event.size.height });
					HexMap::view.setSize(sf::Vector2f((float)event.size.width, (float)event.size.height));
					HexMap::view.setViewport(sf::FloatRect(0.0f, 0.0f, 1.0f, 1.0f));
					UI::view.setSize(sf::Vector2f(roundf((float)event.size.width), roundf((float)event.size.height)));
					UI::view.setViewport(sf::FloatRect(0.0f, 0.0f, 1.0f, 1.0f));
					UI::view.setCenter(UI::view.getSize() / 2.0f);
					//
					HEXMAP.constrainView(HexMap::view);
					HEXMAP.calculateViewArea(HexMap::view);
				}
				if (event.type == sf::Event::Closed) {
					window->close();
					UI::end();
					//return;
				}
				else { states.top()->input(event); }
				UI::resetInputFlags();
			}
			//
			lastFrame = fr.restart();
			states.top()->update();
			desktop.Update(lastFrame.asSeconds());
			//
			window->clear(clearColor);
			states.top()->render(*window);
			sfgui.Display(*window);
			window->display();
		}
	}
	catch(runtime_error e) {
		cerr << e.what();
		UI::end();
		window->close();
	}
}

void SFMLEngine::pushState(GameState* newState) {
	if (states.size())
		newState->prev = states.top();
	states.push(newState);
	states.top()->engine = this;
	states.top()->init();
}
void SFMLEngine::popState() {
	if (!states.empty()) {
		states.top()->end();
		delete states.top();
		states.pop();
		while (window->pollEvent(event));
	}
}
void SFMLEngine::popAllStates() {
	if (states.empty()) return;
	while (states.size()) {
		states.top()->end();
		delete states.top();
		states.pop();
	}
}
int SFMLEngine::getFPS() const {
	return (int)(1000000LL / max(lastFrame.asMicroseconds(), 1LL));
}
const sf::Time& SFMLEngine::getLastTick() const {
	return lastFrame;
}
void SFMLEngine::setClearColor(sf::Color& set) {
	clearColor = set;
}