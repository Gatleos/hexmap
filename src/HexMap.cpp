#include <fstream>
#include <sstream>
#include <iostream>
#include "HexMap.h"
#include "simplexnoise.h"
#include "lerp.h"
#include "clamp.h"
#include "Site.h"
#include "MapEntity.h"


const int HexMap::CHUNK_SIZE = 16;
const int HexMap::CHUNK_SQUARED = CHUNK_SIZE * CHUNK_SIZE;

int HexMap::hexRad_[ZOOM_LEVELS] = { 37, 18, 11 };
// hexSize_ = { round(hexRadius * 0.864864), hexRadius };
sf::Vector2i HexMap::hexSize_[ZOOM_LEVELS] = { { 64, 74 }, { 32, 36 }, { 20, 22 } };
// hexOffset_ = { roundf(hexRad_ * ROOT3), roundf(hexRad_ * 3.0 / 2.0) };
sf::Vector2f HexMap::hexAdvance_[ZOOM_LEVELS] = { { 64, 56 }, { 32, 27 }, { 20, 16 } };
// mapOrigin_ = { hexSize_.x * 0.5, hexSize_.y * 0.5 };
sf::Vector2f HexMap::mapOrigin_[ZOOM_LEVELS] = { { 32, 37 }, { 16, 18 }, { 10, 11 } };

array<Road, 6> roadSprites;
std::vector<sf::FloatRect> mountainSprites;

float distHex(sf::Vector2f& a, sf::Vector2f& b)
{
	return (abs(a.x - b.x) + abs(a.x + a.y - b.x - b.y) + abs(a.y - b.y)) / 2;
}
float distHex(sf::Vector2i& a, sf::Vector2i& b)
{
	return (float)((abs(a.x - b.x) + abs(a.x + a.y - b.x - b.y) + abs(a.y - b.y)) / 2);
}
void polarToCartesian(sf::Vector2f& p)
{
	float tx = p.y * cos(p.x);
	p.y = p.y * sin(p.x);
	p.x = tx;
}
sf::Vector2f roundvf(sf::Vector2f p)
{
	p.x = roundf(p.x);
	p.y = roundf(p.y);
	return p;
}

HexMap::HexMap() :
activeBgVertices_(&bgVertices_[0]),
nextUnitId(0),
nextSiteId(0)
{
}

bool HexMap::walkable(sf::Vector2i& c)
{
	return true;
}

int HexMap::heuristic(sf::Vector2i& a, sf::Vector2i& b)
{
	return (int)((abs(a.x - b.x) + abs(a.x + a.y - b.x - b.y) + abs(a.y - b.y)) / 2);
	//return abs(a.x - b.x) + abs(a.y - b.y);
}

int HexMap::moveCost(sf::Vector2i& current, sf::Vector2i& next)
{
	return 1;
}

std::deque<sf::Vector2i>& HexMap::getPath(std::deque<sf::Vector2i>& path, sf::Vector2i startAxial, sf::Vector2i goalAxial)
{
	if (!isAxialInBounds(startAxial) || !getAxial(startAxial.x, startAxial.y).hts->walkable || !isAxialInBounds(goalAxial) || !getAxial(goalAxial.x, goalAxial.y).hts->walkable) {
		return path;
	}
	sf::Vector2i start = startAxial;
	sf::Vector2i goal = goalAxial;
	cameFrom.clear();
	costSoFar.clear();
	frontier.clear();
	std::deque<sf::Vector2i> n;
	frontier.insert(std::pair<int, sf::Vector2i>(0, start));
	cameFrom[start] = start;
	costSoFar[start] = 0;
	while (!frontier.empty()) {
		auto current = frontier.begin()->second;
		frontier.erase(frontier.begin());
		if (current == goal) {
			break;
		}
		n.clear();
		for (auto next : neighbors(current, n)) {
			if (!isAxialInBounds(next) || !getAxial(next.x, next.y).hts->walkable) {
				continue;
			}
			int newCost = costSoFar[current] + moveCost(current, next);
			if (!costSoFar.count(next) || newCost < costSoFar[next]) {
				costSoFar[next] = newCost;
				int priority = newCost + heuristic(next, goal);
				frontier.insert(std::pair<int, sf::Vector2i>(priority, next));
				cameFrom[next] = current;
			}
		}
	}
	if (cameFrom.find(goal) == cameFrom.end()) {
		// A path was not found
		return path;
	}
	for (sf::Vector2i c = goal; c != start; c = cameFrom.find(c)->second) {
		path.push_front(c);
	}
	return path;
}
const int& HexMap::getHexRadius(int zoom)
{
	return hexRad_[zoom];
}

