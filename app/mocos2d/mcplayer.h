
#pragma once

#include "mocos2d.h"

namespace mocos2d {

class MCPlayer : public Node {
public:	
	virtual void init(Vec3 pos, char idleAni);

	virtual void setIdleAni(char ani);
	void freePlay();
	virtual void play(char ani, bool loop, bool immediate = false);
	Vec2 getWorldPos(Vec2 pos);
	char getAniId(char ani);	

	virtual Rect getHitRect() = 0;
	virtual void setAnimations() = 0;
	virtual float getAnimDuration(char ani) = 0;
	virtual void playLoopAni(char loopAni) = 0;
	virtual void playLoopAniAfterAni(char ani, char loopAni) = 0;
	virtual void stopAllAni() = 0;
	virtual char getBoreAni(char idleAni) = 0;

	virtual void moveTo(Vec2 pos);
	virtual void remove();
	
	void enableBore(bool enable);

protected:
	void _wait(float dt);

protected:
	vector<char> mAvAni;
	char mIdleAni;
	DrawNode* mDraw;
	bool mBorable;
};

}