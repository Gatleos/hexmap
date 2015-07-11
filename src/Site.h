#ifndef SITE_H
#define SITE_H

#include "MapEntity.h"
#include "Species.h"



class Site : public MapEntity
{
	char type;
	string name;
public:
	Site(Faction* parent);
	void update(const sf::Time& timeElapsed);
};

#endif