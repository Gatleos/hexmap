#ifndef SETTLEMENT_H
#define SETTLEMENT_H

#include "Site.h"

class SiteSettlement : public Site {
public:
	Population pop;
	array<float, MapEntityS::RESOURCE_NUM> resources;
	//
	SiteSettlement(const SiteS* sSite, HexMap* hmSet, Faction* parent);
	virtual void advanceTurn();
	void deployUnit(const MapUnit& u);
	void updateResources();
	void select();
	void deselect();
};

#endif