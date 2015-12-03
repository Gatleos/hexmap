#ifndef SITE_H
#define SITE_H

#include "MapEntity.h"
#include "Species.h"
#include "MapUnit.h"

class SiteS : public MapEntityS {
public:
	enum {
		NONE, CITY, TOWN, VILLAGE, SITE_NUM
	};
private:
	SiteS(string id);
	static std::array<std::unique_ptr<SiteS>, SITE_NUM> site;
public:
	static void loadJson(string filename);
	static const SiteS& get(int id);
};

class Site : public MapEntity {
	const SiteS* ss;
	Site* parent;
	std::vector<Site*> children;
public:
	Population pop;
	array<float, MapEntityS::RESOURCE_NUM> resources;
	Site(const SiteS* sSite, HexMap* hmSet, Faction* parent);
	const SiteS* sSite();
	void update(const sf::Time& timeElapsed);
	void preTurn();
	void advanceTurn();
	void postTurn();
	void select();
	void deselect();
	void setPath(sf::Vector2i dest);
	void addChild(Site* s);
	void removeChild(Site* s);
	void clearChildren(Site* s);
	void deployUnit(const MapUnit& u);
	void updateResources();
	void takeDamage(double proportion);
	int getAttackStrength();
	int getDefenseStrength();
	void setGoal(sf::Vector2i dest);
	void drawSelectors(sf::RenderTarget& target, sf::RenderStates states = sf::RenderStates::Default) const;
	void drawHUD(sf::RenderTarget& target, sf::RenderStates states = sf::RenderStates::Default) const;
};

#endif