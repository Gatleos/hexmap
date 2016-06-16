#include "States.h"
#include "HexMap.h"
#include "clamp.h"

std::shared_ptr<DungeonState> DungeonState::instance() {
	static auto mcs = std::make_shared<DungeonState>(DungeonState());
	return mcs;
}
DungeonState::DungeonState() :
vOffset(30.0f, 30.0f),
vSize(200.0f, 200.0f) {
	rectShape.setSize(vSize);
	rectShape.setPosition(vOffset);
	rectShape.setOutlineThickness(1.0f);
	rectShape.setFillColor(sf::Color::Transparent);
	vColors = { sf::Color(244, 46, 11), sf::Color(244, 104, 11), sf::Color(217, 227, 15), sf::Color(86, 220, 22),
		sf::Color(26, 217, 121), sf::Color(26, 217, 217), sf::Color(28, 117, 215), sf::Color(89, 64, 232),
		sf::Color(213, 36, 194) };
}
void DungeonState::init() {
	engine->clearColor = { 0, 0, 0, 255 };
	chart();
}
void DungeonState::end() {
}
void DungeonState::update() {
}
void DungeonState::render(sf::RenderWindow &window) {
	window.draw(rectShape);
	window.draw(vArray);
}
void DungeonState::input(sf::Event &e) {
	if (e.type == sf::Event::KeyPressed) {
		if (e.key.code == sf::Keyboard::Return) {
			chart();
		}
	}
}

void DungeonState::chart() {
	vArray.clear();
	Faction f;
	int turnIterations = 1000;
	int dangerDelta = 3;
	int iterations = 29;
	int columnWidth = vSize.x / (iterations + 1);
	float maxGold = turnIterations;
	float startY = vOffset.y + vSize.y;
	engine->window->setTitle(to_string(maxGold));
	//
	for (int i = 0, danger = 150 - (iterations / 2) * dangerDelta; i < iterations; i++, danger += dangerDelta) {
		float xPos = vOffset.x + columnWidth * (i + 1);
		for (int j = 0; j < 10; j++) {
			SiteDungeon dungeon(&SiteS::get(SiteS::DUNGEON), &HEXMAP, &f);
			UIdef::setDungeon(dungeon);
			dungeon.danger = danger;
			for (int a = 0; a < turnIterations; a++) {
				dungeon.advanceTurn();
			}
			float goldScaled = (dungeon.gold * vSize.y) / maxGold;
			sf::Vector2f pos(xPos, startY - goldScaled);
			vArray.append(sf::Vertex(pos, danger == 150 ? sf::Color::White : vColors[i % vColors.size()]));
		}
	}
}