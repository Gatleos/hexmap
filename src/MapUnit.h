#ifndef MAP_UNIT_H
#define MAP_UNIT_H

#include "MapEntity.h"
#include "HealthBar.h"
#include "UI.h"

class MapUnitS : public MapEntityS {
public:
	enum {
		NONE, UNIT_ARMY, UNIT_CARAVAN, UNIT_NUM
	};
private:
	MapUnitS(std::string id);
	static std::array<std::unique_ptr<MapUnitS>, UNIT_NUM> unit;
public:
	int maxHitPoints;
	int type;
	static void loadJson(std::string filename);
	static const MapUnitS& get(int id);
};

// A map entity with added movement functions
class MapUnit : public MapEntity {
public:
	enum {
		AI_IDLE, AI_MOVE, AI_ATTACK, AI_GUARD
	};
	class task {
		int t;
		sf::Vector2i position;
		MapEntity* ent;
	public:
		task(const task& ts) :
			t(ts.t), position(ts.position), ent(ts.ent) {}
		task(int t, sf::Vector2i position) :
			t(t), position(position), ent(nullptr) {}
		task(int t, MapEntity* ent) :
			t(t), position(UI::invalidHex), ent(ent) {}
		const sf::Vector2i& getMapPos() const {
			if (ent == nullptr) {
				return position;
			}
			else {
				return ent->getMapPos();
			}
		}
		int getType() const {
			return t;
		}
	};
	std::deque<task> tasks;
private:
	const MapUnitS* su;
	// The unit's current movement path
	std::deque<sf::Vector2i> path;
	int aiType;
	int moveTimer;
	HealthBar hp;
	void pushTask(const task& t);
public:
	MapUnit(const MapUnitS* sUnit, HexMap* hmSet, Faction* parent);
	void setStaticUnit(const MapUnitS* sUnit);
	bool walkPath();
	void setPath(sf::Vector2i dest);
	void setHealth(int health);
	int getHealth() const;
	void setFood(int foodAmount);
	int getFood() const;
	int getMemberType() const;
	void setAiType(const task& t);
	void appendPath(sf::Vector2i dest);
	void update(const sf::Time& timeElapsed);
	void advanceTurn();
	void select();
	void deselect();
	void setGoal(sf::Vector2i dest);
	void draw(sf::RenderTarget& target, sf::RenderStates states = sf::RenderStates::Default) const;
	void drawSelectors(sf::RenderTarget& target, sf::RenderStates states = sf::RenderStates::Default) const;
	void drawHUD(sf::RenderTarget& target, sf::RenderStates states = sf::RenderStates::Default) const;
};

#endif