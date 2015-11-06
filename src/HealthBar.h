#ifndef HEALTHBAR_H
#define HEALTHBAR_H

#include <array>
#include <SFML/Graphics.hpp>
#include "ResourceLoader.h"

#define HEALTH_TIER_NUM 7

class HealthBar : public sf::Drawable, public sf::Transformable {
	static SpriteSheet* sheet;
	static const sf::Texture* tex;
	// Health bar template
	static sf::Vector2f barPos;
	static sf::Vector2f barSize;
	static int barPadding;
	// Food orbs template
	static const sf::FloatRect* fullRect;
	static const sf::FloatRect* halfRect;
	static const sf::FloatRect* emptyRect;
	static int orbAmount;
	static sf::VertexArray foodOrbsDefault;
	// Frame template
	static const sf::FloatRect* frameRect;
	static sf::Sprite frameSprite;
	// Health info
	sf::VertexArray foodOrbs;
	int health;
	int healthTier;
	int food;
	int foodTurns;
	int oldFoodTurns;
	sf::RectangleShape rectBottom;
	sf::RectangleShape rectTop;
	void setTier(int tier);
public:
	static const std::array<int, HEALTH_TIER_NUM> tierValues;
	static const std::array<sf::Color, HEALTH_TIER_NUM> tierColors;
	static void loadJson(std::string filename);
	HealthBar();
	void setHealth(int health);
	int getHealth() const;
	int getTier() const;
	void setFood(int foodAmount);
	int getFood() const;
	int getFoodTurns();
	// Returns true if the healthTier has changed
	bool updateBars();
	virtual void draw(sf::RenderTarget &target, sf::RenderStates states = sf::RenderStates::Default) const;
};

#endif