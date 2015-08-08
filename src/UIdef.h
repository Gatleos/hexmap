#ifndef UIDEF_H
#define UIDEF_H

#include "UI.h"
#include <array>
#include "Site.h"

namespace UIdef {
	//class site : public UILayout {
	//public:
	//};
	class MapGenDebug : public UILayout {
	public:
		MapGenDebug();
		shared_ptr<sfg::Button> gen;
		shared_ptr<sfg::Button> reload;
		array<shared_ptr<sfg::Label>, 6U> debugInfo;
		shared_ptr<sfg::Entry> seedBox;
		shared_ptr<sfg::CheckButton> randomSeed;
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
		void setSite(Site& site);
		shared_ptr<sfg::Window> window;
	};
	class DeployGroupMenu : public UILayout {
		DeployGroupMenu();
		Site* site_;
		sf::Vector2i deployTo_;
		bool closeClicked_;
		shared_ptr<sfg::Button> selectCoordButton;
	public:
		static shared_ptr<DeployGroupMenu> instance();
		void setSite(Site& site);
		shared_ptr<sfg::Window> window;
	};
	void setSite(Site& site);
}

#endif