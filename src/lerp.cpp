#include "lerp.h"

inline int interpolate(int a, int b, float val)
{
	return a + (val * (b - a));
}

void lerpColors(vector<sf::Color>& output, const sf::Color& a, const sf::Color& b, int numColors)
{
	for (int x = 0; x < numColors; x++) {
		float fraction = x / (float)numColors;
		output.emplace_back();
		output.back().r = interpolate(a.r, b.r, fraction);
		output.back().g = interpolate(a.g, b.g, fraction);
		output.back().b = interpolate(a.b, b.b, fraction);
	}
}

void lerpColorRange(vector<sf::Color>& output, const vector<sf::Color>& keyColors, const vector<int> keyIndices)
{
	int range = keyColors.size() - 1;
	int prevIndex = 0;
	for (int c = 0; c < range; c++) {
		lerpColors(output, keyColors[c], keyColors[c + 1], keyIndices[c] - prevIndex);
		prevIndex = keyIndices[c];
	}
}