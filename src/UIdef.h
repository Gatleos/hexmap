#ifndef UIDEF_H
#define UIDEF_H

#include "UI.h"
#include <array>

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
}

#endif