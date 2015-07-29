#ifndef UI_H
#define UI_H

#include <SFGUI/SFGUI.hpp>
#include <SFGUI/Widgets.hpp>

using namespace std;

struct UIAlign
{
	sf::FloatRect alloc_;
	unsigned char FLAGS_;
	UIAlign(sf::FloatRect alloc, unsigned char FLAGS = 0);
	void resize();
};

class UILayout
{
public:
	vector<pair<shared_ptr<sfg::Widget>, UIAlign>> windows;
	void show(bool show = true);
	void addWindow(shared_ptr<sfg::Widget> newWin, UIAlign a);
};

namespace UI
{
	enum align{
		ALIGN_NONE = 0, ALIGN_FRAC_POSX = 1, ALIGN_FRAC_POSY = 2, ALIGN_FRAC_SIZEX = 4, ALIGN_FRAC_SIZEY = 8,
		ALIGN_RIGHT = 16, ALIGN_BOTTOM = 32, ALIGN_CENTERX = 64, ALIGN_CENTERY = 128
	};
	void init(sfg::Desktop* d);
	void end();
	void pushLayout(shared_ptr<UILayout> layout, bool replacePrevious = false);
	void popLayout();
	void addWindow(shared_ptr<sfg::Window> newWin, UIAlign a);
	// Add a layout to the desktop; layouts won't show up until you add them here
	void addNewLayout(shared_ptr<UILayout> layout);
	// Update the size of the overall app window
	void setAppSize(sf::Vector2f size);
	void resetInputFlags();
	// Did the UI process the last input?
	bool gotMouseInput();
};

#endif