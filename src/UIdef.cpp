#include "UIdef.h"
#include "HexTile.h"
#include "MapEntity.h"
#include "States.h"

namespace UIdef {
	shared_ptr<MapGenDebug> MapGenDebug::instance()
	{
		static auto mgd = make_shared<MapGenDebug>(MapGenDebug());
		return mgd;
	}
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
		UI::connectKeyboardInputFlag(seedBox);
		randomSeed = sfg::CheckButton::Create("Random seed");
		randomSeed->SetActive(true);
		UI::connectMouseInputFlag(randomSeed);
		auto box1 = sfg::Box::Create(sfg::Box::Orientation::VERTICAL, 10.0f);
		auto box2 = sfg::Box::Create(sfg::Box::Orientation::HORIZONTAL, 5.0f);
		gen = sfg::Button::Create("Generate");
		UI::connectMouseInputFlag(gen);
		reload = sfg::Button::Create("Reload Files");
		UI::connectMouseInputFlag(reload);
		box1->Pack(table1);
		box1->Pack(box2);
		box1->Pack(gen);
		box1->Pack(reload);
		box2->Pack(randomSeed);
		box2->Pack(seedBox);
		window1->Add(box1);
		addWindow(window1, UIAlign({ 1.0f, 0.0f, 210.0f, 120.0f }, UI::ALIGN_RIGHT | UI::ALIGN_FRAC_POSX));
	}
	void MapGenDebug::updateDebugInfo(sf::Vector2i& mousePos, sf::Vector2i& tilePos, sf::Vector2i& camPos)
	{
		stringstream ss;
		// camPos
		ss << camPos.x << "," << camPos.y;
		debugInfo[0]->SetText(ss.str());
		ss.str(std::string());
		// mousePos
		ss << mousePos.x << "," << mousePos.y;
		debugInfo[1]->SetText(ss.str());
		ss.str(std::string());
		// tilePos
		ss << tilePos.x << "," << tilePos.y;
		debugInfo[2]->SetText(ss.str());
		ss.str(std::string());
		// chunks
		const sf::IntRect& ir = HEXMAP.getChunkViewArea();
		ss << "(" << ir.left << ", " << ir.top << " / " << ir.width << ", " << ir.height << ")";
		debugInfo[5]->SetText(ss.str());
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
		addWindow(window, UIAlign({ 1.0f, 1.0f, 250.0f, 300.0f }, UI::ALIGN_RIGHT | UI::ALIGN_FRAC_POSX | UI::ALIGN_FRAC_POSY | UI::ALIGN_BOTTOM));
	}
	void SiteMenu::setEntity(MapEntity& ent) {
		ent_ = &ent;
		for (int indexTab = 0; indexTab < ent.pop.activities().size(); indexTab++) {
			int size = ent.pop.activities()[indexTab].size();
			stringstream ss;
			ss << "Pop: " << ent.pop.size(indexTab);
			population[indexTab]->SetText(ss.str());
			ss.str(std::string());
			ss << "Idle: " << ent.pop.activities()[indexTab][Population::IDLE] << "%";
			idlePercent[indexTab]->SetText(ss.str());
			for (int index = 1; index < size; index++) {
				// Link up the slider value to one of the population percentages
				sliders[indexTab][index - 1]->GetSignal(sfg::Adjustment::OnChange).Connect(bind(&SiteMenu::adjust, this, indexTab, index));
				sliders[indexTab][index - 1]->SetValue(ent.pop.activities()[indexTab][index]);
			}
		}
	}
	void SiteMenu::updateSitePop()
	{
		stringstream ss;
		for (int indexTab = 0; indexTab < ent_->pop.activities().size(); indexTab++) {
			ss << "Pop: " << ent_->pop.size(indexTab);
			population[indexTab]->SetText(ss.str());
			ss.str(std::string());
		}
	}
	void SiteMenu::adjust(int group, int act) {
		sliders[group][act - 1]->SetValue(ent_->pop.set(group, act, sliders[group][act - 1]->GetValue()));
		stringstream ss;
		ss << "Idle: " << ent_->pop.activities()[group][Population::IDLE] << "%";
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
		window->GetSignal(sfg::Window::OnCloseButton).Connect(bind([](shared_ptr<sfg::Window> win) {win->Show(false); }, window));
		auto mainBox = sfg::Box::Create(sfg::Box::Orientation::VERTICAL);
		// Position
		///////////
		auto posFrame = sfg::Frame::Create("Position");
		auto posBox = sfg::Box::Create();
		coordLabel_ = sfg::Label::Create();
		setCoord({ -1, -1 });
		posBox->Pack(coordLabel_);
		selectCoordButton_ = sfg::Button::Create("Choose");
		UI::connectMouseInputFlag(selectCoordButton_);
		posBox->Pack(selectCoordButton_);
		posFrame->Add(posBox);
		mainBox->Pack(posFrame);
		// People
		/////////
		auto peopleBox = sfg::Box::Create(sfg::Box::Orientation::VERTICAL);
		auto peopleFrame = sfg::Frame::Create("People");
		for (auto& g : Population::groupNames) {
			auto boxH = sfg::Box::Create();
			// Group name
			auto label = sfg::Label::Create(g);
			label->SetAlignment({ 0.0f, 0.0f });
			boxH->Pack(label, false);
			// Spin button for adjustment
			auto spin = sfg::SpinButton::Create(0.0f, 100.0f, 1.0f);
			popAdjust.push_back(spin->GetAdjustment());
			boxH->Pack(spin, false);
			label->SetRequisition({ 60.0f, 20.0f });
			spin->SetRequisition({ 80.0f, 20.0f });
			// Pop total label
			popLabel_.emplace_back(sfg::Label::Create());
			popLabel_.back()->SetRequisition({ 60.0f, 20.0f });
			boxH->Pack(popLabel_.back(), false);
			peopleBox->Pack(boxH, true, false);
		}
		peopleFrame->Add(peopleBox);
		mainBox->Pack(peopleFrame);
		// Resources
		////////////
		auto resourceBox = sfg::Box::Create(sfg::Box::Orientation::VERTICAL);
		auto resourceFrame = sfg::Frame::Create("Resources");
		for (auto& r : MapEntityS::resourceNames) {
			auto boxH = sfg::Box::Create();
			// Resource name
			auto label = sfg::Label::Create(r);
			label->SetAlignment({ 0.0f, 0.0f });
			boxH->Pack(label, false);
			// Spin button for adjustment
			auto spin = sfg::SpinButton::Create(0.0f, 100.0f, 1.0f);
			resAdjust.push_back(spin->GetAdjustment());
			boxH->Pack(spin, false);
			label->SetRequisition({ 60.0f, 20.0f });
			spin->SetRequisition({ 80.0f, 20.0f });
			// Pop total label
			resLabel_.emplace_back(sfg::Label::Create());
			resLabel_.back()->SetRequisition({ 60.0f, 20.0f });
			boxH->Pack(resLabel_.back(), false);
			resourceBox->Pack(boxH, true, false);
		}
		resourceFrame->Add(resourceBox);
		mainBox->Pack(resourceFrame);
		// Options
		//////////
		auto boxH = sfg::Box::Create();
		// Deploy
		auto deployButton = sfg::Button::Create("Deploy");
		deployButton->GetSignal(sfg::Button::OnLeftClick).Connect([]() {
			auto dMenu = DeployGroupMenu::instance();
			if (dMenu->deployTo_ != sf::Vector2i(-1, -1)) {
				auto* deployed = HEXMAP.addMapUnit(&SiteS::get(SiteS::CITY), HEXMAP.addFaction());
				deployed->initMapPos(dMenu->deployTo_);
				for (int i = 0; i < Population::groupNames.size(); i++) {
					deployed->pop.setSize(i, dMenu->popAdjust[i]->GetValue());
					dMenu->ent->pop.addSize(i, -dMenu->popAdjust[i]->GetValue());
					dMenu->popAdjust[i]->SetValue(0.0f);
				}
				UIdef::updateSitePop();
				dMenu->window->Show(false);
			}
		});
		boxH->Pack(deployButton, true, false);
		UI::connectMouseInputFlag(deployButton);
		// Cancel
		auto cancelButton = sfg::Button::Create("Cancel");
		//addTooltip(cancelButton)->Add(sfg::Label::Create("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua."));
		cancelButton->GetSignal(sfg::Button::OnLeftClick).Connect(bind([](shared_ptr<sfg::Window> win) {win->Show(false); }, window));
		UI::connectMouseInputFlag(cancelButton);
		boxH->Pack(cancelButton, true, false);
		mainBox->Pack(boxH, true, false);
		// Alignment
		////////////
		window->Add(mainBox);
		addWindow(window, UIAlign({ 0.5f, 0.5f, 0.0f, 0.0f },
			UI::ALIGN_CENTERX | UI::ALIGN_FRAC_POSX | UI::ALIGN_CENTERY | UI::ALIGN_FRAC_POSY, false));
	}
	void DeployGroupMenu::setEntity(MapEntity& s)
	{
		ent = &s;
		static auto createSelection = [](MapEntity* ent) {
			auto vs = make_shared<VectorSet>();
			HexMap::neighbors(ent->getMapPos(), *vs);
			SFMLEngine::instance().pushState(std::shared_ptr<GameState>(new SelectState(vs, setSelection)));
		};
		setCoord({ -1, -1 });
		selectCoordButton_->GetSignal(sfg::Button::OnLeftClick).Connect(bind(createSelection, ent));
		updateSitePop();
		updateSiteResources();
	}
	void DeployGroupMenu::setCoord(const sf::Vector2i& coord)
	{
		deployTo_ = coord;
		if (HEXMAP.isAxialInBounds(coord)) {
			stringstream ss;
			ss << "(" << coord.x << ", " << coord.y << ")";
			coordLabel_->SetText(ss.str());
		}
		else {
			coordLabel_->SetText("(?, ?)");
		}
	}
	void DeployGroupMenu::updateSitePop()
	{
		stringstream ss;
		for (int i = 0; i < popLabel_.size(); i++) {
			auto p = popLabel_[i];
			ss << " / " << (int)ent->pop.size(i);
			p->SetText(ss.str());
			ss.str(string());
			popAdjust[i]->SetUpper((int)ent->pop.size(i));
		}
	}
	void DeployGroupMenu::updateSiteResources()
	{
		stringstream ss;
		for (int i = 0; i < resLabel_.size(); i++) {
			auto r = resLabel_[i];
			ss << " / " << (int)ent->resources[i];
			r->SetText(ss.str());
			ss.str(string());
			resAdjust[i]->SetUpper((int)ent->resources[i]);
		}
	}


	void setEntity(MapEntity& ent)
	{
		SiteMenu::instance()->setEntity(ent);
		DeployGroupMenu::instance()->setEntity(ent);
	}
	void updateSitePop()
	{
		SiteMenu::instance()->updateSitePop();
		DeployGroupMenu::instance()->updateSitePop();
	}
	void updateSiteResources()
	{
		DeployGroupMenu::instance()->updateSiteResources();
	}
	void setSelection(const sf::Vector2i& selection)
	{
		DeployGroupMenu::instance()->setCoord(selection);
	}
}