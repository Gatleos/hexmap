#include <iostream>
#include <SFGUI/Renderers.hpp>
#include "SFMLEngine.h"



SFMLEngine& SFMLEngine::instance() {
	static SFMLEngine instance;
	return instance;
}
void SFMLEngine::init(sf::RenderWindow* windowset) {
	window = windowset;
	window->setFramerateLimit(60);
	clearColor = sf::Color::Black;
	auto renderer = sfg::VertexBufferRenderer::Create(); // Fix a NonLegacyRenderer-related text bug
	sfg::Renderer::Set(renderer);
	UI::setAppSize((sf::Vector2f)window->getSize());
	UI::init(&desktop); // Construct the gui
}
void SFMLEngine::start() {
	try {
		sf::Clock fr;
		while (window->isOpen()) {
			while (window->pollEvent(event)) {
				desktop.HandleEvent(event);
				if (event.type == sf::Event::Resized) {
					UI::setAppSize({ (float)event.size.width, (float)event.size.height });
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