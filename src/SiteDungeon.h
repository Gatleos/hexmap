#include "Site.h"

class SiteDungeon : public Site {
	unsigned char danger;
	unsigned char targetDanger;
	char popularity;
	int gold;
	//
public:
	SiteDungeon(const SiteS* sSite, HexMap* hmSet, Faction* parent);
};