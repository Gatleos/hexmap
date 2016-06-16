#include <iostream>
#include <fstream>
#include <sstream>
#include "States.h"
#include "HexMap.h"
#include "imgui.h"
#include "imgui-rendering-SFML.h"
#include "imgui-events-SFML.h"

std::shared_ptr<UIState> UIState::instance() {
	static auto uis = std::make_shared<UIState>(UIState());
	return uis;
}
UIState::UIState() {
}
void UIState::init() {
	ImGui::SFML::SetWindow(*engine->window);
	ImGui::SFML::SetRenderTarget(*engine->window);
	ImGui::SFML::InitImGuiEvents();
	ImGui::SFML::InitImGuiRendering();
	int x = 0;
}
void UIState::end() {
}
void UIState::update() {
	ImGui::SFML::UpdateImGui();
	ImGui::SFML::UpdateImGuiRendering();
}
void UIState::render(sf::RenderWindow &window) {
	ImGui::ShowStyleEditor();
	ImGui::Render();
}
void UIState::input(sf::Event &e) {
}
