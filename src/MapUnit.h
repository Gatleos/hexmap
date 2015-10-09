#ifndef MAP_UNIT_H
#define MAP_UNIT_H

#include "MapEntity.h"
#include "HealthBar.h"

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
	static void loadJson(std::string filename);
	static const MapUnitS& get(int id);
};

// A map entity with added movement functions
class MapUnit : public MapEntity {
	const MapUnitS* su;
	// The unit's current movement path
	std::deque<sf::Vector2i> path;
	int moveTimer;
	HealthBar hp;
	int hitPoints;
	int rations;
public:
	MapUnit(const MapUnitS* sUnit, HexMap* hmSet, Faction* parent);
	void setStaticUnit(const MapUnitS* sUnit);
	bool walkPath();
	void setPath(sf::Vector2i dest);
	void setHealth(int health);
	int getHealth();
	void appendPath(sf::Vector2i dest);
	void update(const sf::Time& timeElapsed);
	void advanceTurn();
	void select();
	void deselect();
	void move();
	void draw(sf::RenderTarget& target, sf::RenderStates states = sf::RenderStates::Default) const;
};

#endif