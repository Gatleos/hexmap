#include <stack>
#include "UI.h"

sfg::Desktop* UI_desktop;
vector<pair<shared_ptr<UILayout>, bool>> UI_layoutStack;
sf::Vector2f UI_appSize;

/////////////////
// UIAlign //////
/////////////////

UIAlign::UIAlign(sf::FloatRect alloc, unsigned char FLAGS) :
alloc_(alloc),
FLAGS_(FLAGS)
{
}

void UIAlign::resize()
{
	if (FLAGS_ & UI::ALIGN_FRAC_POSX) {
		alloc_.left *= UI_appSize.x;
	}
	if (FLAGS_ & UI::ALIGN_FRAC_POSY) {
		alloc_.top *= UI_appSize.y;
	}
	if (FLAGS_ & UI::ALIGN_FRAC_SIZEX) {
		alloc_.width *= UI_appSize.x;
	}
	if (FLAGS_ & UI::ALIGN_FRAC_SIZEY) {
		alloc_.height *= UI_appSize.y;
	}
	if (FLAGS_ & UI::ALIGN_CENTERX) {
		alloc_.left -= alloc_.width * 0.5f;
	}
	else if (FLAGS_ & UI::ALIGN_RIGHT) {
		alloc_.left -= alloc_.width;
	}
	if (FLAGS_ & UI::ALIGN_CENTERY) {
		alloc_.top -= alloc_.height * 0.5f;
	}
	else if (FLAGS_ & UI::ALIGN_BOTTOM) {
		alloc_.top -= alloc_.height;
	}
}

/////////////////
// UILayout /////
/////////////////

void UILayout::show(bool show)
{
	if (show) {
		for (auto w : windows) {
			w.first->Show(true);
			w.second.resize();
			w.first->SetAllocation(w.second.alloc_);
		}
	}
	else {
		for (auto& w : windows) {
			w.first->Show(false);
		}
	}
}

void UILayout::addWindow(shared_ptr<sfg::Window> newWin, UIAlign a)
{
	newWin->Show(false);
	windows.push_back(make_pair(newWin, a));
}

/////////////////
// UI ///////////
/////////////////

void UI::setAppSize(sf::Vector2f size)
{
	UI_appSize = size;
	if (!UI_layoutStack.empty()) {
		for (auto l = UI_layoutStack.rbegin(); l != UI_layoutStack.rend(); l++) {
			for (auto w : l->first->windows) {
				w.second.resize();
				w.first->SetAllocation(w.second.alloc_);
			}
			if (l->second) {
				break;
			}
		}
	}
}

void UI::init(sfg::Desktop* d)
{
	UI_desktop = d;
	UI_layoutStack.reserve(100);
}

void UI::end()
{
	UI_layoutStack.clear();
}

void UI::addWindow(shared_ptr<sfg::Window> newWin, UIAlign a)
{
	UI_desktop->Add(newWin);
	a.resize();
	newWin->SetAllocation(a.alloc_);
}

void UI::addNewLayout(shared_ptr<UILayout> layout)
{
	for (auto& w : layout->windows) {
		UI_desktop->Add(w.first);
	}
}

void UI::pushLayout(shared_ptr<UILayout> layout, bool replacePrevious)
{
	if (!UI_layoutStack.empty() && !replacePrevious) {
		UI_layoutStack.back().first->show(false);
	}
	UI_layoutStack.push_back(make_pair(layout, replacePrevious));
	UI_layoutStack.back().first->show(true);
}

void UI::popLayout()
{
	if (UI_layoutStack.empty()) {
		return;
	}
	bool replacePrevious = false;
	UI_layoutStack.back().first->show(false);
	replacePrevious = UI_layoutStack.back().second;
	UI_layoutStack.pop_back();
	if (replacePrevious) {
		UI_layoutStack.back().first->show(true);
	}
}
