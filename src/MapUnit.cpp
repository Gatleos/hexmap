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
maxHitPoints(0) {
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
			element = "maxHitPoints";
			un->maxHitPoints = std::max(1, udata.get(element, "maxHitPoints").asInt());
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
	hp.setSize({ 50.0f, 10.0f });
	hp.setPosition({ -25.0f, -50.0f });
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
	hitPoints = health;
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

int MapUnit::getHealth() {
	return hitPoints;
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
	hp.draw(target, states);
}