
#pragma once

#include "mcplayer.h"
#include "mcutil.h"

using namespace mocos2d;

namespace cardlib {

class CL2DPlayer : public MCPlayer {
public:
	virtual ~CL2DPlayer();

	virtual void init(Vec2 pos, float rot);
	virtual void createBody() = 0;
	virtual void remove();

protected:
	Node* mBody;
	Vec2 mPos;
	float mRot;
};

}