#include "MapUnit.h"
#include "HexMap.h"


MapUnit::MapUnit(const MapEntityS* sEnt, HexMap* hmSet, Faction* parent) :
moveTimer(0),
MapEntity(sEnt, hmSet, parent) {
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

}