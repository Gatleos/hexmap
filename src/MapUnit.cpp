#include <iostream>
#include "MapUnit.h"
#include "HexMap.h"

using namespace std;

#define unitptr(x) unique_ptr<MapUnitS>(new MapUnitS(x))

std::array<std::unique_ptr<MapUnitS>, MapUnitS::UNIT_NUM> MapUnitS::unit = { {
		unitptr("un_null"), unitptr("un_army")
	} };

MapUnitS::MapUnitS(std::string id) :
MapEntityS(id) {
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

}