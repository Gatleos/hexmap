#include "UIdef.h"
#include "HexTile.h"
#include "MapEntity.h"

namespace UIdef {
	MapGenDebug::MapGenDebug() {
		auto window1 = sfg::Window::Create(sfg::Window::Style::BACKGROUND);
		auto table1 = sfg::Table::Create();
		array<const char*, 6U> s = { "Camera:", "Mouse:", "Hex:", "Terrain:", "Gen (ms):", "Chunks:" };
		array<shared_ptr<sfg::Label>, 6U> label2;
		for (int a = 0; a < (int)debugInfo.size(); a++) {
			label2[a] = sfg::Label::Create(s[a]);
			label2[a]->SetAlignment({ 0.0f, 0.0f });
			table1->Attach(label2[a], { 0U, (sf::Uint32)a, 1U, 1U });
		}
		for (int a = 0; a < (int)debugInfo.size(); a++) {
			debugInfo[a] = sfg::Label::Create("");
			debugInfo[a]->SetAlignment({ 0.0f, 0.0f });
			table1->Attach(debugInfo[a], { 1U, (sf::Uint32)a, 1U, 1U });
		}
		seedBox = sfg::Entry::Create();
		seedBox->SetAllocation({ 0.0f, 0.0f, 75.0f, 5.0f });
		seedBox->SetRequisition({ 75.0f, 5.0f });
		seedBox->SetMaximumLength(8);
		randomSeed = sfg::CheckButton::Create("Random seed");
		randomSeed->SetActive(true);
		auto box1 = sfg::Box::Create(sfg::Box::Orientation::VERTICAL, 10.0f);
		auto box2 = sfg::Box::Create(sfg::Box::Orientation::HORIZONTAL, 5.0f);
		gen = sfg::Button::Create("Generate");
		reload = sfg::Button::Create("Reload Files");
		box1->Pack(table1);
		box1->Pack(box2);
		box1->Pack(gen);
		box1->Pack(reload);
		box2->Pack(randomSeed);
		box2->Pack(seedBox);
		window1->Add(box1);
		addWindow(window1, UIAlign({ 1.0f, 0.0f, 210.0f, 120.0f }, UI::ALIGN_RIGHT | UI::ALIGN_FRAC_POSX));
	}

	SiteMenu::SiteMenu() {
		window = sfg::Window::Create(sfg::Window::Style::BACKGROUND | sfg::Window::Style::TITLEBAR);
		auto sWindow = sfg::ScrolledWindow::Create(sfg::Adjustment::Create(), sfg::Adjustment::Create());
		int indexTab = 0;
		auto notebook = sfg::Notebook::Create();
		array<const char*, 4> tabs = { "Civilian", "Military", "Prisoner", "Undead" };
		for (const auto& t : tabs) {
			int index = 0;
			auto table = sfg::Table::Create();
			for (const auto& p : Population::activityNames) {
				auto label = sfg::Label::Create(p);
				label->SetAlignment({ 0.0f, 0.0f });
				table->Attach(label, { 0U, (sf::Uint32)index, 1U, 1U });
				auto slider = sfg::Scale::Create(0.0f, 100.0f, 1.0f);
				auto spin = sfg::SpinButton::Create(slider->GetAdjustment());
				table->Attach(slider, { 1U, (sf::Uint32)index, 1U, 1U });
				table->Attach(spin, { 2U, (sf::Uint32)index, 1U, 1U });
				index++;
			}
			notebook->AppendPage(table, sfg::Label::Create(tabs[indexTab]));
			indexTab++;
		}
		sWindow->AddWithViewport(notebook);
		sWindow->SetScrollbarPolicy(sfg::ScrolledWindow::ScrollbarPolicy::HORIZONTAL_NEVER | sfg::ScrolledWindow::ScrollbarPolicy::VERTICAL_AUTOMATIC);
		window->Add(sWindow);
		addWindow(window, UIAlign({ 1.0f, 1.0f, 250.0f, 300.0f }, UI::ALIGN_RIGHT | UI::ALIGN_FRAC_POSX | UI::ALIGN_BOTTOM | UI::ALIGN_FRAC_POSY));
	}
	void SiteMenu::setSite(const Site& site) {
	}
}