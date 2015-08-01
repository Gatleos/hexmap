#include <iostream>
#include "Site.h"
#include "config.h"
#include "ResourceLoader.h"

using namespace std;

#define siteptr(x) unique_ptr<SiteS>(new SiteS(x))

array<unique_ptr<SiteS>, SiteS::SITE_NUM> SiteS::site = { {
		siteptr("si_null"), siteptr("si_city"), siteptr("si_town"), siteptr("si_village")
	} };

Site::Site(const SiteS* sSite, HexMap* hmSet, Faction* parent) :MapEntity(sSite, hmSet, parent), ss(sSite){}

void Site::update(const sf::Time& timeElapsed)
{

}

SiteS::SiteS(string id) :
MapEntityS(id)
{
}

void SiteS::loadJson(string filename)
{
	Json::Value root = config::openJson(filename);
	if (root.begin() == root.end()) {
		cerr << "ERROR: couldn't open file \"" << filename << "\"\n";
		return;
	}
	// Cycle through defined site types; make sure to skip si_null!
	for (int a = 1; a < SITE_NUM; a++) {
		auto& si = site[a];
		Json::Value sdata = root.get(si->id_, Json::Value::null);
		string element;
		try {
			si->loadEntityJson(sdata, element, si->id_);
		}
		catch (runtime_error e) { // report the error with the name of the object and member
			cerr << "[" << filename << ", " << si->id_ << ", " << element << "] " << e.what() << "\n";
		}
	}
}

const SiteS& SiteS::get(int id)
{
	return *site[id];
}