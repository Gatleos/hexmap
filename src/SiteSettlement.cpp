#include <assert.h>
#include "SiteSettlement.h"
#include "UIdef.h"

SiteSettlement::SiteSettlement(const SiteS* sSite, HexMap* hmSet, Faction* parent) :
Site(sSite, hmSet, parent) {
	for (auto& r : resources) {
		r = 0.0f;
	}
}

void SiteSettlement::advanceTurn() {
	Site::advanceTurn();
	pop.popGrowth(1);
}

void SiteSettlement::deployUnit(const MapUnit& u) {
	// there's a problem if we're trying to create a group larger than our total population
	assert(resources[MapEntityS::FOOD] >= u.getFood());
	assert(pop.size(u.getMemberType()) >= u.getHealth());
	// subtract it all out
	resources[MapEntityS::FOOD] -= u.getFood();
	pop.addSize(u.getMemberType(), -u.getHealth());
}

void SiteSettlement::updateResources() {
	resources[MapEntityS::WOOD] += pop.activities()[Population::GROUP_CIV][Population::CIV_WOOD] * pop.size(Population::GROUP_CIV);
}

void SiteSettlement::select() {
	UIdef::SiteMenu::instance()->show(true);
	UIdef::setSettlement(*this);
}

void SiteSettlement::deselect() {
	UIdef::SiteMenu::instance()->show(false);
}