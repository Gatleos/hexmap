#include <iostream>
#include "config.h"
#include "UI.h"

static const vector<string> bindingNames = {
	"scroll_left", "scroll_right", "scroll_up", "scroll_down", "generate", "fps_display", "debug"
};
static map<string, config::KeyBinding> bindings;

void config::loadKeyJson(std::string file)
{
	Json::Value root = openJson(file);
	if (root.begin() == root.end()) {
		return;
	}
	for (auto& b : bindingNames) {
		auto k = root.get(b, Json::Value::null);
		if (k.isNull()) {
			cerr << "ERROR: couldn't find key binding for \"" << b << "\"\n";
			continue;
		}
		if (!k.isArray() || k.size() != 0U && k.size() != 2U && k.size() != 4U && k.size() != 6U) {
			cerr << "[" << file << ", " << b << "] bad definition\n";
			continue;
		}
		try {
			KeyBinding& cBinding = bindings[b];
			cBinding.keys.resize(k.size() / 2);
			int index = 0;
			for (auto& key : cBinding.keys) {
				if (k[index].asString() == "Key") {
					auto keyIt = keyNames.find(k[index + 1].asString());
					if (keyIt == keyNames.end()) {
						throw runtime_error("key does not exist!");
					}
					cBinding.typePress = sf::Event::EventType::KeyPressed;
					cBinding.typeRelease = sf::Event::EventType::KeyReleased;
					key = keyIt->second;
				}
				else if (k[index].asString() == "Mouse") {
					auto keyIt = keyNames.find(k[index + 1].asCString());
					if (keyIt == keyNames.end()) {
						throw runtime_error("key does not exist!");
					}
					cBinding.typePress = sf::Event::EventType::MouseButtonPressed;
					cBinding.typePress = sf::Event::EventType::MouseButtonReleased;
					key = keyIt->second;
				}
				else {
					throw runtime_error("input type does not exist!");
				}
				index += 2;
			}
		}
		catch (std::runtime_error e) {
			cerr << "[" << file << ", " << b << "] " << e.what() << "\n";
		}
	}
}

bool config::pressed(sf::Event& e, std::string binding) {
	KeyBinding& k = bindings[binding];
	if (k.typePress != e.type) {
		return false;
	}
	for (auto& key : k.keys) {
		if (sf::Event::EventType::KeyPressed && key == e.key.code) {
			return true;
		}
		if (k.typePress == sf::Event::EventType::MouseButtonPressed && key == e.mouseButton.button) {
			return true;
		}
	}
	return false;
}

bool config::released(sf::Event& e, std::string binding) {
	KeyBinding& k = bindings[binding];
	if (k.typeRelease != e.type) {
		return false;
	}
	for (auto& key : k.keys) {
		if (sf::Event::EventType::KeyPressed && key == e.key.code) {
			return true;
		}
		if (k.typeRelease == sf::Event::EventType::MouseButtonPressed && key == e.mouseButton.button) {
			return true;
		}
	}
	return false;
}

const char *config::getMouseButtonName(const sf::Mouse::Button button) {
	switch (button) {
	default:
		return "Unknown";
	case sf::Mouse::Left:
		return "Left";
	case sf::Mouse::Right:
		return "Right";
	case sf::Mouse::Middle:
		return "Middle";
	case sf::Mouse::XButton1:
		return "XButton1";
	case sf::Mouse::XButton2:
		return "XButton2";
	}
}

