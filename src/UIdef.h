#ifndef UIDEF_H
#define UIDEF_H

#include "UI.h"
#include <array>
#include "Site.h"
#include "MapUnit.h"

namespace UIdef {
	extern MapEntity* selectedEnt;
	class MapGenDebug : public UILayout {
		MapGenDebug();
	public:
		static shared_ptr<MapGenDebug> instance();
		shared_ptr<sfg::Button> gen;
		shared_ptr<sfg::Button> reload;
		array<shared_ptr<sfg::Label>, 6U> debugInfo;
		shared_ptr<sfg::Entry> seedBox;
		shared_ptr<sfg::CheckButton> randomSeed;
		void updateDebugInfo(sf::Vector2i& mousePos, sf::Vector2i& tilePos, sf::Vector2i& camPos);
	};
	class SiteMenu : public UILayout {
		vector<vector<shared_ptr<sfg::Adjustment>>> sliders;
		vector<shared_ptr<sfg::Label>> idlePercent;
		vector<shared_ptr<sfg::Label>> population;
		void adjust(int group, int act);
		Site* site_;
		SiteMenu();
	public:
		static shared_ptr<SiteMenu> instance();
		void setSite(Site& ent);
		void updateSitePop();
		shared_ptr<sfg::Window> window;
	};
	class DeployGroupMenu : public UILayout {
		DeployGroupMenu();
		// UI elements
		shared_ptr<sfg::Label> coordLabel_;
		shared_ptr<sfg::Button> selectCoordButton_;
		shared_ptr<sfg::Canvas> preview_;
		vector<shared_ptr<sfg::Label>> armyLabel_;
		shared_ptr<sfg::ComboBox> typeList_;
		//
		sf::Vector2i deployTo_;
		int deploySignal_;
		MapUnit unit_;
		sf::View previewView_;
	public:
		Site* site_;
		vector<shared_ptr<sfg::Adjustment>> armyAdjust;
		static shared_ptr<DeployGroupMenu> instance();
		void setSite(Site& site);
		void setCoord(const sf::Vector2i& coord);
		void updateSitePop();
		void updateSiteResources();
		void reset();
		bool optionsValid();
		void update(const sf::Time& timeElapsed);
		void recenterPreview();
		void updateType();
		shared_ptr<sfg::Window> window;
	};
	class MapUnitInfo : public UILayout {
		MapUnitInfo();
	public:
		static shared_ptr<MapUnitInfo> instance();
	};
	// Set site-related UI elements to point to this site
	void setSite(Site& site);
	// Set unit-related UI elements to point to this unit
	void setUnit(MapUnit& unit);
	// Update all UI elements that refer to ent pop sizes
	void updateSitePop();
	// Update all UI elements that refer to ent resource amounts
	void updateSiteResources();
	void setSelection(const sf::Vector2i& selection);
}

#endif