const sf::Vector2i& HexMap::getHexSize(int zoom)
{
	return hexSize_[zoom];
}

const sf::Vector2f& HexMap::getHexAdvance(int zoom)
{
	return hexAdvance_[zoom];
}

const sf::Vector2f& HexMap::getMapOrigin(int zoom)
{
	return mapOrigin_[zoom];
}

std::deque<sf::Vector2i>& HexMap::neighborsBounded(sf::Vector2i posAxial, std::deque<sf::Vector2i>& n)
{
	static sf::Vector2i v;
	for (int x = 0; x < 6; x++) {
		v = { posAxial.x + directions[x].x, posAxial.y + directions[x].y };
		if (isAxialInBounds(v)) {
			n.push_back({ v.x, v.y });
		}
	}
	return n;
}

const sf::Time& HexMap::getLifetime()
{
	return lifetime;
}

const sf::Vector2i& HexMap::getMapSize() const
{
	return mapSize_;
}
void HexMap::init(int width, int height)
{
	int currentVertices = 0;
	sf::Vector2i chunkOffset = { 0, 0 };
	sf::Vector2i tileOffset = { 0, 0 };
	// Generate all hex tiles and their corresponding vertices
	mapSize_ = { width, height };
	mapSizeChunks_ = { width / CHUNK_SIZE, height / CHUNK_SIZE };
	if (width % CHUNK_SIZE != 0) {
		mapSizeChunks_.x++;
	}
	if (height % CHUNK_SIZE != 0) {
		mapSizeChunks_.y++;
	}
	xRange.param(uniform_int_distribution<int>(0, width - 1).param());
	yRange.param(uniform_int_distribution<int>(0, height - 1).param());
	hexes_.set(width, height);
	for (int bv = 0; bv < ZOOM_LEVELS; bv++) {
		hexExtent_[currentVertices] = { (mapSize_.x - 1) * hexAdvance_[currentVertices].x, (mapSize_.y - 1) * hexAdvance_[currentVertices].y };
		setZoomLevel(currentVertices);
		sf::Vector2f p;
		int chunkIndex = 0;
		auto& b = bgVertices_[bv];
		b.set(mapSizeChunks_.x, mapSizeChunks_.y);
		chunkOffset = { 0, 0 };
		for (int chunkIndex = 0; chunkIndex < (int)b.size(); chunkIndex++) { // iterate through all chunks, row order
			int index = 0;
			auto& bc = b[chunkIndex];
			chunkOffset.x = chunkIndex % mapSizeChunks_.x * CHUNK_SIZE;
			chunkOffset.y = chunkIndex / mapSizeChunks_.x * CHUNK_SIZE;
			chunkOffset.x += -chunkOffset.y / 2;
			bc.setPrimitiveType(sf::PrimitiveType::Quads);
			bc.resize((size_t)CHUNK_SQUARED * 4);
			for (int r = 0; r < CHUNK_SIZE; r++) { // iterate through every tile in the chunk
				for (int q = 0, qoff = -r / 2; q < CHUNK_SIZE; q++, qoff++) {
					p = { (float)qoff + chunkOffset.x, (float)r + chunkOffset.y };
					p = hexToPixel(p);
					bc[index++].position = p;
					p.x += hexSize_[zoomLevel].x;
					bc[index++].position = p;
					p.y += hexSize_[zoomLevel].y;
					bc[index++].position = p;
					p.x -= hexSize_[zoomLevel].x;
					bc[index++].position = p;
					sizeof(int);
				}
			}
		}
		currentVertices++;
	}
	setZoomLevel(0);
}

