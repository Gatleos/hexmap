#include "States.h"
#include "HexMap.h"
#include "clamp.h"
#include "UI2.h"

sf::RenderTexture* rtex;
AnimHandler animHandler;

std::shared_ptr<MapControlState> MapControlState::instance() {
	static auto mcs = std::make_shared<MapControlState>(MapControlState());
	return mcs;
}
MapControlState::MapControlState() :
viewSize_(HEXMAP.view.getSize()),
viewCenter_(HEXMAP.view.getCenter()) {
}
void MapControlState::init() {
	rtex = new sf::RenderTexture;
	const AnimationData& animData = *RESOURCE.anim("monsters.anim");
	animHandler.setAnimationData(animData);
	rtex->create(300, 300);
}
void MapControlState::end() {
	delete rtex;
}
void MapControlState::update() {
	float move = 60.0f / engine->getFPS();
	if (camDelta_.x != 0 || camDelta_.y != 0) {
		HEXMAP.view.move({ (float)camDelta_.x * 20 * move, (float)camDelta_.y * 20 * move });
		HEXMAP.constrainView(HEXMAP.view);
		HEXMAP.calculateViewArea(HEXMAP.view);
	}
	sf::Vector2f timeOffset = HEXMAP.view.getCenter();
	timeOffset.x += HEXMAP.getLifetime().asSeconds() * HexMap::cloudSpeed;
	timeOffset.y += HEXMAP.getLifetime().asSeconds() * HexMap::cloudSpeed;
	HEXMAP.cloudShader.setParameter("offset", timeOffset);
	animHandler.updateAnimation(engine->getLastTick());
	prev->update();
}
void MapControlState::render(sf::RenderWindow &window) {
	static bool randomSeed = true;
	static int genTime = 1000;
	prev->render(window);
	ui2::mapInfoMenu((sf::Vector2i)mouseMapPos, tilePos, (sf::Vector2i)viewCenter_, terrainName_, genTime, randomSeed, animHandler);
	ImGui::ShowMetricsWindow();
}
void MapControlState::input(sf::Event &e) {
	if (e.type == sf::Event::MouseMoved) {
		if (HEXMAP.isGrabbed) {
			
			HEXMAP.view.move(sf::Vector2f(-ImGui::GetIO().MouseDelta.x, -ImGui::GetIO().MouseDelta.y));
			HEXMAP.constrainView(HEXMAP.view);
			HEXMAP.calculateViewArea(HEXMAP.view);
		}
		viewSize_ = HEXMAP.view.getSize();
		viewCenter_ = HEXMAP.view.getCenter();
		mouseMapPos = { e.mouseMove.x - viewSize_.x / 2.0f + viewCenter_.x, e.mouseMove.y - viewSize_.y / 2.0f + viewCenter_.y };
		tilePos = (sf::Vector2i)HEXMAP.pixelToHex(mouseMapPos);
		if (HEXMAP.isAxialInBounds(tilePos)) {
			terrainName_ = HEXMAP.getAxial((int)tilePos.x, (int)tilePos.y).hts->name;
			HEXMAP.updateCursorPos((sf::Vector2i)tilePos);
		}
		else {
			terrainName_ = "";
			HEXMAP.updateCursorPos({ -10, -10 });
		}
	}
	if (UI::gotInput()) {
		prev->input(e);
		return;
	}
	if (e.type == sf::Event::MouseButtonPressed) {
		if (e.mouseButton.button == sf::Mouse::Middle) {
			HEXMAP.isGrabbed = true;
		}
	}
	else if (e.type == sf::Event::MouseButtonReleased) {
		if (e.mouseButton.button == sf::Mouse::Middle) {
			HEXMAP.isGrabbed = false;
		}
	}
	else if (e.type == sf::Event::MouseWheelMoved) {
		int zoom = HEXMAP.getZoomLevel() + e.mouseWheel.delta;
		if (isInRange(zoom, 0, 2)) {
			sf::Vector2f mouse = (sf::Vector2f)sf::Mouse::getPosition(*engine->window);
			const sf::Vector2f& size = HEXMAP.view.getSize();
			const sf::Vector2f& center = HEXMAP.view.getCenter();
			mouse.x = mouse.x - size.x / 2.0f + center.x, mouse.y = mouse.y - size.y / 2.0f + center.y;
			sf::Vector2f hex = HEXMAP.pixelToHex(mouse);
			HEXMAP.setZoomLevel(zoom);
			sf::Vector2f pixel = HEXMAP.hexToPixel(hex);
			HEXMAP.view.move({ pixel.x - mouse.x, pixel.y - mouse.y });
			HEXMAP.constrainView(HEXMAP.view);
			HEXMAP.calculateViewArea(HEXMAP.view);
			HEXMAP.cloudShader.setParameter("offset", HEXMAP.view.getCenter());
			HEXMAP.cloudShader.setParameter("scale", (1 << zoom) / 1000.0f);
			HexMap::cloudSpeed = 100 >> zoom;
		}
	}
	else if (e.type == sf::Event::MouseLeft) {
		HEXMAP.isGrabbed = false;
	}
	else {
		if (config::pressed(e, "scroll_left")) {
			camDelta_.x = -1;
		}
		else if (config::pressed(e, "scroll_right")) {
			camDelta_.x = 1;
		}
		else if (config::pressed(e, "scroll_up")) {
			camDelta_.y = -1;
		}
		else if (config::pressed(e, "scroll_down")) {
			camDelta_.y = 1;
		}
		else if (config::released(e, "scroll_left") || config::released(e, "scroll_right")) {
			camDelta_.x = 0;
		}
		else if (config::released(e, "scroll_up") || config::released(e, "scroll_down")) {
			camDelta_.y = 0;
		}
	}
	prev->input(e);
}
