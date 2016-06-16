#ifndef MAPENTITY_H
#define MAPENTITY_H

#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>
#include <array>
#include "Faction.h"
#include "Species.h"
#include "config.h"

class HexMap;

class Population {
public:
	struct GroupType {
		std::string id;
		std::string name;
		float growthRate;
	};
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
	static const array<GroupType, GROUP_NUM> groups;
	static const unsigned int POP_LIMIT;
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

class MapEntityS {
public:
	enum anim{ IDLE, WOUND1, WOUND2, WOUND3, WOUND4, WOUND5, ANIM_NUM };
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
class MapEntity : public sf::Drawable, public sf::Transformable {
	const MapEntityS* mes;
protected:
	static int zoomLevel;
	// Current position
	sf::Vector2i pos;
	sf::Vector2i lastPos;
	array<AnimHandler, ZOOM_LEVELS> handlers_;
	HexMap* hm;
	Faction* faction;
	bool acted;
	// called whenever the entity changes position
	virtual void place(sf::Vector2i axialCoord) = 0;
public:
	static void setZoomLevel(int zoom);
	static int getZoomLevel();
	int id;
	MapEntity(const MapEntityS* sEnt, HexMap* hmSet, Faction* parent);
	void setFaction(Faction* fac);
	Faction* getFaction();
	void setStaticEntity(const MapEntityS* sEnt);
	void setAnimationType(MapEntityS::anim num);
	bool initMapPos(sf::Vector2i axialCoord);
	bool setMapPos(sf::Vector2i axialCoord, bool clearPrevious = true);
	void undoMapMove();
	const sf::Vector2i& getMapPos();
	const MapEntityS* sMapEntity();
	virtual void update(const sf::Time& timeElapsed) = 0;
	void updateAnimation(const sf::Time& timeElapsed);
	virtual bool isInMotion() = 0;
	// Has it acted yet this turn?
	bool hasActed();
	// Actions to be taken before any units act
	virtual void preTurn() = 0;
	// Runs once per map turn
	virtual void advanceTurn() = 0;
	// Actions to be taken after all units act
	virtual void postTurn() = 0;
	virtual int getAttackStrength() = 0;
	virtual int getDefenseStrength() = 0;
	virtual void attack(MapEntity* ent, mt19937& urng);
	virtual void takeDamage(double proportion) = 0;
	virtual void select() = 0;
	virtual void deselect() = 0;
	virtual void setPath(sf::Vector2i dest) = 0;
	virtual void setGoal(sf::Vector2i dest) = 0;
	// Draw the entity itself
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states = sf::RenderStates::Default) const;
	// Draw selection indicators on the map which are relevant to this entity
	virtual void drawSelectors(sf::RenderTarget& target, sf::RenderStates states = sf::RenderStates::Default) const;
	// Draw the entity's HUD
	virtual void drawHUD(sf::RenderTarget& target, sf::RenderStates states = sf::RenderStates::Default) const;
};

#endif