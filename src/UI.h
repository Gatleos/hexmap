#ifndef UI_H
#define UI_H

#include <SFGUI/SFGUI.hpp>
#include <SFGUI/Widgets.hpp>
#include "ResourceLoader.h"
#include "MapEntity.h"

using namespace std;

struct UIAlign {
	sf::FloatRect alloc_;
	unsigned char FLAGS_;
	bool autoResize_;
	UIAlign(sf::FloatRect alloc, unsigned char FLAGS = 0, bool autoResize = true);
	void resize(shared_ptr<sfg::Widget> win);
};

class UILayout {
	bool visible;
public:
	vector<pair<shared_ptr<sfg::Widget>, UIAlign>> windows;
	void show(bool show = true);
	void addWindow(shared_ptr<sfg::Widget> newWin, UIAlign a);
	void bringToFront();
	virtual void update(const sf::Time& timeElapsed);
	bool isVisible();
};

namespace UI {
	enum align{
		ALIGN_NONE = 0, ALIGN_FRAC_POSX = 1, ALIGN_FRAC_POSY = 2, ALIGN_FRAC_SIZEX = 4, ALIGN_FRAC_SIZEY = 8,
		ALIGN_RIGHT = 16, ALIGN_BOTTOM = 32, ALIGN_CENTERX = 64, ALIGN_CENTERY = 128
	};
	extern sfg::Desktop* desktop;
	extern sf::View view;
	extern sf::Vector2i lastMousePos;
	extern sf::Sprite hexSelector;
	void init(sfg::Desktop* d);
	void end();
	void pushLayout(shared_ptr<UILayout> layout, bool replacePrevious = false);
	void popLayout();
	void updateLayouts(const sf::Time& timeElapsed);
	void addWindow(shared_ptr<sfg::Window> newWin);
	void addWindow(shared_ptr<sfg::Window> newWin, UIAlign a);
	// Add a layout to the desktop; layouts won't show up until you add them here
	void addNewLayout(shared_ptr<UILayout> layout);
	// Update the size of the overall app window
	void setAppSize(sf::Vector2f size);
	void resetInputFlags();
	// Did the UI process the last mouse input?
	bool gotMouseInput();
	// Did the UI process the last keyboard input?
	bool gotKeyboardInput();
	// Did the UI process the last mouse or keyboard input?
	bool gotInput();
	// This widget will set the mouse input flag for this loop when clicked, "consuming"
	// the input; automatically applied to newly added windows, needs to be added to
	// Buttons and CheckButtons
	void connectMouseInputFlag(shared_ptr<sfg::Widget> w);
	// This widget will set the keyboard input flag for this loop when it receives
	// keystrokes, "consuming" the input; should be added to any text input widget
	void connectKeyboardInputFlag(shared_ptr<sfg::Widget> w);
	void setMouseFlag();
	const sf::Image& image();
	const sf::Texture& texture();
	SpriteSheet& sprites();
	// Lose focus on all windows and contained widgets
	void dropFocus();
	void selectHex(sf::Vector2f& hexCoord);
};

#endif