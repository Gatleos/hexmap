#include <iostream>
#include <fstream>
#include <sstream>
#include "States.h"
#include "HexMap.h"

const sf::Color SelectState::selectCol{ 100, 100, 255 };
const sf::Color SelectState::validCol{ 100, 255, 100 };
const sf::Color SelectState::invalidCol{ 255, 100, 100 };

SelectState::SelectState(shared_ptr<VectorSet> selectable, std::function<void(const sf::Vector2i&)> selectCallback) :
selectableCoords_(selectable),
tilePos_(-1.0f, -1.0f),
inBounds_(false),
selectCallback_(selectCallback)
{
}
void SelectState::init()
{
	static auto setupSprite = [](sf::Sprite& spr, sf::Vector2f& hexCoord) {
		spr.setTexture(UI::texture());
		spr.setTextureRect((sf::IntRect)*UI::sprites().spr("/select/full"));
		spr.setColor(selectCol);
		spr.setPosition(HEXMAP.hexToPixel(hexCoord) - HEXMAP.getOrigin() - sf::Vector2f(1.0f, 1.0f));
	};
	for (auto& v : *selectableCoords_) {
		selectable_.push_back(sf::Sprite());
		setupSprite(selectable_.back(), (sf::Vector2f)v);
	}
	setupSprite(selected_, sf::Vector2f(0.0f, 0.0f));
}
void SelectState::end()
{
}
void SelectState::update()
{
	prev->update();
}
void SelectState::render(sf::RenderWindow &window)
{
	prev->render(window);
	window.setView(HexMap::view);
	for (auto& s : selectable_) {
		window.draw(s);
	}
	if (inBounds_) {
		window.draw(selected_);
	}
}
void SelectState::input(sf::Event &e)
{
	if (e.type == sf::Event::MouseMoved) {
		const sf::Vector2f& size = HexMap::view.getSize();
		const sf::Vector2f& center = HexMap::view.getCenter();
		sf::Vector2f newTilePos = HEXMAP.pixelToHex({ e.mouseMove.x - size.x / 2.0f + center.x, e.mouseMove.y - size.y / 2.0f + center.y });
		if (tilePos_ != newTilePos) {
			if (HEXMAP.isAxialInBounds((sf::Vector2i)newTilePos)) {
				if (selectableCoords_->find((sf::Vector2i)newTilePos) == selectableCoords_->end()) {
					selected_.setColor(invalidCol);
				}
				else {
					selected_.setColor(validCol);
				}
				tilePos_ = newTilePos;
				selected_.setPosition(HEXMAP.hexToPixel(tilePos_) - HEXMAP.getOrigin() - sf::Vector2f(1.0f, 1.0f));
				inBounds_ = true;
			}
			else {
				tilePos_ = { -1.0f, -1.0f };
				inBounds_ = false;
			}
		}
	}
	else if (e.type == sf::Event::MouseButtonPressed) {
		if (UI::gotMouseInput()) {
			engine->popState();
			return;
		}
		if (e.mouseButton.button == sf::Mouse::Left) {
			if (selectableCoords_->find((sf::Vector2i)tilePos_) == selectableCoords_->end()) {
				selectCallback_(sf::Vector2i(-1, -1));
			}
			else {
				selectCallback_((sf::Vector2i)tilePos_);
			}
			engine->popState();
		}
	}
}