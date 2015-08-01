#include <iostream>
#include "ResourceLoader.h"
#include "Species.h"
#include "MapEntity.h"
#include "HexMap.h"
#include "clamp.h"
#include "config.h"

const array<string, MapEntityS::ANIM_NUM> MapEntityS::animTypes = { {
		"idle"
	} };
const char* Population::activityNames[ACTIVITY_NUM] = {
	"Idle", "Guard", "Farm", "Wood", "Mine"
};
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

bool MapEntity::initMapPos(sf::Vector2i axialCoord)
{
	if (!hm->isAxialInBounds(axialCoord)) {
		return false;
	}
	pos = axialCoord;
	sf::Vector2i offset = HexMap::axialToOffset(pos);
	hm->setEntity(offset, this);
	for (int a = 0; a < 3; a++) {
		handlers_[a].setPosition((sf::Vector2f)hm->hexToPixel(pos, a) + HexMap::getMapOrigin(a));
	}
	return true;
}

bool MapEntity::setMapPos(sf::Vector2i axialCoord)
{
	if (!hm->isAxialInBounds(axialCoord)) {
		return false;
	}
	sf::Vector2i offset = HexMap::axialToOffset(pos);
	hm->setEntity(offset, nullptr);
	pos = axialCoord;
	offset = HexMap::axialToOffset(pos);
	hm->setEntity(offset, this);
	for (int a = 0; a < 3; a++) {
		handlers_[a].setPosition((sf::Vector2f)hm->hexToPixel(pos, a) + HexMap::getMapOrigin(a));
	}
	return true;
}

MapEntityS::MapEntityS(string id) :
id_(id)
{
}

void MapEntityS::loadEntityJson(Json::Value& edata, string& element, string id)
{
	id_ = id;
	//name
	element = "name";
	name_ = edata.get(element, "").asString();
	// anims
	const sf::FloatRect* rectData = nullptr;
	for (int i = 0; i < ZOOM_LEVELS; i++) {
		element = config::rectNames[i];
		auto anims = edata.get(element, Json::Value::null);
		if (anims.begin() == anims.end()) {
			// Silently skip undefined animation, using default
			animData_[i] = &AnimationData::defaultAnim;
			continue;
		}
		element = "animFile";
		animData_[i] = RESOURCE.anim(anims.get("animFile", Json::Value::null).asString());
		if (animData_[i] == nullptr) {
			std::cerr << "\t(requested by \""<< element << "\" in \"" << id_ << "\")\n";
			continue;
		}
		for (int s = 0; s < ANIM_NUM; s++) {
			element = animTypes[s];
			const string* animName = &animTypes[s];
			animNames_[i][s] = anims.get(*animName, Json::Value::null).asString();
		}
	}
}

MapEntity::MapEntity(const MapEntityS* sEnt, HexMap* hmSet, Faction* parent)
{
	mes = sEnt;
	hm = hmSet;
	faction = parent;
	// Get animation data from associated MapEntityS
	for (int i = 0; i < ZOOM_LEVELS; i++) {
		handlers_[i].setAnimationData(*mes->animData_[i]);
	}
	// Create space for creature populations
	pops.reserve(Species::map.size());
	for (auto& s : Species::map) {
		pops.emplace_back(s.second);
	}
}

void MapEntity::setAnimationType(MapEntityS::anim num)
{	
	for (int i = 0; i < ZOOM_LEVELS; i++) {
		handlers_[i].setAnimation(mes->animNames_[i][num]);
		handlers_[i].randomFrame(rng::r);
	}
}


MapUnit::MapUnit(const MapEntityS* sEnt, HexMap* hmSet, Faction* parent) :moveTimer(0), MapEntity(sEnt, hmSet, parent){}

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
