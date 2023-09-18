
#pragma once

#include "mcsceneinfo.h"
#include "mcplayerinfo.h"
#include "mctoast.h"
#include "mcdialog.h"
#include "mconline.h"

namespace mocos2d {

class MCScene : public Scene, public MCSerializer {
public:
	CREATE_FUNC(MCScene);
	
	MCScene();
	virtual ~MCScene();
	static MCScene* getInstance();
	
	char getMCTag();
	MCSerializer* getBackend();
	Node* getRoot();
	float getWidth();
	float getHeight();
	MCSceneInfo* getSceneInfo();
	bool isOnline();
	bool isChallenge();
	void showDialog(MCDialog* dialog);
	void hideDialog();
	void toast(string msg, float dur = 3.f, float y = 0.1f);
	void hideToast();
	void addClick(Widget* widget, int code);
	void eraseClick(Node* node);
	void playMusic(string path, float volume);
	void setMusicVolume(float volume);
	void setInitToast(string msg, float dur, float y);
	
public:
	virtual uint getSize() override;
	virtual uchar* save() override;
	virtual void load(uchar* buffer) override;
	virtual void onExitTransitionDidStart() override;
	virtual void onEnterTransitionDidFinish() override;
	virtual void onExit() override;

	virtual bool init();	
	virtual void initSimpleOffline();
	virtual void init(MCSerializer* backend, MCSceneInfo* sceneInfo, vector<MCPlayerInfo*>& playerInfo);
	virtual void init(MCSerializer* backend, uchar* buffer);

	virtual void onFoundServer(string address) {};
	virtual void onJoinRequested(MCPlayerInfo* playerInfo) {};
	virtual void onJoinRefused(string address) {};
	virtual void onKicked() {};
	virtual void onPairedWithServer() {};
	virtual void onPairingFailed() {};
	virtual void onConnectingFailed() {};

	virtual void onClientDisconnected(string address) {};
	virtual void onClientReconnected(string address) {};
	
	virtual void onConnected(string address, string myAddress) {};
	virtual void onDisconnected() {};
	virtual void onReconnected() {};

	virtual void onOnlineOpened(bool ok) {};
	virtual void onClickListener(int code, int tag) {};
	virtual void onDialogResult(int code, DialogResult result, int tag) {};

	virtual void onInitialized();

protected:	
	virtual void onFinalized() {};
	virtual void recover() {};
	virtual void advance(uchar* notifyInfo, int len) {};
	virtual void onCustomRecv(uchar* pack, int len, string& address) {};	
	virtual void engineProc(int engineCode) {};
	virtual void onEngineCompleted(int engineCode, float dt) {};	
	virtual MCPlayerInfo* getMyInfo() {return NULL;}

	virtual bool onTouchBegan(Vec2 loc) {return true;};
	virtual void onTouchMoved(Vec2 loc) {};
	virtual void onTouchEnded(Vec2 loc) {};
	virtual void onKeyReleased(EventKeyboard::KeyCode keycode) {};
	virtual void onKeyPressed(EventKeyboard::KeyCode keycode) {};
		
	void notify(uchar* info, int len);
	void runEngine(int engineCode);
	void enableRecv(bool flag);	
	void deleteSaveFile();
	void permutePlayers(vector<char>& permute);
	void changeToBot(char id);
	void changeToUser(char id);
	bool isServer();
	char findPlayerByAddress(string address);
	void setDebugClient(char client);
	void kick(string address);

private:	
	void _initRoot();
	void _initListeners();
	bool _onTouchBegan(Touch* touch, Event* event);
	void _onTouchMoved(Touch* touch, Event* event);
	void _onTouchEnded(Touch* touch, Event* event);
	void _onKeyReleased(EventKeyboard::KeyCode keycode, Event* event);
	void _onKeyPressed(EventKeyboard::KeyCode keycode, Event* event);
	void _onClickListener(Ref* pSender, Widget::TouchEventType type);
	void _onDialogResult(int code, DialogResult result, int tag);
	
	void _initOnline();	
	void _broadcast(uchar* notifyInfo, int len, int code);
	void _push(string address, uchar* pack, int len, int code);
	void _pushToAll(uchar* pack, int len, int code);
	void _onPushed(uchar* pack, int len, int code);
	void _onBroadcasted(uchar* notifyInfo, int len, int code);
	void _onNotified(uchar* notifyInfo, int len, int code, char id);	
	void _send(string address, uchar* packet, int len, MCPacketType type, int code);
	void _engineThread(int engineCode);
	void _monitorEngine(float dt);
	void _monitorOnline(float dt);
	void _monitorConnection(float dt);
	void _exitScene(bool quitOnline);
	void _stopScene();
	void _monitorStop(float dt);	
	bool _onDefaultRecv(MCPacketType type, uchar* pack, int len, int code, string& address);	
	void _uiAssist(Vec2 pos);
	void _debugSwitch();

public:
	void _onClientDisconnected(string address);
	void _onClientReconnected(string address);
		 
	void _onDisconnected();	
	void _onReconnected();
	
	void requestJoin(string address, int code);
	void replaceTo(MCScene* scene, bool quitOnline, float dur);

protected:
	MCSceneInfo* mSceneInfo;
	vector<MCPlayerInfo*> mPlayerInfo;
	Node* mRoot;
	int mWidth;
	int mHeight;
	MCToast* mToast;
	bool mMusicShouldStop;

protected:
	thread _mEngineThread;
	bool _mEngineThreadDetached;
	bool _mEngineThreadProcessing;
	int _mCode;
	bool _mRecvFlag;
	int _mEngineCode;
	double _mEngineStartTime;
	MCSerializer* _mBackend;
	MCScene* _mReplaceScene;
	map<Widget*, int> _mClickMap;
	float _mMusicVolume;
	float _mReplaceDur;
	vector<MCDialog*> _mDialogList;
	string _mInitToast;
	float _mInitToastDur;
	float _mInitToastY;
	char _mPingCount[32];
	Vec2 _mOldUIPos;
	bool _mExiting;
	char _mDebugClient;
	Button* _mSwitchButton;
	Text* _mSwitchText;
};

}