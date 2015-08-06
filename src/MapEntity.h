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
	enum {
		GROUP_CIV, GROUP_MIL, GROUP_PR, GROUP_NUM,
		IDLE = 0, CIV_FARM, CIV_WOOD, CIV_MINE, CIV_ENLIST, CIV_ACTIVITY_NUM,
		MIL_GUARD = 1, MIL_ACTIVITY_NUM,
		PR_FARM = 1, PR_WOOD, PR_MINE, PR_ACTIVITY_NUM
	};
private:
	unsigned int size_;
	array<unsigned int, GROUP_NUM> sizes_;
	array<vector<float>, GROUP_NUM> activities_;
public:
	static array<vector<const char*>, GROUP_NUM> activityNames;
	static array<const char*, GROUP_NUM> groupNames;
	static const unsigned int POP_LIMIT;
	Population();
	// Set the amount for a specific activity group, drawing from idle amount; returns
	// actual amount after adjustment
	float set(unsigned int group, unsigned int activity, float amount);
	void setSize(unsigned int group, unsigned int size);
	void clear();
	const array<vector<float>, GROUP_NUM>& activities() const;
	unsigned int size() const;
	unsigned int size(unsigned int group) const;
};

class MapEntityS
{
public:
	enum anim{ IDLE, ANIM_NUM };
	static const array<string, ANIM_NUM> animTypes;
public:
	void loadEntityJson(Json::Value& edata, string& element, string id);
	MapEntityS(string id);
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
	int id;
	MapEntity(const MapEntityS* sEnt, HexMap* hmSet, Faction* parent);
	void setAnimationType(MapEntityS::anim num);
	Population pop;
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