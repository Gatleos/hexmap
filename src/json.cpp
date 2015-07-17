#include <iostream>
#include <fstream>
#include "json.h"
#include "HexMap.h"
#include "lerp.h"
#include "Species.h"
#include "ResourceLoader.h"

array<const char*, 6> config::roadTypes = { "r_e", "r_ne", "r_nw", "r_w", "r_sw", "r_se" };
const array<const char*, ZOOM_LEVELS> config::rectNames = { "full", "half", "quarter" };
const array<const char*, ZOOM_LEVELS> config::featureNames = { "featureFull", "featureHalf", "featureQuarter" };

Json::Value config::openJson(string file) {
	Json::Value root;
	Json::Reader reader;
	filebuf fb;
	fb.open(file, ios::in);
	istream config_file(&fb);
	if (!reader.parse(config_file, root))
	{
		cerr << "Failed to parse " << file << "\n"
			<< reader.getFormattedErrorMessages();
	}
	return root;
}

void config::loadAllJson() {
	cerr << "\nBegin Json Parsing\n------------------\n";
	TileFeatureS::loadJson("data/feature.json");
	HexTileS::loadJson("data/terrain.json");
	Species::loadJson("data/species.json");
	SiteS::loadJson("data/sites.json");
	cerr << "------------\nParsing Done\n\n";
}
