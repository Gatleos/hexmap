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



#define MAPX 128
#define MAPY 128

sf::Clock frames;
char str[50];
sf::Vector2f tilePos;
bool mButtonPressed = false;
sf::Vector2f mousePos = { 0.0f, 0.0f };
sf::Vector2f mouseMapPos;
sf::Vector2f camPos;
char camDeltaX = 0, camDeltaY = 0;
int mtMilli = 0;
bool timeDisplay = false;
float cloudSpeed = 100.0f;

void EngineState::init()
{
	// SFML init
	engine->clearColor = { 0, 43, 77, 255 };
	engine->window->setFramerateLimit(60);
	// Initialize views
	sf::Vector2u winSize = engine->window->getSize();
	HexMap::view.setSize((sf::Vector2f)winSize);
	winSize.x /= 2, winSize.y /= 2;
	HexMap::view.setCenter((sf::Vector2f)winSize);
	HexMap::view.setViewport(sf::FloatRect(0.0f, 0.0f, 1.0f, 1.0f));
	UI::view = HexMap::view;
	// Set up map
	HEXMAP.init(MAPX, MAPY);
	HEXMAP.setAllTiles(HexTileS::get(HexTileS::OCEAN), rng::r);
	HEXMAP.calculateViewArea(HexMap::view);
	// GUI construction
	mapGenDebug = shared_ptr<UIdef::MapGenDebug>(new UIdef::MapGenDebug);
	mapGenDebug->gen->GetSignal(sfg::Button::OnMouseLeftPress).Connect(std::bind(&EngineState::generate, this));
	mapGenDebug->reload->GetSignal(sfg::Button::OnMouseLeftPress).Connect(std::bind(&EngineState::loadResourcesInPlace, this));
	UI::addNewLayout(mapGenDebug);
	UI::pushLayout(mapGenDebug);
	auto* f = HEXMAP.addFaction();
	Site* site = HEXMAP.addSite(&SiteS::get(SiteS::CITY), f);
	site->pop.setSize(Population::GROUP_CIV, 2000.0f);
	site->pop.setSize(Population::GROUP_PR, 2000.0f);
	site->initMapPos({ 0, 4 });
	siteMenu = UIdef::SiteMenu::instance();
	UI::addNewLayout(siteMenu);
	UI::pushLayout(siteMenu);
	UI::addNewLayout(UIdef::DeployGroupMenu::instance());
	UIdef::setSite(*site);
	// Entities
	//auto* f = HEXMAP.addFaction();
	//for (int x = 0; x < 16384; x++) {
	//	sf::Vector2i pos = { x % 128, x / 128 };
	//	auto* s = HEXMAP.addSite(SiteS::get("si_castle"), f);
	//	s->initMapPos(HexMap::offsetToAxial(pos));
	//	s->setAnimationType(MapEntityS::anim::IDLE);
	//}
	// Shader
	shader.loadFromFile("data/simplex.glsl", sf::Shader::Type::Fragment);
	shader.setParameter("offset", HexMap::view.getCenter());
	shader.setParameter("texture", sf::Shader::CurrentTexture);
	shader.setParameter("scale", 0.001f);
	shader.setParameter("contrast", 0.25f);
	shader.setParameter("brightness", 0.9f);
}
void EngineState::end()
{
}
void EngineState::update()
{
	float move = 60.0f / engine->getFPS();
	const sf::Vector2f& size = HexMap::view.getSize();
	const sf::Vector2f& center = HexMap::view.getCenter();
	camPos = { center.x, center.y };
	if (camDeltaX != 0 || camDeltaY != 0) {
		HexMap::view.move({ (float)camDeltaX * 20 * move, (float)camDeltaY * 20 * move });
		HEXMAP.constrainView(HexMap::view);
		HEXMAP.calculateViewArea(HexMap::view);
		shader.setParameter("offset", HexMap::view.getCenter());
	}
	HEXMAP.update(engine->getLastTick());
	sf::Vector2f timeOffset = HexMap::view.getCenter();
	timeOffset.x += HEXMAP.getLifetime().asSeconds() * cloudSpeed;
	timeOffset.y += HEXMAP.getLifetime().asSeconds() * cloudSpeed;
	shader.setParameter("offset", timeOffset);
}
void EngineState::render(sf::RenderWindow &window)
{
	static float elapsed = 0.0f;
	elapsed += engine->getLastTick().asSeconds();
	if (elapsed >= 0.5f) {
		elapsed = 0.0f;
		if (timeDisplay) {
			snprintf(str, 50, "SFML Test    last tick: %d", engine->getLastTick().asMicroseconds());
		}
		else {
			snprintf(str, 50, "SFML Test    FPS: %d", engine->getFPS());
		}
		window.setTitle(str);
	}
	window.setView(HexMap::view);
	for (int a = 0; a < 1; a++) {
		window.draw(HEXMAP, &shader);
		HEXMAP.drawEnts(window, &shader);
	}
	window.setView(UI::view);
	sf::Vector2i mouse = sf::Mouse::getPosition(*engine->window);
	const sf::Vector2f& size = HexMap::view.getSize();
	const sf::Vector2f& center = HexMap::view.getCenter();
	snprintf(str, 50, "%d,%d", (int)camPos.x, (int)camPos.y);
	mapGenDebug->debugInfo[0]->SetText(str);
	snprintf(str, 50, "%d,%d", (int)mouseMapPos.x, (int)mouseMapPos.y);
	mapGenDebug->debugInfo[1]->SetText(str);
	snprintf(str, 50, "%d,%d", (int)tilePos.x, (int)tilePos.y);
	mapGenDebug->debugInfo[2]->SetText(str);
	snprintf(str, 50, "%d", mtMilli);
	mapGenDebug->debugInfo[4]->SetText(str);
	const sf::IntRect& ir = HEXMAP.getChunkViewArea();
	snprintf(str, 50, "(%d,%d / %d,%d)", ir.left, ir.top, ir.width, ir.height);
	mapGenDebug->debugInfo[5]->SetText(str);
}
void EngineState::input(sf::Event &e)
{
	if (e.type == sf::Event::MouseMoved) {
		if (mButtonPressed) {
			HexMap::view.move({ roundf(mousePos.x - e.mouseMove.x), roundf(mousePos.y - e.mouseMove.y) });
			HEXMAP.constrainView(HexMap::view);
			HEXMAP.calculateViewArea(HexMap::view);
			shader.setParameter("offset", HexMap::view.getCenter());
		}
		mousePos = { (float)e.mouseMove.x, (float)e.mouseMove.y };
		const sf::Vector2f& size = HexMap::view.getSize();
		const sf::Vector2f& center = HexMap::view.getCenter();
		mouseMapPos = { e.mouseMove.x - size.x / 2.0f + center.x, e.mouseMove.y - size.y / 2.0f + center.y };
		tilePos = HEXMAP.pixelToHex(mouseMapPos);
		if (HEXMAP.isAxialInBounds((sf::Vector2i)tilePos)) {
			mapGenDebug->debugInfo[3]->SetText(HEXMAP.getAxial((int)tilePos.x, (int)tilePos.y).hts->name);
			HEXMAP.updateCursorPos((sf::Vector2i)tilePos);
		}
		else {
			mapGenDebug->debugInfo[3]->SetText("");
			HEXMAP.updateCursorPos({ -10, -10 });
		}
	}
	else if (e.type == sf::Event::MouseWheelMoved) {
		int zoom = HEXMAP.getZoomLevel() + e.mouseWheel.delta;
		if (isInRange(zoom, 0, 2)) {
			sf::Vector2f mouse = (sf::Vector2f)sf::Mouse::getPosition(*engine->window);
			const sf::Vector2f& size = HexMap::view.getSize();
			const sf::Vector2f& center = HexMap::view.getCenter();
			mouse.x = mouse.x - size.x / 2.0f + center.x, mouse.y = mouse.y - size.y / 2.0f + center.y;
			sf::Vector2f hex = HEXMAP.pixelToHex(mouse);
			HEXMAP.setZoomLevel(zoom);
			sf::Vector2f pixel = HEXMAP.hexToPixel(hex);
			HexMap::view.move({ pixel.x - mouse.x, pixel.y - mouse.y });
			HEXMAP.constrainView(HexMap::view);
			HEXMAP.calculateViewArea(HexMap::view);
			shader.setParameter("offset", HexMap::view.getCenter());
			shader.setParameter("scale", (1 << zoom) / 1000.0f);
			cloudSpeed = 100 >> zoom;
		}
	}
	else if (e.type == sf::Event::MouseButtonPressed) {
		if (UI::gotMouseInput()) {
			return;
		}
		//else {
		//	UI::dropFocus();
		//}
		if (e.mouseButton.button <= 1) {
			mButtonPressed = true;
		}
	}
	else if (e.type == sf::Event::MouseButtonReleased) {
		if (e.mouseButton.button <= 1) {
			mButtonPressed = false;
		}
	}
	else if (e.type == sf::Event::MouseLeft) {
		mButtonPressed = false;
	}
	else {
		if (config::pressed(e, "scroll_left")) {
			camDeltaX = -1;
		}
		else if (config::pressed(e, "scroll_right")) {
			camDeltaX = 1;
		}
		else if (config::pressed(e, "scroll_up")) {
			camDeltaY = -1;
		}
		else if (config::pressed(e, "scroll_down")) {
			camDeltaY = 1;
		}
		else if (config::released(e, "scroll_left") || config::released(e, "scroll_right")) {
			camDeltaX = 0;
		}
		else if (config::released(e, "scroll_up") || config::released(e, "scroll_down")) {
			camDeltaY = 0;
		}
		else if (config::pressed(e, "fps_display")) {
			timeDisplay = !timeDisplay;
		}
		else if (config::pressed(e, "generate")) {
			generate();
		}
		else if (config::pressed(e, "debug")) {
			Faction* f = HEXMAP.addFaction();
			Site* s = HEXMAP.addSite(&SiteS::get(SiteS::CITY), f);
			s->initMapPos((sf::Vector2i)tilePos);
		}
	}
}

