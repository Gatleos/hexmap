#include <iostream>
#include <fstream>
#include <sstream>
#include <list>
#include "compat.h"
#include "States.h"
#include "HexMap.h"
#include "simplexnoise.h"
#include "clamp.h"
#include "lerp.h"
#include "config.h"
#include "BezierCurve.h"
#include "ResourceLoader.h"
#include "MapEntity.h"
#include <limits>

AnimHandler ah;

#define MAPX 128
#define MAPY 128

std::shared_ptr<EngineState> EngineState::instance() {
	static EngineState* es = new EngineState;
	static std::shared_ptr<EngineState> ptr(es);
	return ptr;
}
EngineState::EngineState() {
}
void EngineState::init() {
	// SFML init
	engine->clearColor = { 0, 43, 77, 255 };
	engine->window->setFramerateLimit(60);
	// Initialize views
	sf::Vector2u winSize = engine->window->getSize();
	HEXMAP.view.setSize((sf::Vector2f)winSize);
	winSize.x /= 2, winSize.y /= 2;
	HEXMAP.view.setCenter((sf::Vector2f)winSize);
	HEXMAP.view.setViewport(sf::FloatRect(0.0f, 0.0f, 1.0f, 1.0f));
	UI::view = HEXMAP.view;
	// Set up map
	HEXMAP.init(MAPX, MAPY);
	HEXMAP.setAllTiles(HexTileS::get(HexTileS::OCEAN), rng::r);
	HEXMAP.calculateViewArea(HEXMAP.view);
	// GUI construction
	UI::addNewLayout(UIdef::MapGenDebug::instance());
	UI::pushLayout(UIdef::MapGenDebug::instance());
	auto* f = HEXMAP.addFaction();
	siteMenu = UIdef::SiteMenu::instance();
	UI::addNewLayout(siteMenu);
	UI::addNewLayout(UIdef::DeployGroupMenu::instance());
	// Entities
	//auto* f = HEXMAP.addFaction();
	//for (int x = 0; x < 16384; x++) {
	//	sf::Vector2i pos = { x % 128, x / 128 };
	//	auto* s = HEXMAP.addSite(SiteS::get("si_castle"), f);
	//	s->initMapPos(HexMap::offsetToAxial(pos));
	//	s->setAnimationType(MapEntityS::anim::IDLE);
	//}
	// Shader
	//shader.loadFromFile("data/simplex.glsl", sf::Shader::Type::Fragment);
	//shader.setParameter("offset", HEXMAP.view.getCenter());
	//shader.setParameter("texture", sf::Shader::CurrentTexture);
	//shader.setParameter("scale", 0.001f);
	//shader.setParameter("contrast", 0.25f);
	//shader.setParameter("brightness", 0.9f);
	//engine->window->setFramerateLimit(600);
	engine->pushState(MapControlState::instance());
	ah.setAnimationData(*RESOURCE.anim("castle.anim"));
	ah.setAnimation("full");
	ah.setPosition({30.0f, 30.0f});
}
void EngineState::end() {
}
void EngineState::update() {
	HEXMAP.update(engine->getLastTick());
	ah.updateAnimation(engine->getLastTick());
}
void EngineState::render(sf::RenderWindow &window) {
	stringstream ss;
	static float elapsed = 0.0f;
	elapsed += engine->getLastTick().asSeconds();
	if (elapsed >= 0.5f) {
		elapsed = 0.0f;
		if (timeDisplay) {
			ss << "SFML Test    last tick: " << engine->getLastTick().asMicroseconds();
		}
		else {
			ss << "SFML Test    FPS: " << engine->getFPS();
		}
		window.setTitle(ss.str());
		ss.str(std::string());
	}
	window.setView(HEXMAP.view);
	for (int a = 0; a < 1; a++) {
		window.draw(HEXMAP);
		HEXMAP.drawEnts(window);
	}
	window.draw(UI::hexSelector);
	window.setView(UI::view);
	ss << mtMilli;
	UIdef::MapGenDebug::instance()->debugInfo[4]->SetText(ss.str());
	ss.str(std::string());
}
void EngineState::input(sf::Event &e) {
	if (UI::gotInput()) {
		return;
	}
	if (e.type == sf::Event::MouseButtonPressed) {
		auto& clicked = MapControlState::instance()->tilePos;
		auto& hex = HEXMAP.getAxial(clicked.x, clicked.y);
		if (e.mouseButton.button == sf::Mouse::Left) {
			if (UIdef::selectedEnt != nullptr) {
				UIdef::selectedEnt->deselect();
			}
			if (hex.ent != nullptr) {
				UIdef::selectedEnt = hex.ent;
				UI::selectHex((sf::Vector2f)clicked);
				hex.ent->select();
			}
		}
		else if (e.mouseButton.button == sf::Mouse::Right) {
			if (UIdef::selectedEnt != nullptr) {
				UIdef::selectedEnt->setPath(clicked);
			}
		}
	}
	else {
		if (config::pressed(e, "fps_display")) {
			timeDisplay = !timeDisplay;
		}
		else if (config::pressed(e, "generate")) {
			generate();
		}
		else if (config::pressed(e, "debug")) {
			HEXMAP.advanceTurn();
		}
	}
}

