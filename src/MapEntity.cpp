#include <iostream>
#include <math.h>
#include "ResourceLoader.h"
#include "Species.h"
#include "MapEntity.h"
#include "HexMap.h"
#include "clamp.h"
#include "config.h"

const array<string, MapEntityS::ANIM_NUM> MapEntityS::animTypes = { {
		"idle"
	} };
const array<vector<std::string>, Population::GROUP_NUM> Population::activityNames = { {
	{ "Idle", "Farm", "Wood", "Mine", "Enlist" },
	{ "Idle", "Guard" },
	{ "Idle", "Farm", "Wood", "Mine", "Breed" }
	} };
const array<std::string, Population::GROUP_NUM> Population::groupNames = { {
		"Civilian", "Military", "Prisoner"
	} };
const array<float, Population::GROUP_NUM> Population::growthRate = { {
		0.0168f, 0.0f, 0.0336f
	} };
const float Population::deathRate = 0.0003f;
const unsigned int Population::POP_LIMIT = 100000u;
//
const array<std::string, MapEntityS::RESOURCE_NUM> MapEntityS::resourceNames = { {
		"Food", "Wood", "Ore"
	} };

Population::Population() :size_(0)
{
	for (auto& s : sizes_) {
		s = 0;
	}
	array<vector<float>*, GROUP_NUM> groups = { 
		&activities_[GROUP_CIV], &activities_[GROUP_MIL], &activities_[GROUP_PR]
	};
	activities_[GROUP_CIV].resize(CIV_ACTIVITY_NUM);
	activities_[GROUP_MIL].resize(MIL_ACTIVITY_NUM);
	activities_[GROUP_PR].resize(PR_ACTIVITY_NUM);
	for (auto g : groups) {
		for (int a = 1; a < g->size(); a++) {
			(*g)[a] = 0.0f;
		}
		(*g)[IDLE] = 100.0f;
	}
}

float Population::set(unsigned int group, unsigned int activity, float amount)
{
	if (!isInRange(group, 0u, (unsigned int)GROUP_NUM) ||
		!isInRange(activity, 0u, (unsigned int)activities_[group].size() - 1)) {
		return -1.0f;
	}
	amount = clamp(amount, 0.0f, activities_[group][activity] + activities_[group][IDLE]);
	activities_[group][IDLE] -= amount - activities_[group][activity];
	activities_[group][activity] = amount;
	return amount;
}

void Population::setSize(unsigned int group, float size)
{
	size_ += size - sizes_[group];
	sizes_[group] = size;
}

void Population::clear()
{
	array<vector<float>*, GROUP_NUM> groups = {
		&activities_[GROUP_CIV], &activities_[GROUP_MIL], &activities_[GROUP_PR]
	};
	for (auto g : groups) {
		for (int a = 1; a < g->size(); a++) {
			(*g)[a] = 0.0f;
		}
		(*g)[IDLE] = 100.0f;
	}
	size_ = 0;
}

const array<vector<float>, Population::GROUP_NUM>& Population::activities() const
{
	return activities_;
}

float Population::size() const
{
	return size_;
}

float Population::size(unsigned int group) const
{
	return sizes_[group];
}

void Population::popGrowth(int turns)
{
	// P = P_0 * e ^ (r * t)
	setSize(GROUP_CIV, sizes_[GROUP_CIV] * std::powf(2.71828f, (float)turns * growthRate[GROUP_CIV]));
	// Prisoners will not breed unless told
	float rate = activities_[GROUP_PR][PR_BREED] * growthRate[GROUP_PR] * 0.01f - deathRate;
	setSize(GROUP_PR, sizes_[GROUP_PR] * std::powf(2.71828f, (float)turns * rate));
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

const sf::Vector2i& MapEntity::getMapPos()
{
	return pos;
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
	for (auto& r : resources) {
		r = 0.0f;
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

void MapUnit::advanceTurn()
{

}