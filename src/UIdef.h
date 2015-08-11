#ifndef UIDEF_H
#define UIDEF_H

#include "UI.h"
#include <array>
#include "Site.h"

namespace UIdef {
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
		MapEntity* ent_;
		SiteMenu();
	public:
		static shared_ptr<SiteMenu> instance();
		void setEntity(MapEntity& ent);
		void updateSitePop();
		shared_ptr<sfg::Window> window;
	};
	class DeployGroupMenu : public UILayout {
		DeployGroupMenu();
		shared_ptr<sfg::Label> coordLabel_;
		shared_ptr<sfg::Button> selectCoordButton_;
		sf::Vector2i deployTo_;
		vector<shared_ptr<sfg::Label>> popLabel_;
		vector<shared_ptr<sfg::Label>> resLabel_;
		int deploySignal_;
	public:
		MapEntity* ent;
		vector<shared_ptr<sfg::Adjustment>> popAdjust;
		vector<shared_ptr<sfg::Adjustment>> resAdjust;
		static shared_ptr<DeployGroupMenu> instance();
		void setEntity(MapEntity& s);
		void setCoord(const sf::Vector2i& coord);
		void updateSitePop();
		void updateSiteResources();
		shared_ptr<sfg::Window> window;
	};
	void setEntity(MapEntity& ent);
	// Update all UI elements that refer to ent pop sizes
	void updateSitePop();
	// Update all UI elements that refer to ent resource amounts
	void updateSiteResources();
	void setSelection(const sf::Vector2i& selection);
}

#endif