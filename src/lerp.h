#include <SFML/Graphics/Color.hpp>
#include <vector>

using namespace std;

void lerpColors(vector<sf::Color>& output, const sf::Color& a, const sf::Color& b, int numColors);
void lerpColorRange(vector<sf::Color>& output, const vector<sf::Color>& keyColors, const vector<int> keyIndices);
