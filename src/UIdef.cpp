#include <assert.h>
#include "UIdef.h"
#include "HexTile.h"
#include "MapEntity.h"
#include "States.h"

namespace UIdef {
	MapEntity* selectedEnt = nullptr;
	MapUnit* selectedUnit = nullptr;
	SiteSettlement* selectedSettlement = nullptr;
	SiteDungeon* selectedDungeon = nullptr;

	shared_ptr<MapGenDebug> MapGenDebug::instance() {
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
		gen->GetSignal(sfg::Button::OnLeftClick).Connect(bind(&EngineState::generate, EngineState::instance().get()));
		reload = sfg::Button::Create("Reload Files");
		UI::connectMouseInputFlag(reload);
		reload->GetSignal(sfg::Button::OnLeftClick).Connect(bind(&EngineState::loadResourcesInPlace, EngineState::instance().get()));
		box1->Pack(table1);
		box1->Pack(box2);
		box1->Pack(gen);
		box1->Pack(reload);
		box2->Pack(randomSeed);
		box2->Pack(seedBox);
		window1->Add(box1);
		addWindow(window1, UIAlign({ 1.0f, 0.0f, 210.0f, 120.0f }, UI::ALIGN_RIGHT | UI::ALIGN_FRAC_POSX));
	}
	void MapGenDebug::updateDebugInfo(const sf::Vector2i& mousePos, const sf::Vector2i& tilePos, const sf::Vector2i& camPos) {
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

	shared_ptr<SiteMenu> SiteMenu::instance() {
		static auto sm = make_shared<SiteMenu>(SiteMenu());
		return sm;
	}
	SiteMenu::SiteMenu() {
		window = sfg::Window::Create(sfg::Window::Style::BACKGROUND);
		auto sWindow = sfg::ScrolledWindow::Create(sfg::Adjustment::Create(), sfg::Adjustment::Create());
		auto mainBox = sfg::Box::Create(sfg::Box::Orientation::VERTICAL);
		auto infoTable = sfg::Table::Create();
		array<const char*, 2U> name = { "Name:", "Type:" };
		for (int a = 0; a < name.size(); a++) {
			auto label = sfg::Label::Create(name[a]);
			infoTable->Attach(label, { 0U, (sf::Uint32)a, 1U, 1U });
		}
		mainBox->Pack(infoTable);
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
		for (const auto& t : Population::groups) {
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
			notebook->AppendPage(boxV, sfg::Label::Create(t.name));
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
	void SiteMenu::updateSiteInfo() {
		assert(selectedSettlement != nullptr);
		for (int indexTab = 0; indexTab < selectedSettlement->pop.activities().size(); indexTab++) {
			int size = selectedSettlement->pop.activities()[indexTab].size();
			stringstream ss;
			ss << "Pop: " << selectedSettlement->pop.size(indexTab);
			population[indexTab]->SetText(ss.str());
			ss.str(std::string());
			ss << "Idle: " << selectedSettlement->pop.activities()[indexTab][Population::IDLE] << "%";
			idlePercent[indexTab]->SetText(ss.str());
			for (int index = 1; index < size; index++) {
				// Link up the slider value to one of the population percentages
				sliders[indexTab][index - 1]->GetSignal(sfg::Adjustment::OnChange).Connect(bind(&SiteMenu::adjust, this, indexTab, index));
				sliders[indexTab][index - 1]->SetValue(selectedSettlement->pop.activities()[indexTab][index]);
			}
		}
	}
	void SiteMenu::updateSitePop() {
		stringstream ss;
		for (int indexTab = 0; indexTab < selectedSettlement->pop.activities().size(); indexTab++) {
			ss << "Pop: " << selectedSettlement->pop.size(indexTab);
			population[indexTab]->SetText(ss.str());
			ss.str(std::string());
		}
	}
	void SiteMenu::adjust(int group, int act) {
		sliders[group][act - 1]->SetValue(selectedSettlement->pop.set(group, act, sliders[group][act - 1]->GetValue()));
		stringstream ss;
		ss << "Idle: " << selectedSettlement->pop.activities()[group][Population::IDLE] << "%";
		idlePercent[group]->SetText(ss.str());
	}



	const float DungeonMenu::DangerBar::horizontalStretch = 2.0f;
	const sf::Vector2f DungeonMenu::DangerBar::size(100.0f * horizontalStretch, 20.0f);
	DungeonMenu::DangerBar::DangerBar() {
		arrow.setTexture(UI::texture());
		sf::FloatRect trect = *UI::sprites().spr("/danger/arrow");
		arrow.setTextureRect((sf::IntRect)trect);
		arrow.setOrigin(sf::Vector2f((arrow.getTextureRect().width / 2) - 1, -size.y / 2.0f));
		dangerText.setFont(UI::font());
		dangerText.setCharacterSize(13u);
	}
	void DungeonMenu::DangerBar::setTargetDanger(unsigned char targetDanger) {
		static const sf::Vector2f yExtents(-15.0f, 6.0f);
		static const sf::Vector2f gExtents(-8.0f, 4.0f);
		red.setSize(size);
		red.setOutlineThickness(1.0f);
		red.setOutlineColor(sf::Color(195, 174, 0));
		red.setFillColor(sf::Color(159, 0, 0));
		red.setOrigin(sf::Vector2f(-1.0f, -1.0f));
		yellow.setSize({ (yExtents.y - yExtents.x) * horizontalStretch, size.y });
		yellow.setOrigin({ -1.0f, -1.0f });
		yellow.setPosition({ (targetDanger + yExtents.x) * horizontalStretch, 0.0f });
		yellow.setFillColor(sf::Color(255, 204, 0));
		green.setSize({ (gExtents.y - gExtents.x) * horizontalStretch, size.y });
		green.setOrigin({ -1.0f, -1.0f });
		green.setPosition({ (targetDanger + gExtents.x) * horizontalStretch, 0.0f });
		green.setFillColor(sf::Color(0, 159, 64));
	}
	void DungeonMenu::DangerBar::setDanger(unsigned char danger) {
		arrow.setPosition(sf::Vector2f(danger * horizontalStretch, 0.0f));
		dangerText.setString(std::to_string(danger));
		auto bounds = dangerText.getLocalBounds();
		auto arrowBounds = arrow.getLocalBounds();
		dangerText.setOrigin(sf::Vector2f((bounds.width / 2) - 1, -(arrowBounds.top + arrowBounds.height) + arrow.getOrigin().y));
		dangerText.setPosition(sf::Vector2f(danger * horizontalStretch, 0.0f));
	}
	void DungeonMenu::DangerBar::draw(sf::RenderTarget& target, sf::RenderStates states) const {
		states.transform *= this->getTransform();
		target.draw(red, states);
		target.draw(yellow, states);
		target.draw(green, states);
		target.draw(arrow, states);
		target.draw(dangerText, states);
	}
	shared_ptr<DungeonMenu> DungeonMenu::instance() {
		static auto dm = make_shared<DungeonMenu>(DungeonMenu());
		return dm;
	}
	DungeonMenu::DungeonMenu() {
	//	unsigned char targetDanger;
	//	float popularity;
	//	unsigned char danger;
	//	int gold;
	//	int wins;
	//	int losses;
		dangerBarCanvas_ = sfg::Canvas::Create();
		//dangerBarCanvas_->GetSignal(sfg::Canvas::OnSizeAllocate).Connect(std::bind(&DungeonMenu::recenterDangerBar, this));
		dangerBarCanvas_->SetRequisition({ 1.0f, 1.0f });
		dangerBarView_.setCenter(0.0f, 0.0f);
		window = sfg::Window::Create(sfg::Window::Style::BACKGROUND);
		auto box = sfg::Box::Create(sfg::Box::Orientation::VERTICAL);
		popularity_ = sfg::Label::Create("");
		gold_ = sfg::Label::Create("");
		box->Pack(popularity_, false);
		box->Pack(gold_, false);
		box->Pack(dangerBarCanvas_);
		window->Add(box);
		addWindow(window, UIAlign({ 1.0f, 1.0f, 250.0f, 300.0f }, UI::ALIGN_RIGHT | UI::ALIGN_FRAC_POSX | UI::ALIGN_FRAC_POSY | UI::ALIGN_BOTTOM));
	}
	void DungeonMenu::updateDungeonInfo() {
		assert(selectedDungeon != nullptr);
		dangerBar_.setDanger(selectedDungeon->danger);
		dangerBar_.setTargetDanger(selectedDungeon->targetDanger);
		popularity_->SetText("Popularity: " + std::to_string((int)std::round(selectedDungeon->popularity)) + "%");
		gold_->SetText("Gold: " + std::to_string(selectedDungeon->gold));
	}
	void DungeonMenu::update(const sf::Time& timeElapsed) {
		//dangerBarCanvas_->SetView(dangerBarView_);
		dangerBarCanvas_->Bind();
		dangerBarCanvas_->Clear(sf::Color::Transparent);
		dangerBarCanvas_->Draw(dangerBar_);
		dangerBarCanvas_->Display();
		dangerBarCanvas_->Unbind();
	}
	void DungeonMenu::recenterDangerBar() {
		auto alloc = dangerBarCanvas_->GetAllocation();
		dangerBarView_.setSize({ alloc.width, alloc.height });
	}



	shared_ptr<DeployGroupMenu> DeployGroupMenu::instance() {
		static auto dgm = make_shared<DeployGroupMenu>(DeployGroupMenu());
		return dgm;
	}
	DeployGroupMenu::DeployGroupMenu() :
		unit_(&MapUnitS::get(MapUnitS::NONE), nullptr, nullptr)
	{
		deploySignal_ = -1;
		window = sfg::Window::Create(sfg::Window::Style::BACKGROUND | sfg::Window::Style::TITLEBAR | sfg::Window::Style::CLOSE);
		window->GetSignal(sfg::Window::OnCloseButton).Connect(bind([](shared_ptr<sfg::Window> win) {win->Show(false); }, window));
		// displays confirmation buttons below main ui
		auto mainBox = sfg::Box::Create(sfg::Box::Orientation::VERTICAL, 8.0f);
		// displays type selection to left of details
		auto typeBox = sfg::Box::Create();
		// position and entity options
		auto detailsBox = sfg::Box::Create(sfg::Box::Orientation::VERTICAL);
		// places preview of entity sprite below type selection combobox
		auto previewBox = sfg::Box::Create(sfg::Box::Orientation::VERTICAL);
		// Type Select
		//////////////
		auto typeFrame = sfg::Frame::Create("Type");
		typeList_ = sfg::ComboBox::Create();
		typeList_->GetSignal(sfg::ComboBox::OnSelect).Connect([](){
			DeployGroupMenu::instance()->updateType();
		});
		typeList_->GetSignal(sfg::ComboBox::OnSelect).Connect(&UI::setMouseFlag);
		UI::connectMouseInputFlag(typeList_);
		for (int u = 1; u < MapUnitS::UNIT_NUM; u++) {
			typeList_->AppendItem(MapUnitS::get(u).name_);
		}
		typeList_->SelectItem(0);
		// Preview
		//////////
		preview_ = sfg::Canvas::Create();
		preview_->GetSignal(sfg::Canvas::OnSizeAllocate).Connect(std::bind(&DeployGroupMenu::recenterPreview, this));
		//preview_->GetSignal(sfg::Canvas::OnSizeAllocate).Connect([=]() mutable {
		//	auto alloc = preview_->GetAllocation();
		//	previewView_.setSize({ alloc.width, alloc.height });
		//});
		preview_->SetRequisition({ 1.0f, 1.0f });
		previewView_.setCenter({ 0.0f, 0.0f });
		previewBox->Pack(typeList_, false);
		previewBox->Pack(preview_);
		typeFrame->Add(previewBox);
		typeBox->Pack(typeFrame);
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
		detailsBox->Pack(posFrame);
		// Settings
		///////////
		auto soldierBox = sfg::Box::Create(sfg::Box::Orientation::VERTICAL, 4.0f);
		auto settingFrame = sfg::Frame::Create("Settings");
		std::array<std::string, 2> aLabel = {"Soldiers: ", "Days of rations: "};
		for (int a = 0; a < 2; a++) {
			auto lBox = sfg::Box::Create();
			lBox->Pack(sfg::Label::Create(aLabel[a]), false);
			soldierBox->Pack(lBox);
			sfg::Box::Create()->Pack(sfg::Label::Create(), false);
			auto boxS = sfg::Box::Create();
			// Spin button for adjustment
			auto spin = sfg::SpinButton::Create(0.0f, 100.0f, 1.0f);
			armyAdjust.push_back(spin->GetAdjustment());
			spin->GetSignal(sfg::SpinButton::OnValueChanged).Connect([]() {
				DeployGroupMenu::instance()->updateSiteResources();
			});
			UI::connectKeyboardInputFlag(spin);
			UI::connectMouseInputFlag(spin);
			boxS->Pack(spin, false);
			spin->SetRequisition({ 140.0f, 20.0f });
			// Label
			auto l = sfg::Label::Create();
			//l->SetRequisition({50.0f, 10.0f});
			armyLabel_.emplace_back(l);
			boxS->Pack(armyLabel_.back(), false);
			boxS->SetRequisition({ 300.0f, 5.0f });
			soldierBox->Pack(boxS, true, false);
		}
		settingFrame->Add(soldierBox);
		detailsBox->Pack(settingFrame);
		typeBox->Pack(detailsBox);
		mainBox->Pack(typeBox);
		// Options
		//////////
		// arranges confirmation buttons horizontally
		auto optionBox = sfg::Box::Create(sfg::Box::Orientation::HORIZONTAL, 10.0f);
		// Deploy
		auto deployButton = sfg::Button::Create("Deploy");
		deployButton->GetSignal(sfg::Button::OnLeftClick).Connect([]() {
			DeployGroupMenu::instance()->deployUnit();
		});
		optionBox->Pack(deployButton, false, false);
		UI::connectMouseInputFlag(deployButton);
		// Cancel
		auto cancelButton = sfg::Button::Create("Cancel");
		//addTooltip(cancelButton)->Add(sfg::Label::Create("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua."));
		cancelButton->GetSignal(sfg::Button::OnLeftClick).Connect(bind([](shared_ptr<sfg::Window> win) {win->Show(false); }, window));
		UI::connectMouseInputFlag(cancelButton);
		optionBox->Pack(cancelButton, false, false);
		mainBox->Pack(optionBox, true, false);
		// Alignment
		////////////
		window->Add(mainBox);
		addWindow(window, UIAlign({ 0.5f, 0.5f, 0.0f, 0.0f },
			UI::ALIGN_CENTERX | UI::ALIGN_FRAC_POSX | UI::ALIGN_CENTERY | UI::ALIGN_FRAC_POSY, false));
	}
	void DeployGroupMenu::updateSiteInfo() {
		assert(selectedSettlement != nullptr);
		static auto createSelection = [](Site* site) {
			auto vs = make_shared<VectorSet>();
			HexMap::neighbors(site->getMapPos(), *vs);
			SFMLEngine::instance().pushState(std::shared_ptr<GameState>(new SelectState(vs, setSelection)));
		};
		setCoord({ -1, -1 });
		if (deploySignal_ >= 0) {
			selectCoordButton_->GetSignal(sfg::Button::OnLeftClick).Disconnect(deploySignal_);
		}
		deploySignal_ = selectCoordButton_->GetSignal(sfg::Button::OnLeftClick).Connect(bind(createSelection, selectedSettlement));
		updateType();
		updateSiteResources();
	}
	void DeployGroupMenu::setCoord(const sf::Vector2i& coord) {
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
	void DeployGroupMenu::updateSitePop() {
		stringstream ss;
		// population related to currently selected unit type
		armyAdjust[0]->SetUpper((int)selectedSettlement->pop.size(unit_.getMemberType()));
		ss << " / " << (int)selectedSettlement->pop.size(unit_.getMemberType());
		armyLabel_[0]->SetText(ss.str());
	}
	void DeployGroupMenu::updateSiteResources() {
		if (armyAdjust[1]->GetValue() == 1) {
			int x = 0;
		}
		stringstream ss;
		// food stocks
		int totalFood = (int)armyAdjust[1]->GetValue() * (int)armyAdjust[0]->GetValue();
		ss << " (" << totalFood << " / " << (int)selectedSettlement->resources[SiteS::FOOD] << " food)";
		if ((int)armyAdjust[0]->GetValue() < 1) {
			armyAdjust[1]->SetUpper(0.0f);
		}
		else {
			armyAdjust[1]->SetUpper((int)selectedSettlement->resources[SiteS::FOOD] / (int)armyAdjust[0]->GetValue());
		}
		armyLabel_[1]->SetText(ss.str());
		unit_.setHealth(armyAdjust[0]->GetValue());
		unit_.setFood(totalFood);
		//ss.str(string());
	}
	void DeployGroupMenu::reset() {
		for (auto a : armyAdjust) {
			a->SetValue(0.0f);
		}
		setCoord({ -1, -1 });
	}
	bool DeployGroupMenu::optionsValid() {
		if (deployTo_ == sf::Vector2i(-1, -1)) {
			return false;
		}
		return true;
	}
	void DeployGroupMenu::update(const sf::Time& timeElapsed) {
		MapEntity::setZoomLevel(0);
		unit_.updateAnimation(timeElapsed);
		preview_->SetView(previewView_);
		preview_->Bind();
		preview_->Clear(sf::Color::Transparent);
		preview_->Draw(unit_);
		preview_->Display();
		preview_->Unbind();
	}
	void DeployGroupMenu::recenterPreview() {
		auto alloc = preview_->GetAllocation();
		previewView_.setSize({alloc.width, alloc.height});
	}
	void DeployGroupMenu::updateType() {
		int index = typeList_->GetSelectedItem();
		unit_.setStaticUnit(&MapUnitS::get(index + 1));
		updateSitePop();
	}
	int DeployGroupMenu::getType() {
		return typeList_->GetSelectedItem() + 1;
	}
	void DeployGroupMenu::deployUnit() {
		if (optionsValid()) {
			int choice = getType();
			auto* deployed = HEXMAP.addMapUnit(&MapUnitS::get(choice), HEXMAP.playerFaction());
			deployed->initMapPos(deployTo_);
			deployed->setHealth(armyAdjust[0]->GetValue());
			deployed->setFood(armyAdjust[1]->GetValue() * armyAdjust[0]->GetValue());
			selectedSettlement->deployUnit(*deployed);
			// Reset UI
			reset();
			//UIdef::updateSitePop();
			//UIdef::updateSiteResources();
			window->Show(false);
		}
	}


	
	shared_ptr<MapUnitInfo> MapUnitInfo::instance() {
		static auto mui = make_shared<MapUnitInfo>(MapUnitInfo());
		return mui;
	}
	MapUnitInfo::MapUnitInfo() {
		auto window = sfg::Window::Create(sfg::Window::Style::TOPLEVEL | sfg::Window::Style::CLOSE);
		auto mainBox = sfg::Box::Create();
		window->Add(mainBox);
		addWindow(window, UIAlign({ 1.0f, 1.0f, 250.0f, 300.0f }, UI::ALIGN_RIGHT | UI::ALIGN_FRAC_POSX | UI::ALIGN_FRAC_POSY | UI::ALIGN_BOTTOM));
	}



	void deselectEnt() {
		if (selectedEnt != nullptr) {
			selectedEnt->deselect();
			selectedEnt = nullptr;
			selectedSettlement = nullptr;
			selectedUnit = nullptr;
		}
	}
	void selectEnt(MapEntity& ent) {
		selectedEnt = &ent;
		ent.select();
	}
	void setSettlement(SiteSettlement& site) {
		selectedEnt = &site;
		selectedSettlement = &site;
		selectedDungeon = nullptr;
		selectedUnit = nullptr;
		SiteMenu::instance()->updateSiteInfo();
		DeployGroupMenu::instance()->updateSiteInfo();
	}
	void setDungeon(SiteDungeon& dungeon) {
		selectedEnt = &dungeon;
		selectedSettlement = nullptr;
		selectedDungeon = &dungeon;
		selectedUnit = nullptr;
		DungeonMenu::instance()->updateDungeonInfo();
	}
	void setUnit(MapUnit& unit) {
		selectedEnt = &unit;
		selectedSettlement = nullptr;
		selectedDungeon = nullptr;
		selectedUnit = &unit;
	}
	void updateSitePop() {
		if (selectedSettlement == nullptr) {
			return;
		}
		SiteMenu::instance()->updateSitePop();
		DeployGroupMenu::instance()->updateSitePop();
	}
	void updateSiteResources() {
		if (selectedSettlement == nullptr) {
			return;
		}
		DeployGroupMenu::instance()->updateSiteResources();
	}
	void setSelection(const sf::Vector2i& selection) {
		DeployGroupMenu::instance()->setCoord(selection);
	}
}
