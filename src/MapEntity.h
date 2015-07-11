#ifndef MAPENTITY_H
#define MAPENTITY_H

#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>
#include <array>
#include "AnimHandler.h"
#include "Faction.h"
#include "Species.h"

class HexMap;

enum{ POP_IDLE, POP_GUARD, POP_FARM, POP_WOOD, POP_MINE, POP_PRISONER, ACTIVITY_NUM };

class Population
{
	int size;
	array<int, ACTIVITY_NUM> activities;
public:
	static const int POP_LIMIT;
	const Species& species;
	Population(const Species& s);
	// Add an amount to a specific activity group
	void add(int activity, int amount);
	// Add another population to this one, keeping it the same; must have same species
	void addPop(const Population& p);
	// Empty other population into this one; must have same species
	void takePop(Population& p);
};

class MapEntity : public AnimHandler
{
	Faction* faction;
protected:
	// Current position
	sf::Vector2i pos;
	AnimHandler* anim;
	HexMap* hm;
public:
	vector<Population> pops;
	MapEntity(Faction* parent);
	void setParentMap(HexMap* hmSet);
	bool setMapPos(sf::Vector2i dest);
	virtual void update(const sf::Time& timeElapsed) = 0;
	bool isOnscreen(const sf::View& mapView);
};

class MapUnit : public MapEntity
{
public:
	// The unit's current movement path
	std::deque<sf::Vector2i> path;
	int moveTimer;
	MapUnit(Faction* parent);
	bool walkPath();
	void setPath(sf::Vector2i dest);
	void appendPath(sf::Vector2i dest);
	void update(const sf::Time& timeElapsed);
};

#endif