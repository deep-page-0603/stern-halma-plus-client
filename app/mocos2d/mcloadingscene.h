
#pragma once

#include "mcscene.h"

namespace mocos2d {

class MCLoadingScene : public MCScene {
public:
	CREATE_FUNC(MCLoadingScene);
	
	virtual bool init();
	void init(MCScene* scene, MCSerializer* backend, MCSceneInfo* sceneInfo, vector<MCPlayerInfo*>& playerInfo);
	void init(MCScene* scene, MCSerializer* backend, uchar* buffer);

	virtual void onEnterTransitionDidFinish() override;
	
protected:
	void _onInitedScene(float dur = _appSetting.replaceSceneDuration);
	void _initScene();

public:
	MCScene* _mNextScene;
	MCSerializer* _mNextBackend;
	MCSceneInfo* _mNextSceneInfo;
	vector<MCPlayerInfo*> _mNextPlayerInfo;
	uchar* _mNextBuffer;
	double _mReplaceStartTime;
};

}