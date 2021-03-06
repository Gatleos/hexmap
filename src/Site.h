#ifndef SITE_H
#define SITE_H

#include "MapEntity.h"
#include "Species.h"

class SiteS : public MapEntityS
{
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

class Site : public MapEntity
{
public:
	const SiteS* ss;
	Site(const SiteS* sSite, HexMap* hmSet, Faction* parent);
	void update(const sf::Time& timeElapsed);
};

#endif