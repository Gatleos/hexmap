#ifndef SITE_H
#define SITE_H

#include "MapEntity.h"
#include "Species.h"

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
	Site(const SiteS* sSite, HexMap* hmSet, Faction* parent);
	const SiteS* sSite();
	void update(const sf::Time& timeElapsed);
	void advanceTurn();
	void addChild(Site* s);
	void removeChild(Site* s);
	void clearChildren(Site* s);
};

#endif