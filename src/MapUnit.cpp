#include <iostream>
#include "MapUnit.h"
#include "HexMap.h"
#include "UIdef.h"
#include "clamp.h"

using namespace std;

#define unitptr(x) unique_ptr<MapUnitS>(new MapUnitS(x))

std::array<std::unique_ptr<MapUnitS>, MapUnitS::UNIT_NUM> MapUnitS::unit = { {
		unitptr("un_null"), unitptr("un_army"), unitptr("un_caravan")
	} };

MapUnitS::MapUnitS(std::string id) :
MapEntityS(id),
maxHitPoints(1),
type(Population::GROUP_CIV) {
}

void MapUnitS::loadJson(std::string filename) {
	Json::Value root = config::openJson(filename);
	if (root.begin() == root.end()) {
		cerr << "ERROR: couldn't open file \"" << filename << "\"\n";
		return;
	}
	// Cycle through defined unit types; make sure to skip un_null!
	for (int a = 1; a < UNIT_NUM; a++) {
		auto& un = unit[a];
		Json::Value udata = root.get(un->id_, Json::Value::null);
		string element;
		try {
			un->loadEntityJson(udata, element, un->id_);
			element = "type";
			std::string typeName = udata.get(element, "").asString();
			bool foundType = false;
			for (int g = 0; g < Population::groups.size(); g++) {
				if (Population::groups[g].id == typeName) {
					un->type = g;
					foundType = true;
				}
			}
			if (!foundType) {
				cerr << "[" << filename << ", " << un->id_ << ", " << element << "] " <<
					"WARNING: group type \"" << typeName << "\" not found; using \"" <<
					Population::groups[Population::GROUP_CIV].id << "\" instead" << "\n";
			}
			element = "maxHitPoints";
			un->maxHitPoints = std::max(1, udata.get(element, 1).asInt());
		}
		catch (runtime_error e) { // report the error with the name of the object and member
			cerr << "[" << filename << ", " << un->id_ << ", " << element << "] " << e.what() << "\n";
		}
	}
}

const MapUnitS& MapUnitS::get(int id) {
	return *unit[id];
}



MapUnit::MapUnit(const MapUnitS* sUnit, HexMap* hmSet, Faction* parent) :
moveTimer(0),
MapEntity(sUnit, hmSet, parent) {
	su = sUnit;
	setHealth(0);
}

void MapUnit::setStaticUnit(const MapUnitS* sUnit) {
	su = sUnit;
	setStaticEntity(sUnit);
	setHealth(getHealth());
}

bool MapUnit::walkPath() {
	if (path.empty()) {
		return false;
	}
	setMapPos(*path.begin());
	path.erase(path.begin());
	return true;
}

void MapUnit::setPath(sf::Vector2i dest) {
	path.clear();
	hm->getPath(path, pos, dest);
}

void MapUnit::setHealth(int health) {
	health = clamp(health, 0, HealthBar::tierValues[HEALTH_TIER_NUM - 1]);
	hp.setHealth(health);
	hp.updateBars();
	switch (hp.getTier()) {
	case 0:
		setAnimationType(MapEntityS::anim::WOUND5);
		break;
	case 1:
		setAnimationType(MapEntityS::anim::WOUND4);
		break;
	case 2:
		setAnimationType(MapEntityS::anim::WOUND3);
		break;
	case 3:
		setAnimationType(MapEntityS::anim::WOUND2);
		break;
	case 4:
		setAnimationType(MapEntityS::anim::WOUND1);
		break;
	case 5:
		setAnimationType(MapEntityS::anim::IDLE);
		break;
	}
}

int MapUnit::getHealth() const {
	return hp.getHealth();
}

void MapUnit::setFood(int foodAmount) {
	hp.setFood(foodAmount);
	hp.updateBars();
}

int MapUnit::getFood() const {
	return hp.getFood();
}

int MapUnit::getMemberType() const {
	return su->type;
}

void MapUnit::appendPath(sf::Vector2i dest) {
	hm->getPath(path, pos, dest);
}

void MapUnit::update(const sf::Time& timeElapsed) {
	moveTimer += timeElapsed.asMilliseconds();
	if (moveTimer >= 500) {
		walkPath();
		moveTimer = moveTimer % 500;
	}
}

void MapUnit::advanceTurn() {
	walkPath();
}

void MapUnit::select() {
	UIdef::MapUnitInfo::instance()->show(true);
	UIdef::setUnit(*this);
}

void MapUnit::deselect() {
	UIdef::MapUnitInfo::instance()->show(false);
}

void MapUnit::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	states.transform *= this->getTransform();
	handlers_[zoomLevel].draw(target, states);
}

void MapUnit::drawSelectors(sf::RenderTarget& target, sf::RenderStates states) const {
	UI::drawHexSelector((sf::Vector2f)pos, sf::Color::Red, target);
	if (!path.empty()) {
		UI::drawHexSelector((sf::Vector2f)path.back(), sf::Color::Green, target);
	}
}

void MapUnit::drawHUD(sf::RenderTarget& target, sf::RenderStates states) const {
	states.transform *= this->getTransform();
	states.transform *= handlers_[zoomLevel].getTransform();
	hp.draw(target, states);
}