sf::Vector2f HexMap::roundHex(sf::Vector2f hex)
{
	cubepoint c;
	c.x = hex.x;
	c.z = hex.y;
	c.y = -c.x - c.z;
	cubepoint r(roundf(c.x), roundf(c.y), roundf(c.z));
	cubepoint diff(abs(r.x - c.x), abs(r.y - c.y), abs(r.z - c.z));
	if (diff.x > diff.y && diff.x > diff.z) { r.x = -r.y - r.z; }
	else if (diff.y > diff.z) { r.y = -r.x - r.z; }
	else { r.z = -r.x - r.y; }
	hex.x = r.x;
	hex.y = r.z;
	return hex;
}
sf::Vector2f HexMap::hexToPixel(sf::Vector2f hex) const
{
	hex.x = hexAdvance_[zoomLevel].x * (hex.x + hex.y / 2.0f);
	hex.y = hexAdvance_[zoomLevel].y * hex.y;
	return hex;
}
sf::Vector2i HexMap::hexToPixel(sf::Vector2i hex) const
{
	hex.x = (int)(hexAdvance_[zoomLevel].x * (hex.x + hex.y / 2.0));
	hex.y = (int)hexAdvance_[zoomLevel].y * hex.y;
	return hex;
}
sf::Vector2f HexMap::hexToPixel(sf::Vector2f hex, int zoom) const
{
	hex.x = hexAdvance_[zoom].x * (hex.x + hex.y / 2.0f);
	hex.y = hexAdvance_[zoom].y * hex.y;
	return hex;
}
sf::Vector2i HexMap::hexToPixel(sf::Vector2i hex, int zoom) const
{
	hex.x = (int)(hexAdvance_[zoom].x * (hex.x + hex.y / 2.0));
	hex.y = (int)hexAdvance_[zoom].y * hex.y;
	return hex;
}
sf::Vector2f HexMap::pixelToHex(sf::Vector2f pixel) const
{
	//hexAdvance_ = { roundf(hexRad_ * ROOT3), roundf(hexRad_ * 3.0 / 2.0) }
	// compute it
	cubepoint c;
	pixel.x = pixel.x / hexAdvance_[zoomLevel].x - pixel.y / hexAdvance_[zoomLevel].y * 0.5f;
	pixel.y = pixel.y / hexAdvance_[zoomLevel].y;
	// round it
	return roundHex(pixel);
}

bool HexMap::isAxialInBounds(sf::Vector2i posAxial)
{
	posAxial.x -= -floorf(posAxial.y / 2.0);
	if (posAxial.x < 0 || posAxial.x >= mapSize_.x || posAxial.y < 0 || posAxial.y >= mapSize_.y) {
		return false;
	}
	return true;
}

bool HexMap::isOffsetInBounds(sf::Vector2i posOffset)
{
	if (posOffset.x < 0 || posOffset.x >= mapSize_.x || posOffset.y < 0 || posOffset.y >= mapSize_.y) {
		return false;
	}
	return true;
}

std::deque<sf::Vector2i>& HexMap::neighbors(sf::Vector2i i, std::deque<sf::Vector2i>& n)
{
	for (int x = 0; x < 6; x++) {
		n.push_back(sf::Vector2i(i.x + directions[x].x, i.y + directions[x].y));
	}
	return n;
}
VectorSet& HexMap::area(sf::Vector2i h, int radius, VectorSet& n)
{
	cubepoint cp(h.x, 0, h.y);
	cp.y = -cp.x - cp.z;
	cubepoint d;
	cubepoint np;
	np.y = (float)radius;
	for (d.x = -np.y; d.x <= np.y; d.x++) {
		for (d.y = std::max(-np.y, -d.x - np.y); d.y <= std::min(np.y, -d.x + np.y); d.y++) {
			d.z = -d.x - d.y;
			np = d + cp;
			n.insert({ (int)np.x, (int)np.z });
		}
	}
	return n;
}
sf::Vector2f HexMap::offsetToAxial(sf::Vector2f v)
{
	//x = col - (row - (row & 1)) / 2
	//z = row
	//y = -x - z
	v.x -= floorf((v.y - ((int)v.y & 1)) / 2.0f);
	return v;
}
sf::Vector2f HexMap::axialToOffset(sf::Vector2f v)
{
	v.x += floorf((v.y - ((int)v.y & 1)) / 2.0f);
	return v;
}
sf::Vector2i HexMap::offsetToAxial(sf::Vector2i v)
{
	v.x -= (v.y - (v.y & 1)) / 2;
	return v;
}
sf::Vector2i HexMap::axialToOffset(sf::Vector2i v)
{
	v.x += (v.y - (v.y & 1)) / 2;
	return v;
}

