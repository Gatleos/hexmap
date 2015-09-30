#include <iostream>
#include <functional>
#include "config.h"
#include "TileFeature.h"
#include "clamp.h"
#include "HexMap.h"

#define ftrptr(x) unique_ptr<TileFeatureS>(new TileFeatureS(x))

array<unique_ptr<TileFeatureS>, TileFeatureS::FEATURE_NUM> TileFeatureS::feature = { {
		ftrptr("f_null"), ftrptr("f_mountain"), ftrptr("f_forest_s"), ftrptr("f_forest_m"), ftrptr("f_forest_l"),
		ftrptr("f_taiga_s"), ftrptr("f_taiga_m"), ftrptr("f_taiga_l"), ftrptr("f_taiga_snow_s"), ftrptr("f_taiga_snow_m"),
		ftrptr("f_taiga_snow_l")
	} };

RandomRect::RandomRect() :rectChance(new std::discrete_distribution <int>{ 1 }) {
	rects.resize(1U);
	pos.resize(1U);
}
int RandomRect::randomize(std::mt19937& urng) const {
	return (*rectChance)(urng);
}
const sf::FloatRect& RandomRect::getRect(int index) const {
	return rects[index];
}
const sf::Vector2f& RandomRect::getPos(int index) const {
    return pos[index];
}
void RandomRect::setToDefaultRect() {
	rects.clear();
	rects.resize(1U);
	pos.resize(1U);
	rectChance = unique_ptr<std::discrete_distribution<int>>(new std::discrete_distribution <int>{ 1 });
}
void RandomRect::loadJson(Json::Value& rData, SpriteSheet* sheet, const sf::Vector2i& hexSize) {
	// If a rect isn't provided, stick with the default
	if (rData.isNull() || rData.empty()) {
		return;
	}
	auto setPos = [hexSize, this](Json::Value& r, const sf::FloatRect& rectData) {
		if (r.size() == 3) {
			if (r[2].asString() == "center") {
				pos.emplace_back((hexSize.x - rectData.width) / 2.0f, (hexSize.y - rectData.height) / 2.0f);
			}
			else {
				cerr << "Warning: unrecognized position token \"" << r[2].asString() << "\"";
			}
		}
		else {
			pos.emplace_back(r[2].asFloat(), r[3].asFloat());
		}
	};
	std::vector<double> probs;
	rects.clear();
	pos.clear();
	const sf::FloatRect* rectData = nullptr;
	if (rData.size() > 1) { // A series of rects and their probabilities
		for (auto& r : rData) {
			int size = r.size();
			if (size < 3 || size > 4) {
				throw runtime_error("incorrect number of arguments");
			}
			// probability
			probs.push_back(clamp(r[0].asInt(), 0, 100));
			// rect
			rectData = sheet->spr(r[1].asString());
			if (rectData == nullptr) {
				throw runtime_error("couldn't find sprite");
			}
			rects.emplace_back(*rectData);
			// pos
			setPos(r, *rectData);
		}
		// OH MY GOD
		// This is just a hoop you have to jump through to get a dynamic-sized array
		// into a discrete_distribution. It's a lambda function that loops through the
		// vector of probabilities and inserts them.
		rectChance = unique_ptr<std::discrete_distribution<int>>(
			new std::discrete_distribution<int>(probs.size(), 0, probs.size(), [probs](double x)->double {
				return probs[(int)x];
			}
		));
	}
	else if(rData.size() == 1) { // A single rect, drawn 100% of the time
		rectData = sheet->spr(rData[0][1].asString());
		if (rectData == nullptr) {
			throw runtime_error("couldn't find sprite");
		}
		rects.push_back(*rectData);
		setPos(rData[0], *rectData);
	}
	else {
		setToDefaultRect();
	}
}
bool RandomRect::empty() {
	return rects.empty();
}


const unsigned char TileFeatureS::fade = static_cast<unsigned char>(100);
const sf::Texture* TileFeatureS::tex = nullptr;

const TileFeatureS& TileFeatureS::get(int t) {
	return *feature[t];
}

const sf::Texture& TileFeatureS::getTexture() {
	return *tex;
}

const TileFeatureS& TileFeatureS::get(string t) {
	for (auto& tf : feature) {
		if (tf->id_ == t) {
			return *tf;
		}
	}
	cerr << "Warning: couldn't find feature \"" << t << "\"\n";
	return *feature[TileFeatureS::NONE];
}

TileFeatureS::TileFeatureS(string id) :
vert_(sf::PrimitiveType::Quads, 4U),
id_(id),
moveCost(0) {
}

void TileFeatureS::loadJson(string filename) {
	Json::Value root = config::openJson(filename);
	if (root.begin() == root.end()) {
		cerr << "ERROR: couldn't open file \"" << filename << "\"\n";
		return;
	}
	string spriteSheet = root.get("spriteSheet", "").asString();
	SpriteSheet* sheet = RESOURCE.sh(spriteSheet);
	if (sheet == nullptr) {
		cerr << "\t(requested by \"" << filename << "\")\n";
		return;
	}
	tex = RESOURCE.tex(sheet->getImageName());
	if (tex == nullptr) {
		cerr << "\t(requested by \"" << filename << "\")\n";
		return;
	}
	// Cycle through defined feature types; make sure to skip f_null!
	for (int a = 1; a < TileFeatureS::FEATURE_NUM; a++) {
		auto& feat = TileFeatureS::feature[a];
		Json::Value fdata = root.get(feat->id_, Json::Value::null);
		if (fdata.isNull()) { // feature not found
			cerr << "Warning: feature type \"" << feat->id_ << "\" was not found in \"" << filename << "\"\n";
			continue;
		}
		string element;
		try {
			//name
			element = "name";
			feat->name_ = fdata.get(element, "").asString();
			// rect
			for (int i = 0; i < 3; i++) {
				element = config::rectNames[i];
				Json::Value rData = fdata.get(element, Json::Value::null);
				feat->rects_[i].loadJson(rData, sheet, HexMap::getHexSize(i));
			}
			// moveCost
			element = "moveCost";
			feat->moveCost = fdata.get(element, 0).asInt();
		}
		catch (runtime_error e) { // report the error with the name of the object and member
			// make sure we have placeholder drawing rects!
			for (int r = 0; r < 3; r++) {
				if (feat->rects_[r].empty()) {
					feat->rects_[r].setToDefaultRect();
				}
			}
			cerr << "[" << filename << ", " << feat->id_ << ", " << element << "] " << e.what() << "\n";
		}
	}
}
