#include <iostream>
#include <assert.h>
#include "Site.h"
#include "config.h"
#include "ResourceLoader.h"
#include "UIdef.h"
#include "HexMap.h"

using namespace std;

#define siteptr(x) unique_ptr<SiteS>(new SiteS(x))

array<unique_ptr<SiteS>, SiteS::SITE_NUM> SiteS::site = { {
		siteptr("si_null"), siteptr("si_city"), siteptr("si_town"), siteptr("si_village"), siteptr("si_dungeon")
	} };

Site::Site(const SiteS* sSite, HexMap* hmSet, Faction* parent) : MapEntity(sSite, hmSet, parent), ss(sSite), parent(nullptr) {
}

const SiteS* Site::sSite() {
	return ss;
}

void Site::place(sf::Vector2i axialCoord) {

}

void Site::update(const sf::Time& timeElapsed) {

}

void Site::preTurn() {
	acted = false;
}

void Site::advanceTurn() {
	acted = true;
}

void Site::postTurn() {

}

void Site::select() {
}

void Site::deselect() {
}

void Site::setPath(sf::Vector2i dest) {

}

void Site::addChild(Site* s) {
	children.push_back(s);
	s->parent = this;
}

void Site::removeChild(Site* s) {
	for (auto it = children.begin(); it != children.end(); it++) {
		if (*it == s) {
			s->parent = nullptr;
			children.erase(it);
			return;
		}
	}
}

void Site::clearChildren(Site* s) {
	for (auto ptr : children) {
		ptr->parent = nullptr;
	}
	children.clear();
}

void Site::takeDamage(double proportion) {
}

int Site::getAttackStrength() {
	return 1;
}

int Site::getDefenseStrength() {
	return 1;
}

bool Site::isInMotion() {
	return false;
}

void Site::setGoal(sf::Vector2i dest) {

}

void Site::drawSelectors(sf::RenderTarget& target, sf::RenderStates states) const {
	UI::drawHexSelector((sf::Vector2f)pos, sf::Color::Red, target);
}

void Site::drawHUD(sf::RenderTarget& target, sf::RenderStates states) const {
}

SiteS::SiteS(string id) :
MapEntityS(id) {
}

void SiteS::loadJson(string filename) {
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

const SiteS& SiteS::get(int id) {
	return *site[id];
}