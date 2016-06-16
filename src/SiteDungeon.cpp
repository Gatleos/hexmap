#include "UIdef.h"
#include "SiteDungeon.h"
#include "HexMap.h"
#include "clamp.h"

#define PI 3.14159265359

double interpolateCosine(double a, double b, double mu) {
	double mu2;
	mu2 = (1.0 - cos(mu * PI)) / 2.0;
	return (a * (1.0 - mu2) + b * mu2);
}

SiteDungeon::SiteDungeon(const SiteS* sSite, HexMap* hmSet, Faction* parent) :
Site(sSite, hmSet, parent) {
	targetDanger = 100;
	danger = 40;
	wins = losses = 0;
	gold = 20;
	popularity = 100.0f;
}

void SiteDungeon::place(sf::Vector2i axialCoord) {
	// TODO: query nearby settlements to determine average adventurer skill
	// and set target danger level
}

void SiteDungeon::select() {
	UIdef::setDungeon(*this);
	UIdef::DungeonMenu::instance()->show(true);
}

void SiteDungeon::deselect() {
	UIdef::DungeonMenu::instance()->show(false);
}

void SiteDungeon::advanceTurn() {
	Site::advanceTurn();

	// standard deviation of party skill levels
	static const double skillSigma = 5.0;
	// negative relative skill amount required to reach a ~0% success rate (from ~10%)
	static const double lowerSkillMargin = 35.0;
	// positive relative skill amount required to reach a ~100% success rate (from ~10%)
	static const double upperSkillMargin = 75.0;
	// how much gold does an adventurer party carry per skill level?
	static const double goldFactor = 20.0;
	// how many times larger is the jackpot vs adventurer gold?
	static const double jackpotFactor = 3.0;
	// amount of popularity lost when adventurers beat the dungeon but the gold stocks are empty
	static const float noGoldPopLoss = 20.0f;

	if (!rng::xInY(popularity, 100.0, rng::r)) {
		// no adventurers visited this turn
		return;
	}
	std::normal_distribution<double> skillRange(targetDanger, skillSigma);
	double partySkill = skillRange(rng::r);
	int partyGold = partySkill * goldFactor;
	partyGold = 1;
	double relativeSkill = partySkill - danger;
	double successChance = 0.0;
	if (relativeSkill >= 0.0) {
		successChance = interpolateCosine(10.0, 100.0, clamp(relativeSkill / upperSkillMargin, 0.0, 1.0));
	}
	else {
		successChance = interpolateCosine(10.0, 0.0, clamp(-relativeSkill / lowerSkillMargin, 0.0, 1.0));
	}
	// use our calculated success chance
	if (rng::xInY(successChance, 100.0, rng::r)) {
		// adventurers win!
		float jackpot = partyGold * jackpotFactor;
		if (jackpot > gold) {
			// we don't have enough gold to pay the jackpot, lose popularity
			float popLoss = gold / jackpot;
			popularity = clamp(popularity - (noGoldPopLoss - (popLoss * noGoldPopLoss)), 0.0f, 100.0f);
		}
		gold = max(0, gold - (int)round(partyGold * jackpotFactor));
		wins++;
	}
	else {
		// adventurers lose!
		gold += partyGold;
		losses++;
	}
	// dungeon gains/loses popularity based on how easy it is to "win"
	popularity = clamp(popularity + ((float)successChance - 9.0f) * 0.2f, 0.0f, 100.0f);

	UIdef::DungeonMenu::instance()->updateDungeonInfo();
}