HexTile& HexMap::getAxial(int x, int y)
{
	double tempX = x, tempY = y;
	tempX += floor((tempY - ((int)tempY & 1)) / 2.0);
	return hexes_((int)tempX, (int)tempY);
}
HexTile& HexMap::getOffset(int x, int y)
{
	return hexes_(x, y);
}
void HexMap::floodSelect(VectorSet& fill, int minHeight, int maxHeight)
{ // UNFINISHED
	return;
	std::deque<sf::Vector2i> neighbors;
	VectorSet frontier = fill;
	while (!frontier.empty()) {
		for (auto f : frontier) {
			neighborsBounded(f, neighbors);
		}
		frontier.clear();
		for (auto n : neighbors) {
			HexTile& t = getAxial(n.x, n.y);
			if (fill.find(n) == fill.end() && frontier.find(n) == frontier.end() && t.height >= minHeight && t.height < maxHeight) {
				frontier.insert(n);
			}
		}
		fill.insert(frontier.begin(), frontier.end());
		neighbors.clear();
	}
	frontier.clear();
}


int HexMap::getZoomLevel()
{
	return zoomLevel;
}
void HexMap::setZoomLevel(int zoom)
{
	zoomLevel = clamp(zoom, 0, 2);
	activeBgVertices_ = &bgVertices_[zoomLevel];
	setOrigin(mapOrigin_[zoomLevel]);
}
const sf::Vector2f HexMap::getHexAdvance()
{
	return hexAdvance_[zoomLevel];
}
void HexMap::setAllTiles(const HexTileS& hts, mt19937& urng)
{
	sf::Vector2i chunkPos;
	sf::Vector2i tilePos;
	int a = 0;
	for (auto& v : bgVertices_) {
		int rNum = hts.tiles[a].randomize(urng);
		const sf::FloatRect& rect = hts.tiles[a].getRect(rNum);
		int chunkIndex = 0;
		for (auto* c : v) { // iterate through all chunks, row order
			chunkPos = { chunkIndex % (int)mapSizeChunks_.x * CHUNK_SIZE, chunkIndex / (int)mapSizeChunks_.x * CHUNK_SIZE };
			for (int h = 0, index = 0; index < CHUNK_SQUARED; index++) {
				tilePos = { index % CHUNK_SIZE, index / CHUNK_SIZE };
				tilePos += chunkPos;
				HexTile& hex = getOffset(tilePos.x, tilePos.y);
				hex.hts = &hts;
				(*c)[h++].texCoords = { rect.left, rect.top };
				(*c)[h++].texCoords = { rect.left + rect.width, rect.top };
				(*c)[h++].texCoords = { rect.left + rect.width, rect.top + rect.height };
				(*c)[h++].texCoords = { rect.left, rect.top + rect.height };
			}
			chunkIndex++;
		}
		a++;
	}
	sf::Vector2f pix;
	for (int s = 0; s < 3; s++) {
		if (hts.features[s] != nullptr) {
			auto& feat = hts.features[s]->rects_[s];
			int index = 0;
			int rNum = 0;
			for (auto* h : hexes_) {
				h->spr[s].setTexture(TileFeatureS::getTexture());
				rNum = feat.randomize(urng);
				h->spr[s].setTextureRect((sf::IntRect)feat.getRect(rNum));
				pix = hexToPixel((sf::Vector2f)offsetToAxial(sf::Vector2f((float)(index % mapSize_.y), (float)(index / mapSize_.y))), s);
				h->spr[s].setPosition(pix + feat.getPos(rNum));
				index++;
			}
		}
	}
}
void HexMap::setTile(sf::Vector2i offsetPos, const HexTileS& hts, mt19937& urng)
{
	auto& hex = hexes_(offsetPos.x, offsetPos.y);
	hex.hts = &hts;
	sf::Vector2i chunkPos = { (offsetPos.x / CHUNK_SIZE), (offsetPos.y / CHUNK_SIZE) };
	int index = ((offsetPos.y % CHUNK_SIZE) * CHUNK_SIZE + (offsetPos.x % CHUNK_SIZE)) * 4;
	int rNum = 0;
	for (int a = 0; a < 3; a++) {
		rNum = hts.tiles[a].randomize(urng);
		const sf::FloatRect& rect = hts.tiles[a].getRect(rNum);
		if (hts.features[a] != nullptr) {
			setTileFeature(offsetPos, *hts.features[a], a, urng);
		}
		sf::VertexArray& chunk = bgVertices_[a](chunkPos.x, chunkPos.y);
		chunk[index].texCoords = { rect.left, rect.top };
		chunk[index + 1].texCoords = { rect.left + rect.width, rect.top };
		chunk[index + 2].texCoords = { rect.left + rect.width, rect.top + rect.height };
		chunk[index + 3].texCoords = { rect.left, rect.top + rect.height };
	}
}
void HexMap::setTileColor(sf::Vector2i offsetPos, sf::Color col)
{
	sf::Vector2i chunkPos = { (offsetPos.x / CHUNK_SIZE), (offsetPos.y / CHUNK_SIZE) };
	int index = ((offsetPos.y % CHUNK_SIZE) * CHUNK_SIZE + (offsetPos.x % CHUNK_SIZE)) * 4;
	for (int a = 0; a < 3; a++) {
		sf::VertexArray& chunk = bgVertices_[a](chunkPos.x, chunkPos.y);
		chunk[index].color = col;
		chunk[index + 1].color = col;
		chunk[index + 2].color = col;
		chunk[index + 3].color = col;
	}
}
void HexMap::setTileFeature(sf::Vector2i offsetPos, const TileFeatureS& tfs, mt19937& urng)
{
	auto& spr = hexes_(offsetPos.x, offsetPos.y).spr;
	int rNum = 0;
	for (int s = 0; s < 3; s++) {
		sf::Vector2f pix = hexToPixel((sf::Vector2f)offsetToAxial(offsetPos), s);
		spr[s].setTexture(TileFeatureS::getTexture());
		rNum = tfs.rects_[s].randomize(urng);
		spr[s].setTextureRect((sf::IntRect)tfs.rects_[s].getRect(rNum));
		spr[s].setPosition(pix + tfs.rects_[s].getPos(rNum));
	}
}
void HexMap::setTileFeature(sf::Vector2i offsetPos, const TileFeatureS& tfs, int zoom, mt19937& urng)
{
	auto& spr = hexes_(offsetPos.x, offsetPos.y).spr;
	sf::Vector2f pix = hexToPixel((sf::Vector2f)offsetToAxial(offsetPos), zoom);
	spr[zoom].setTexture(TileFeatureS::getTexture());
	int rNum = tfs.rects_[zoom].randomize(urng);
	spr[zoom].setTextureRect((sf::IntRect)tfs.rects_[zoom].getRect(rNum));
	spr[zoom].setPosition(pix + tfs.rects_[zoom].getPos(rNum));
}
void HexMap::setFeatureColor(sf::Vector2i offsetPos, const sf::Color& col)
{
	auto& spr = hexes_(offsetPos.x, offsetPos.y).spr;
	for (int s = 0; s < 3; s++) {
		spr[s].setColor(col);
	}
}
void HexMap::clearTileFeatures()
{
	for (int s = 0; s < 3; s++) {
		for (auto* h : hexes_) {
			h->spr[s].setTextureRect({ 0, 0, 0, 0 });
		}
	}
}
void HexMap::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	states.transform *= getTransform();
	states.texture = &HexTileS::getTexture();
	for (int h = chunkDrawingBounds.top; h <= chunkDrawingBounds.height; h++) {
		for (int w = chunkDrawingBounds.left; w <= chunkDrawingBounds.width; w++) {
			auto& m = (*activeBgVertices_)(w, h);
			target.draw(m, states);
		}
	}
}

