#ifndef HEXMAP_H
#define HEXMAP_H

#include <deque>
#include <set>
#include <functional>
#include "Compare.h"
#include "Site.h"
#include "Faction.h"
#include "HexTile.h"
#include "Array2d.h"
#include "MapUnit.h"

typedef set<sf::Vector2i, Vector2iCompare> VectorSet;


enum { SIMPLEX_HEIGHT, SIMPLEX_TEMP };

typedef pair<sf::Vector2i, sf::FloatRect> Road;
extern array<Road, 6> roadSprites;
extern vector<sf::FloatRect> mountainSprites;

class Region {
public:
	unsigned int size;
	sf::Vector2i coord;
	Region(unsigned int sizeSet, sf::Vector2i coordSet) : size(sizeSet), coord(coordSet) {}
};

#define HEXMAP HexMap::instance()

class HexMap : public sf::Drawable, public sf::Transformable {
public:
	enum dir{ NORTHEAST, NORTHWEST, WEST, SOUTHWEST, SOUTHEAST, EAST, SIZE };
	static const sf::Vector2i directions[dir::SIZE];
private:
	struct cubepoint {
		float x;
		float y;
		float z;
		cubepoint(){ x = 0; y = 0; z = 0; }
		cubepoint(float cx, float cy, float cz){ x = cx; y = cy; z = cz; }
		cubepoint operator+= (cubepoint& cp){ x += cp.x; y += cp.y; z += cp.z; return *this; }
		cubepoint operator+ (cubepoint& cp){ return cubepoint(*this) += cp; }
	};
	class FloodFill {
		sf::Vector2i start_;
		int totalSize_;
		// set to fill
		VectorSet* container_;
		int sizeLimit_;
		// stores neighboring tiles
		VectorSet adj_;
		// new tiles to query
		vector<sf::Vector2i> frontier_;
		// temporary frontier to populate and swap with the old
		vector<sf::Vector2i> newFrontier_;
		// which tiles have we already checked?
		VectorSet* seen_;
		// parent map
		HexMap* hm_;
		std::function<bool(HexTile&)>* condition_;
	public:
		FloodFill(int sizeLimit, VectorSet* seen, HexMap* hm, std::function<bool(HexTile&)>& condition);
		bool iterate();
		void run();
		void clear();
		int getSize();
		void initFill(sf::Vector2i start);
		void setOutputContainer(VectorSet* container);
	};

	static const int CHUNK_SIZE;
	static const int CHUNK_SQUARED;
	static int hexRad_[3];
	static sf::Vector2i hexSize_[3];
	static sf::Vector2f hexAdvance_[3];
	static sf::Vector2f mapOrigin_[3];
	sf::Vector2i mapSize_;
	sf::Vector2i mapSizeChunks_;
	sf::Vector2f hexExtent_[3];
	sf::Vector2i cursorPos_;
	sf::IntRect drawingBounds;
	sf::IntRect chunkDrawingBounds;
	// 0-2, 0 is the closest
	int zoomLevel;
	// How long has the map been updating?
	sf::Time lifetime;
	// HexTile data
	Array2D<HexTile> hexes_;
	// Drawing vertices for tiles
	array<Array2D<sf::VertexArray>, ZOOM_LEVELS> bgVertices_;
	// Currently used vertices
	Array2D<sf::VertexArray>* activeBgVertices_;
	std::deque<sf::Vector2f> land;
	list<Faction> factions;
	unsigned int nextSiteId;
	map<int, Site> sites;
	unsigned int nextUnitId;
	map<int, MapUnit> units;
	// Distribution for rng along map's x coordinate
	uniform_int_distribution<int> xRange;
	// Distribution for rng along map's y coordinate
	uniform_int_distribution<int> yRange;
	// private constructor
	HexMap();
public:
	// Get singleton instance
	static HexMap& instance();
	static float cloudSpeed;
	sf::View view;
	bool isGrabbed;
	sf::Shader cloudShader;
	// Create hex tiles and vertices (for drawing), and initialize size values
	void init(int width, int height);
	const sf::Vector2i& getMapSize() const;
	const sf::Time& getLifetime();
	void updateCursorPos(sf::Vector2i cursorPos);

	/////////////////
	// Pathfinding //
	/////////////////

private:
	int heuristic(sf::Vector2i& a, sf::Vector2i& b);
	int moveCost(sf::Vector2i& current, sf::Vector2i& next);
public:
	deque<sf::Vector2i>& getPath(deque<sf::Vector2i>& path, sf::Vector2i startAxial, sf::Vector2i goalAxial);
	int getPathCost(sf::Vector2i startAxial, sf::Vector2i goalAxial);

	/////////////////
	// Measurement //
	/////////////////

