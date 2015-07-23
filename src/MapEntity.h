#ifndef MAPENTITY_H
#define MAPENTITY_H

#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>
#include <array>
#include "Faction.h"
#include "Species.h"
#include "config.h"

class HexMap;

class Population
{
public:
	enum{ IDLE, GUARD, FARM, WOOD, MINE, PRISONER, ACTIVITY_NUM };
private:
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

class MapEntityS
{
public:
	enum anim{ IDLE, ANIM_NUM };
	static const array<string, ANIM_NUM> animTypes;
public:
	void loadEntityJson(Json::Value& edata, string& element, string id);
	MapEntityS();
	string id_;
	string name_;
	array<array<string, ANIM_NUM>, ZOOM_LEVELS> animNames_;
	array<const AnimationData*, ZOOM_LEVELS> animData_;
};

// Anything on a HexMap which has a hex position and is drawn along
// with the map is derived from this
class MapEntity
{
	const MapEntityS* mes;
protected:
	// Current position
	sf::Vector2i pos;
	array<AnimHandler, ZOOM_LEVELS> handlers_;
	HexMap* hm;
	Faction* faction;
public:
	MapEntity(const MapEntityS* sEnt, HexMap* hmSet, Faction* parent);
	void setAnimationType(MapEntityS::anim num);
	vector<Population> pops;
	bool initMapPos(sf::Vector2i axialCoord);
	bool setMapPos(sf::Vector2i axialCoord);
	virtual void update(const sf::Time& timeElapsed) = 0;
	friend HexMap;
};

// A map entity with added movement functions
class MapUnit : public MapEntity
{
	// The unit's current movement path
	std::deque<sf::Vector2i> path;
	int moveTimer;
public:
	MapUnit(const MapEntityS* sEnt, HexMap* hmSet, Faction* parent);
	bool walkPath();
	void setPath(sf::Vector2i dest);
	void appendPath(sf::Vector2i dest);
	void update(const sf::Time& timeElapsed);
};

#endif