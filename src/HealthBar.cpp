#include <iostream>
#include "clamp.h"
#include "HealthBar.h"
#include "ResourceLoader.h"
#include "config.h"

const std::array<int, HEALTH_TIER_NUM> HealthBar::tierValues = { 0, 100, 500, 1500, 3000, 6000, 10000 };
const std::array<sf::Color, HEALTH_TIER_NUM> HealthBar::tierColors = { sf::Color(0, 0, 0), sf::Color(255, 0, 0), sf::Color(255, 127, 39),
sf::Color(252, 241, 41), sf::Color(64, 251, 43), sf::Color(43, 163, 251), sf::Color(136, 45, 249) };

sf::Vector2f HealthBar::barPos;
sf::Vector2f HealthBar::barSize;
int HealthBar::barPadding;

SpriteSheet* HealthBar::sheet = nullptr;
const sf::Texture* HealthBar::tex = nullptr;
const sf::FloatRect* HealthBar::fullRect = nullptr;
const sf::FloatRect* HealthBar::halfRect = nullptr;
const sf::FloatRect* HealthBar::emptyRect = nullptr;
int HealthBar::orbAmount = 0;
sf::VertexArray HealthBar::foodOrbsDefault;

const sf::FloatRect* HealthBar::frameRect;
sf::Sprite HealthBar::frameSprite;

void HealthBar::loadJson(Json::Value& root) {
	string spriteSheet = root.get("spriteSheet", "").asString();
	sheet = RESOURCE.sh(spriteSheet);
	if (sheet == nullptr) {
		std::cerr << "\t(requested by \"unitStatus\")\n";
		return;
	}
	tex = RESOURCE.tex(sheet->getImageName());
	if (tex == nullptr) {
		std::cerr << "\t(requested by \"ui/unitStatus\")\n";
		return;
	}
	try {
		// Unit Status
		Json::Value uidata = root.get("unitStatus", Json::Value::null);
		if (uidata.isNull()) {

		}
		else {
			Json::Value sdata = uidata.get("healthBar", Json::Value::null);
			if (sdata.isNull()) {

			}
			else {
				// Load health bar template data
				Json::Value arr = sdata.get("barPos", Json::Value::null);
				barPos.x = arr[0].asInt(); barPos.y = arr[1].asInt();
				arr = sdata.get("barSize", Json::Value::null);
				barSize.x = arr[0].asInt(); barSize.y = arr[1].asInt();
				barPadding = sdata.get("barPadding", Json::Value::null).asInt();
			}
			sdata = uidata.get("foodOrbs", Json::Value::null);
			if (sdata.isNull()) {

			}
			else {
				// Load food orb template data
				fullRect = sheet->spr(sdata.get("fullSprite", Json::Value::null).asString());
				halfRect = sheet->spr(sdata.get("halfSprite", Json::Value::null).asString());
				emptyRect = sheet->spr(sdata.get("emptySprite", Json::Value::null).asString());
				orbAmount = sdata.get("amount", Json::Value::null).asInt();
				// Construct the vertex array
				foodOrbsDefault.setPrimitiveType(sf::PrimitiveType::Quads);
				foodOrbsDefault.resize((size_t)(orbAmount * 4));
				float advance = std::round((barSize.x - emptyRect->width) / std::max(orbAmount - 1, 1));
				sf::FloatRect posRect;
				posRect.left = barPos.x;
				posRect.top = barPos.y + barSize.y + barPadding;
				posRect.width = emptyRect->width;
				posRect.height = emptyRect->height;
				for (int o = 0; o < orbAmount; o++) {
					int indexOffset = o * 4;
					setQuad(foodOrbsDefault, indexOffset, posRect, *emptyRect);
					posRect.left += advance;
				}
			}
			sdata = uidata.get("frame", Json::Value::null);
			if (sdata.isNull()) {

			}
			else {
				// frame sprite
				frameRect = sheet->spr(sdata.get("sprite", Json::Value::null).asString());
				if (frameRect != nullptr) {
					frameSprite.setTextureRect((sf::IntRect)*frameRect);
				}
				frameSprite.setTexture(*tex);
				// frame offset
				Json::Value offsetArr = sdata.get("framePos", Json::Value::null);
				sf::Vector2f offset = { offsetArr[0].asFloat(), offsetArr[1].asFloat() };
				frameSprite.setPosition(barPos + offset);
			}
		}
	}
	catch (std::runtime_error e) {
		std::cerr << e.what();
	}
}

HealthBar::HealthBar() {
	deathRate = 0.0f;
	oldHealth = health = 0;
	healthTier = 0;
	rectTop.setPosition(barPos);
	rectTop.setSize(barSize);
	rectBottom.setPosition(barPos);
	rectBottom.setSize(barSize);
	foodOrbs = foodOrbsDefault;
	oldFoodTurns = foodTurns = 0;
}

void HealthBar::setHealth(int health) {
	this->health = clamp(health, 0, tierValues[HEALTH_TIER_NUM - 1]);
}

void HealthBar::updateOldHealth() {
	oldHealth = this->health;
}

int HealthBar::getHealth() const {
	return health;
}

int HealthBar::getOldHealth() const {
	return oldHealth;
}

int HealthBar::getTier() const {
	return healthTier;
}

void HealthBar::setTier(int tier) {
	healthTier = clamp(tier, 0, HEALTH_TIER_NUM - 2);
	rectTop.setFillColor(tierColors[healthTier + 1]);
	rectBottom.setFillColor(tierColors[healthTier]);
}

void HealthBar::setFood(int foodAmount) {
	food = clamp(foodAmount, 0, 1000000000);
	oldFoodTurns = foodTurns;
	foodTurns = food / std::max(health, 1);
}

int HealthBar::getFood() const {
	return food;
}

int HealthBar::getFoodTurns() {
	return foodTurns;
}

void HealthBar::consumeFood() {
	if (foodTurns > 0) {
		setFood(food - health);
	}
	else {
		deathRate += 0.01f;
	}
	setHealth(health - (int)(1000.0f * deathRate));
}

bool HealthBar::updateBars() {
	// Health
	int oldTier = healthTier;
	setTier(HEALTH_TIER_NUM - 2);
	for (int t = 0; t < tierValues.size(); t++) {
		if (health < tierValues[t]) {
			setTier(t - 1);
			break;
		}
	}
	float ratio = (float)(health - tierValues[healthTier]) / (float)(tierValues[healthTier + 1] - tierValues[healthTier]);
	rectTop.setSize({ barSize.x * ratio, barSize.y });
	// Food
	if (oldFoodTurns != foodTurns) {
		int foodCounter = 0;
		for (int o = 0; o < orbAmount; o++, foodCounter += 2) {
			if (foodTurns <= foodCounter) {
				setTexQuad(foodOrbs, o * 4, *emptyRect);
			}
			else if (foodTurns == foodCounter + 1) {
				setTexQuad(foodOrbs, o * 4, *halfRect);
			}
			else {
				setTexQuad(foodOrbs, o * 4, *fullRect);
			}
		}
		oldFoodTurns = foodTurns;
	}
	// Return true if tier changed
	return (oldTier != healthTier);
}

void HealthBar::draw(sf::RenderTarget &target, sf::RenderStates states) const {
	states.transform *= this->getTransform();
	target.draw(frameSprite, states);
	target.draw(rectBottom, states);
	target.draw(rectTop, states);
	states.texture = tex;
	target.draw(foodOrbs, states);
}