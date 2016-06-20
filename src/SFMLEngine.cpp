#include <iostream>
#include <SFGUI/Renderers.hpp>
#include "SFMLEngine.h"
#include "HexMap.h"
#include "imgui.h"
#include "imgui-rendering-SFML.h"
#include "imgui-events-SFML.h"



SFMLEngine& SFMLEngine::instance() {
	static SFMLEngine instance;
	return instance;
}
void SFMLEngine::init(sf::RenderWindow* windowset) {
	window = windowset;
	window->setFramerateLimit(60);
	clearColor = sf::Color::Black;
	ImGui::SFML::SetWindow(*windowset);
	ImGui::SFML::SetRenderTarget(*windowset);
	ImGui::SFML::InitImGuiEvents();
	ImGui::SFML::InitImGuiRendering();
}
void SFMLEngine::start() {
	try {
		sf::Clock fr;
		while (window->isOpen()) {
			ImGui::SFML::UpdateImGui();
			ImGui::SFML::UpdateImGuiRendering();
			while (window->pollEvent(event)) {
				//desktop.HandleEvent(event);
				ImGui::SFML::ProcessEvent(event);
				if (event.type == sf::Event::Closed) {
					window->close();
					UI::end();
				}
				else if ((event.type == sf::Event::MouseButtonPressed
					|| event.type == sf::Event::MouseWheelMoved) && ImGui::GetIO().WantCaptureMouse) {
					continue; // our mouse press was "consumed" by the UI layer, ignore it
				}
				else if (event.type == sf::Event::KeyPressed && ImGui::GetIO().WantCaptureKeyboard) {
					continue; // our key press was "consumed" by the UI layer, ignore it
				}
				else {
					// pass input on to the current state
					states.top()->input(event);
				}
				if (event.type == sf::Event::Resized) {
					HEXMAP.view.setSize(sf::Vector2f((float)event.size.width, (float)event.size.height));
					HEXMAP.view.setViewport(sf::FloatRect(0.0f, 0.0f, 1.0f, 1.0f));
					HEXMAP.constrainView(HEXMAP.view);
					HEXMAP.calculateViewArea(HEXMAP.view);
				}
			}
			//
			lastFrame = fr.restart();
			states.top()->update();
			//
			window->clear(clearColor);
			states.top()->render(*window);
			ImGui::Render();
			window->display();
		}
	}
	catch(runtime_error e) {
		cerr << e.what();
		UI::end();
		window->close();
	}
}

void SFMLEngine::pushState(std::shared_ptr<GameState> newState) {
	if (states.size()) {
		newState->prev = states.top().get();
	}
	states.push(std::move(newState));
	states.top()->engine = this;
	states.top()->init();
}
void SFMLEngine::popState() {
	if (!states.empty()) {
		states.top()->end();
		states.pop();
		while (window->pollEvent(event));
	}
}
void SFMLEngine::popAllStates() {
	if (states.empty()) {
		return;
	}
	while (states.size()) {
		states.top()->end();
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