void EngineState::generate() {
	HEXMAP.clearTileFeatures();
	unsigned long hexSeed = 0;
	if (UIdef::MapGenDebug::instance()->randomSeed->IsActive()) {
		hexSeed = rng::r();
	}
	else {
		string seed = (string)UIdef::MapGenDebug::instance()->seedBox->GetText();
		if (seed.empty()) {
			seed = "0";
		}
		hexSeed = stoul(seed, nullptr, 16);
	}
	customSeed.seed(hexSeed);
	sf::Clock mtClock;
	HEXMAP.generateBiomes(customSeed);
	for (int a = 0; a < config::gen::mountNum; a++) {
		HEXMAP.generateMountainRange(customSeed);
	}
	HEXMAP.placeSites(customSeed);
	sf::Time mtTime = mtClock.getElapsedTime();
	mtMilli = mtTime.asMilliseconds();
	stringstream ss;
	ss << std::hex << hexSeed;
	UIdef::MapGenDebug::instance()->seedBox->SetText(ss.str());
}

// Clear and reload all resource files, then generate the map
// with the same seed so we can see the changes
void EngineState::loadResourcesInPlace() {
	config::load();
	RESOURCE.releaseAll();
	config::loadAllJson();
	string seed = (string)UIdef::MapGenDebug::instance()->seedBox->GetText();
	if (seed.empty()) {
		seed = "0";
	}
	unsigned long hexSeed = stoul(seed, nullptr, 16);
	customSeed.seed(hexSeed);
	sf::Clock mtClock;
	HEXMAP.generateBiomes(customSeed);
	for (int a = 0; a < config::gen::mountNum; a++) {
		HEXMAP.generateMountainRange(customSeed);
	}
	HEXMAP.placeSites(customSeed);
	sf::Time mtTime = mtClock.getElapsedTime();
	mtMilli = mtTime.asMilliseconds();
	stringstream ss;
	ss << std::hex << hexSeed;
	UIdef::MapGenDebug::instance()->seedBox->SetText(ss.str());
	UI::desktop->LoadThemeFromFile("data/test.theme");
	// Shader
	//shader.loadFromFile("data/simplex.glsl", sf::Shader::Type::Fragment);
	//shader.setParameter("offset", HEXMAP.view.getCenter());
	//shader.setParameter("texture", sf::Shader::CurrentTexture);
	//shader.setParameter("scale", 0.001f);
	//shader.setParameter("contrast", 0.25f);
	//shader.setParameter("brightness", 0.9f);
}
