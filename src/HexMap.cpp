#include <fstream>
#include <sstream>
#include <iostream>
#include "HexMap.h"
#include "simplexnoise.h"
#include "lerp.h"
#include "clamp.h"
#include "Site.h"
#include "MapEntity.h"


const sf::Vector2i HexMap::directions[dir::SIZE] = { { 1, -1 }, { 0, -1 }, { -1, 0 }, { -1, 1 }, { 0, 1 }, { 1, 0 } };

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

HexMap::FloodFill::FloodFill(int sizeLimit, VectorSet* seen, HexMap* hm, std::function<bool(HexTile&)>& condition) :
totalSize_(0), container_(nullptr), sizeLimit_(sizeLimit),
seen_(seen), hm_(hm), condition_(&condition)
{
}
bool HexMap::FloodFill::iterate()
{
	for (auto& f : frontier_) {
		hm_->clipToBounds(neighbors(f, adj_));
		for (auto it = adj_.begin(); it != adj_.end(); it++) {
			if (seen_->find(*it) != seen_->end()) {
				continue;
			}
			seen_->insert(*it);
			sf::Vector2i off = axialToOffset(*it);
			if (!(*condition_)(hm_->hexes_(off.x, off.y))) {
				continue;
			}
			if (container_ != nullptr) {
				container_->insert(*it);
			}
			newFrontier_.push_back(*it);
			totalSize_++;
			if (totalSize_ >= sizeLimit_) {
				return false;
			}
		}
	}
	if (newFrontier_.empty()) {
		return false;
	}
	frontier_.clear();
	std::swap(frontier_, newFrontier_);
	return true;
}
void HexMap::FloodFill::run()
{
	while (iterate()) {}
}
int HexMap::FloodFill::getSize()
{
	return totalSize_;
}
void HexMap::FloodFill::initFill(sf::Vector2i start)
{
	frontier_.clear();
	newFrontier_.clear();
	if (!(*condition_)(hm_->getAxial(start.x, start.y))) {
		return;
	}
	start_ = start;
	totalSize_ = 1;
	container_->insert(start);
	frontier_.push_back(start);
	seen_->insert(start);
}
void HexMap::FloodFill::setOutputContainer(VectorSet* container)
{
	container_ = container;
}

HexMap::HexMap() :
activeBgVertices_(&bgVertices_[0]),
nextUnitId(0),
nextSiteId(0)
{
}

int HexMap::heuristic(sf::Vector2i& a, sf::Vector2i& b)
{
	return (int)((abs(a.x - b.x) + abs(a.x + a.y - b.x - b.y) + abs(a.y - b.y)) / 2);
	//return abs(a.x - b.x) + abs(a.y - b.y);
}

int HexMap::moveCost(sf::Vector2i& current, sf::Vector2i& next)
{
	auto& h = getAxial(next.x, next.y);
	return h.hts->moveCost + (h.tfs == nullptr ? 0 : h.tfs->moveCost);
}

