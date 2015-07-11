#include <iostream>
#include "json.h"
#include "TileFeature.h"
#include "ResourceLoader.h"
#include "clamp.h"

#define ftrptr(x) unique_ptr<TileFeatureS>(new TileFeatureS(x))

array<unique_ptr<TileFeatureS>, featuretypes::SIZE> TileFeatureS::feature = { {
	ftrptr("f_null"), ftrptr("f_mountain")
} };

const sf::Color TileFeatureS::fade = { 255, 255, 255, 64 };

const TileFeatureS& TileFeatureS::get(int t)
{
	return *feature[t];
}

TileFeatureS::TileFeatureS(string id) :
vert_(sf::PrimitiveType::Quads, 4U),
id_(id),
pos_(0.0f, 0.0f)
{

}

const sf::FloatRect* TileFeatureS::getRect(int rectNum, mt19937& urng) const
{
	if (!randomRect[rectNum]) {
		return &rect_[rectNum].begin()->second;
	}
	int r = rng::getInt(probTotal_[rectNum] - 1, urng);
	for (auto f = rect_[rectNum].rbegin(); f != rect_[rectNum].rend(); f++) {
		if (r < f->first) {
			return &f->second;
		}
		r -= f->first;
	}
	return nullptr;
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
	const char* rectNames[] = { "rect", "rectHalf", "rectQuart" };
	// Cycle through defined feature types; make sure to skip f_null!
	for (int a = 1; a < featuretypes::SIZE; a++) {
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
			element = "pos";
			Json::Value j = fdata.get(element, Json::Value::null);
			feat->pos_ = { j[0].asFloat(), j[1].asFloat() };
			// rect
			const sf::FloatRect* rectData = nullptr;
			for (int i = 0; i < 3; i++) {
				element = rectNames[i];
				j = fdata.get(element, Json::Value::null);
				feat->probTotal_[i] = 0;
				if (j.isArray()) {
					feat->randomRect[i] = true;
					if (j.size() & 1) {
						throw runtime_error("incorrect number of arguments");
					}
					for (int f = 0; f < j.size(); f += 2) {
						rectData = sheet->spr(j[f].asString());
						if (rectData == nullptr) {
							throw runtime_error("couldn't find tile sprite");
						}
						int prob = clamp(j[f + 1].asInt(), 0, 100);
						feat->probTotal_[i] += prob;
						feat->rect_[i].emplace(make_pair(clamp(j[f + 1].asInt(), 0, 100), *rectData));
					}
				}
				else {
					feat->randomRect[i] = false;
					rectData = sheet->spr(j.asString());
					if (rectData == nullptr) {
						throw runtime_error("couldn't find tile sprite");
					}
					feat->rect_[i].emplace(make_pair(0, *rectData));
				}
			}
		}
		catch (runtime_error e) { // report the error with the name of the object and member
			// make sure we have placeholder drawing rects!
			for (int r = 0; r < 3; r++) {
				auto& rect = feat->rect_[r];
				if (rect.empty()) {
					rect.emplace(make_pair(0, sf::FloatRect()));
					feat->randomRect[r] = false;
				}
			}
			cerr << "[" << filename << ", " << feat->id_ << ", " << element << "] " << e.what() << "\n";
		}
	}
}
