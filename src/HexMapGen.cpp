#include <iostream>
#include <memory>
#include "HexMap.h"
#include "lerp.h"
#include "simplexnoise.h"
#include "clamp.h"
#include "BezierCurve.h"
#include "config.h"
#include "rng.h"

void HexMap::generateBiomes(mt19937& urng)
{
	land.clear();
	static uniform_real_distribution<float> offset(-10000000.0f, 10000000.0f);
	int townChance = 16384 / 24;
	const HexTileS* type = &HexTileS::get(HexTileS::NONE);
	int colorIndex = 0;
	sf::Vector2f offsetHeight = { offset(urng), offset(urng) };
	sf::Vector2f offsetTemperature = { offset(urng), offset(urng) };
	sf::Vector2f offsetMoisture = { offset(urng), offset(urng) };
	sf::Vector2f offsetDrainage = { offset(urng), offset(urng) };
	sf::Vector2i hexPos;
	sf::Vector2f p;
	float heightVal = 0.0f;
	float tempVal = 0.0f;
	float moistVal = 0.0f;
	float drainVal = 0.0f;
	for (int r = 0; r < mapSize_.y; r++) {
		for (int q = 0, qoff = (int)-floor(r / 2.0); q < mapSize_.x; q++, qoff++) {
			hexPos = { q, r };
			p = { qoff + r * 0.5f, (float)r };
			heightVal = scaled_octave_noise_2d(config::gen::heightParams[0], config::gen::heightParams[1], config::gen::heightParams[2], 0.0f, 255.0f, p.x + offsetHeight.x, p.y + offsetHeight.y);
			tempVal = scaled_octave_noise_2d(config::gen::tempParams[0], config::gen::tempParams[1], config::gen::tempParams[2], 96.0f, 160.0f, p.x + offsetTemperature.x, p.y + offsetTemperature.y);
			moistVal = scaled_octave_noise_2d(config::gen::moistParams[0], config::gen::moistParams[1], config::gen::moistParams[2], 0.0f, 255.0f, p.x + offsetMoisture.x, p.y + offsetMoisture.y);
			drainVal = scaled_octave_noise_2d(config::gen::drainParams[0], config::gen::drainParams[1], config::gen::drainParams[2], 0.0f, 255.0f, p.x + offsetDrainage.x, p.y + offsetDrainage.y);
			switch (0) { // fix
			case 0:
				tempVal += (r - 64) * 1.5f;
				break;
			case 1:
				tempVal -= (r - 64) * 1.5f;
				break;
			case 2:
				tempVal += (q - 64) * 1.5f;
				break;
			case 3:
				tempVal -= (q - 64) * 1.5f;
			}
			// Determine the terrain type based on elevation, temperature, moisture and drainage
			if (heightVal < SEA_LEVEL) {
				type = &HexTileS::get(HexTileS::OCEAN);
				colorIndex = (int)heightVal;
			}
			else {
				land.push_back(p);
				colorIndex = (int)heightVal - SEA_LEVEL;
				if (tempVal < config::gen::cold) {
					if (moistVal >= config::gen::forest[0]) {
						if (tempVal < 50.0f) {
							if (moistVal >= config::gen::forest[2]) {
								type = &HexTileS::get(HexTileS::TAIGA_SNOW_L);
							}
							else if (moistVal >= config::gen::forest[1]) {
								type = &HexTileS::get(HexTileS::TAIGA_SNOW_M);
							}
							else {
								type = &HexTileS::get(HexTileS::TAIGA_SNOW_S);
							}
						}
						else {
							if (moistVal >= config::gen::forest[2]) {
								type = &HexTileS::get(HexTileS::TAIGA_L);
							}
							else if (moistVal >= config::gen::forest[1]) {
								type = &HexTileS::get(HexTileS::TAIGA_M);
							}
							else {
								type = &HexTileS::get(HexTileS::TAIGA_S);
							}
						}
					}
					else {
						if (tempVal < 50.0f) {
							type = &HexTileS::get(HexTileS::TUNDRA_SNOW);
						}
						else {
							type = &HexTileS::get(HexTileS::TUNDRA);
						}
					}
				}
				else if (tempVal < config::gen::hot) {
					if (moistVal >= config::gen::forest[0]) {
						if (drainVal >= config::gen::swamp) {
							if (moistVal >= config::gen::forest[2]) {
								type = &HexTileS::get(HexTileS::FOREST_L);
							}
							else if (moistVal >= config::gen::forest[1]) {
								type = &HexTileS::get(HexTileS::FOREST_M);
							}
							else {
								type = &HexTileS::get(HexTileS::FOREST_S);
							}
						}
						else {
							type = &HexTileS::get(HexTileS::SWAMP);
						}
					}
					else if (moistVal >= config::gen::desert) {
						type = &HexTileS::get(HexTileS::GRASSLAND);
					}
					else {
						if (drainVal >= config::gen::sand) {
							type = &HexTileS::get(HexTileS::SEMIARID);
						}
						else {
							type = &HexTileS::get(HexTileS::DESERT);
						}
					}
				}
				else {
					if (moistVal >= config::gen::forest[0]) {
						if (drainVal >= config::gen::swamp) {
							if (moistVal >= config::gen::forest[2]) {
								type = &HexTileS::get(HexTileS::JUNGLE_L);
							}
							else if (moistVal >= config::gen::forest[1]) {
								type = &HexTileS::get(HexTileS::JUNGLE_M);
							}
							else {
								type = &HexTileS::get(HexTileS::JUNGLE_S);
							}
						}
						else {
							type = &HexTileS::get(HexTileS::SWAMP);
						}
					}
					else if (moistVal >= config::gen::desert) {
						type = &HexTileS::get(HexTileS::SAVANNA);
					}
					else {
						if (drainVal >= config::gen::sand) {
							type = &HexTileS::get(HexTileS::SEMIARID);
						}
						else {
							type = &HexTileS::get(HexTileS::DESERT);
						}
					}
				}
			}
			getOffset(hexPos.x, hexPos.y).FLAGS = 0;
			setTile(hexPos, *type, urng);
			if (type->FLAGS[HexTileS::GRADIENT]) {
				pushTileColor(hexPos, type->colors[colorIndex]);
			}
			else {
				pushTileColor(hexPos, sf::Color::White);
			}
		}
	}
}

