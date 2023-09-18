
#include "mctoast.h"

namespace mocos2d {

bool MCToast::init() {
	if (!Node::init()) return false;

	mToastBk = ImageView::create("mocos/toast_bk.png");
	mToastBk->setScale9Enabled(true);
	mToastBk->setCapInsets(Rect(70.f, 48.f, 70.f, 48.f));
	addChild(mToastBk);

	mToastText = Text::create("", _appSetting.defaultFont, 36.f);
	addChild(mToastText);

	mToastBk->setOpacity(0);
	mToastText->setOpacity(0);
	return true;
}

void MCToast::show(string msg, float dur) {
	mToastBk->stopAllActions();
	mToastText->stopAllActions();
	mToastText->setString(msg);
	
	mToastBk->setContentSize(mToastText->getContentSize() + Size(150.f, 70.f));
	mToastBk->setOpacity(200);
	mToastText->setOpacity(255);
	mToastBk->runAction(Sequence::createWithTwoActions(DelayTime::create(dur), FadeTo::create(1.f, 0)));
	mToastText->runAction(Sequence::createWithTwoActions(DelayTime::create(dur), FadeTo::create(1.f, 0)));
}

void MCToast::hide() {
	mToastBk->stopAllActions();
	mToastText->stopAllActions();
	mToastBk->setOpacity(0);
	mToastText->setOpacity(0);
}

}