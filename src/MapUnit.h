#ifndef MAP_UNIT_H
#define MAP_UNIT_H

#include "MapEntity.h"

// A map entity with added movement functions
class MapUnit : public MapEntity {
	// The unit's current movement path
	std::deque<sf::Vector2i> path;
	int moveTimer;
public:
	MapUnit(const MapEntityS* sEnt, HexMap* hmSet, Faction* parent);
	bool walkPath();
	void setPath(sf::Vector2i dest);
	void appendPath(sf::Vector2i dest);
	void update(const sf::Time& timeElapsed);
	void advanceTurn();
};

#endif