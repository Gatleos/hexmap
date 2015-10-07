#include <stack>
#include <SFGUI/Renderers.hpp>
#include "UI.h"
#include "HexMap.h"

sfg::Desktop* UI::desktop = nullptr;
sf::View UI::view;
sf::Vector2i UI::lastMousePos(0, 0);
sf::Sprite UI::hexSelector;
static vector<pair<shared_ptr<UILayout>, bool>> UI_layoutStack;
static sf::Vector2f UI_appSize;
static bool UI_gotMouseInput = false;
static bool UI_gotKeyboardInput = false;
static sf::Image UI_image;
static const sf::Texture* UI_texture = nullptr;
static SpriteSheet* UI_sprites = nullptr;
static vector<shared_ptr<UILayout>> UI_layouts;

/////////////////
// UIAlign //////
/////////////////

UIAlign::UIAlign(sf::FloatRect alloc, unsigned char FLAGS, bool autoResize) :
alloc_(alloc),
FLAGS_(FLAGS),
autoResize_(autoResize) {
}

void UIAlign::resize(shared_ptr<sfg::Widget> win) {
	auto alloc = alloc_;
	const auto& req = win->GetRequisition();
	if (alloc.width == 0.0f) {
		alloc.width = req.x;
	}
	else if (FLAGS_ & UI::ALIGN_FRAC_SIZEX) {
		alloc.width *= UI_appSize.x;
	}
	if (alloc.height == 0.0f) {
		alloc.height = req.y;
	}
	else if (FLAGS_ & UI::ALIGN_FRAC_SIZEY) {
		alloc.width *= UI_appSize.x;
	}
	if (FLAGS_ & UI::ALIGN_FRAC_POSX) {
		alloc.left *= UI_appSize.x;
	}
	if (FLAGS_ & UI::ALIGN_FRAC_POSY) {
		alloc.top *= UI_appSize.y;
	}
	if (FLAGS_ & UI::ALIGN_CENTERX) {
		alloc.left -= alloc.width * 0.5f;
	}
	else if (FLAGS_ & UI::ALIGN_RIGHT) {
		alloc.left -= alloc.width;
	}
	if (FLAGS_ & UI::ALIGN_CENTERY) {
		alloc.top -= alloc.height * 0.5f;
	}
	else if (FLAGS_ & UI::ALIGN_BOTTOM) {
		alloc.top -= alloc.height;
	}
	win->SetAllocation(alloc);
}

/////////////////
// UILayout /////
/////////////////

void UILayout::show(bool show) {
	if (show) {
		for (auto w : windows) {
			w.first->Show(true);
			w.second.resize(w.first);
		}
	}
	else {
		for (auto& w : windows) {
			w.first->Show(false);
		}
	}
	visible = show;
}

void UILayout::addWindow(shared_ptr<sfg::Widget> newWin, UIAlign a) {
	newWin->Show(false);
	windows.push_back(make_pair(newWin, a));
}

void UILayout::bringToFront() {
	for (auto& w : windows) {
		UI::desktop->BringToFront(w.first);
	}
}

void UILayout::update(const sf::Time& timeElapsed) {

}

bool UILayout::isVisible() {
	return visible;
}

/////////////////
// UI ///////////
/////////////////

void UI::setAppSize(sf::Vector2f size) {
	UI_appSize = size;
	if (!UI_layouts.empty()) {
		for (auto l = UI_layouts.rbegin(); l != UI_layouts.rend(); l++) {
			for (auto w : l->get()->windows) {
				if (w.second.autoResize_) {
					w.second.resize(w.first);
				}
			}
		}
	}
}

void UI::resetInputFlags() {
	UI_gotMouseInput = false;
	UI_gotKeyboardInput = false;
}

bool UI::gotMouseInput() {
	return UI_gotMouseInput;
}

bool UI::gotKeyboardInput() {
	return UI_gotKeyboardInput;
}

bool UI::gotInput() {
	return UI_gotMouseInput || UI_gotKeyboardInput;
}

void UI::connectMouseInputFlag(shared_ptr<sfg::Widget> w) {
	static auto setMouseFlag = [](){UI_gotMouseInput = true; };
	w->GetSignal(sfg::Window::OnMouseLeftPress).Connect(setMouseFlag);
	w->GetSignal(sfg::Window::OnMouseRightPress).Connect(setMouseFlag);
}

void UI::connectKeyboardInputFlag(shared_ptr<sfg::Widget> w) {
	static auto setKeyboardFlag = [](){UI_gotKeyboardInput = true; };
	w->GetSignal(sfg::Window::OnKeyPress).Connect(setKeyboardFlag);
	w->GetSignal(sfg::Window::OnKeyPress).Connect(setKeyboardFlag);
}

const sf::Image& UI::image() {
	return UI_image;
}
const sf::Texture& UI::texture() {
	return *UI_texture;
}
SpriteSheet& UI::sprites() {
	return *UI_sprites;
}

void UI::init(sfg::Desktop* d) {
	desktop = d;
	auto renderer = sfg::VertexBufferRenderer::Create(); // Fix a NonLegacyRenderer-related text bug
	sfg::Renderer::Set(renderer);
	UI_layoutStack.reserve(100);
	UI_sprites = RESOURCE.sh("ui.sprites");
	UI_texture = RESOURCE.tex(UI_sprites->getImageName());
	UI_image = UI_texture->copyToImage();
	hexSelector.setTexture(UI::texture());
	hexSelector.setTextureRect((sf::IntRect)*UI::sprites().spr("/select/full"));
	hexSelector.setColor(sf::Color::Red);
}

void UI::end() {
	UI_layoutStack.clear();
	UI_layouts.clear();
}

void UI::addWindow(shared_ptr<sfg::Window> newWin) {
	desktop->Add(newWin);
	connectMouseInputFlag(newWin);
}

void UI::addWindow(shared_ptr<sfg::Window> newWin, UIAlign a) {
	desktop->Add(newWin);
	connectMouseInputFlag(newWin);
	a.resize(newWin);
}

void UI::addNewLayout(shared_ptr<UILayout> layout) {
	UI_layouts.push_back(layout);
	for (auto& w : layout->windows) {
		desktop->Add(w.first);
		connectMouseInputFlag(w.first);
	}
}

void UI::pushLayout(shared_ptr<UILayout> layout, bool replacePrevious) {
	if (!UI_layoutStack.empty() && replacePrevious) {
		UI_layoutStack.back().first->show(false);
	}
	UI_layoutStack.push_back(make_pair(layout, replacePrevious));
	UI_layoutStack.back().first->show(true);
}

void UI::popLayout() {
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

void UI::updateLayouts(const sf::Time& timeElapsed) {
	for (auto l : UI_layouts) {
		if (l->isVisible()) {
			l->update(timeElapsed);
		}
	}
}

void UI::dropFocus() {
	static auto UI_dummyWindow = sfg::Window::Create();
	UI_dummyWindow->GrabFocus();
}

void UI::selectHex(sf::Vector2f& hexCoord) {
	hexSelector.setPosition(HEXMAP.hexToPixel(hexCoord) - HEXMAP.getOrigin() - sf::Vector2f(1.0f, 1.0f));
}