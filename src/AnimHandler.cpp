#include <iostream>
#include "AnimHandler.h"
#include "ResourceLoader.h"


const sf::FloatRect* SpriteSheet::spr(string spriteName) {
	const auto& spr = sprites.find(spriteName);
	if (spr == sprites.end()) {
		cerr << "ERROR: couldn't find sprite \"" << spriteName << "\" in file \"" << RESOURCE.getRoot() + name << "\"\n";
		return nullptr;
	}
	return &spr->second;
}

const std::string& SpriteSheet::getImageName() {
	return imageName;
}


const AnimationData AnimationData::defaultAnim(true);

AnimationData::frame::frame() :
delay(1),
sprites(sf::PrimitiveType::Quads) {
}

AnimationData::anim::anim(int frameNum) :
frames(frameNum),
loops(0) {
}

AnimationData::AnimationData(bool dummy) {
	if (dummy) {
		animations.emplace(make_pair("", anim(1)));
	}
}

const char* AnimationData::darkFunctionVersion() {
	return "1.3";
}

std::string AnimationData::getSheetName() {
	return sheetName;
}

const sf::Texture& AnimationData::getTexture() {
	return *tx;
}


AnimHandler::AnimHandler() :cframe(0), ctime(0){}

//Update the current frame (must be called at least once before draw()!)
void AnimHandler::updateAnimation(const sf::Time& time_elapsed) {
	ctime += (int)time_elapsed.asMicroseconds();
	if (ctime < currentFrame->delay) return;
	cframe++;
	if (cframe >= (int)currentAnim->frames.size() || cframe < 0) {
		cframe = 0;
	}
	ctime = ctime % currentFrame->delay;
	currentFrame = &currentAnim->frames[cframe];
}

void AnimHandler::draw(sf::RenderTarget &target, sf::RenderStates states) const {
	states.texture = animInfo->tx;
	states.transform *= getTransform();
	target.draw(currentFrame->sprites, states);
}

void AnimHandler::setAnimationData(const AnimationData &ai) {
	animInfo = &ai;
	currentAnim = &animInfo->animations.begin()->second;
	cframe = 0;
	currentFrame = &currentAnim->frames[0];
}

void AnimHandler::setAnimation(string name) {
	const auto& a = animInfo->animations.find(name);
	if (a == animInfo->animations.end()) {
		cerr << "ERROR: Couldn't find animation \"" << name << "\"\n";
		return;
	}
	currentAnim = &a->second;
	cframe = 0;
	currentFrame = &currentAnim->frames[0];
}

void AnimHandler::setFrame(int num) {
	currentFrame = &currentAnim->frames[num];
	cframe = num;
}

void AnimHandler::randomFrame(std::mt19937& urng) {
	int num = rng::getInt(currentAnim->frames.size() - 1, urng);
	currentFrame = &currentAnim->frames[num];
	cframe = num;
	ctime = rng::getInt(currentFrame->delay, urng);
}