void HexMap::drawEnts(sf::RenderTarget& target, sf::RenderStates states) const
{
	states.transform *= getTransform();
	for (int h = drawingBounds.top; h <= drawingBounds.height; h++) {
		for (int w = drawingBounds.left; w <= drawingBounds.width; w++) {
			auto& hex = hexes_(w, h);
			if (hex.ent != nullptr) {
				hex.ent->handlers_[zoomLevel].draw(target, states);
			}
			if (hex.spr[zoomLevel].getTextureRect().width != 0) {
				target.draw(hex.spr[zoomLevel], states);
			}
		}
	}
}

void HexMap::calculateViewArea(const sf::View& view)
{
	const sf::Vector2f& center = view.getCenter();
	sf::Vector2f size = view.getSize();
	size /= 2.0f;
	sf::Vector2f lower = center - size;
	// Add a hex onto the upper bound to prevent the edges from popping in
	sf::Vector2f upper = center + size + hexAdvance_[zoomLevel];
	lower.x /= hexAdvance_[zoomLevel].x;
	drawingBounds.left = clamp((int)lower.x, 0, (int)mapSize_.x - 1);
	lower.x /= CHUNK_SIZE;
	chunkDrawingBounds.left = clamp((int)lower.x, 0, mapSizeChunks_.x - 1);
	lower.y /= hexAdvance_[zoomLevel].y;
	drawingBounds.top = clamp((int)lower.y, 0, (int)mapSize_.y - 1);
	lower.y /= CHUNK_SIZE;
	chunkDrawingBounds.top = clamp((int)lower.y, 0, mapSizeChunks_.y - 1);
	upper.x /= hexAdvance_[zoomLevel].x;
	drawingBounds.width = clamp((int)upper.x, 0, (int)mapSize_.x - 1);
	upper.x /= CHUNK_SIZE;
	chunkDrawingBounds.width = clamp((int)upper.x, 0, mapSizeChunks_.x - 1);
	upper.y /= hexAdvance_[zoomLevel].y;
	drawingBounds.height = clamp((int)upper.y, 0, (int)mapSize_.y - 1);
	upper.y /= CHUNK_SIZE;
	chunkDrawingBounds.height = clamp((int)upper.y, 0, mapSizeChunks_.y - 1);
}

