#include <iostream>
#include <fstream>
#include <sstream>
#include "States.h"
#include "HexMap.h"
#include "UI2.h"

std::shared_ptr<UIState> UIState::instance() {
	static auto uis = std::make_shared<UIState>(UIState());
	return uis;
}
UIState::UIState() {
}
void UIState::init() {
	ifstream in;
	in.open("data/default.style");
	in.read((char*)&ImGui::GetStyle(), sizeof(ImGuiStyle));
	in.close();
}
void UIState::end() {
	ofstream out;
	out.open("data/default.style");
	out.write((char*)&ImGui::GetStyle(), sizeof(ImGuiStyle));
	out.close();
}
void UIState::update() {
}
void UIState::render(sf::RenderWindow &window) {
	static bool randomSeed = true;
	//ImGui::ShowStyleEditor(&ImGui::GetStyle());
	//ui2::mapInfoMenu(HEXMAP, randomSeed);
}
void UIState::input(sf::Event &e) {
}