const char *config::getKeyName(const sf::Keyboard::Key key) {
	switch (key) {
	default:
	case sf::Keyboard::Unknown:
		return "Unknown";
	case sf::Keyboard::A:
		return "A";
	case sf::Keyboard::B:
		return "B";
	case sf::Keyboard::C:
		return "C";
	case sf::Keyboard::D:
		return "D";
	case sf::Keyboard::E:
		return "E";
	case sf::Keyboard::F:
		return "F";
	case sf::Keyboard::G:
		return "G";
	case sf::Keyboard::H:
		return "H";
	case sf::Keyboard::I:
		return "I";
	case sf::Keyboard::J:
		return "J";
	case sf::Keyboard::K:
		return "K";
	case sf::Keyboard::L:
		return "L";
	case sf::Keyboard::M:
		return "M";
	case sf::Keyboard::N:
		return "N";
	case sf::Keyboard::O:
		return "O";
	case sf::Keyboard::P:
		return "P";
	case sf::Keyboard::Q:
		return "Q";
	case sf::Keyboard::R:
		return "R";
	case sf::Keyboard::S:
		return "S";
	case sf::Keyboard::T:
		return "T";
	case sf::Keyboard::U:
		return "U";
	case sf::Keyboard::V:
		return "V";
	case sf::Keyboard::W:
		return "W";
	case sf::Keyboard::X:
		return "X";
	case sf::Keyboard::Y:
		return "Y";
	case sf::Keyboard::Z:
		return "Z";
	case sf::Keyboard::Num0:
		return "Num0";
	case sf::Keyboard::Num1:
		return "Num1";
	case sf::Keyboard::Num2:
		return "Num2";
	case sf::Keyboard::Num3:
		return "Num3";
	case sf::Keyboard::Num4:
		return "Num4";
	case sf::Keyboard::Num5:
		return "Num5";
	case sf::Keyboard::Num6:
		return "Num6";
	case sf::Keyboard::Num7:
		return "Num7";
	case sf::Keyboard::Num8:
		return "Num8";
	case sf::Keyboard::Num9:
		return "Num9";
	case sf::Keyboard::Escape:
		return "Escape";
	case sf::Keyboard::LControl:
		return "LControl";
	case sf::Keyboard::LShift:
		return "LShift";
	case sf::Keyboard::LAlt:
		return "LAlt";
	case sf::Keyboard::LSystem:
		return "LSystem";
	case sf::Keyboard::RControl:
		return "RControl";
	case sf::Keyboard::RShift:
		return "RShift";
	case sf::Keyboard::RAlt:
		return "RAlt";
	case sf::Keyboard::RSystem:
		return "RSystem";
	case sf::Keyboard::Menu:
		return "Menu";
	case sf::Keyboard::LBracket:
		return "LBracket";
	case sf::Keyboard::RBracket:
		return "RBracket";
	case sf::Keyboard::SemiColon:
		return "SemiColon";
	case sf::Keyboard::Comma:
		return "Comma";
	case sf::Keyboard::Period:
		return "Period";
	case sf::Keyboard::Quote:
		return "Quote";
	case sf::Keyboard::Slash:
		return "Slash";
	case sf::Keyboard::BackSlash:
		return "BackSlash";
	case sf::Keyboard::Tilde:
		return "Tilde";
	case sf::Keyboard::Equal:
		return "Equal";
	case sf::Keyboard::Dash:
		return "Dash";
	case sf::Keyboard::Space:
		return "Space";
	case sf::Keyboard::Return:
		return "Return";
	case sf::Keyboard::BackSpace:
		return "BackSpace";
	case sf::Keyboard::Tab:
		return "Tab";
	case sf::Keyboard::PageUp:
		return "PageUp";
	case sf::Keyboard::PageDown:
		return "PageDown";
	case sf::Keyboard::End:
		return "End";
	case sf::Keyboard::Home:
		return "Home";
	case sf::Keyboard::Insert:
		return "Insert";
	case sf::Keyboard::Delete:
		return "Delete";
	case sf::Keyboard::Add:
		return "Add";
	case sf::Keyboard::Subtract:
		return "Subtract";
	case sf::Keyboard::Multiply:
		return "Multiply";
	case sf::Keyboard::Divide:
		return "Divide";
	case sf::Keyboard::Left:
		return "Left";
	case sf::Keyboard::Right:
		return "Right";
	case sf::Keyboard::Up:
		return "Up";
	case sf::Keyboard::Down:
		return "Down";
	case sf::Keyboard::Numpad0:
		return "Numpad0";
	case sf::Keyboard::Numpad1:
		return "Numpad1";
	case sf::Keyboard::Numpad2:
		return "Numpad2";
	case sf::Keyboard::Numpad3:
		return "Numpad3";
	case sf::Keyboard::Numpad4:
		return "Numpad4";
	case sf::Keyboard::Numpad5:
		return "Numpad5";
	case sf::Keyboard::Numpad6:
		return "Numpad6";
	case sf::Keyboard::Numpad7:
		return "Numpad7";
	case sf::Keyboard::Numpad8:
		return "Numpad8";
	case sf::Keyboard::Numpad9:
		return "Numpad9";
	case sf::Keyboard::F1:
		return "F1";
	case sf::Keyboard::F2:
		return "F2";
	case sf::Keyboard::F3:
		return "F3";
	case sf::Keyboard::F4:
		return "F4";
	case sf::Keyboard::F5:
		return "F5";
	case sf::Keyboard::F6:
		return "F6";
	case sf::Keyboard::F7:
		return "F7";
	case sf::Keyboard::F8:
		return "F8";
	case sf::Keyboard::F9:
		return "F9";
	case sf::Keyboard::F10:
		return "F10";
	case sf::Keyboard::F11:
		return "F11";
	case sf::Keyboard::F12:
		return "F12";
	case sf::Keyboard::F13:
		return "F13";
	case sf::Keyboard::F14:
		return "F14";
	case sf::Keyboard::F15:
		return "F15";
	case sf::Keyboard::Pause:
		return "Pause";
	}
}

