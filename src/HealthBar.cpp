#include "clamp.h"
#include "HealthBar.h"

std::array<int, TIER_NUM> HealthBar::tierValues = { 0, 100, 500, 1500, 3000, 6000, 10000 };
std::array<sf::Color, TIER_NUM> HealthBar::tierColors = { sf::Color(0, 0, 0), sf::Color(255, 0, 0), sf::Color(255, 127, 39), sf::Color(252, 241, 41),
sf::Color(64, 251, 43), sf::Color(43, 163, 251), sf::Color(136, 45, 249) };

HealthBar::HealthBar() {
	health = 0;
	healthTier = 0;
}

void HealthBar::setSize(const sf::Vector2f& size) {
	this->size = size;
	rectBottom.setSize(size);
}

void HealthBar::setHealth(int health) {
	this->health = clamp(health, 0, tierValues[TIER_NUM - 1]);
}

int HealthBar::getHealth() {
	return health;
}

int HealthBar::getTier() {
	return healthTier;
}

void HealthBar::setTier(int tier) {
	healthTier = clamp(tier, 0, TIER_NUM - 2);
	rectTop.setFillColor(tierColors[healthTier + 1]);
	rectBottom.setFillColor(tierColors[healthTier]);
}

bool HealthBar::updateBars() {
	int oldTier = healthTier;
	setTier(TIER_NUM - 2);
	for (int t = 0; t < tierValues.size(); t++) {
		if (health < tierValues[t]) {
			setTier(t - 1);
			break;
		}
	}
	float ratio = (float)(health - tierValues[healthTier]) / (float)(tierValues[healthTier + 1] - tierValues[healthTier]);
	rectTop.setSize({ size.x * ratio, size.y });
	return (oldTier != healthTier);
}

void HealthBar::draw(sf::RenderTarget &target, sf::RenderStates states) const {
	states.transform *= this->getTransform();
	target.draw(rectBottom, states);
	target.draw(rectTop, states);
}