void EngineState::generate()
{
	HEXMAP.clearTileFeatures();
	unsigned long hexSeed = 0;
	if (mapGenDebug->randomSeed->IsActive()) {
		hexSeed = rng::r();
	}
	else {
		string seed = (string)mapGenDebug->seedBox->GetText();
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
	//HEXMAP.placeSites(customSeed);
	sf::Time mtTime = mtClock.getElapsedTime();
	mtMilli = mtTime.asMilliseconds();
	stringstream ss;
	ss << std::hex << hexSeed;
	mapGenDebug->seedBox->SetText(ss.str());
}

// Clear and reload all resource files, then generate the map
// with the same seed so we can see the changes
void EngineState::loadResourcesInPlace()
{
	config::load();
	RESOURCE.releaseAll();
	config::loadAllJson();
	string seed = (string)mapGenDebug->seedBox->GetText();
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
	//HEXMAP.placeSites(customSeed);
	sf::Time mtTime = mtClock.getElapsedTime();
	mtMilli = mtTime.asMilliseconds();
	stringstream ss;
	ss << std::hex << hexSeed;
	mapGenDebug->seedBox->SetText(ss.str());
	// Shader
	shader.loadFromFile("data/simplex.glsl", sf::Shader::Type::Fragment);
	shader.setParameter("offset", HexMap::view.getCenter());
	shader.setParameter("texture", sf::Shader::CurrentTexture);
	shader.setParameter("scale", 0.001f);
	shader.setParameter("contrast", 0.25f);
	shader.setParameter("brightness", 0.9f);
}