void HexMap::generateMountainRange(mt19937& urng)
{
	static sf::Color mt(128, 88, 44);
	static vector<VectorSet> splat = { { { 1, -1 }, { 2, -1 }, { 0, 0 }, { 1, 0 }, { -1, 1 }, { 0, 1 }, { -2, 2 }, { -1, 2 }, { 0, 2 } },
	{ { 1, -2 }, { 0, -1 }, { 1, -1 }, { -1, 0 }, { 0, 0 }, { 1, 0 }, { -1, 1 }, { 0, 1 }, { -1, 2 }, { 0, 2 } },
	{ { 0, -1 }, { 1, -1 }, { -1, 0 }, { 0, 0 }, { 1, 0 }, { -1, 1 }, { 0, 1 } },
	{ { 0, -1 }, { 1, -1 }, { 2, -1 }, { -1, 0 }, { 0, 0 }, { 1, 0 }, { -1, 1 }, { 0, 1 }, { 1, 1 } },
	{ { 0, -2 }, { 1, -2 }, { -1, -1 }, { 0, -1 }, { 1, -1 }, { -1, 0 }, { 0, 0 }, { 1, 0 }, { -1, 1 } },
	{ { -1, -1 }, { 0, -1 }, { -1, 0 }, { 0, 0 }, { 1, 0 }, { -2, 1 }, { -1, 1 }, { 0, 1 } },
	{ { -1, 0 }, { 1, 0 }, { 0, 0 }, { -2, 1 }, { -1, 1 }, { 0, 1 }, { 1, 1 }, { -2, 2 }, { -1, 2 }, { 0, 2 } },
	{ { 0, -2 }, { 1, -2 }, { 0, -1 }, { 1, -1 }, { 2, -1 }, { -1, 0 }, { 0, 0 }, { 1, 0 }, { 0, 1 } } };
	sf::VertexArray va;
	static sf::Vector2f w[4];
	sf::Vector2f offset = { (float)rng::getInt(2, 85, urng), (float)rng::getInt(2, 85, urng) };
	for (int a = 0; a < 100; a++) {
		w[0] = { (float)xRange(urng), (float)yRange(urng) };
		if (isAxialInBounds((sf::Vector2i)w[0]) && getAxial((int)w[0].x, (int)w[0].y).hts->FLAGS[HexTileS::WALKABLE]) {
			break;
		}
	}
	w[3] = { (float)rng::radians(urng), (float)rng::getInt(10, 40, urng) };
	polarToCartesian(w[3]);
	w[3] = roundvf(w[0] + w[3]);
	sf::Vector2f avg = { (w[3] + w[0]) / 2.0f };
	// sqrt(a^2 + b^2)
	int dist = (int)sqrtf(powf(abs(w[3].x - w[0].x), 2.0f) + powf(abs(w[3].x - w[0].x), 2.0f));
	for (int a = 1; a < 3; a++) {
		// Distance between the middle points and the endpoint average cannot be greater
		// than the distance between the endpoints, to prevent any super-sharp curves
		w[a] = { (float)rng::radians(urng), (float)rng::getInt(10, clamp(dist, 10, 40), urng) };
		polarToCartesian(w[a]);
		w[a] = roundvf(w[a] + avg);
	}
	float advance = 1.0f / Bezier::lengthCubic(w);
	VectorSet h;
	sf::Vector2f p;
	for (float f = 0.0f; f < 1.0f; f += advance) {
		Bezier::curveCubic(p, f, w);
		if (!isAxialInBounds((sf::Vector2i)p) || !getAxial((int)p.x, (int)p.y).hts->FLAGS[HexTileS::WALKABLE]) {
			break;
		}
		VectorSet& s = splat[rng::getInt(0, splat.size() - 1, urng)];
		for (auto r : s) {
			h.insert((sf::Vector2i)axialToOffset(roundHex((sf::Vector2f)r + p)));
		}
		p = { 0.0f, 0.0f };
	}
	HexTile* cTile = nullptr;
	for (auto& l : h) {
		cTile = &getOffset(l.x, l.y);
		if (!isOffsetInBounds((sf::Vector2i)l) || !cTile->hts->FLAGS[HexTileS::WALKABLE]) {
			continue;
		}
		//pushTileColor((sf::Vector2i)l, sf::Color::White);
		setTileFeature((sf::Vector2i)l, TileFeatureS::get(TileFeatureS::MOUNTAIN), urng);
		cTile->FLAGS[HexTile::MOUNTAINS] = true;
	}
}

