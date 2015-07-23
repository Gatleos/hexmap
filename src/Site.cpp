#include <iostream>
#include "Site.h"
#include "config.h"
#include "ResourceLoader.h"

using namespace std;

map<string, SiteS> SiteS::sites;

Site::Site(const SiteS* sSite, HexMap* hmSet, Faction* parent) :MapEntity(sSite, hmSet, parent), ss(sSite){}

void Site::update(const sf::Time& timeElapsed)
{

}

void SiteS::loadJson(string filename)
{
	Json::Value root = config::openJson(filename);
	if (root.begin() == root.end()) {
		cerr << "ERROR: couldn't open file \"" << filename << "\"\n";
		return;
	}
	auto elementNames = root.getMemberNames();
	for (string e : elementNames) {
		SiteS& site = sites.insert(make_pair(e, SiteS())).first->second;
		Json::Value sdata = root.get(e, Json::Value::null);
		string element;
		try {
			site.loadEntityJson(sdata, element, e);
		}
		catch (runtime_error e) { // report the error with the name of the object and member
			cerr << "[" << filename << ", " << site.id_ << ", " << element << "] " << e.what() << "\n";
		}
	}
}

const SiteS* SiteS::get(string id)
{
	auto s = sites.find(id);
	if (s == sites.end()) {
		return nullptr;
	}
	return &s->second;
}