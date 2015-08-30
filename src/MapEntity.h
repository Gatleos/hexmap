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
		PR_FARM = 1, PR_WOOD, PR_MINE, PR_BREED, PR_ACTIVITY_NUM
	};
private:
	float size_;
	array<float, GROUP_NUM> sizes_;
	array<vector<float>, GROUP_NUM> activities_;
public:
	static const array<vector<std::string>, GROUP_NUM> activityNames;
	static const array<std::string, GROUP_NUM> groupNames;
	static const unsigned int POP_LIMIT;
	static const array<float, GROUP_NUM> growthRate;
	static const float deathRate;
	Population();
	// Set the amount for a specific activity group, drawing from idle amount; returns
	// actual amount after adjustment
	float set(unsigned int group, unsigned int activity, float amount);
	void setSize(unsigned int group, float size);
	void addSize(unsigned int group, float amount);
	void clear();
	const array<vector<float>, GROUP_NUM>& activities() const;
	float size() const;
	float size(unsigned int group) const;
	void popGrowth(int turns);
};

class MapEntityS
{
public:
	enum anim{ IDLE, ANIM_NUM };
	static const array<string, ANIM_NUM> animTypes;
public:
	enum res {
		FOOD, WOOD, ORE, RESOURCE_NUM
	};
	static const array<std::string, RESOURCE_NUM> resourceNames;
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
	array<float, MapEntityS::RESOURCE_NUM> resources;
	bool initMapPos(sf::Vector2i axialCoord);
	bool setMapPos(sf::Vector2i axialCoord);
	const sf::Vector2i& getMapPos();
	const MapEntityS* sMapEntity();
	virtual void update(const sf::Time& timeElapsed) = 0;
	// Runs once per map turn
	virtual void advanceTurn() = 0;
	void updateResources();
	friend HexMap;
};

#endif