const map<string, sf::Keyboard::Key> config::keyNames = { make_pair("Unknown", sf::Keyboard::Unknown), make_pair("A", sf::Keyboard::A), make_pair("B", sf::Keyboard::B), make_pair("C", sf::Keyboard::C), make_pair("D", sf::Keyboard::D), make_pair("E", sf::Keyboard::E), make_pair("F", sf::Keyboard::F), make_pair("G", sf::Keyboard::G), make_pair("H", sf::Keyboard::H), make_pair("I", sf::Keyboard::I), make_pair("J", sf::Keyboard::J), make_pair("K", sf::Keyboard::K), make_pair("L", sf::Keyboard::L), make_pair("M", sf::Keyboard::M), make_pair("N", sf::Keyboard::N), make_pair("O", sf::Keyboard::O), make_pair("P", sf::Keyboard::P), make_pair("Q", sf::Keyboard::Q), make_pair("R", sf::Keyboard::R), make_pair("S", sf::Keyboard::S), make_pair("T", sf::Keyboard::T), make_pair("U", sf::Keyboard::U), make_pair("V", sf::Keyboard::V), make_pair("W", sf::Keyboard::W), make_pair("X", sf::Keyboard::X), make_pair("Y", sf::Keyboard::Y), make_pair("Z", sf::Keyboard::Z), make_pair("Num0", sf::Keyboard::Num0), make_pair("Num1", sf::Keyboard::Num1), make_pair("Num2", sf::Keyboard::Num2), make_pair("Num3", sf::Keyboard::Num3), make_pair("Num4", sf::Keyboard::Num4), make_pair("Num5", sf::Keyboard::Num5), make_pair("Num6", sf::Keyboard::Num6), make_pair("Num7", sf::Keyboard::Num7), make_pair("Num8", sf::Keyboard::Num8), make_pair("Num9", sf::Keyboard::Num9), make_pair("Escape", sf::Keyboard::Escape), make_pair("LControl", sf::Keyboard::LControl), make_pair("LShift", sf::Keyboard::LShift), make_pair("LAlt", sf::Keyboard::LAlt), make_pair("LSystem", sf::Keyboard::LSystem), make_pair("RControl", sf::Keyboard::RControl), make_pair("RShift", sf::Keyboard::RShift), make_pair("RAlt", sf::Keyboard::RAlt), make_pair("RSystem", sf::Keyboard::RSystem), make_pair("Menu", sf::Keyboard::Menu), make_pair("LBracket", sf::Keyboard::LBracket), make_pair("RBracket", sf::Keyboard::RBracket), make_pair("SemiColon", sf::Keyboard::SemiColon), make_pair("Comma", sf::Keyboard::Comma), make_pair("Period", sf::Keyboard::Period), make_pair("Quote", sf::Keyboard::Quote), make_pair("Slash", sf::Keyboard::Slash), make_pair("BackSlash", sf::Keyboard::BackSlash), make_pair("Tilde", sf::Keyboard::Tilde), make_pair("Equal", sf::Keyboard::Equal), make_pair("Dash", sf::Keyboard::Dash), make_pair("Space", sf::Keyboard::Space), make_pair("Return", sf::Keyboard::Return), make_pair("BackSpace", sf::Keyboard::BackSpace), make_pair("Tab", sf::Keyboard::Tab), make_pair("PageUp", sf::Keyboard::PageUp), make_pair("PageDown", sf::Keyboard::PageDown), make_pair("End", sf::Keyboard::End), make_pair("Home", sf::Keyboard::Home), make_pair("Insert", sf::Keyboard::Insert), make_pair("Delete", sf::Keyboard::Delete), make_pair("Add", sf::Keyboard::Add), make_pair("Subtract", sf::Keyboard::Subtract), make_pair("Multiply", sf::Keyboard::Multiply), make_pair("Divide", sf::Keyboard::Divide), make_pair("Left", sf::Keyboard::Left), make_pair("Right", sf::Keyboard::Right), make_pair("Up", sf::Keyboard::Up), make_pair("Down", sf::Keyboard::Down), make_pair("Numpad0", sf::Keyboard::Numpad0), make_pair("Numpad1", sf::Keyboard::Numpad1), make_pair("Numpad2", sf::Keyboard::Numpad2), make_pair("Numpad3", sf::Keyboard::Numpad3), make_pair("Numpad4", sf::Keyboard::Numpad4), make_pair("Numpad5", sf::Keyboard::Numpad5), make_pair("Numpad6", sf::Keyboard::Numpad6), make_pair("Numpad7", sf::Keyboard::Numpad7), make_pair("Numpad8", sf::Keyboard::Numpad8), make_pair("Numpad9", sf::Keyboard::Numpad9), make_pair("F1", sf::Keyboard::F1), make_pair("F2", sf::Keyboard::F2), make_pair("F3", sf::Keyboard::F3), make_pair("F4", sf::Keyboard::F4), make_pair("F5", sf::Keyboard::F5), make_pair("F6", sf::Keyboard::F6), make_pair("F7", sf::Keyboard::F7), make_pair("F8", sf::Keyboard::F8), make_pair("F9", sf::Keyboard::F9), make_pair("F10", sf::Keyboard::F10), make_pair("F11", sf::Keyboard::F11), make_pair("F12", sf::Keyboard::F12), make_pair("F13", sf::Keyboard::F13), make_pair("F14", sf::Keyboard::F14), make_pair("F15", sf::Keyboard::F15), make_pair("Pause", sf::Keyboard::Pause) };
