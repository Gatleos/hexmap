#include <cstring>
#include <iostream>
#include "VERSION.h"
#include "rng.h"
#include "States.h"

int main(int argc, char* argv[])
{
	if (argc == 2) {
		if (!std::strcmp(argv[1], "-v")) {
			std::cout << PROG_VERSION << " compiled " << __DATE__ << " " << __TIME__;
		}
	}
	else {
		// Seed the default rng
		rng::init();
		// Resources
		config::load();
		RESOURCE.setRoot(config::resourceRoot);
		config::loadAllJson();
		// Create our game engine
		auto& s = SFMLEngine::instance();
		sf::RenderWindow window(sf::VideoMode(640, 512), "", sf::Style::Default, sf::ContextSettings());
		s.init(&window);
		s.pushState(new EngineState());
		s.start();
	}
	return 0;
}
