#include "lerp.h"

const sf::Color lerp::orange(255, 128, 0);
const sf::Color lerp::purple(128, 0, 255);
const sf::Color lerp::turquoise(0, 255, 128);
const sf::Color lerp::brown(128, 64, 0);
const sf::Color lerp::limeGreen(128, 255, 0);

inline int interpolate(int a, int b, float val) {
	return a + (val * (b - a));
}

void lerp::colors(vector<sf::Color>& output, const sf::Color& a, const sf::Color& b, int numColors) {
	for (int x = 0; x < numColors; x++) {
		float fraction = x / (float)numColors;
		output.emplace_back();
		output.back().r = interpolate(a.r, b.r, fraction);
		output.back().g = interpolate(a.g, b.g, fraction);
		output.back().b = interpolate(a.b, b.b, fraction);
	}
}

void lerp::colorRange(vector<sf::Color>& output, const vector<sf::Color>& keyColors, const vector<int> keyIndices) {
	int range = keyColors.size() - 1;
	int prevIndex = 0;
	for (int c = 0; c < range; c++) {
		colors(output, keyColors[c], keyColors[c + 1], keyIndices[c] - prevIndex);
		prevIndex = keyIndices[c];
	}
}