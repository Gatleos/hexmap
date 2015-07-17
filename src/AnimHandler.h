#ifndef ANIMHANDLER_H
#define ANIMHANDLER_H

#include <map>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <pugixml.hpp>



class AnimHandler;
class ResourceLoader;

class SpriteSheet
{
	std::string name;
	std::map<std::string, sf::FloatRect> sprites;
	std::string imageName;
	friend ResourceLoader;
public:
	const sf::FloatRect* spr(std::string spriteName);
	const std::string& getImageName();
};

class AnimationData
{
	static const char* darkFunctionVersion();
	struct sprite
	{
		sf::FloatRect draw;
		sf::Vector2f offset;
		bool flipH;
		bool flipV;
		sprite() :flipH(false), flipV(false){}
	};
	struct frame
	{
		unsigned int delay;
		sf::VertexArray sprites;
		frame() :sprites(sf::PrimitiveType::Quads){}
	};
	struct anim
	{
		std::string name;
		unsigned int loops;
		std::vector<frame> frames;
	};
	// Name of .sprite file
	std::string sheetName;
	// Image to draw sprites from
	const sf::Texture *tx;
	// List of all animations
	std::map<std::string, anim> animations;
public:
	// Return the name of the spritesheet
	std::string getSheetName();
	// Get the texture this animation draws from
	const sf::Texture& getTexture();
	friend AnimHandler;
	friend ResourceLoader;
};//AnimationData

class AnimHandler : public sf::Transformable, public sf::Drawable
{
	const AnimationData *animInfo;
	const AnimationData::anim *currentAnim;
	const AnimationData::frame *currentFrame;
	int cframe;
	int ctime;
public:
	AnimHandler();
	void updateAnimation(const sf::Time& timeElapsed);
	virtual void draw(sf::RenderTarget &target, sf::RenderStates states = sf::RenderStates::Default) const;
	void setAnimationData(const AnimationData &ai);
	void setAnimation(std::string name);
	void setFrame(int num);
};//AnimHandler

#endif
