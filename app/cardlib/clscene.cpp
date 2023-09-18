
#include "clscene.h"

namespace cardlib {

void CLScene::onInitialized() {
	MCOnline::stopListening();

	memset(&mNotify, 0, sizeof(NotifyInfo));

	_initCamera();
	initState();
	initUI();
	initPlayers();
	initCache();

	MCScene::onInitialized();
}

void CLScene::onFinalized() {}

void CLScene::advance(uchar* notifyInfo, int len) {
	if (len != sizeof(NotifyInfo)) return;
	memcpy(&mNotify, notifyInfo, sizeof(NotifyInfo));
	advance();
}

void CLScene::_initCamera() {
	MCUtil::initCameras(mRoot, mWidth, mHeight);
}

void CLScene::notify() {
	MCScene::notify((uchar*) &mNotify, sizeof(NotifyInfo));
}

char CLScene::getFrontTurn(char turn) {
	return (mPlayerInfo.size() + turn - mSceneInfo->getMe()) % mPlayerInfo.size();
}

char CLScene::getBackTurn(char turn) {
	return (turn + mSceneInfo->getMe()) % mPlayerInfo.size();
}

}