#ifndef CLAMP_H
#define CLAMP_H

#include <algorithm>

template <typename T>
T clamp(const T& x, const T& lo, const T& hi) {
	return std::max(lo, std::min(x, hi));
}
template <typename T>
T wrap(T x, const T& lo, const T& hi) {
	auto diff = hi - lo;
	while (x<lo)x += diff;
	while (x>hi)x -= diff;
	return x;
}
template <typename T>
// Returns true if (lo <= x <= hi)
bool isInRange(const T& x, const T& lo, const T& hi) {
	return x<lo ? false : x>hi ? false : true;
}
template <typename T>
bool isInRect(const T& x, const T& y, const T& rx, const T& ry, const T& rw, const T& rh) {
	return (x >= rx && x < rx + rw && y >= ry && y < ry + rh);
}

#endif
