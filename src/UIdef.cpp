#include "UIdef.h"
#include "HexTile.h"
#include "MapEntity.h"
#include "States.h"

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

	shared_ptr<SiteMenu> SiteMenu::instance()
	{
		static auto sm = make_shared<SiteMenu>(SiteMenu());
		return sm;
	}
	SiteMenu::SiteMenu() {
		window = sfg::Window::Create(sfg::Window::Style::BACKGROUND);
		auto sWindow = sfg::ScrolledWindow::Create(sfg::Adjustment::Create(), sfg::Adjustment::Create());
		auto mainBox = sfg::Box::Create(sfg::Box::Orientation::VERTICAL);
		auto deployGroupButton = sfg::Button::Create("Deploy Group");
		deployGroupButton->GetSignal(sfg::Button::OnLeftClick).Connect([]() {
			DeployGroupMenu::instance()->show(true);
			DeployGroupMenu::instance()->bringToFront();
			// Simulate a mouse button released event when the window
			// is spawned, since that event isn't caught when the window
			// is closed. It's SFGUI, not me, man.
			sf::Event e;
			e.type = sf::Event::MouseButtonReleased;
			e.mouseButton.button = sf::Mouse::Left;
			DeployGroupMenu::instance()->window->HandleEvent(e);
		});
		UI::connectMouseInputFlag(deployGroupButton);
		mainBox->Pack(deployGroupButton);
		// Population activities
		////////////////////////
		int indexTab = 0;
		auto notebook = sfg::Notebook::Create();
		for (const auto& t : Population::groupNames) {
			sliders.push_back({});
			auto& group = Population::activityNames[indexTab];
			auto& sliderBack = sliders.back();
			auto boxV = sfg::Box::Create(sfg::Box::Orientation::VERTICAL);
			shared_ptr<sfg::Box> boxH = sfg::Box::Create();
			population.push_back(sfg::Label::Create());
			auto pop = population.back();
			boxH->Pack(pop, false);
			boxV->Pack(boxH, false);
			//
			boxH = sfg::Box::Create();
			idlePercent.push_back(sfg::Label::Create());
			auto ip = idlePercent.back();
			boxH->Pack(ip, false);
			boxV->Pack(boxH, false);
			boxH->SetRequisition({ 20.0f, 30.0f });
			for (int index = 1; index < group.size(); index++) {
				boxH = sfg::Box::Create();
				// The name of the activity
				auto label = sfg::Label::Create(group[index]);
				label->SetAlignment({ 0.0f, 0.0f });
				boxH->Pack(label, false);
				// Horizontal percentage slider
				auto slider = sfg::Scale::Create(0.0f, 100.0f, 1.0f);
				// Spin button for adjustment
				auto spin = sfg::SpinButton::Create(slider->GetAdjustment());
				sliderBack.push_back(slider->GetAdjustment());
				boxH->Pack(slider, true);
				boxH->Pack(spin, false);
				label->SetRequisition({ 40.0f, 20.0f });
				spin->SetRequisition({ 50.0f, 20.0f });
				boxV->Pack(boxH, false);
			}
			notebook->AppendPage(boxV, sfg::Label::Create(t));
			indexTab++;
		}
		//auto boxH = sfg::Box::Create();
		//mainBox->Pack(sfg::Separator::Create());
		mainBox->Pack(notebook);
		sWindow->AddWithViewport(mainBox);
		sWindow->SetScrollbarPolicy(sfg::ScrolledWindow::ScrollbarPolicy::HORIZONTAL_NEVER | sfg::ScrolledWindow::ScrollbarPolicy::VERTICAL_AUTOMATIC);
		window->Add(sWindow);
		addWindow(window, UIAlign({ 1.0f, 0.0f, 250.0f, 300.0f }, UI::ALIGN_RIGHT | UI::ALIGN_FRAC_POSX | UI::ALIGN_FRAC_POSY));
	}
	void SiteMenu::setSite(Site& site) {
		site_ = &site;
		for (int indexTab = 0; indexTab < site.pop.activities().size(); indexTab++) {
			int size = site.pop.activities()[indexTab].size();
			stringstream ss;
			ss << "Pop: " << site.pop.size(indexTab);
			population[indexTab]->SetText(ss.str());
			ss.str(std::string());
			ss << "Idle: " << site.pop.activities()[indexTab][Population::IDLE] << "%";
			idlePercent[indexTab]->SetText(ss.str());
			for (int index = 1; index < size; index++) {
				// Link up the slider value to one of the population percentages
				sliders[indexTab][index - 1]->GetSignal(sfg::Adjustment::OnChange).Connect(bind(&SiteMenu::adjust, this, indexTab, index));
				sliders[indexTab][index - 1]->SetValue(site.pop.activities()[indexTab][index]);
			}
		}
	}
	void SiteMenu::adjust(int group, int act) {
		sliders[group][act - 1]->SetValue(site_->pop.set(group, act, sliders[group][act - 1]->GetValue()));
		stringstream ss;
		ss << "Idle: " << site_->pop.activities()[group][Population::IDLE] << "%";
		idlePercent[group]->SetText(ss.str());
	}

	shared_ptr<DeployGroupMenu> DeployGroupMenu::instance()
	{
		static auto dgm = make_shared<DeployGroupMenu>(DeployGroupMenu());
		return dgm;
	}
	DeployGroupMenu::DeployGroupMenu()
	{
		window = sfg::Window::Create(sfg::Window::Style::TOPLEVEL | sfg::Window::Style::CLOSE);
		window->GetSignal(sfg::Window::OnCloseButton).Connect(bind([](shared_ptr<sfg::Window> win) {
			win->Show(false);
		}, window));
		auto mainBox = sfg::Box::Create(sfg::Box::Orientation::VERTICAL);
		selectCoordButton = sfg::Button::Create("Choose");
		mainBox->Pack(selectCoordButton);
		window->Add(mainBox);
		addWindow(window, UIAlign({ 0.5f, 0.5f, 250.0f, 300.0f },
			UI::ALIGN_CENTERX | UI::ALIGN_FRAC_POSX | UI::ALIGN_CENTERY | UI::ALIGN_FRAC_POSY, false));
	}
	void DeployGroupMenu::setSite(Site& site)
	{
		site_ = &site;
		static auto createSelection = [](Site* site) {
			auto vs = make_shared<VectorSet>();
			HexMap::neighbors(site->getMapPos(), *vs);
			SFMLEngine::instance().pushState(new SelectState(vs));
		};
		deployTo_ = { -1, -1 };
		selectCoordButton->GetSignal(sfg::Button::OnLeftClick).Connect(bind(createSelection, &site));
		UI::connectMouseInputFlag(selectCoordButton);
	}

	void setSite(Site& site)
	{
		SiteMenu::instance()->setSite(site);
		DeployGroupMenu::instance()->setSite(site);
	}
}