	// Get exact distance between axial coordinates
	static float distAxial(sf::Vector2f& a, sf::Vector2f& b);
	// Get exact distance between axial coordinates
	static float distAxial(sf::Vector2i& a, sf::Vector2i& b);
	// Round a floating point axial coordinate to the nearest hex
	static sf::Vector2f roundHex(sf::Vector2f hex);
	// odd-r offset -> axial coordinate
	static sf::Vector2f offsetToAxial(sf::Vector2f v);
	// axial coordinate -> odd-r offset
	static sf::Vector2f axialToOffset(sf::Vector2f v);
	// odd-r offset -> axial coordinate
	static sf::Vector2i offsetToAxial(sf::Vector2i v);
	// axial coordinate -> odd-r offset
	static sf::Vector2i axialToOffset(sf::Vector2i v);
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
	bool isAxialInBounds(sf::Vector2i posAxial) const;
	// Check if offset coordinate is within map bounds
	bool isOffsetInBounds(sf::Vector2i posOffset) const;
	// Return the tile coordinate in the given direction (use HexMap::dir and HexMap::directions)
	static sf::Vector2i neighbor(const sf::Vector2i& centerAxial, int dir);
	// Return a list of hex tile neighbors (axial)
	static VectorSet& neighbors(const sf::Vector2i& centerAxial, VectorSet& neighbors);
	// Return a list of hex tiles contained in a given radius (axial)
	static VectorSet& area(sf::Vector2i centerAxial, int radius, VectorSet& n);
	// Return a list of hex tiles with the given distance (radius) from center (axial)
	static VectorSet& ring(sf::Vector2i centerAxial, int radius, VectorSet& n);
	// Remove all coordinates which fall outside the map
	VectorSet& clipToBounds(VectorSet& boundedAxial);
	// Remove all coordinates whose hexes do not meet condition()
	VectorSet& clip(VectorSet& listAxial, std::function<bool(HexTile&)>& condition);

	////////////
	// Access //
	////////////

	// Retrieve hex based on coordinate (axial)
	HexTile& getAxial(int x, int y);
	// Retrieve hex based on coordinate (offset)
	HexTile& getOffset(int x, int y);
	// Flood select, stopping at tiles which do not meet condition()
	VectorSet& floodSelect(VectorSet& fill, sf::Vector2i start, int sizeLimit, std::function<bool(HexTile&)>& condition);
	// Create a vector of non-intersecting flood select regions simultaneously,
	// stopping at tiles which do not meet condition()
	std::vector<VectorSet>& floodSelectParallel(std::vector<VectorSet>& fill, std::vector<sf::Vector2i>& start, int sizeLimit, std::function<bool(HexTile&)>& condition);
	// Return size of the flood region, stopping at tiles which do not meet condition()
	int floodSelectSize(sf::Vector2i start, std::function<bool(HexTile&)>& condition);

	/////////////
	// Drawing //
	/////////////

	static const int& getHexRadius(int zoom);
	static const sf::Vector2i& getHexSize(int zoom);
	// The amount in pixels that a hex tile's center is offset from the previous one;
	// use this one to measure screen space in hexes, not hexSize!
	static const sf::Vector2f& getHexAdvance(int zoom);
	// The origin (drawing offset) for the whole map
	static const sf::Vector2f& getMapOrigin(int zoom);
	// 0-2, 0 is the closest
	int getZoomLevel();
	// 0-2, 0 is the closest
	void setZoomLevel(int zoom);
	void setTile(sf::Vector2i posOffset, const HexTileS& hts, mt19937& urng);
	void setAllTiles(const HexTileS& hts, mt19937& urng);
	void pushTileColor(sf::Vector2i posOffset, sf::Color col);
	void popTileColor(sf::Vector2i posOffset);
	void setTileFeature(sf::Vector2i posOffset, const TileFeatureS& tfs, mt19937& urng);
	void setTileFeature(sf::Vector2i posOffset, const TileFeatureS& tfs, int zoom, mt19937& urng);
	void setFeatureColor(sf::Vector2i posOffset, const sf::Color& col);
	// Fade out the feature on this tile? Overridden to true automatically
	// if there is an entity on the tile
	void setFeatureFade(sf::Vector2i posOffset, bool fade);
	void clearTileFeatures();
	void draw(sf::RenderTarget& target, sf::RenderStates states = sf::RenderStates::Default) const;
	void drawEnts(sf::RenderTarget& target, sf::RenderStates states = sf::RenderStates::Default) const;
	// Determine which map chunks to draw based on a view, culling the rest
	void calculateViewArea(const sf::View& view);
	const sf::IntRect& getViewArea() const;
	void setViewArea(sf::IntRect viewArea);
	const sf::IntRect& getChunkViewArea() const;
	// Keep a view from moving too far outside the map boundaries
	void constrainView(sf::View& view);

	/////////////////
	// MapEntities //
	/////////////////

	Faction* playerFaction();
	Faction* addFaction();
	Site* addSite(const SiteS* sSite, Faction* parent);
	MapUnit* addMapUnit(const MapEntityS* sEnt, Faction* parent);
	void clearEntities();
	void setEntity(sf::Vector2i posOffset, MapEntity* ent);
	void update(const sf::Time& timeElapsed);
	void advanceTurn();

	////////////
	// MapGen //
	////////////

	void generateBiomes(mt19937& urng);
	void generateMountainRange(mt19937& urng);
	void findRegions();
	void placeSites(mt19937& urng);
};

void polarToCartesian(sf::Vector2f& p);
sf::Vector2f roundvf(sf::Vector2f p);

#endif
