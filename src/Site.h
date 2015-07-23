#ifndef SITE_H
#define SITE_H

#include "MapEntity.h"
#include "Species.h"

class SiteS : public MapEntityS
{
	static map<string, SiteS> sites;
public:
	static void loadJson(string filename);
	static const SiteS* get(string id);
};

class Site : public MapEntity
{
public:
	const SiteS* ss;
	Site(const SiteS* sSite, HexMap* hmSet, Faction* parent);
	void update(const sf::Time& timeElapsed);
};

#endif