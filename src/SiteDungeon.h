#ifndef SITE_DUNGEON_H
#define SITE_DUNGEON_H

#include "Site.h"

class SiteDungeon : public Site {
	//
	void place(sf::Vector2i axialCoord);
public:
	unsigned char targetDanger;
	float popularity;
	unsigned char danger;
	int gold;
	int wins;
	int losses;
	SiteDungeon(const SiteS* sSite, HexMap* hmSet, Faction* parent);
	void select();
	void deselect();
	void advanceTurn();
};

#endif