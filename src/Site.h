#ifndef SITE_H
#define SITE_H

#include "MapEntity.h"
#include "Species.h"
#include "MapUnit.h"

class SiteS : public MapEntityS {
public:
	enum {
		NONE, CITY, TOWN, VILLAGE, DUNGEON, SITE_NUM
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
	Site(const SiteS* sSite, HexMap* hmSet, Faction* parent);
	const SiteS* sSite();
	void update(const sf::Time& timeElapsed);
	virtual void preTurn();
	virtual void advanceTurn();
	virtual void postTurn();
	virtual void select();
	virtual void deselect();
	void setPath(sf::Vector2i dest);
	void addChild(Site* s);
	void removeChild(Site* s);
	void clearChildren(Site* s);
	void takeDamage(double proportion);
	int getAttackStrength();
	int getDefenseStrength();
	bool isInMotion();
	void setGoal(sf::Vector2i dest);
	void drawSelectors(sf::RenderTarget& target, sf::RenderStates states = sf::RenderStates::Default) const;
	virtual void drawHUD(sf::RenderTarget& target, sf::RenderStates states = sf::RenderStates::Default) const;
};

#endif