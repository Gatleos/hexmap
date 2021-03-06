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

sf::View mapView;
sf::View miniMapView;
sf::View uiView;

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
const char* resourceRoot = "data/sprites/";
float cloudSpeed = 100.0f;

void EngineState::init()
{
	// SFML init
	engine->clearColor = { 0, 43, 77, 255 };
	engine->window->setFramerateLimit(60);
	// GUI construction
	mapGenDebug = shared_ptr<UIdef::MapGenDebug>(new UIdef::MapGenDebug);
	mapGenDebug->gen->GetSignal(sfg::Button::OnMouseLeftPress).Connect(std::bind(&EngineState::generate, this));
	mapGenDebug->reload->GetSignal(sfg::Button::OnMouseLeftPress).Connect(std::bind(&EngineState::loadResourcesInPlace, this));
	UI::addNewLayout(mapGenDebug);
	UI::pushLayout(mapGenDebug);
	// Initialize views
	sf::Vector2u winSize = engine->window->getSize();
	mapView.setSize((sf::Vector2f)winSize);
	winSize.x /= 2, winSize.y /= 2;
	mapView.setCenter((sf::Vector2f)winSize);
	mapView.setViewport(sf::FloatRect(0.0f, 0.0f, 1.0f, 1.0f));
	uiView = mapView;
	// Resources
	config::load();
	RESOURCE.setRoot(resourceRoot);
	config::loadAllJson();
	// Set up map
	hg.init(MAPX, MAPY);
	hg.setAllTiles(HexTileS::get(HexTileS::OCEAN), rng::r);
	hg.calculateViewArea(mapView);
	// Entities
	//auto* f = hg.addFaction();
	//for (int x = 0; x < 16384; x++) {
	//	sf::Vector2i pos = { x % 128, x / 128 };
	//	auto* s = hg.addSite(SiteS::get("si_castle"), f);
	//	s->initMapPos(HexMap::offsetToAxial(pos));
	//	s->setAnimationType(MapEntityS::anim::IDLE);
	//}
	// Shader
	shader.loadFromFile("data/simplex.glsl", sf::Shader::Type::Fragment);
	shader.setParameter("offset", mapView.getCenter());
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
	const sf::Vector2f& size = mapView.getSize();
	const sf::Vector2f& center = mapView.getCenter();
	camPos = { center.x, center.y };
	if (camDeltaX != 0 || camDeltaY != 0) {
		mapView.move({ (float)camDeltaX * 20 * move, (float)camDeltaY * 20 * move });
		hg.constrainView(mapView);
		hg.calculateViewArea(mapView);
		shader.setParameter("offset", mapView.getCenter());
	}
	hg.update(engine->getLastTick());
	sf::Vector2f timeOffset = mapView.getCenter();
	timeOffset.x += hg.getLifetime().asSeconds() * cloudSpeed;
	timeOffset.y += hg.getLifetime().asSeconds() * cloudSpeed;
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
	window.setView(mapView);
	for (int a = 0; a < 1; a++) {
		window.draw(hg, &shader);
		hg.drawEnts(window, &shader);
	}
	window.setView(uiView);
	sf::Vector2i mouse = sf::Mouse::getPosition(*engine->window);
	const sf::Vector2f& size = mapView.getSize();
	const sf::Vector2f& center = mapView.getCenter();
	snprintf(str, 50, "%d,%d", (int)camPos.x, (int)camPos.y);
	mapGenDebug->debugInfo[0]->SetText(str);
	snprintf(str, 50, "%d,%d", (int)mouseMapPos.x, (int)mouseMapPos.y);
	mapGenDebug->debugInfo[1]->SetText(str);
	snprintf(str, 50, "%d,%d", (int)tilePos.x, (int)tilePos.y);
	mapGenDebug->debugInfo[2]->SetText(str);
	snprintf(str, 50, "%d", mtMilli);
	mapGenDebug->debugInfo[4]->SetText(str);
	const sf::IntRect& ir = hg.getChunkViewArea();
	snprintf(str, 50, "(%d,%d / %d,%d)", ir.left, ir.top, ir.width, ir.height);
	mapGenDebug->debugInfo[5]->SetText(str);
}
void EngineState::input(sf::Event &e)
{
	if (e.type == sf::Event::MouseMoved) {
		if (mButtonPressed) {
			mapView.move({ roundf(mousePos.x - e.mouseMove.x), roundf(mousePos.y - e.mouseMove.y) });
			hg.constrainView(mapView);
			hg.calculateViewArea(mapView);
			shader.setParameter("offset", mapView.getCenter());
		}
		mousePos = { (float)e.mouseMove.x, (float)e.mouseMove.y };
		const sf::Vector2f& size = mapView.getSize();
		const sf::Vector2f& center = mapView.getCenter();
		mouseMapPos = { e.mouseMove.x - size.x / 2.0f + center.x, e.mouseMove.y - size.y / 2.0f + center.y };
		tilePos = hg.pixelToHex(mouseMapPos);
		if (hg.isAxialInBounds((sf::Vector2i)tilePos)) {
			mapGenDebug->debugInfo[3]->SetText(hg.getAxial((int)tilePos.x, (int)tilePos.y).hts->name);
			hg.updateCursorPos((sf::Vector2i)tilePos);
		}
		else {
			mapGenDebug->debugInfo[3]->SetText("");
			hg.updateCursorPos({ -10, -10 });
		}
	}
	else if (e.type == sf::Event::MouseWheelMoved) {
		int zoom = hg.getZoomLevel() + e.mouseWheel.delta;
		if (isInRange(zoom, 0, 2)) {
			sf::Vector2f mouse = (sf::Vector2f)sf::Mouse::getPosition(*engine->window);
			const sf::Vector2f& size = mapView.getSize();
			const sf::Vector2f& center = mapView.getCenter();
			mouse.x = mouse.x - size.x / 2.0f + center.x, mouse.y = mouse.y - size.y / 2.0f + center.y;
			sf::Vector2f hex = hg.pixelToHex(mouse);
			hg.setZoomLevel(zoom);
			sf::Vector2f pixel = hg.hexToPixel(hex);
			mapView.move({ pixel.x - mouse.x, pixel.y - mouse.y });
			hg.constrainView(mapView);
			hg.calculateViewArea(mapView);
			shader.setParameter("offset", mapView.getCenter());
			shader.setParameter("scale", (1 << zoom) / 1000.0f);
			cloudSpeed = 100 >> zoom;
		}
	}
	else if (e.type == sf::Event::MouseButtonPressed) {
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
	else if (e.type == sf::Event::Resized)
	{
		mapView.setSize(sf::Vector2f((float)e.size.width, (float)e.size.height));
		mapView.setViewport(sf::FloatRect(0.0f, 0.0f, 1.0f, 1.0f));
		uiView.setSize(sf::Vector2f(roundf((float)e.size.width), roundf((float)e.size.height)));
		uiView.setViewport(sf::FloatRect(0.0f, 0.0f, 1.0f, 1.0f));
		uiView.setCenter(uiView.getSize() / 2.0f);
		//
		hg.constrainView(mapView);
		hg.calculateViewArea(mapView);
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
	}
}

void EngineState::generate()
{
	hg.clearTileFeatures();
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
	hg.generateBiomes(customSeed);
	for (int a = 0; a < config::gen::mountNum; a++) {
		hg.generateMountainRange(customSeed);
	}
	hg.placeSites(customSeed);
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
	RESOURCE.setRoot(resourceRoot);
	config::loadAllJson();
	string seed = (string)mapGenDebug->seedBox->GetText();
	if (seed.empty()) {
		seed = "0";
	}
	unsigned long hexSeed = stoul(seed, nullptr, 16);
	customSeed.seed(hexSeed);
	sf::Clock mtClock;
	hg.generateBiomes(customSeed);
	for (int a = 0; a < config::gen::mountNum; a++) {
		hg.generateMountainRange(customSeed);
	}
	hg.placeSites(customSeed);
	sf::Time mtTime = mtClock.getElapsedTime();
	mtMilli = mtTime.asMilliseconds();
	stringstream ss;
	ss << std::hex << hexSeed;
	mapGenDebug->seedBox->SetText(ss.str());
	// Shader
	shader.loadFromFile("data/simplex.glsl", sf::Shader::Type::Fragment);
	shader.setParameter("offset", mapView.getCenter());
	shader.setParameter("texture", sf::Shader::CurrentTexture);
	shader.setParameter("scale", 0.001f);
	shader.setParameter("contrast", 0.25f);
	shader.setParameter("brightness", 0.9f);
}
