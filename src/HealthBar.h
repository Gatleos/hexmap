#ifndef HEALTHBAR_H
#define HEALTHBAR_H

#include <array>
#include <SFML/Graphics.hpp>

#define TIER_NUM 7

class HealthBar : public sf::Drawable, public sf::Transformable {
	static std::array<int, TIER_NUM> tierValues;
	static std::array<sf::Color, TIER_NUM> tierColors;
	int health;
	int healthTier;
	sf::RectangleShape rectBottom;
	sf::RectangleShape rectTop;
	sf::Vector2f size;
	void setTier(int tier);
public:
	HealthBar();
	void setSize(const sf::Vector2f& size);
	void setHealth(int health);
	int getHealth();
	int getTier();
	// Returns true if the healthTier has changed
	bool updateBars();
	virtual void draw(sf::RenderTarget &target, sf::RenderStates states = sf::RenderStates::Default) const;
};

#endif