std::deque<sf::Vector2i>& HexMap::getPath(std::deque<sf::Vector2i>& path, sf::Vector2i startAxial, sf::Vector2i goalAxial)
{
	if (!isAxialInBounds(startAxial) || !getAxial(startAxial.x, startAxial.y).hts->FLAGS[HexTileS::WALKABLE] || !isAxialInBounds(goalAxial) || !getAxial(goalAxial.x, goalAxial.y).hts->FLAGS[HexTileS::WALKABLE]) {
		return path;
	}
	// traceback
	unordered_map<sf::Vector2i, sf::Vector2i, Vector2iHash> cameFrom;
	// combined cost of tiles leading to this one
	unordered_map<sf::Vector2i, int, Vector2iHash> costSoFar;
	// new tiles to query
	multimap<int, sf::Vector2i> frontier;
	sf::Vector2i start = startAxial;
	sf::Vector2i goal = goalAxial;
	VectorSet n;
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
			if (!isAxialInBounds(next) || !getAxial(next.x, next.y).hts->FLAGS[HexTileS::WALKABLE]) {
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

int HexMap::getPathCost(sf::Vector2i startAxial, sf::Vector2i goalAxial)
{
	if (!isAxialInBounds(startAxial) || !getAxial(startAxial.x, startAxial.y).hts->FLAGS[HexTileS::WALKABLE] || !isAxialInBounds(goalAxial) || !getAxial(goalAxial.x, goalAxial.y).hts->FLAGS[HexTileS::WALKABLE]) {
		return 9001;
	}
	// combined cost of tiles leading to this one
	unordered_map<sf::Vector2i, int, Vector2iHash> costSoFar;
	// new tiles to query
	multimap<int, sf::Vector2i> frontier;
	sf::Vector2i start = startAxial;
	sf::Vector2i goal = goalAxial;
	VectorSet n;
	frontier.insert(std::pair<int, sf::Vector2i>(0, start));
	costSoFar[start] = 0;
	while (!frontier.empty()) {
		auto current = frontier.begin()->second;
		frontier.erase(frontier.begin());
		if (current == goal) {
			break;
		}
		n.clear();
		for (auto next : neighbors(current, n)) {
			if (!isAxialInBounds(next) || !getAxial(next.x, next.y).hts->FLAGS[HexTileS::WALKABLE]) {
				continue;
			}
			int newCost = costSoFar[current] + moveCost(current, next);
			if (!costSoFar.count(next) || newCost < costSoFar[next]) {
				costSoFar[next] = newCost;
				int priority = newCost + heuristic(next, goal);
				frontier.insert(std::pair<int, sf::Vector2i>(priority, next));
			}
		}
	}
	auto distance = costSoFar.find(goal);
	if (distance == costSoFar.end()) {
		// A path was not found
		return 9001;
	}
	return distance->second;
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

void HexMap::updateCursorPos(sf::Vector2i cursorPos)
{
	if (cursorPos_ == cursorPos) {
		return;
	}
	VectorSet n;
	if (isAxialInBounds(cursorPos_)) {
		n.emplace(cursorPos_);
	}
	clipToBounds(neighbors(cursorPos_, n));
	for (auto& h : n) {
		setFeatureFade(axialToOffset(h), false);
	}
	cursorPos_ = cursorPos;
	n.clear();
	if (isAxialInBounds(cursorPos_)) {
		n.emplace(cursorPos_);
	}
	clipToBounds(neighbors(cursorPos_, n));
	for (auto& h : n) {
		setFeatureFade(axialToOffset(h), true);
	}
}

float HexMap::distAxial(sf::Vector2f& a, sf::Vector2f& b)
{
	return (abs(a.x - b.x) + abs(a.x + a.y - b.x - b.y) + abs(a.y - b.y)) / 2;
}
float HexMap::distAxial(sf::Vector2i& a, sf::Vector2i& b)
{
	return (float)((abs(a.x - b.x) + abs(a.x + a.y - b.x - b.y) + abs(a.y - b.y)) / 2);
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

bool HexMap::isAxialInBounds(sf::Vector2i axialPos) const
{
	axialPos.x -= -floorf(axialPos.y / 2.0);
	if (axialPos.x < 0 || axialPos.x >= mapSize_.x || axialPos.y < 0 || axialPos.y >= mapSize_.y) {
		return false;
	}
	return true;
}

bool HexMap::isOffsetInBounds(sf::Vector2i offsetPos) const
{
	if (offsetPos.x < 0 || offsetPos.x >= mapSize_.x || offsetPos.y < 0 || offsetPos.y >= mapSize_.y) {
		return false;
	}
	return true;
}

sf::Vector2i HexMap::neighbor(sf::Vector2i centerAxial, int dir)
{
	return sf::Vector2i(centerAxial.x + directions[dir].x, centerAxial.y + directions[dir].y);
}
VectorSet& HexMap::neighbors(sf::Vector2i centerAxial, VectorSet& n)
{
	for (int x = 0; x < 6; x++) {
		n.insert(sf::Vector2i(centerAxial.x + directions[x].x, centerAxial.y + directions[x].y));
	}
	return n;
}
VectorSet& HexMap::area(sf::Vector2i centerAxial, int radius, VectorSet& n)
{
	//var results = []
	//for each - N ≤ dx ≤ N :
	//	for each max(-N, -dx - N) ≤ dy ≤ min(N, -dx + N) :
	//		var dz = -dx - dy
	//		results.append(cube_add(center, Cube(dx, dy, dz)))
	cubepoint cp((float)centerAxial.x, (float)radius, (float)centerAxial.y);
	cubepoint d;
	cubepoint np;
	for (d.x = -cp.y; d.x <= cp.y; d.x++) {
		for (d.y = std::max(-cp.y, -d.x - cp.y); d.y <= std::min(cp.y, -d.x + cp.y); d.y++) {
			d.z = -d.x - d.y;
			np = d + cp;
			n.insert({ (int)np.x, (int)np.z });
		}
	}
	return n;
}
VectorSet& HexMap::ring(sf::Vector2i centerAxial, int radius, VectorSet& n)
{
	if (radius == 0) {
		n.insert(centerAxial);
		return n;
	}
	centerAxial.y += radius;
	for (int d = 0; d < 6; d++) {
		for (int r = 0; r < radius; r++) {
			n.insert(centerAxial);
			centerAxial = neighbor(centerAxial, d);
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
VectorSet& HexMap::clipToBounds(VectorSet& boundedAxial)
{
	for (auto it = boundedAxial.begin(); it != boundedAxial.end();) {
		if (!isAxialInBounds(*it)) {
			it = boundedAxial.erase(it);
			continue;
		}
		it++;
	}
	return boundedAxial;
}
VectorSet& HexMap::clip(VectorSet& listAxial, std::function<bool(HexTile&)>& condition)
{
	for (auto it = listAxial.begin(); it != listAxial.end();) {
		if (!condition(getAxial(it->x, it->y))) {
			it = listAxial.erase(it);
			continue;
		}
		it++;
	}
	return listAxial;
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
VectorSet& HexMap::floodSelect(VectorSet& fill, sf::Vector2i start, int sizeLimit, std::function<bool(HexTile&)>& condition)
{
	VectorSet seen;
	FloodFill f(sizeLimit, &seen, this, condition);
	f.setOutputContainer(&fill);
	f.initFill(start);
	f.run();
	return fill;
}
std::vector<VectorSet>& HexMap::floodSelectParallel(std::vector<VectorSet>& fill, std::vector<sf::Vector2i>& start, int sizeLimit, std::function<bool(HexTile&)>& condition)
{
	VectorSet seen;
	// pointers to the VectorSets, so we can remove them
	// as they finish filling
	std::vector<FloodFill> fillPtr;
	int index = 0;
	for (auto& f : fill) {
		fillPtr.emplace_back(sizeLimit, &seen, this, condition);
		fillPtr.back().setOutputContainer(&f);
		fillPtr.back().initFill(start[index]);
		index++;
	}
	while (!fillPtr.empty()) {
		for (auto it = fillPtr.begin(); it != fillPtr.end();) {
			if (!it->iterate()) { // the FloodFill has either reached capacity or run out of tiles
				it = fillPtr.erase(it);
				continue;
			}
			it++;
		}
	}
	return fill;
}
int HexMap::floodSelectSize(sf::Vector2i start, std::function<bool(HexTile&)>& condition)
{
	VectorSet seen;
	FloodFill f(std::numeric_limits<int>::max(), &seen, this, condition);
	f.initFill(start);
	f.run();
	return f.getSize();
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
				h->tfs = hts.features[0];
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
void HexMap::pushTileColor(sf::Vector2i offsetPos, sf::Color col)
{
	hexes_(offsetPos.x, offsetPos.y).color.push(col);
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
void HexMap::popTileColor(sf::Vector2i offsetPos)
{
	auto& hex = hexes_(offsetPos.x, offsetPos.y);
	hex.color.pop();
	const sf::Color* col = nullptr;
	if (hex.color.empty()) {
		col = &sf::Color::White;
	}
	else {
		col = &hex.color.top();
	}
	sf::Vector2i chunkPos = { (offsetPos.x / CHUNK_SIZE), (offsetPos.y / CHUNK_SIZE) };
	int index = ((offsetPos.y % CHUNK_SIZE) * CHUNK_SIZE + (offsetPos.x % CHUNK_SIZE)) * 4;
	for (int a = 0; a < 3; a++) {
		sf::VertexArray& chunk = bgVertices_[a](chunkPos.x, chunkPos.y);
		chunk[index].color = *col;
		chunk[index + 1].color = *col;
		chunk[index + 2].color = *col;
		chunk[index + 3].color = *col;
	}
}
void HexMap::setTileFeature(sf::Vector2i offsetPos, const TileFeatureS& tfs, mt19937& urng)
{
	auto& hex = hexes_(offsetPos.x, offsetPos.y);
	int rNum = 0;
	hex.tfs = &tfs;
	for (int s = 0; s < 3; s++) {
		sf::Vector2f pix = hexToPixel((sf::Vector2f)offsetToAxial(offsetPos), s);
		hex.spr[s].setTexture(TileFeatureS::getTexture());
		rNum = tfs.rects_[s].randomize(urng);
		hex.spr[s].setTextureRect((sf::IntRect)tfs.rects_[s].getRect(rNum));
		hex.spr[s].setPosition(pix + tfs.rects_[s].getPos(rNum));
	}
}
void HexMap::setTileFeature(sf::Vector2i offsetPos, const TileFeatureS& tfs, int zoom, mt19937& urng)
{
	auto& hex = hexes_(offsetPos.x, offsetPos.y);
	hex.tfs = &tfs;
	sf::Vector2f pix = hexToPixel((sf::Vector2f)offsetToAxial(offsetPos), zoom);
	hex.spr[zoom].setTexture(TileFeatureS::getTexture());
	int rNum = tfs.rects_[zoom].randomize(urng);
	hex.spr[zoom].setTextureRect((sf::IntRect)tfs.rects_[zoom].getRect(rNum));
	hex.spr[zoom].setPosition(pix + tfs.rects_[zoom].getPos(rNum));
}
void HexMap::setFeatureColor(sf::Vector2i offsetPos, const sf::Color& col)
{
	auto& spr = hexes_(offsetPos.x, offsetPos.y).spr;
	for (int s = 0; s < 3; s++) {
		spr[s].setColor(col);
	}
}
void HexMap::setFeatureFade(sf::Vector2i offsetPos, bool fade)
{
	auto& hex = hexes_(offsetPos.x, offsetPos.y);
	if (hex.ent != nullptr) {
		fade = true;
	}
	for (int s = 0; s < 3; s++) {
		sf::Color f = hex.spr[s].getColor();
		f.a = fade ? TileFeatureS::fade : 255;
		hex.spr[s].setColor(f);
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
	parent->sites.insert(nextSiteId);
	s.id = nextSiteId;
	nextSiteId++;
	return &s;
}

MapUnit* HexMap::addMapUnit(const MapEntityS* sEnt, Faction* parent)
{
	auto& u = units.emplace(nextUnitId, MapUnit(sEnt, this, parent)).first->second;
	parent->units.insert(nextUnitId);
	u.id = nextUnitId;
	nextUnitId++;
	return &u;
}

void HexMap::clearEntities()
{
	factions.clear();
	for (auto& s : sites) {
		sf::Vector2i& pos = s.second.pos;
		getAxial(pos.x, pos.y).ent = nullptr;
		setFeatureColor(axialToOffset(pos), sf::Color::White);
	}
	sites.clear();
	for (auto& u : units) {
		sf::Vector2i& pos = u.second.pos;
		getAxial(pos.x, pos.y).ent = nullptr;
		setFeatureColor(axialToOffset(pos), sf::Color::White);
	}
	units.clear();
}

void HexMap::setEntity(sf::Vector2i offsetPos, MapEntity* ent)
{
	getOffset(offsetPos.x, offsetPos.y).ent = ent;
	// We use false here because entity presence will override it
	setFeatureFade(offsetPos, false);
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
