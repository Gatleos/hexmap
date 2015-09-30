#ifndef BEZIERCURVE_H
#define BEZIERCURVE_H

//http://pomax.github.io/bezierinfo/

#include <SFML/Graphics.hpp>
#include <vector>



class Bezier {
	static int binomial(int n, int k) {
		static vector<vector<int>> lut = { { 1 }, { 1, 1 }, { 1, 2, 1 }, { 1, 3, 3, 1 }, { 1, 4, 6, 4, 1 }, { 1, 5, 10, 10, 5, 1 }, { 1, 6, 15, 20, 15, 6, 1 } };
		while (n >= (int)lut.size()) {
			int s = lut.size();
			vector<int> nextRow;
			nextRow.reserve(s + 1);
			nextRow[0] = 1;
			for (int i = 1, prev = s - 1; i < prev; i++) {
				nextRow[i] = lut[prev][i - 1] + lut[prev][i];
			}
			nextRow[s] = 1;
			lut.push_back(nextRow);
		}
		return lut[n][k];
	}
public:
	static void curve(sf::Vector2f& p, int n, float t, sf::Vector2f* points) {
		//function Bezier(n, t, points[]) :
		//	sum = 0
		//for (k = 0; k<n; k++) :
		//	sum += points[k] * binomial(n, k) * (1 - t) ^ (n - k) * t ^ (k)
		for (int k = 0; k < n; k++) {
			p.x += points[k].x * binomial(n, k) * pow((1.0f - t), (n - k)) * pow(t, k);
			p.y += points[k].y * binomial(n, k) * pow((1.0f - t), (n - k)) * pow(t, k);
		}
	}
	// Calculate a point on a quadratic bezier curve (3 points)
	static void curveQuad(sf::Vector2f& result, float t, sf::Vector2f* points) {
		float t2 = t * t;
		float mt = 1.0f - t;
		float mt2 = mt * mt;
		result.x = points[0].x * mt2 + points[1].x * 2 * mt*t + points[2].x * t2;
		result.y = points[0].y * mt2 + points[1].y * 2 * mt*t + points[2].y * t2;
	}
	// Calculate a point on a cubic bezier curve (4 points)
	static void curveCubic(sf::Vector2f& result, float t, sf::Vector2f* points) {
		float t2 = t * t;
		float t3 = t2 * t;
		float mt = 1.0f - t;
		float mt2 = mt * mt;
		float mt3 = mt2 * mt;
		result.x = points[0].x * mt3 + 3 * points[1].x * mt2*t + 3 * points[2].x * mt*t2 + points[3].x * t3;
		result.y = points[0].y * mt3 + 3 * points[1].y * mt2*t + 3 * points[2].y * mt*t2 + points[3].y * t3;
	}
	// Approximate the length of a cubic curve
	static float lengthCubic(sf::Vector2f* points) {
		//http://www.lemoda.net/maths/bezier-length/index.html
		float t;
		int i;
		int steps;
		sf::Vector2f dot;
		sf::Vector2f previous_dot;
		float length = 0.0f;
		steps = 10;
		for (i = 0; i <= steps; i++) {
			t = (float)i / (float)steps;
			curveCubic(dot, t, points);
			if (i > 0) {
				sf::Vector2f diff = dot - previous_dot;
				length += sqrt(diff.x * diff.x + diff.y * diff.y);
			}
			previous_dot = dot;
		}
		return length;
	}
};

#endif
