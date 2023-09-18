
#include "mcplayer.h"
#include "mcscene.h"

namespace mocos2d {

void MCPlayer::init(Vec3 pos, char idleAni) {
	setAnimations();

	mIdleAni = idleAni;
	mBorable = true;
	setPosition3D(pos);

	MCScene::getInstance()->getRoot()->addChild(this);
	setCameraMask((ushort) CameraFlag::USER2);

	if (_appSetting.uiAssist) {
		mDraw = DrawNode::create();
		mDraw->setCameraMask((ushort) CameraFlag::USER7);
		MCScene::getInstance()->getRoot()->addChild(mDraw);
		mDraw->setName("hit_rect");
		Rect rect = getHitRect();
		mDraw->drawRect(Vec2(rect.getMinX(), rect.getMinY()), Vec2(rect.getMaxX(), rect.getMaxY()), Color4F::RED);
	} else {
		mDraw = NULL;
	}
}

void MCPlayer::setIdleAni(char ani) {
	mIdleAni = ani;
}

void MCPlayer::enableBore(bool enable) {
	mBorable = enable;
}

char MCPlayer::getAniId(char ani) {
	for (char i = 0; i < mAvAni.size(); ++i) {
		if (mAvAni[i] == ani) return i;
	}
	return -1;
}

void MCPlayer::freePlay() {
	play(mIdleAni, true);
}

void MCPlayer::play(char ani, bool loop, bool immediate) {
	stopAllAni();

	if (loop) {
		playLoopAni(ani);
	} else {
		playLoopAniAfterAni(ani, mIdleAni);
	}
	if (loop && (ani == mIdleAni)) {
		schedule(schedule_selector(MCPlayer::_wait), (rand() % 5) + 5.f);
	} else {
		unscheduleAllCallbacks();
	}
}

Vec2 MCPlayer::getWorldPos(Vec2 pos) {
	return getPosition() + pos;
}

void MCPlayer::_wait(float dt) {
	if (!mBorable) return;
	if (rand() % 10 >= 3) return;	
	char boreAni = getBoreAni(mIdleAni);
	if (getAniId(boreAni) < 0) return;
	play(boreAni, false);
}

void MCPlayer::moveTo(Vec2 pos) {
	setPosition(pos);
}

void MCPlayer::remove() {
	if (mDraw) mDraw->removeFromParent();
	removeFromParent();
}

}