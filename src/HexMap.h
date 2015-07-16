#ifndef HEXMAP_H
#define HEXMAP_H

#include <deque>
#include <set>
#include "Compare.h"
#include "Site.h"
#include "Faction.h"
#include "HexTile.h"
#include "Array2d.h"

typedef set<sf::Vector2i, Vector2iCompare> VectorSet;

enum { DIR_EAST, DIR_NORTHEAST, DIR_NORTHWEST, DIR_WEST, DIR_SOUTHWEST, DIR_SOUTHEAST };
static const sf::Vector2i directions[] = { { 1, 0 }, { 1, -1 }, { 0, -1 }, { -1, 0 }, { -1, 1 }, { 0, 1 } };

enum { SIMPLEX_HEIGHT, SIMPLEX_TEMP };

typedef pair<sf::Vector2i, sf::FloatRect> Road;
extern array<Road, 6> roadSprites;
extern vector<sf::FloatRect> mountainSprites;

class Region
{
public:
	unsigned int size;
	sf::Vector2i coord;
	Region(unsigned int sizeSet, sf::Vector2i coordSet) : size(sizeSet), coord(coordSet) {}
};

class HexMap : public sf::Drawable, public sf::Transformable
{
	struct cubepoint
	{
		float x;
		float y;
		float z;
		cubepoint(){ x = 0; y = 0; z = 0; }
		cubepoint(float cx, float cy, float cz){ x = cx; y = cy; z = cz; }
		cubepoint operator+= (cubepoint& cp){ x += cp.x; y += cp.y; z += cp.z; return *this; }
		cubepoint operator+ (cubepoint& cp){ return cubepoint(*this) += cp; }
	};
	static const int CHUNK_SIZE;
	static const int CHUNK_SQUARED;
	static int hexRad_[3];
	static sf::Vector2i hexSize_[3];
	static sf::Vector2f hexAdvance_[3];
	static sf::Vector2f mapOrigin_[3];
	sf::Vector2f hexExtent_[3];
	sf::Texture* tex_;
	sf::IntRect drawingBounds;
	sf::IntRect chunkDrawingBounds;
	// Properties
	sf::Vector2i mapSize_;
	sf::Vector2i mapSizeChunks_;
	// 0-2, 0 is the closest
	int zoomLevel;
	// Contents
	Array2D<HexTile> hexes_;
	array<Array2D<sf::VertexArray>, ZOOM_LEVELS> bgVertices_;
	Array2D<sf::VertexArray>* activeBgVertices_;
	// Pathfinding
	unordered_map<sf::Vector2i, sf::Vector2i, Vector2iHash> cameFrom;
	unordered_map<sf::Vector2i, int, Vector2iHash> costSoFar;
	multimap<int, sf::Vector2i> frontier;
	bool walkable(sf::Vector2i& c);
	int heuristic(sf::Vector2i& a, sf::Vector2i& b);
	int moveCost(sf::Vector2i& current, sf::Vector2i& next);
	// MapEntities
	vector<Faction> factions;
	unsigned int nextSiteId;
	map<int, Site> sites;
	unsigned int nextUnitId;
	map<int, MapUnit> units;
	// MapGen
	uniform_int_distribution<int> xRange;
	uniform_int_distribution<int> yRange;
public:
	// Return a list of hex tile neighbors (axial)
	static deque<sf::Vector2i>& neighbors(sf::Vector2i h, deque<sf::Vector2i>& n);
	// Return a list of hex tiles contained in a given radius (axial)
	static VectorSet& area(sf::Vector2i h, int radius, VectorSet& n);
	// odd-r offset -> axial coordinate
	static sf::Vector2f offsetToAxial(sf::Vector2f v);
	// axial coordinate -> odd-r offset
	static sf::Vector2f axialToOffset(sf::Vector2f v);
	// odd-r offset -> axial coordinate
	static sf::Vector2i offsetToAxial(sf::Vector2i v);
	// axial coordinate -> odd-r offset
	static sf::Vector2i axialToOffset(sf::Vector2i v);
	//
	HexMap();
	// Create hex tiles and vertices (for drawing), and initialize size values
	void init(int width, int height);
	const sf::Vector2i& getMapSize() const;
	deque<sf::Vector2i>& getPath(deque<sf::Vector2i>& path, sf::Vector2i startAxial, sf::Vector2i goalAxial);
	static const int& getHexRadius(int zoom);
	static const sf::Vector2i& getHexSize(int zoom);
	static const sf::Vector2f& getHexAdvance(int zoom);
	static const sf::Vector2f& getMapOrigin(int zoom);
	deque<sf::Vector2i>& neighborsBounded(sf::Vector2i posAxial, deque<sf::Vector2i>& n);
	// Measurement
	// Round a floating point axial coordinate to the nearest hex
	static sf::Vector2f roundHex(sf::Vector2f hex);
	// Convert axial hex coordinate to local pixel coordinate
	sf::Vector2f hexToPixel(sf::Vector2f hex) const;
	// Convert axial hex coordinate to local pixel coordinate
	sf::Vector2i hexToPixel(sf::Vector2i hex) const;
	// Convert axial hex coordinate to local pixel coordinate
	sf::Vector2f hexToPixel(sf::Vector2f hex, int zoom) const;
	// Convert axial hex coordinate to local pixel coordinate
	sf::Vector2i hexToPixel(sf::Vector2i hex, int zoom) const;
	// Convert local pixel coordinate to axial hex coordinate
	sf::Vector2f pixelToHex(sf::Vector2f pixel) const;
	// Check if axial coordinate is within map bounds
	bool isAxialInBounds(sf::Vector2i posAxial);
	// Check if offset coordinate is within map bounds
	bool isOffsetInBounds(sf::Vector2i posOffset);
	// Access
	// Retrieve hex based on coordinate (axial)
	HexTile& getAxial(int x, int y);
	// Retrieve hex based on coordinate (offset)
	HexTile& getOffset(int x, int y);
	void floodSelect(VectorSet& fill, int minHeight, int maxHeight);
	// Drawing
	// 0-2, 0 is the closest
	int getZoomLevel();
	// 0-2, 0 is the closest
	void setZoomLevel(int zoom);
	// The amount that a hex tile's center is offset from the previous one;
	// use this one to measure screen space in hexes, not hexSize!
	const sf::Vector2f getHexAdvance();
	// Texture used to draw hex tiles
	void setTexture(sf::Texture& tex);
	void setTile(sf::Vector2i offsetPos, const HexTileS& hts, mt19937& urng);
	void setAllTiles(const HexTileS& hts, mt19937& urng);
	void setTileColor(sf::Vector2i offsetPos, sf::Color col);
	void setTileFeature(sf::Vector2i offsetPos, const TileFeatureS& tfs, mt19937& urng);
	void setTileFeature(sf::Vector2i offsetPos, const TileFeatureS& tfs, int zoom, mt19937& urng);
	void setFeatureColor(sf::Vector2i offsetPos, const sf::Color& col);
	void clearTileFeatures();
	void draw(sf::RenderTarget& target, sf::RenderStates states) const;
	// Determine which map chunks to draw based on a view, culling the rest
	void calculateViewArea(const sf::View& view);
	const sf::IntRect& getViewArea() const;
	void setViewArea(sf::IntRect viewArea);
	const sf::IntRect& getChunkViewArea() const;
	// Keep a view from moving too far outside the map boundaries
	void constrainView(sf::View& view);
	// MapEntities
	Faction* addFaction();
	Site* addSite(const SiteS* sSite, Faction* parent);
	MapUnit* addMapUnit(const MapEntityS* sEnt, Faction* parent);
	void update(const sf::Time& timeElapsed);
	// MapGen
	void generateBiomes(mt19937& urng);
	void generateMountainRange(mt19937& urng);
	void findRegions();
};

float distHex(sf::Vector2f& a, sf::Vector2f& b);
float distHex(sf::Vector2i& a, sf::Vector2i& b);
void polarToCartesian(sf::Vector2f& p);
sf::Vector2f roundvf(sf::Vector2f p);

#endif
