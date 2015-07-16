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
#include "json.h"
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

void EngineState::init()
{
	// SFML init
	engine->clearColor = { 0, 43, 77, 255 };
	engine->window->setFramerateLimit(60);
	// GUI construction
	mapGenDebug = UILayout::create();
	auto window1 = sfg::Window::Create(sfg::Window::Style::BACKGROUND);
	auto table1 = sfg::Table::Create();
	array<const char*, 6U> s = { "Camera:", "Mouse:", "Hex:", "Terrain:", "Gen (ms):", "Chunks:" };
	array<shared_ptr<sfg::Label>, 6U> label2;
	for (int a = 0; a < debugInfo.size(); a++) {
		label2[a] = sfg::Label::Create(s[a]);
		label2[a]->SetAlignment({ 0.0f, 0.0f });
		table1->Attach(label2[a], { 0U, (sf::Uint32)a, 1U, 1U });
	}
	for (int a = 0; a < debugInfo.size(); a++) {
		debugInfo[a] = sfg::Label::Create("");
		debugInfo[a]->SetAlignment({ 0.0f, 0.0f });
		table1->Attach(debugInfo[a], { 1U, (sf::Uint32)a, 1U, 1U });
	}
	seedBox = sfg::Entry::Create();
	seedBox->SetAllocation({ 0.0f, 0.0f, 75.0f, 5.0f });
	seedBox->SetRequisition({ 75.0f, 5.0f });
	seedBox->SetMaximumLength(8);
	randomSeed = sfg::CheckButton::Create("Random seed");
	randomSeed->SetActive(true);
	auto box1 = sfg::Box::Create(sfg::Box::Orientation::VERTICAL, 10.0f);
	auto box2 = sfg::Box::Create(sfg::Box::Orientation::HORIZONTAL, 5.0f);
	auto gen = sfg::Button::Create("Generate");
	auto reload = sfg::Button::Create("Reload Files");
	gen->GetSignal(sfg::Button::OnMouseLeftPress).Connect(std::bind(&EngineState::generate, this));
	reload->GetSignal(sfg::Button::OnMouseLeftPress).Connect(std::bind(&EngineState::loadResourcesInPlace, this));
	box1->Pack(table1);
	box1->Pack(box2);
	box1->Pack(gen);
	box1->Pack(reload);
	box2->Pack(randomSeed);
	box2->Pack(seedBox);
	window1->Add(box1);
	mapGenDebug->addWindow(window1, UIAlign({ 1.0f, 0.0f, 210.0f, 120.0f }, UI::ALIGN_RIGHT | UI::ALIGN_FRAC_POSX));
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
	RESOURCE.setRoot("data/");
	config::loadAllJson();
	// Set up map
	hex.loadFromFile("data/iso.png");
	hg.init(MAPX, MAPY);
	hg.setTexture(hex);
	hg.setAllTiles(HexTileS::get(HexTileS::OCEAN));
	hg.calculateViewArea(mapView);
	// Entities
	//uniform_int_distribution<int> size(0, 127);
	//auto* f = hg.addFaction();
	//for (int x = 0; x < 300; x++) {
	//	sf::Vector2i pos = { size(rng::r), size(rng::r) };
	//	auto* s = hg.addSite(SiteS::get("si_castle"), f);
	//	s->initMapPos(HexMap::offsetToAxial(pos));
	//}
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
	}
	hg.update(engine->getLastTick());
}
void EngineState::render(sf::RenderWindow &window)
{
	static float elapsed = 0.0f;
	elapsed += engine->getLastTick().asSeconds();
	if (elapsed >= 0.5f) {
		elapsed = 0.0f;
		if (timeDisplay) {
			snprintf(str, 50, "SFML Test    last tick (ms): %d", engine->getLastTick().asMilliseconds());
		}
		else {
			snprintf(str, 50, "SFML Test    FPS: %d", engine->getFPS());
		}
		window.setTitle(str);
	}
	window.setView(mapView);
	for (int a = 0; a < 1; a++) {
		window.draw(hg);
	}
	window.setView(uiView);
	sf::Vector2i mouse = sf::Mouse::getPosition(*engine->window);
	const sf::Vector2f& size = mapView.getSize();
	const sf::Vector2f& center = mapView.getCenter();
	snprintf(str, 50, "%d,%d", (int)camPos.x, (int)camPos.y);
	debugInfo[0]->SetText(str);
	snprintf(str, 50, "%d,%d", (int)mouseMapPos.x, (int)mouseMapPos.y);
	debugInfo[1]->SetText(str);
	snprintf(str, 50, "%d,%d", (int)tilePos.x, (int)tilePos.y);
	debugInfo[2]->SetText(str);
	snprintf(str, 50, "%d", mtMilli);
	debugInfo[4]->SetText(str);
	const sf::IntRect& ir = hg.getChunkViewArea();
	snprintf(str, 50, "(%d,%d / %d,%d)", ir.left, ir.top, ir.width, ir.height);
	debugInfo[5]->SetText(str);
}
void EngineState::input(sf::Event &e)
{
	if (e.type == sf::Event::MouseMoved) {
		if (mButtonPressed) {
			mapView.move({ roundf(mousePos.x - e.mouseMove.x), roundf(mousePos.y - e.mouseMove.y) });
			hg.constrainView(mapView);
			hg.calculateViewArea(mapView);
		}
		mousePos = { (float)e.mouseMove.x, (float)e.mouseMove.y };
		const sf::Vector2f& size = mapView.getSize();
		const sf::Vector2f& center = mapView.getCenter();
		mouseMapPos = { e.mouseMove.x - size.x / 2.0f + center.x, e.mouseMove.y - size.y / 2.0f + center.y };
		tilePos = hg.pixelToHex(mouseMapPos);
		if (hg.isAxialInBounds((sf::Vector2i)tilePos)) {
			debugInfo[3]->SetText(hg.getAxial(tilePos.x, tilePos.y).hts->name);
		}
		else {
			debugInfo[3]->SetText("");
		}
	}
	else if (e.type == sf::Event::MouseWheelMoved) {
		int zoom = hg.getZoomLevel();
		if (isInRange(zoom + e.mouseWheel.delta, 0, 2)) {
			sf::Vector2f mouse = (sf::Vector2f)sf::Mouse::getPosition(*engine->window);
			const sf::Vector2f& size = mapView.getSize();
			const sf::Vector2f& center = mapView.getCenter();
			mouse.x = mouse.x - size.x / 2.0f + center.x, mouse.y = mouse.y - size.y / 2.0f + center.y;
			sf::Vector2f hex = hg.pixelToHex(mouse);
			hg.setZoomLevel(zoom + e.mouseWheel.delta);
			sf::Vector2f pixel = hg.hexToPixel(hex);
			mapView.move({ pixel.x - mouse.x, pixel.y - mouse.y });
			hg.constrainView(mapView);
			hg.calculateViewArea(mapView);
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
	else if (e.type == sf::Event::KeyReleased) {
		if (e.key.code == sf::Keyboard::Left || 
			e.key.code == sf::Keyboard::Right ||
			e.key.code == sf::Keyboard::A ||
			e.key.code == sf::Keyboard::D) {
			camDeltaX = 0;
		}
		else if (e.key.code == sf::Keyboard::Up ||
			e.key.code == sf::Keyboard::Down || 
			e.key.code == sf::Keyboard::W || 
			e.key.code == sf::Keyboard::S) {
			camDeltaY = 0;
		}
	}
	else if (e.type == sf::Event::KeyPressed) {
		if (e.key.code == sf::Keyboard::Num1) {
		}
		else if (e.key.code == sf::Keyboard::Num2) {
		}
		else if (e.key.code == sf::Keyboard::Left || e.key.code == sf::Keyboard::A) {
			camDeltaX = -1;
		}
		else if (e.key.code == sf::Keyboard::Right || e.key.code == sf::Keyboard::D) {
			camDeltaX = 1;
		}
		else if (e.key.code == sf::Keyboard::Up || e.key.code == sf::Keyboard::W) {
			camDeltaY = -1;
		}
		else if (e.key.code == sf::Keyboard::Down || e.key.code == sf::Keyboard::S) {
			camDeltaY = 1;
		}
		else if (e.key.code == sf::Keyboard::BackSpace) {
			timeDisplay = !timeDisplay;
		}
		else if (e.key.code == sf::Keyboard::Return) {
			generate();
		}
	}
	else if (e.type == sf::Event::Resized)
	{
		mapView.setSize(sf::Vector2f(e.size.width, e.size.height));
		mapView.setViewport(sf::FloatRect(0.0f, 0.0f, 1.0f, 1.0f));
		uiView.setSize(sf::Vector2f(roundf(e.size.width), roundf(e.size.height)));
		uiView.setViewport(sf::FloatRect(0.0f, 0.0f, 1.0f, 1.0f));
		uiView.setCenter(uiView.getSize() / 2.0f);
		miniMapView = mapView;
		miniMapView.setViewport(sf::FloatRect(0.9f, 0.9f, 1.0f, 1.0f));
		//
		hg.constrainView(mapView);
		hg.calculateViewArea(mapView);
	}
}

void EngineState::generate()
{
	hg.clearTileFeatures();
	unsigned long hexSeed = 0;
	if (randomSeed->IsActive()) {
		hexSeed = rng::r();
	}
	else {
		string seed = (string)seedBox->GetText();
		if (seed.empty()) {
			seed = "0";
		}
		hexSeed = stoul(seed, nullptr, 16);
	}
	customSeed.seed(hexSeed);
	sf::Clock mtClock;
	hg.generateBiomes(customSeed);
	for (int a = 0; a < config::mountNum; a++) {
		hg.generateMountainRange(customSeed);
	}
	sf::Time mtTime = mtClock.getElapsedTime();
	mtMilli = mtTime.asMilliseconds();
	stringstream ss;
	ss << std::hex << hexSeed;
	seedBox->SetText(ss.str());
}

void EngineState::loadResourcesInPlace()
{
	config::load();
	RESOURCE.setRoot("data/");
	config::loadAllJson();
	string seed = (string)seedBox->GetText();
	if (seed.empty()) {
		seed = "0";
	}
	unsigned long hexSeed = stoul(seed, nullptr, 16);
	customSeed.seed(hexSeed);
	sf::Clock mtClock;
	hg.generateBiomes(customSeed);
	for (int a = 0; a < config::mountNum; a++) {
		hg.generateMountainRange(customSeed);
	}
	sf::Time mtTime = mtClock.getElapsedTime();
	mtMilli = mtTime.asMilliseconds();
	stringstream ss;
	ss << std::hex << hexSeed;
	seedBox->SetText(ss.str());
}