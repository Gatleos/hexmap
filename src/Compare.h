#ifndef VECTOR2FHASH_H
#define VECTOR2FHASH_H

#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Rect.hpp>

// Simple hash, not meant for large values
struct Vector2iHash
{
	size_t operator()(const sf::Vector2i &k) const
	{
		return (size_t)(k.x * 256 + k.y);
	}
};

struct Vector2iCompare
{
	bool operator() (const sf::Vector2i& lhs, const sf::Vector2i& rhs) const
	{
		bool x = (lhs.x * 256 + lhs.y) < (rhs.x * 256 + rhs.y);
		return x;
	}
};

struct Vector2fHash
{
	size_t operator()(const sf::Vector2f &k) const
	{
		return (size_t)(k.x * 256.f + k.y);
	}
};

struct Vector2fCompare
{
	bool operator() (const sf::Vector2f& lhs, const sf::Vector2f& rhs) const
	{
		bool x = (lhs.x*256.f + lhs.y) < (rhs.x*256.f + rhs.y);
		return x;
	}
};

#endif
