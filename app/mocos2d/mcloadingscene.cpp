
#include "mcloadingscene.h"

namespace mocos2d {

bool MCLoadingScene::init() {
	if (!MCScene::init()) return false;

	_mNextScene = NULL;
	_mNextSceneInfo = NULL;
	_mNextBackend = NULL;
	_mNextBuffer = NULL;
	return true;
}

void MCLoadingScene::init(MCScene* scene, MCSerializer* backend, MCSceneInfo* sceneInfo, vector<MCPlayerInfo*>& playerInfo) {
	_mNextScene = scene;
	_mNextSceneInfo = sceneInfo;
	_mNextPlayerInfo = playerInfo;
	_mNextBackend = backend;
	((Scene*) scene)->retain();
	initSimpleOffline();
}

void MCLoadingScene::init(MCScene* scene, MCSerializer* backend, uchar* buffer) {
	_mNextScene = scene;
	_mNextBackend = backend;
	_mNextBuffer = buffer;
	((Scene*) scene)->retain();
	initSimpleOffline();
}

void MCLoadingScene::onEnterTransitionDidFinish() {
	Scene::onEnterTransitionDidFinish();
	_mReplaceStartTime = utils::gettime();
}

void MCLoadingScene::_onInitedScene(float dur) {
	double consumed = utils::gettime() - _mReplaceStartTime;

	if (consumed >= _appSetting.loadingSceneDuration) {
		replaceTo(_mNextScene, false, 0);
	} else {
		runAction(Sequence::createWithTwoActions(DelayTime::create(_appSetting.loadingSceneDuration - consumed),
			CallFunc::create(bind(&MCLoadingScene::replaceTo, this, _mNextScene, false, dur))));
	}
}

void MCLoadingScene::_initScene() {
	if (_mNextSceneInfo) {
		_mNextScene->init(_mNextBackend, _mNextSceneInfo, _mNextPlayerInfo);
	} else {
		_mNextScene->init(_mNextBackend, _mNextBuffer);
		free(_mNextBuffer);
	}
}

}