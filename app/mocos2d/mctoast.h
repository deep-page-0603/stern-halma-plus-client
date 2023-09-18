
#pragma once

#include "mocos2d.h"

namespace mocos2d {

class MCToast : public Node {
public:
	CREATE_FUNC(MCToast);
	
	virtual bool init();
	void show(string msg, float dur = 2.f);
	void hide();

public:
	ImageView* mToastBk;
	Text* mToastText;
};

}