#ifndef MAP_UNIT_H
#define MAP_UNIT_H

#include "MapEntity.h"

class MapUnitS : public MapEntityS {
public:
	enum {
		NONE, ARMY, UNIT_NUM
	};
private:
	MapUnitS(std::string id);
	static std::array<std::unique_ptr<MapUnitS>, UNIT_NUM> unit;
public:
	static void loadJson(std::string filename);
	static const MapUnitS& get(int id);
};

// A map entity with added movement functions
class MapUnit : public MapEntity {
	// The unit's current movement path
	std::deque<sf::Vector2i> path;
	int moveTimer;
public:
	MapUnit(const MapUnitS* sUnit, HexMap* hmSet, Faction* parent);
	bool walkPath();
	void setPath(sf::Vector2i dest);
	void appendPath(sf::Vector2i dest);
	void update(const sf::Time& timeElapsed);
	void advanceTurn();
	void select();
	void deselect();
	void move();
};

#endif