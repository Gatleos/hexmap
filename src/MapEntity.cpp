#include <iostream>
#include <math.h>
#include "ResourceLoader.h"
#include "Species.h"
#include "MapEntity.h"
#include "HexMap.h"
#include "clamp.h"
#include "config.h"
#include <assert.h>

const array<string, MapEntityS::ANIM_NUM> MapEntityS::animTypes = { {
		"idle", "wound1", "wound2", "wound3", "wound4", "wound5"
	} };
const array<vector<std::string>, Population::GROUP_NUM> Population::activityNames = { { { "Idle", "Farm", "Wood", "Mine", "Enlist" }, { "Idle", "Guard" }, { "Idle", "Farm", "Wood", "Mine", "Breed" }
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

Population::Population() :size_(0) {
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

float Population::set(unsigned int group, unsigned int activity, float amount) {
	if (!isInRange(group, 0u, (unsigned int)GROUP_NUM) ||
		!isInRange(activity, 0u, (unsigned int)activities_[group].size() - 1)) {
		return -1.0f;
	}
	amount = clamp(amount, 0.0f, activities_[group][activity] + activities_[group][IDLE]);
	activities_[group][IDLE] -= amount - activities_[group][activity];
	activities_[group][activity] = amount;
	return amount;
}

void Population::setSize(unsigned int group, float size) {
	size_ += size - sizes_[group];
	sizes_[group] = size;
}

void Population::addSize(unsigned int group, float amount) {
	if (amount < 0.0f) {
		amount = max(amount, -sizes_[group]);
	}
	size_ += amount;
	sizes_[group] += amount;
}

void Population::clear() {
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

const array<vector<float>, Population::GROUP_NUM>& Population::activities() const {
	return activities_;
}

float Population::size() const {
	return size_;
}

float Population::size(unsigned int group) const {
	return sizes_[group];
}

void Population::popGrowth(int turns) {
	// P = P_0 * e ^ (r * t)
	setSize(GROUP_CIV, sizes_[GROUP_CIV] * std::pow(2.71828f, (float)turns * growthRate[GROUP_CIV]));
	// Prisoners will not breed unless told
	float rate = activities_[GROUP_PR][PR_BREED] * growthRate[GROUP_PR] * 0.01f - deathRate;
	setSize(GROUP_PR, sizes_[GROUP_PR] * std::pow(2.71828f, (float)turns * rate));
}

bool MapEntity::initMapPos(sf::Vector2i axialCoord) {
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

bool MapEntity::setMapPos(sf::Vector2i axialCoord) {
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

const sf::Vector2i& MapEntity::getMapPos() {
	return pos;
}

const MapEntityS* MapEntity::sMapEntity() {
	return mes;
}

void MapEntity::updateAnimation(const sf::Time& timeElapsed) {
	handlers_[zoomLevel].updateAnimation(timeElapsed);
}

void MapEntity::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	states.transform *= this->getTransform();
	handlers_[zoomLevel].draw(target, states);
}




MapEntityS::MapEntityS(string id) :
id_(id) {
	for (auto& ad : animData_) {
		ad = &AnimationData::defaultAnim;
	}
}

void MapEntityS::loadEntityJson(Json::Value& edata, string& element, string id) {
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
	// Make sure somebody messing with the animation type defs
	// doesn't RUIN EVERYTHING
	assert(anim::WOUND1 == anim::WOUND5 - 4);
	// More advanced wound animations fall back to previous ones
	// if not specified (idle<--wound1<--wound2...)
	for (int i = 0; i < ZOOM_LEVELS; i++) {
		for (int a = anim::WOUND1; a <= anim::WOUND5; a++) {
			if (animNames_[i][a] == "") {
				animNames_[i][a] = animNames_[i][a - 1];
			}
		}
	}
}

MapEntity::MapEntity(const MapEntityS* sEnt, HexMap* hmSet, Faction* parent) {
	hm = hmSet;
	faction = parent;
	setStaticEntity(sEnt);
}

void MapEntity::setStaticEntity(const MapEntityS* sEnt) {
	mes = sEnt;
	// Get animation data from associated MapEntityS
	for (int i = 0; i < ZOOM_LEVELS; i++) {
		handlers_[i].setAnimationData(*mes->animData_[i]);
	}
}

void MapEntity::setAnimationType(MapEntityS::anim num) {	
	for (int i = 0; i < ZOOM_LEVELS; i++) {
		handlers_[i].setAnimation(mes->animNames_[i][num]);
		handlers_[i].randomFrame(rng::r);
	}
}

int MapEntity::zoomLevel = 0;

void MapEntity::setZoomLevel(int zoom) {
	zoomLevel = zoom;
}

int MapEntity::getZoomLevel() {
	return zoomLevel;
}
