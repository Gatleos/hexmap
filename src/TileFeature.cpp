#include <iostream>
#include "json.h"
#include "TileFeature.h"
#include "clamp.h"

#define ftrptr(x) unique_ptr<TileFeatureS>(new TileFeatureS(x))

array<unique_ptr<TileFeatureS>, TileFeatureS::FEATURE_NUM> TileFeatureS::feature = { {
		ftrptr("f_null"), ftrptr("f_mountain"), ftrptr("f_forest_l")
	} };

RandomRect::RandomRect() :active(false)
{
	rect.emplace(make_pair(0, sf::FloatRect()));
}
const sf::FloatRect* RandomRect::getRect(mt19937& urng) const
{
	if (!active) {
		return &rect.begin()->second;
	}
	int r = rng::getInt(probTotal - 1, urng);
	for (auto f = rect.rbegin(); f != rect.rend(); f++) {
		if (r < f->first) {
			return &f->second;
		}
		r -= f->first;
	}
	return nullptr;
}
bool RandomRect::operator!()
{
	return !active;
}
void RandomRect::setToDefaultRect()
{
	rect.clear();
	rect.emplace(make_pair(0, sf::FloatRect()));
	active = false;
}
void RandomRect::loadJson(Json::Value& rdata, SpriteSheet* sheet)
{
	// If a rect isn't provided, stick with the default
	if (rdata.isNull()) {
		return;
	}
	rect.clear();
	const sf::FloatRect* rectData = nullptr;
	probTotal = 0;
	if (rdata.isArray()) { // A series of rects and their probabilities
		active = true;
		if (rdata.size() & 1) {
			throw runtime_error("incorrect number of arguments");
		}
		for (int f = 0; f < rdata.size(); f += 2) {
			rectData = sheet->spr(rdata[f].asString());
			if (rectData == nullptr) {
				throw runtime_error("couldn't find sprite");
			}
			int prob = clamp(rdata[f + 1].asInt(), 0, 100);
			probTotal += prob;
			rect.emplace(make_pair(clamp(rdata[f + 1].asInt(), 0, 100), *rectData));
		}
	}
	else { // A single rect, drawn 100% of the time
		active = false;
		rectData = sheet->spr(rdata.asString());
		if (rectData == nullptr) {
			throw runtime_error("couldn't find sprite");
		}
		rect.emplace(make_pair(100, *rectData));
		probTotal = 100;
	}
}
bool RandomRect::empty()
{
	return rect.empty();
}


const sf::Color TileFeatureS::fade = { 255, 255, 255, 64 };
const sf::Texture* TileFeatureS::tex = nullptr;

const TileFeatureS& TileFeatureS::get(int t)
{
	return *feature[t];
}

const sf::Texture& TileFeatureS::getTexture()
{
	return *tex;
}

const TileFeatureS& TileFeatureS::get(string t)
{
	for (auto& tf : feature) {
		if (tf->id_ == t) {
			return *tf;
		}
	}
	return *feature[0];
}

TileFeatureS::TileFeatureS(string id) :
vert_(sf::PrimitiveType::Quads, 4U),
id_(id)
{
}

void TileFeatureS::loadJson(string filename)
{
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
	const char* posNames[] = { "posFull", "posHalf", "posQuart" };
	const char* rectNames[] = { "rectFull", "rectHalf", "rectQuart" };
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
			// pos
			Json::Value j;
			for (int i = 0; i < 3; i++) {
				element = posNames[i];
				j = fdata.get(element, Json::Value::null);
				feat->pos_[i] = { j[0].asFloat(), j[1].asFloat() };
			}
			// rect
			for (int i = 0; i < 3; i++) {
				element = rectNames[i];
				j = fdata.get(element, Json::Value::null);
				feat->rects_[i].loadJson(j, sheet);
			}
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