// Detect contiguous terrain tiles and store them in the regions list
void HexMap::findRegions()
{
	// new tiles to query
	multimap<int, sf::Vector2i> frontier;
	int i = (int)(mapSize_.x*mapSize_.y);
	HexTile* h = nullptr;
	sf::Vector2i p;
	std::unique_ptr<bool> seen(new bool[i]);
	for (int a = 0; a < i; a++) {
		seen.get()[a] = false;
	}
	std::deque<sf::Vector2i> peaks;
	std::deque<Region> regions;

	Region* currentRegion;
	for (int r = 0; r < mapSize_.y; r++) {
		for (int q = 0, qoff = (int)-floor(r / 2.0); q < mapSize_.x; q++, qoff++) {
			i = q + mapSize_.x * r;
			if (seen.get()[i]) {
				continue;
			}
			regions.emplace_back(1, sf::Vector2i(qoff, r));
			currentRegion = &regions.back();
			h = &getAxial(qoff, r);
			frontier.insert(make_pair(0, sf::Vector2i(qoff, r)));
			VectorSet adj;
			while (!frontier.empty()) {
				for (auto f : frontier) {
					clipToBounds(neighbors(f.second, adj));
				}
				frontier.clear();
				for (auto n : adj) {
					p = axialToOffset(n);
					i = p.x + p.y * mapSize_.x;
					if (seen.get()[i]) {
						continue;
					}
					HexTile& t = getAxial(n.x, n.y);
					if (t.height >= 200) {
						peaks.push_back(n);
					}
					if (t.hts == h->hts) {
						frontier.insert(make_pair(0, n));
						seen.get()[i] = true;
						(*currentRegion).size++;
					}
				}
				adj.clear();
			}
		}
	}
}

void HexMap::placeSites(mt19937& urng)
{
	array<sf::Color, 13> tColors = { sf::Color::Blue, sf::Color::Cyan, sf::Color::Black, sf::Color::Green, sf::Color::Magenta,
	sf::Color::Red, sf::Color::White, sf::Color::Yellow, lerp::brown, lerp::limeGreen, lerp::orange, lerp::purple, lerp::turquoise};
	static function<bool(HexTile&)> condition = [](HexTile& hex){ return hex.hts->FLAGS[HexTileS::WALKABLE] && !hex.FLAGS[HexTile::MOUNTAINS]; };
	clearSites();
	clearMapUnits();
	auto* fac = addFaction();
	std::uniform_int_distribution<int> landChance(0, land.size() - 1);
	std::vector<sf::Vector2i> territories;
	for (int a = 0; a < 40; a++) {
		sf::Vector2i aPos = (sf::Vector2i)offsetToAxial(land[landChance(urng)]);
		territories.push_back(aPos);
	}
	std::vector<VectorSet> fill;
	fill.resize(territories.size());
	floodSelectParallel(fill, territories, 126, condition);
	int index = 0;
	std::vector<sf::Vector2i> output;
	for (auto& f : fill) {
		for (auto& h : f) {
			sf::Vector2i oPos = axialToOffset(h);
			pushTileColor(oPos, tColors[index%tColors.size()]);
			setTile(oPos, HexTileS::get(HexTileS::TUNDRA_SNOW), urng);
		}
		f.erase(f.find(territories[index]));
		int size = (int)(0.1 * f.size());
		for (int a = 0; a < size; a++) {
			std::uniform_int_distribution<int> range(0, f.size() - 1);
			auto it = f.begin();
			std::advance(it, range(urng));
			output.push_back(*it);
			f.erase(it);
		}
		auto* c = addSite(SiteS::get("si_castle"), fac);
		c->setAnimationType(MapEntityS::anim::IDLE);
		c->initMapPos(territories[index]);
		for (auto& o : output) {
			auto* s = addSite(SiteS::get(rng::boolean(urng)?"si_town":"si_village"), fac);
			s->setAnimationType(MapEntityS::anim::IDLE);
			s->initMapPos(o);
		}
		output.clear();
		index++;
	}
}