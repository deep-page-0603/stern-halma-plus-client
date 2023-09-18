
#include "mcscene.h"
#include "mcutil.h"
#include "mcui.h"
#include "mcsock.h"

#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
#include "mcjni.h"
#endif

namespace mocos2d {

MCScene* instance = NULL;

MCScene::MCScene() {
	_mBackend = NULL;
	mSceneInfo = NULL;
	_mReplaceScene = NULL;
	_mExiting = false;
	_mOldUIPos = Vec2(0, 0);
	_mDebugClient = -1;
	_mSwitchButton = NULL;
	mMusicShouldStop = true;
}

MCScene::~MCScene() {
	if (mSceneInfo) delete mSceneInfo;

	for (MCPlayerInfo* playerInfo : mPlayerInfo) {
		delete playerInfo;
	}
	mPlayerInfo.clear();
	if (_mBackend) delete _mBackend;
}

void MCScene::onExit() {
	if (!_mExiting) {
		_exitScene(true);
	}
	Scene::onExit();
}

MCScene* MCScene::getInstance() {
	return instance;
}

bool MCScene::init() {
	if (!Scene::init()) return false;
	return true;
}

void MCScene::initSimpleOffline() {
	vector<MCPlayerInfo*> playerInfo = vector<MCPlayerInfo*>();
	init(NULL, NULL, playerInfo);
}

void MCScene::init(MCSerializer* backend, MCSceneInfo* sceneInfo, vector<MCPlayerInfo*>& playerInfo) {
	instance = this;

	mSceneInfo = sceneInfo;
	mPlayerInfo = playerInfo;
	_mBackend = backend;

	_mEngineThreadProcessing = false;
	_mEngineThreadDetached = false;
	_mCode = 0;
	_mRecvFlag = true;

	_initRoot();
	_initListeners();
	_initOnline();

	onInitialized();
}

void MCScene::onInitialized() {
	if (mSceneInfo && isServer() && mSceneInfo->isEpisode()) {
		_pushToAll(NULL, 0, _mCode + 1);
	}
}

void MCScene::init(MCSerializer* backend, uchar* buffer) {
	_mBackend = backend;
	load(buffer);
	init(backend, mSceneInfo, mPlayerInfo);
}

uint MCScene::getSize() {
	return sizeof(int) + sizeof(MCSceneInfo::Serial) + sizeof(MCPlayerInfo::Serial) * _appSetting.maxPlayerCount
		+ (_mBackend ? _mBackend->getSize() : 0);
}

uchar* MCScene::save() {
	uchar* buffer = (uchar*) malloc(getSize());
	*((int*) buffer) = mPlayerInfo.size();
	uchar* pt = buffer + sizeof(int);
	
	int size = _mBackend->getSize();
	uchar* b = _mBackend->save();
	memcpy(pt, b, size);
	free(b);
	pt += size;

	size = sizeof(MCSceneInfo::Serial);
	b = mSceneInfo->save();
	memcpy(pt, b, size);
	free(b);
	pt += size;

	for (char i = 0; i < _appSetting.maxPlayerCount; ++i) {
		size = sizeof(MCPlayerInfo::Serial);

		if (i < mPlayerInfo.size()) {
			b = mPlayerInfo[i]->save();
			memcpy(pt, b, size);
			free(b);
		}
		pt += size;
	}	
	return buffer;
}

void MCScene::load(uchar* buffer) {
	if (mSceneInfo) delete mSceneInfo;

	for (MCPlayerInfo* playerInfo : mPlayerInfo) {
		delete playerInfo;
	}
	mPlayerInfo.clear();

	int playerCount = *((int*) buffer);
	uchar* pt = buffer + sizeof(int);
	
	if (_mBackend) {
		_mBackend->load(pt);
		pt += _mBackend->getSize();
	}
	mSceneInfo = MCSceneInfo::create(pt);
	pt += sizeof(MCSceneInfo::Serial);

	for (char i = 0; i < _appSetting.maxPlayerCount; ++i) {
		if (i < playerCount) mPlayerInfo.push_back(MCPlayerInfo::create(pt));
		pt += sizeof(MCPlayerInfo::Serial);
	}	
}

char MCScene::getMCTag() {
	return mSceneInfo ? mSceneInfo->getTag() : -1;
}

void MCScene::setDebugClient(char client) {
	if (_appSetting.productType != product_debug) return;
	if (!isServer()) return;

	_mDebugClient = client;
	mPlayerInfo[_mDebugClient]->setBot(false);

	if (_mSwitchButton) {
		_mSwitchButton->removeFromParent();
		_mSwitchText->removeFromParent();
	}
	_mSwitchButton = MCUI::button("mocos/button_debug.png", "", "", Vec2(170.f, mHeight - 50.f), 8, 65535, 0);
	_mSwitchText = MCUI::text("Debug Server", _appSetting.defaultFont, 28.f, _mSwitchButton->getPosition());
	_mSwitchText->setCameraMask((ushort) CameraFlag::USER8);
}

void MCScene::_debugSwitch() {
	if (_mDebugClient < 0) return;
	bool wasServer = isServer();

	saveFile(MCUtil::createStr("debug_online.sav"));
	loadFile(MCUtil::createStr("debug_online.sav"));

	if (wasServer) {
		mSceneInfo->setMe(_mDebugClient);
	} else {
		mSceneInfo->setMe(0);
	}
	recover();
	_mSwitchText->setString(isServer() ? "Debug Server" : "Debug Client");
}

MCSerializer* MCScene::getBackend() {
	return _mBackend;
}

Node* MCScene::getRoot() {
	return mRoot;
}

float MCScene::getWidth() {
	return mWidth;
}

float MCScene::getHeight() {
	return mHeight;
}

void MCScene::_exitScene(bool quitOnline) {
	_mExiting = true;
	MCUtil::mute(this);
	if (quitOnline) MCOnline::quit();

	if (_mEngineThreadProcessing) {
		schedule(schedule_selector(MCScene::_monitorStop));
	} else {
		_stopScene();
	}
}

void MCScene::_stopScene() {
	try {
		if (_mEngineThread.joinable()) _mEngineThread.join();
	} catch (...) {
	}
	onFinalized();

	if (_mReplaceScene) {
		_mReplaceScene->autorelease();
		if (_mReplaceDur == 0)
			Director::getInstance()->replaceScene(TransitionCrossFade::create(0.2f, _mReplaceScene));
		else
			Director::getInstance()->replaceScene(TransitionFade::create(_mReplaceDur, _mReplaceScene));
	} else {
		Director::getInstance()->end();
	}
}

void MCScene::_monitorStop(float dt) {
	if (!_mEngineThreadProcessing) {
		try {
			if (_mEngineThread.joinable()) _mEngineThread.join();
		} catch (...) {
		}
		unschedule(schedule_selector(MCScene::_monitorStop));
		_stopScene();
	}
}

void MCScene::_onBroadcasted(uchar* notifyInfo, int len, int code) {
	if (!_mRecvFlag) return;
	if (!mSceneInfo->isEpisode() || (code == _mCode)) {
		_mCode++;
		advance(notifyInfo, len);

		if (!mSceneInfo->isOnline() && !mSceneInfo->isChallenge()) saveFile(string(mSceneInfo->getSavePath()));
	} else {
		_send(mPlayerInfo[0]->getAddress(), NULL, 0, packet_request_push, _mCode);
	}
}

void MCScene::_onNotified(uchar* notifyInfo, int len, int code, char id) {
	if (!_mRecvFlag) return;

	if (!mSceneInfo->isEpisode() || (code == _mCode)) {
		_broadcast(notifyInfo, len, code);
	} else {
		uchar* buffer = save();
		_push(mPlayerInfo[id]->getAddress(), buffer, getSize(), _mCode);
		free(buffer);
	}
}

void MCScene::notify(uchar* notifyInfo, int len) {
	if (isServer() || (mSceneInfo->getMe() == _mDebugClient)) {
		_onNotified(notifyInfo, len, _mCode, 0);
	} else {
		_send(mPlayerInfo[0]->getAddress(), notifyInfo, len, packet_notify, _mCode);
	}
}

void MCScene::_send(string address, uchar* packet, int len, MCPacketType type, int code) {
	MCOnline::addSendPacket(address, packet, len, type, code);
}

void MCScene::kick(string address) {
	MCSock::kick(address);
}

void MCScene::_pushToAll(uchar* pack, int len, int code) {
	for (char i = 1; i < mPlayerInfo.size(); ++i) {
		MCPlayerInfo* pi = mPlayerInfo[i];
		if (pi->isBot() || !pi->isValid()) continue;
		if (i == _mDebugClient) continue;
		_push(pi->getAddress(), pack, len, code);
	}
	_onPushed(pack, len, code);
}

void MCScene::_onPushed(uchar* pack, int len, int code) {
	if (!_mRecvFlag) return;
	if ((len != sizeof(int) + sizeof(MCSceneInfo::Serial) + (_mBackend ? _mBackend->getSize() : 0) +
		sizeof(MCPlayerInfo::Serial) * _appSetting.maxPlayerCount) && (len != 0)) return;

	if (_mCode == code) return;
	_mCode = code;
	char me = mSceneInfo->getMe();
	if (len > 0) load(pack);
	mSceneInfo->setMe(me);
	recover();
}

void MCScene::_push(string address, uchar* pack, int len, int code) {	
	_send(address, pack, len, packet_push, code);
}

void MCScene::_broadcast(uchar* notifyInfo, int len, int code) {
	for (char i = 1; i < mPlayerInfo.size(); ++i) {
		MCPlayerInfo* pi = mPlayerInfo[i];
		if (pi->isBot() || !pi->isValid()) continue;
		if (i == _mDebugClient) continue;
		_send(pi->getAddress(), notifyInfo, len, packet_broadcast, code);
	}
	_onBroadcasted(notifyInfo, len, code);
}

void MCScene::_initOnline() {
	if (!mSceneInfo || !mSceneInfo->isOnline()) return;

	if (MCOnline::init()) {
		_mRecvFlag = true;
		memset(_mPingCount, 0, 32);
		schedule(schedule_selector(MCScene::_monitorOnline));
	}
}

void MCScene::runEngine(int engineCode) {
	if (_mEngineThreadProcessing) return;
	
	_mEngineStartTime = utils::gettime();
	schedule(schedule_selector(MCScene::_monitorEngine));
	_mEngineThread = thread(&MCScene::_engineThread, this, engineCode);	
}

void MCScene::_engineThread(int engineCode) {
	_mEngineThreadProcessing = true;
	engineProc(engineCode);
	_mEngineCode = engineCode;
	_mEngineThreadDetached = true;
	_mEngineThreadProcessing = false;
}

void MCScene::_monitorEngine(float dt) {
	if (!_mEngineThreadDetached) return;
	_mEngineThread.detach();
	_mEngineThreadDetached = false;	
	schedule_selector(MCScene::_monitorEngine);
	onEngineCompleted(_mEngineCode, (float) (utils::gettime() - _mEngineStartTime));
}

void MCScene::enableRecv(bool flag) {
	_mRecvFlag = flag;
}

void MCScene::permutePlayers(vector<char>& permute) {
	vector<MCPlayerInfo*> newPlayerInfo;
	char pingCount[32];

	for (char i = 0; i < permute.size(); ++i) {
		newPlayerInfo.push_back(mPlayerInfo[permute[i]]);
		pingCount[i] = _mPingCount[permute[i]];
	}
	memcpy(_mPingCount, pingCount, 32);
	
	for (char i = 0; i < permute.size(); ++i) {
		mPlayerInfo[i] = newPlayerInfo[i];
	}
}

void MCScene::changeToBot(char id) {
	mPlayerInfo[id]->setBot(true);
	_mPingCount[id] = 0;
}

void MCScene::changeToUser(char id) {
	mPlayerInfo[id]->setBot(false);
	_mPingCount[id] = 0;
}

char MCScene::findPlayerByAddress(string address) {
	for (char i = 0; i < mPlayerInfo.size(); ++i) {
		if (address.compare(mPlayerInfo[i]->getAddress()) == 0) return i;
	}
	return -1;
}

void MCScene::_monitorConnection(float dt) {
	if (isServer()) {
		vector<string> disconnects;

		for (char i = 1; i < mPlayerInfo.size(); ++i) {
			if ((_mPingCount[i] > 5) && mPlayerInfo[i]->isConnected() && mPlayerInfo[i]->isValid() && !mPlayerInfo[i]->isBot()) {
				disconnects.push_back(mPlayerInfo[i]->getAddress());
			}
		}
		for (string address : disconnects) {
			char id = findPlayerByAddress(address.c_str());
			_onClientDisconnected(address.c_str());
		}
		for (char i = 1; i < mPlayerInfo.size(); ++i) {
			if (mPlayerInfo[i]->isBot() || !mPlayerInfo[i]->isConnected() || !mPlayerInfo[i]->isValid()) continue;
			_send(mPlayerInfo[i]->getAddress(), NULL, 0, packet_ping, _mCode);
			if (_mPingCount[i] < 10) ++_mPingCount[i];
		}
	} else {
		if ((_mPingCount[0] > 5) && mPlayerInfo[0]->isConnected()) {
			MCOnline::onDisconnected();
		}
		_send(mPlayerInfo[0]->getAddress(), NULL, 0, packet_ping, _mCode);
	}
}

void MCScene::_monitorOnline(float dt) {
	MCOnline::dispatchSend();

	if (!_mRecvFlag) return;

	uchar pack[MAX_PACKET];
	int len, code;
	string address;
	MCPacketType type = MCOnline::dispatchRecv(address, pack, len, code);

	if (type == packet_invalid) return;
	if (!_onDefaultRecv(type, pack, len, code, address)) onCustomRecv(pack, len, address);
}

void MCScene::requestJoin(string address, int code) {
	MCPlayerInfo* myInfo = getMyInfo();
	uchar* buffer = myInfo->save();
	_send(address, buffer, myInfo->getSize(), packet_join, code);
	free(buffer);
	delete myInfo;
}

bool MCScene::isServer() {
	return mSceneInfo->getMe() == 0;
}

bool MCScene::_onDefaultRecv(MCPacketType type, uchar* pack, int len, int code, string& address) {
	char id = findPlayerByAddress(address.c_str());
#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
	string allAddress = "";

	for (char i = 0; i < mPlayerInfo.size(); i++) {
		allAddress.append(mPlayerInfo[i]->getAddress().c_str());
		if (i < mPlayerInfo.size() - 1) allAddress.append(",");
	}
#endif
	if ((type == packet_ping) && (len != 0)) return true;

	if (id >= 0) {
		if (((type == packet_ping) && (code == _mCode)) || !mSceneInfo || !mSceneInfo->isEpisode()) _mPingCount[id] = 0;
		
		if (!mPlayerInfo[id]->isConnected() && mPlayerInfo[id]->isValid()) {
			if (isServer()) {
				_onClientReconnected(address.c_str());
			} else if (id == 0) {
				_onReconnected();
			}
		}
	} else if (type == packet_find) {
		if (len != 0) return true;
		if (!isServer()) return true;
		if (code != mSceneInfo->getTag()) return true;
		_send(address, NULL, 0, packet_server, code);
		return true;
	} else if (type == packet_server) {
		if (len != 0) return true;
		if (isServer() || !_mcOnlineFinding) return true;
		onFoundServer(address.c_str());
		return true;
	} else if (type == packet_join) {
		if (len != sizeof(MCPlayerInfo::Serial)) return true;
		if (!isServer()) return true;

		MCPlayerInfo* playerInfo = MCPlayerInfo::create(pack);
		playerInfo->setAddress(address.c_str());
		onJoinRequested(playerInfo);
		return true;
	} else if (type == packet_allow) {
		if (isServer() || !_mcOnlineFinding) return true;
		string myAddress = string((char*) pack);
		if (len != myAddress.length() + 1) return true;
		MCOnline::onConnected(address.c_str(), myAddress);
		return true;
	} else if (type == packet_refuse) {
		if (len != 0) return true;
		if (isServer() || !_mcOnlineFinding) return true;
		onJoinRefused(address.c_str());
		return true;
	}
	if (!isServer()) id = 0;
	if (type != packet_user) {
		if (id < 0) return true;

		switch (type) {
		case packet_kick:
			if (len != 0) return true;
			if (isServer() || _mcOnlineFinding) return true;
			onKicked();
			return true;
		case packet_broadcast:
			if (isServer()) return true;
			_onBroadcasted(pack, len, code);
			return true;
		case packet_notify:
			if (len > MAX_PACKET) return true;
			if (!isServer()) return true;

			_onNotified(pack, len, code, id);
			return true;
		case packet_push:
			if (isServer()) return true;
			_onPushed(pack, len, code);
			return true;
		case packet_request_push:
			if (len != 0) return true;
			if (!isServer()) return true;
			if (code == _mCode) return true;
			{
				uchar* buffer = save();
				_push(address, buffer, getSize(), _mCode);
				free(buffer);
			}
			return true;
		case packet_quit:
			if (len != 0) return true;
			if (isServer()) {
				MCOnline::onClientDisconnected(address.c_str());
			} else {
				MCOnline::onDisconnected();
			}
			return true;
		}
	}
	return false;
}

MCSceneInfo* MCScene::getSceneInfo() {
	return mSceneInfo;
}

void MCScene::_initRoot() {
	string csb = mSceneInfo ? mSceneInfo->getCsb() : "";
	mRoot = (csb.compare("") != 0) ? CSLoader::createNode(csb) : Node::create();
	addChild(mRoot);

	MCScreenOrientationType orientation = mSceneInfo ? mSceneInfo->getScreenOrientation() : screen_orientation_landscape;

	if (orientation == screen_orientation_portrait) {
		mWidth = _appSetting.screenHeight;
		mHeight = _appSetting.screenWidth;

		if ((CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID) || (_appSetting.screenLayoutType == screen_layout_mobile)) {
			mRoot->setPosition(Vec2(_appSetting.screenWidth, 0));
			mRoot->setRotation(-90.f);			
		} else {
			mRoot->setScale(_appSetting.screenHeight / _appSetting.screenWidth);
			float fitWid = _appSetting.screenHeight * _appSetting.screenHeight / _appSetting.screenWidth;
			mRoot->setPosition(Vec2((_appSetting.screenWidth - fitWid) / 2, 0));
		}
	} else {
		mWidth = _appSetting.screenWidth;
		mHeight = _appSetting.screenHeight;
	}
	mToast = MCToast::create();
	mToast->setPosition(Vec2(mWidth / 2, mHeight * 0.1f));
	mToast->setCameraMask((ushort) CameraFlag::USER8);
	mRoot->addChild(mToast, MAX_Z_ORDER);
}

void MCScene::_initListeners() {
	auto touchListener = EventListenerTouchOneByOne::create();
	touchListener->onTouchBegan = CC_CALLBACK_2(MCScene::_onTouchBegan, this);
	touchListener->onTouchMoved = CC_CALLBACK_2(MCScene::_onTouchMoved, this);
	touchListener->onTouchEnded = CC_CALLBACK_2(MCScene::_onTouchEnded, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(touchListener, mRoot);

	auto keyListener = EventListenerKeyboard::create();
	keyListener->onKeyReleased = CC_CALLBACK_2(MCScene::_onKeyReleased, this);
	keyListener->onKeyPressed = CC_CALLBACK_2(MCScene::_onKeyPressed, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(keyListener, mRoot);
}

void MCScene::_onClickListener(Ref* pSender, Widget::TouchEventType type) {	
	if (type != Widget::TouchEventType::ENDED) return;
	
	Button* button = (Button*) pSender;
	if (_appSetting.uiAssist) _uiAssist(button->getTouchEndPosition());

	int tag = button->getTag();
	if (_mClickMap.size() == 0) return;
	int code = _mClickMap.find(button)->second;

	if (code == 65535) {
		_debugSwitch();
		return;
	}
	bool isDlg = _mDialogList.size() > 0 && MCUtil::isDescendant(button, _mDialogList[_mDialogList.size() - 1]);
	
	if (isDlg) {
		auto dlg =  _mDialogList[_mDialogList.size() - 1];
		if (!dlg->isShowAnimFinished()) return;
		int dlgCode = dlg->getDialogCode(button);

		if (dlgCode >= 0) {
			dlg->onClickListener(dlgCode, tag);
		} else {
			DialogResult result = (DialogResult) dlg->getSceneCode(button);
			_onDialogResult(code, result, tag);
		}
	} else {
		if (_mDialogList.size() == 0) onClickListener(code, tag);
	}
}

void MCScene::_onDialogResult(int code, DialogResult result, int tag) {
	if (result < dlg_resultc) hideDialog();
	onDialogResult(code, result, tag);	
}

bool MCScene::_onTouchBegan(Touch* touch, Event* event) {
	if (_appSetting.uiAssist) _uiAssist(touch->getLocation());

	if (_mDialogList.size() > 0) return true;
	return onTouchBegan(touch->getLocation());
}

void MCScene::_onTouchMoved(Touch* touch, Event* event) {
	if (_mDialogList.size() > 0) return;
	onTouchMoved(touch->getLocation());
}

void MCScene::_onTouchEnded(Touch* touch, Event* event) {
	if (_mDialogList.size() > 0) {
		auto dlg = _mDialogList[_mDialogList.size() - 1];
		if (dlg->isShowAnimFinished()) dlg->touch(touch->getLocation());
		return;
	}
	onTouchEnded(touch->getLocation());
}

void MCScene::_onKeyReleased(EventKeyboard::KeyCode keycode, Event* event) {
	if (keycode == EventKeyboard::KeyCode::KEY_BACK) {
		if (_mDialogList.size() > 0) {
			auto dlg = _mDialogList[_mDialogList.size() - 1];
			if (dlg->isFastCancel()) {
				_onDialogResult(dlg->getTag(), dlg_result_cancel, -1);
			}
			return;
		}
	}
	onKeyReleased(keycode);
}

void MCScene::_onKeyPressed(EventKeyboard::KeyCode keycode, Event* event) {
	onKeyPressed(keycode);
}

void MCScene::deleteSaveFile() {
	if (isServer()) FileUtils::getInstance()->removeFile(MCUtil::getWritablePath(mSceneInfo->getSavePath()));
}

void MCScene::replaceTo(MCScene* scene, bool quitOnline, float dur) {
	_mReplaceScene = scene;
	_mReplaceDur = dur;

	if (scene) scene->retain();
	_exitScene(quitOnline);
}

void MCScene::toast(string msg, float dur, float y) {
	mToast->setPositionY(mHeight * y);
	mToast->show(msg, dur);
}

void MCScene::hideToast() {
	mToast->hide();
}

void MCScene::addClick(Widget* widget, int code) {
	_mClickMap.erase(widget);
	_mClickMap.insert(pair<Widget*, int>(widget, code));
	widget->addTouchEventListener(CC_CALLBACK_2(MCScene::_onClickListener, this));
}

void MCScene::playMusic(string path, float volume) {
	if (musicId < 0) {
		if (androidVer < 28) {
			musicId = AudioEngine::play2d(path, true, volume);
		} else {
			SimpleAudioEngine::getInstance()->playBackgroundMusic(path.c_str(), true);
			musicId = 0;
		}		
	}
}

void MCScene::setMusicVolume(float volume) {
	if (musicId >= 0) {
		if (androidVer < 28) {
			AudioEngine::setVolume(musicId, volume);
		} else {
			SimpleAudioEngine::getInstance()->setBackgroundMusicVolume(volume);
		}
	}
}

void MCScene::setInitToast(string msg, float dur, float y) {
	_mInitToast = msg;
	_mInitToastDur = dur;
	_mInitToastY = y;
}

void MCScene::onExitTransitionDidStart() {
	Scene::onExitTransitionDidStart();
	
	if (mMusicShouldStop) {
		if (androidVer < 28) {
			AudioEngine::stopAll();
		} else {
			SimpleAudioEngine::getInstance()->stopBackgroundMusic();
		}
		musicId = -1;
	}
}

void MCScene::onEnterTransitionDidFinish() {
	Scene::onEnterTransitionDidFinish();

	if (_mInitToast.compare("") != 0) toast(_mInitToast.c_str(), _mInitToastDur, _mInitToastY);
}

void MCScene::showDialog(MCDialog* dialog) {
	_mDialogList.push_back(dialog);
	dialog->show();
}

void MCScene::hideDialog() {
	if (_mDialogList.size() > 0) {
		auto dlg = _mDialogList[_mDialogList.size() - 1];
		dlg->hide();
		_mDialogList.erase(_mDialogList.end() - 1);
	}
}

bool MCScene::isOnline() {
	return mSceneInfo->isOnline();
}

bool MCScene::isChallenge() {
	return mSceneInfo->isChallenge();
}

void MCScene::eraseClick(Node* node) {
	for (Node* child : MCUtil::getChildren(node)) {
		_mClickMap.erase((Widget*) child);
	}
}

void MCScene::_onClientDisconnected(string address) {
	char id = findPlayerByAddress(address);
	if ((id < 0) || !mPlayerInfo[id]->isConnected()) return;
	mPlayerInfo[id]->setConnected(false);
	onClientDisconnected(address);
}

void MCScene::_onClientReconnected(string address) {
	char id = findPlayerByAddress(address);
	if ((id < 0) || mPlayerInfo[id]->isConnected()) return;
	mPlayerInfo[id]->setConnected(true);

	if (mSceneInfo->isEpisode()) {
		uchar* buffer = save();
		_push(string(address), buffer, getSize(), _mCode);
		free(buffer);
	}
	onClientReconnected(address);
}

void MCScene::_onDisconnected() {
	if (mPlayerInfo.empty()) return;
	if (!mPlayerInfo[0]->isConnected()) return;
	mPlayerInfo[0]->setConnected(false);
	onDisconnected();
}

void MCScene::_onReconnected() {
	if (mPlayerInfo.empty()) return;
	if (mPlayerInfo[0]->isConnected()) return;
	
	mPlayerInfo[0]->setConnected(true);	
	
	if (mSceneInfo->isEpisode()) _send(mPlayerInfo[0]->getAddress(), NULL, 0, packet_request_push, _mCode);
	onReconnected();
}

void MCScene::_uiAssist(Vec2 pos) {
	Vec2 dist = Vec2(fabsf(pos.x - _mOldUIPos.x), fabsf(pos.y - _mOldUIPos.y));
	_mOldUIPos = pos;

	auto assist = Sprite::create("mocos/ui_assist.png");
	assist->setScale(0);
	assist->setOpacity(0);
	assist->setPosition(pos);
	addChild(assist, MAX_Z_ORDER);
	assist->setCameraMask((ushort) CameraFlag::USER8);
	
	assist->runAction(Sequence::createWithTwoActions(
		Spawn::createWithTwoActions(
			ScaleTo::create(1.f, 2.f),
			Sequence::createWithTwoActions(
				FadeTo::create(0.5f, 255),
				FadeTo::create(0.5f, 0)
			)
		),
		RemoveSelf::create()
		));
	toast(MCUtil::createStr("x=%.1f, y=%.1f, w=%.1f, h=%.1f", pos.x, pos.y, dist.x, dist.y), 1.f);
}

}
