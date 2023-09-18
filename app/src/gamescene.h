
#pragma once

#include "global.h"
#include "backend.h"

#if CC_TARGET_PLATFORM == CC_PLATFORM_WIN32

#define PY_THINK
//#define SHOW_VALUE_BOARD

#ifdef PY_THINK
#include "network/httpclient.h"
using namespace cocos2d::network;
#endif

#endif

#define ROOT_3 1.732051f
#define ORTH_LEN 68.f
#define V_CENTER_OFFSET -60.f

class GameScene : public CLScene {
public:
	enum Stage {
		stage_init, stage_play, stage_effect, stage_game_over
	};
	enum Code {
		code_undo
	};
	class Piece : public Sprite {
	public:
		CREATE_FUNC(Piece);
		Piece();
		~Piece();

		static Piece* createPiece(short pt, char player);
		void initPiece(ushort pt, char player);

		void select(bool sel);
		void goThrough(vector<ushort>& path);
		void stepOnce();

	public:
		GameScene* scene;
		ushort pos;
		bool isSelected;
		char player;
		float scale;
		vector<ushort> path;
		char curPathId;
		Sprite* selectRing;
		Sprite* arriveRing;
		Animate* arriveAni;
	};

	class Player : public Node {
	public:
		CREATE_FUNC(Player);
		Player();
		~Player();

		static Player* createPlayer(char character, char turn);
		void resetChair();
		void think();
		void go();

	public:
		GameScene* scene;
		char character;
		char turn;		
		Sprite* chairSp;
		Sprite* faceSp;
	};
public:
	CREATE_FUNC(GameScene);
	virtual ~GameScene();

	static GameScene* createSimpleOffline();
	virtual void engineProc(int engineCode) override;
	virtual void onEngineCompleted(int engineCode, float dt) override;

	virtual void recover() override;
	virtual void advance() override;

	virtual bool onTouchBegan(Vec2 loc) override;
	virtual void onTouchMoved(Vec2 loc) override;
	virtual void onTouchEnded(Vec2 loc) override;
	virtual void onKeyReleased(EventKeyboard::KeyCode keycode) override;
	virtual void onKeyPressed(EventKeyboard::KeyCode keycode) override;

	virtual void onClickListener(int code, int tag) override;

private:
	virtual void initUI() override;
	virtual void initPlayers() override;
	virtual void initState() override;
	virtual void onInitialized() override;

	Vec2 ptToScreen(ushort pt);
	void initBoard();
	void refreshBoard();
	void destroyPieces();
	Animate* get2dEffect(const char* baseName, int count, float delay);
	void onPieceArrived(uint act);
	void think();
	void go();
	void undo();
	void onGameOver(Backend::Exit exit, uchar* ranks);
	void hideLegalEffects();
	void showActMarks(bool show = true);
	Vec2 safeCourtPos(Vec2 pos);
	void setCourtBound();
	void onMouseScroll(EventMouse* event);

#ifdef PY_THINK
private:
	void pyThink(char* serverAddress);
	void onPyThinkCompleted(HttpClient* sender, HttpResponse* response);
#endif

private:
	Node* court;
	Node* board;
	Backend* backend;
	uchar me;
	Stage stage;
	float edgeLen;
	Vec2 origPos;
	float pieceRad;
	vector<Piece*> pieces;
	Sprite* startMark;
	Sprite* endMark;
	Piece* selectedPiece;
	uint thought;
	vector<Sprite*> legalEffects;
	Text* touchMe;
	bool touchable;
	bool isBoardInited;
	Vec2 firstTouch;
	Vec2 courtPos;
	Rect courtBound;
	float boardScale;
	Size courtSize;
	Scale9Sprite* headerBar;
	Player* players[3];
	Text* progLabel;
	Text* undoCountLabel;
	Text* exitLabel;
	char tempBoard[MAX_PT];
};
