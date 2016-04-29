#include <SFML/Graphics/Color.hpp>
#include <vector>

using namespace std;

namespace lerp {
	sf::Color interpolateColor(const sf::Color& a, const sf::Color& b, float fraction);
	void colors(vector<sf::Color>& output, const sf::Color& a, const sf::Color& b, int numColors);
	void colorRange(vector<sf::Color>& output, const vector<sf::Color>& keyColors, const vector<int> keyIndices);
	extern const sf::Color orange;
	extern const sf::Color purple;
	extern const sf::Color turquoise;
	extern const sf::Color brown;
	extern const sf::Color limeGreen;
}

