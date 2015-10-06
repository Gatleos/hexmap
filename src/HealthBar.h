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
	void setSize(sf::Vector2f& size);
	void setHealth(int health);
	int getHealth();
	int getTier();
	bool updateBars();
	virtual void draw(sf::RenderTarget &target, sf::RenderStates states = sf::RenderStates::Default) const;
};