#include "UIdef.h"
#include "HexTile.h"

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
		addWindow(window, UIAlign({ 1.0f, 1.0f, 350.0f, 120.0f }, UI::ALIGN_RIGHT | UI::ALIGN_FRAC_POSX | UI::ALIGN_BOTTOM | UI::ALIGN_FRAC_POSY));
	}
	void SiteMenu::setSite(const Site& site) {
		auto sWindow = sfg::ScrolledWindow::Create(sfg::Adjustment::Create(), sfg::Adjustment::Create());
		auto table = sfg::Table::Create();
		int index = 0;
		for (auto& p : site.pops) {
			auto label = sfg::Label::Create(p.species.name);
			label->SetAlignment({ 0.0f, 0.0f });
			table->Attach(label, { 0U, (sf::Uint32)index, 1U, 1U });
			for (int a = 1; a <= Population::ACTIVITY_NUM; a++) {
				auto spinButton = sfg::SpinButton::Create(0.0f, 100.0f, 1.0f);
				spinButton->SetDigits(0U);
				spinButton->SetRequisition({ 50.0f, 20.0f });
				table->Attach(spinButton, { (sf::Uint32)a, (sf::Uint32)index, 1U, 1U });
			}
			index++;
		}
		//window->GetSignal(sfg::Window::OnLeftClick).Connect(UI::dropFocus);
		//auto image = sfg::Image::Create(HexTileS::getTexture().copyToImage());
		//table->Attach(image, { 1U, 0U, 1U, 1U });
		auto box = sfg::Box::Create();
		box->Pack(table);
		sWindow->AddWithViewport(box);
		//sWindow->SetRequisition(sf::Vector2f(100.f, 100.f));
		sWindow->SetScrollbarPolicy(sfg::ScrolledWindow::ScrollbarPolicy::HORIZONTAL_NEVER | sfg::ScrolledWindow::ScrollbarPolicy::VERTICAL_ALWAYS);
		window->Add(sWindow);
	}
}