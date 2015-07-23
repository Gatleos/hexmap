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
	for (string eName : elementNames) {
		SiteS& site = sites.insert(make_pair(eName, SiteS())).first->second;
		Json::Value sdata = root.get(eName, Json::Value::null);
		string element;
		try {
			site.loadEntityJson(sdata, element, eName);
		}
		catch (runtime_error e) { // report the error with the name of the object and member
			cerr << "[" << filename << ", " << eName << ", " << element << "] " << e.what() << "\n";
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