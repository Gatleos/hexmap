#include <stack>
#include <SFGUI/Renderers.hpp>
#include "UI.h"

sfg::Desktop* UI::desktop = nullptr;
sf::View UI::view;
static vector<pair<shared_ptr<UILayout>, bool>> UI_layoutStack;
static sf::Vector2f UI_appSize;
static bool UI_gotMouseInput = false;
static sf::Image UI_image;
static const sf::Texture* UI_texture = nullptr;
static SpriteSheet* UI_sprites = nullptr;

/////////////////
// UIAlign //////
/////////////////

UIAlign::UIAlign(sf::FloatRect alloc, unsigned char FLAGS, bool autoResize) :
alloc_(alloc),
FLAGS_(FLAGS),
autoResize_(autoResize)
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

void UILayout::addWindow(shared_ptr<sfg::Widget> newWin, UIAlign a)
{
	newWin->Show(false);
	windows.push_back(make_pair(newWin, a));
}

void UILayout::bringToFront()
{
	for (auto& w : windows) {
		UI::desktop->BringToFront(w.first);
	}
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
				if (w.second.autoResize_) {
					w.second.resize();
					w.first->SetAllocation(w.second.alloc_);
				}
			}
			if (l->second) {
				break;
			}
		}
	}
}

void UI::resetInputFlags()
{
	UI_gotMouseInput = false;
}

bool UI::gotMouseInput()
{
	return UI_gotMouseInput;
}

void UI::connectMouseInputFlag(shared_ptr<sfg::Widget> w)
{
	static auto setMouseFlag = [](){UI_gotMouseInput = true; };
	w->GetSignal(sfg::Window::OnMouseLeftPress).Connect(setMouseFlag);
	w->GetSignal(sfg::Window::OnMouseLeftRelease).Connect(setMouseFlag);
	w->GetSignal(sfg::Window::OnMouseRightPress).Connect(setMouseFlag);
	w->GetSignal(sfg::Window::OnMouseRightRelease).Connect(setMouseFlag);
}

const sf::Image& UI::image()
{
	return UI_image;
}
const sf::Texture& UI::texture()
{
	return *UI_texture;
}
SpriteSheet& UI::sprites()
{
	return *UI_sprites;
}

void UI::init(sfg::Desktop* d)
{
	desktop = d;
	auto renderer = sfg::VertexBufferRenderer::Create(); // Fix a NonLegacyRenderer-related text bug
	sfg::Renderer::Set(renderer);
	UI_layoutStack.reserve(100);
	UI_sprites = RESOURCE.sh("ui.sprites");
	UI_texture = RESOURCE.tex(UI_sprites->getImageName());
	UI_image = UI_texture->copyToImage();
}

void UI::end()
{
	UI_layoutStack.clear();
}

void UI::addWindow(shared_ptr<sfg::Window> newWin)
{
	desktop->Add(newWin);
}

void UI::addWindow(shared_ptr<sfg::Window> newWin, UIAlign a)
{
	desktop->Add(newWin);
	a.resize();
	newWin->SetAllocation(a.alloc_);
}

void UI::addNewLayout(shared_ptr<UILayout> layout)
{
	for (auto& w : layout->windows) {
		desktop->Add(w.first);
		connectMouseInputFlag(w.first);
	}
}

void UI::pushLayout(shared_ptr<UILayout> layout, bool replacePrevious)
{
	if (!UI_layoutStack.empty() && replacePrevious) {
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
