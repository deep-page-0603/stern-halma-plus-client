
#include "cl2dplayer.h"

namespace cardlib {

CL2DPlayer::~CL2DPlayer() {}

void CL2DPlayer::init(Vec2 pos, float rot) {
	mPos = pos;
	mRot = rot;
	setRotation(rot);
	createBody();

	MCPlayer::init(MCUtil::cvtToVec3(pos), -1);
}

void CL2DPlayer::remove() {
	MCPlayer::remove();
}

}