const sf::IntRect& HexMap::getViewArea() const
{
	return drawingBounds;
}

void HexMap::setViewArea(sf::IntRect viewArea)
{
	drawingBounds = viewArea;
	chunkDrawingBounds = { viewArea.left / CHUNK_SIZE, viewArea.top / CHUNK_SIZE,
		viewArea.width / CHUNK_SIZE, viewArea.height / CHUNK_SIZE};
}

const sf::IntRect& HexMap::getChunkViewArea() const
{
	return chunkDrawingBounds;
}

void HexMap::constrainView(sf::View& view)
{
	const sf::Vector2f& center = view.getCenter();
	sf::Vector2f size = view.getSize();
	size /= 2.0f;
	size -= hexAdvance_[zoomLevel] * 5.0f;
	sf::Vector2f lower = { max(center.x, size.x), max(center.y, size.y) };
	view.setCenter(lower);
	lower = hexExtent_[zoomLevel] - size;
	// The hexes are row offset orientation, so bump the upper x bound a bit to compensate
	lower.x += hexAdvance_[zoomLevel].x / 2.0f;
	sf::Vector2f upper = { min(lower.x, center.x), min(lower.y, center.y) };
	view.setCenter(upper);
}

Faction* HexMap::addFaction()
{
	factions.emplace_back();
	return &factions.back();
}

Site* HexMap::addSite(const SiteS* sSite, Faction* parent)
{
	auto& s = sites.emplace(nextSiteId, Site(sSite, this, parent)).first->second;
	nextSiteId++;
	return &s;
}

MapUnit* HexMap::addMapUnit(const MapEntityS* sEnt, Faction* parent)
{
	auto& u = units.emplace(nextUnitId, MapUnit(sEnt, this, parent)).first->second;
	nextUnitId++;
	return &u;
}

void HexMap::update(const sf::Time& timeElapsed)
{
	lifetime += timeElapsed;
	for (int h = drawingBounds.top; h <= drawingBounds.height; h++) {
		for (int w = drawingBounds.left; w <= drawingBounds.width; w++) {
			auto& hex = hexes_(w, h);
			if (hex.ent != nullptr) {
				hex.ent->handlers_[zoomLevel].updateAnimation(timeElapsed);
			}
		}
	}
}
