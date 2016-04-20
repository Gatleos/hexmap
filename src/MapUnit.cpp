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
	tasks.emplace_back(AI_IDLE, nullptr);
	pathLine.setPrimitiveType(sf::PrimitiveType::LinesStrip);
	blockedLastTurn = false;
}

void MapUnit::setStaticUnit(const MapUnitS* sUnit) {
	su = sUnit;
	setStaticEntity(sUnit);
	setHealth(getHealth());
}

bool MapUnit::walkPath() {
	if (path.empty()) {
		hm->getAxial(pos.x, pos.y).FLAGS[HexTile::OCCUPIED_NEXT_TURN] = true;
		return false;
	}
	if (hm->moveCost(pos, *path.begin()) >= HexMap::BIG_COST) { // impassable
		if (blockedLastTurn) {
			recalcPath();
		}
		blockedLastTurn = !blockedLastTurn;
		if (path.empty()) {
			return false;
		}
	}
	auto& curHex = hm->getAxial(pos.x, pos.y);
	auto& nextHex = hm->getAxial(path.begin()->x, path.begin()->y);
	if (nextHex.ent == nullptr || nextHex.ent->isInMotion() && !nextHex.ent->hasActed()) {
		// move
		setMapPos(*path.begin());
		path.erase(path.begin());
		// set occupied flags
		if (!path.empty()) {
			curHex.FLAGS[HexTile::OCCUPIED_NEXT_TURN] = false;
			nextHex.FLAGS[HexTile::OCCUPIED_NEXT_TURN] = true;
		}
	}
	else {
		// uh-oh, we need to roll back some moves
		if (curHex.ent != this) {
			curHex.ent->undoMapMove();
			curHex.ent = this;
		}
	}
	return true;
}

void MapUnit::setPath(sf::Vector2i dest) {
	path.clear();
	pathLine.clear();
	hm->getPath(path, pos, dest, true);
	for (auto p : path) {
		sf::Vector2f tilePos = (sf::Vector2f)hm->hexToPixel(p);
		pathLine.append(sf::Vertex(tilePos));
	}
}

void MapUnit::appendPath(sf::Vector2i dest) {
	hm->getPath(path, pos, dest);
}

void MapUnit::recalcPath() {
	if (path.empty()) {
		return;
	}
	setPath(path.back());
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

int MapUnit::getOldHealth() const {
	return hp.getOldHealth();
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

void MapUnit::pushTask(const task& t) {
	tasks.push_back(t);
	path.clear();
	switch (t.getType()) {
	case AI_MOVE:
		if (t.getEntity() == nullptr) {
			setPath(tasks.back().getMapPos());
		}
		else {
			setPath(tasks.back().getMapPos());
			//hm->getPath(path, getMapPos(), tasks.back().getMapPos(), true);
			if (!path.empty()) {
				path.pop_back();
			}
		}
		if (path.empty()) {
			tasks.pop_back();
		}
		break;
	case AI_ATTACK:
		if (HexMap::distAxial(pos, t.getMapPos()) > 1.0f) {
			pushTask(task(AI_MOVE, t.getEntity()));
		}
		break;
	}
}

void MapUnit::setAiType(const task& t) {
	aiType = t.getType();
	while (tasks.size() > 1) {
		tasks.pop_back();
	}
	switch(aiType) {
	default:
		pushTask(t);
	}
}

// unused
void MapUnit::update(const sf::Time& timeElapsed) {
	moveTimer += timeElapsed.asMilliseconds();
	if (moveTimer >= 500) {
		walkPath();
		moveTimer = moveTimer % 500;
	}
}

void MapUnit::takeDamage(double proportion) {
	proportion = clamp(proportion, 0.0, 1.0);
	int damage = hp.getHealth() * proportion;
	hp.setHealth(hp.getHealth() - damage);
}

int MapUnit::getAttackStrength() {
	int forceMult = 0;
	switch (su->type) {
	case MapUnitS::UNIT_CARAVAN:
		forceMult = 1;
		break;
	case MapUnitS::UNIT_ARMY:
		forceMult = 8;
		break;
	}
	return hp.getOldHealth() * forceMult;
}

int MapUnit::getDefenseStrength() {
	return getAttackStrength();
}

bool MapUnit::isInMotion() {
	return !path.empty();
}

void MapUnit::preTurn() {
	hp.updateOldHealth();
	switch (tasks.back().getType()) {
	case AI_MOVE:
		break;
	}
	acted = false;
}

void MapUnit::advanceTurn() {
	// update actions based on current ai state
	switch (tasks.back().getType()) {
	case AI_MOVE:
		if (!walkPath()) {
			// either we reached the destination, or there is no viable path
			tasks.pop_back();
		}
		break;
	case AI_ATTACK:
		if (HexMap::distAxial(pos, tasks.back().getMapPos()) > 1.0f) {
			pushTask(task(AI_MOVE, tasks.back().getEntity()));
		}
		else {
			attack(tasks.back().getEntity(), rng::r);
		}
		break;
	}
	hp.consumeFood();
	hp.updateBars();
	acted = true;
}

void MapUnit::postTurn() {
	if (hp.getHealth() <= 0) {
		// dead
	}
}

void MapUnit::select() {
	UIdef::MapUnitInfo::instance()->show(true);
	UIdef::setUnit(*this);
}

void MapUnit::deselect() {
	UIdef::MapUnitInfo::instance()->show(false);
}

void MapUnit::setGoal(sf::Vector2i dest) {
	MapEntity* ent = hm->getAxial(dest.x, dest.y).ent;
	if (ent == nullptr) {
		setAiType(task(AI_MOVE, dest));
	}
	else {
		setAiType(task(AI_ATTACK, ent));
	}
}

void MapUnit::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	states.transform *= this->getTransform();
	handlers_[zoomLevel].draw(target, states);
}

void MapUnit::drawSelectors(sf::RenderTarget& target, sf::RenderStates states) const {
	states.transform *= this->getTransform();
	UI::drawHexSelector((sf::Vector2f)pos, sf::Color::Red, target);
	if (!path.empty()) {
		UI::drawHexSelector((sf::Vector2f)path.back(), sf::Color::Green, target);
		target.draw(pathLine, states);
	}
}

void MapUnit::drawHUD(sf::RenderTarget& target, sf::RenderStates states) const {
	states.transform *= this->getTransform();
	states.transform *= handlers_[zoomLevel].getTransform();
	hp.draw(target, states);
}