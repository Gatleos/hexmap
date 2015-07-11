#include "Species.h"
#include "MapEntity.h"
#include "HexMap.h"
#include "clamp.h"


const int Population::POP_LIMIT = 100000;

Population::Population(const Species& s) :species(s), size(0)
{
	for (auto& a : activities) {
		a = 0;
	}
}

void Population::add(int activity, int amount)
{
	if (!isInRange(activity, 0, ACTIVITY_NUM - 1)) {
		return;
	}
	if (amount < 0) {
		amount = max(amount, -activities[activity]);
	}
	size += amount;
	activities[activity] += amount;
}

void Population::addPop(const Population& p)
{
	if (species.id != p.species.id) {
		return;
	}
	for (int a = 0; a < ACTIVITY_NUM; a++) {
		add(a, p.activities[a]);
	}
}

void Population::takePop(Population& p)
{
	if (species.id != p.species.id) {
		return;
	}
	for (int a = 0; a < ACTIVITY_NUM; a++) {
		add(a, p.activities[a]);
		p.activities[a] = 0;
	}
}

MapEntity::MapEntity(Faction* parent) :hm(nullptr)
{
	faction = parent;
	pops.reserve(Species::map.size());
	for (auto& s : Species::map) {
		pops.emplace_back(s.second);
	}
}

void MapEntity::setParentMap(HexMap* hmSet)
{
	hm = hmSet;
}

bool MapEntity::setMapPos(sf::Vector2i axialCoord)
{
	if (!hm->isAxialInBounds(axialCoord)) {
		return false;
	}
	sf::Vector2i offset = HexMap::axialToOffset(pos);
	hm->getAxial(pos.x, pos.y).ent = nullptr;
	hm->setFeatureColor(offset, sf::Color::White);
	pos = axialCoord;
	offset = HexMap::axialToOffset(pos);
	hm->getAxial(pos.x, pos.y).ent = this;
	hm->setFeatureColor(offset, TileFeatureS::fade);
	setPosition((sf::Vector2f)hm->hexToPixel(pos) + hm->getOrigin());
	return true;
}

bool MapEntity::isOnscreen(const sf::View& mapView)
{
	return true;
}


MapUnit::MapUnit(Faction* parent) :moveTimer(0), MapEntity(parent){}

bool MapUnit::walkPath()
{
	if (path.empty()) {
		return false;
	}
	setMapPos(*path.begin());
	path.erase(path.begin());
	return true;
}

void MapUnit::setPath(sf::Vector2i dest)
{
	path.clear();
	hm->getPath(path, pos, dest);
}

void MapUnit::appendPath(sf::Vector2i dest)
{
	hm->getPath(path, pos, dest);
}

void MapUnit::update(const sf::Time& timeElapsed)
{
	moveTimer += timeElapsed.asMilliseconds();
	if (moveTimer >= 500) {
		walkPath();
		moveTimer = moveTimer % 500;
	}
}
