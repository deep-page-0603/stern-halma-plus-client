
#include "gamescene.h"

GameScene::Player::Player() {
}

GameScene::Player::~Player() {
}

GameScene::Player* GameScene::Player::createPlayer(char character, char turn) {
	Player* player = Player::create();
	player->character = character;
	player->turn = turn;
	player->scene = (GameScene*) MCScene::getInstance();
	player->setPosition(Vec2(85.f + turn * 100.f, 55.f));
	player->scene->headerBar->addChild(player);

	player->chairSp = Sprite::create("images/game/chair_neutral.png");
	player->addChild(player->chairSp);

	player->faceSp = Sprite::create(MCUtil::createStr("images/faces/%02d.png", character));
	player->addChild(player->faceSp);

	if (turn > 0) {
		for (char i = 0; i < 2; ++i) {
			auto dotSp = Sprite::create("images/game/white_dot.png");
			dotSp->setPosition(Vec2(-50.f, -6.f + 12.f * i));
			player->addChild(dotSp);
		}
	}
	return player;
}

void GameScene::Player::resetChair() {
	chairSp->initWithFile(MCUtil::createStr("images/game/chair_%d.png", turn));
}

void GameScene::Player::think() {
	chairSp->runAction(RepeatForever::create(Sequence::createWithTwoActions(
		TintBy::create(1.f, -100, -100, -100),
		TintBy::create(1.f, 100, 100, 100)
	)));
}

void GameScene::Player::go() {
	chairSp->stopAllActions();
	chairSp->setColor(Color3B::WHITE);
}

GameScene::Piece* GameScene::Piece::createPiece(short pos, char player) {
	Piece* piece = Piece::create();
	piece->scene = (GameScene*) MCScene::getInstance();
	piece->initPiece(pos, player);	
	piece->scene->board->addChild(piece, 1);
	return piece;
}

GameScene::Piece::Piece() {
	arriveAni = nullptr;
}

GameScene::Piece::~Piece() {
	if (arriveAni) arriveAni->release();
}

void GameScene::Piece::initPiece(ushort pos, char player) {
	initWithFile(MCUtil::createStr("images/pieces/00_%02d_n.png", player));
	this->pos = pos;
	this->player = player;

	isSelected = false;
	setPosition(scene->ptToScreen(pos));
	
	selectRing = Sprite::create("images/game/select_ring.png");
	selectRing->setPosition(Vec2(getContentSize().width / 2, getContentSize().height / 2));
	selectRing->setScale(0.65f);
	selectRing->setOpacity(120);
	selectRing->setVisible(false);
	addChild(selectRing, 1);
	
	arriveRing = Sprite::createWithSpriteFrameName("B000.png");
	arriveRing->setPosition(Vec2(getContentSize().width / 2, getContentSize().height / 2));
	arriveRing->setScale(0.15f);
	arriveRing->setVisible(false);
	addChild(arriveRing, 1);

	scale = 2 * scene->pieceRad / getContentSize().width;
	setScale(scale);
	
	arriveAni = scene->get2dEffect("B", 5, 0.07f);
	arriveAni->retain();
}

void GameScene::Piece::select(bool sel) {
	if (sel == isSelected) return;
	isSelected = sel;
	initWithFile(MCUtil::createStr(sel ? "images/pieces/00_%02d_s.png" : "images/pieces/00_%02d_n.png", player));

	if (sel) {
		runAction(RepeatForever::create(Sequence::createWithTwoActions(
			ScaleTo::create(0.3f, scale * 1.3f),
			ScaleTo::create(0.3f, scale * 1.1f)
			)));
		selectRing->runAction(RepeatForever::create(RotateBy::create(3.f, 360)));
		selectRing->setVisible(true);
	} else {
		stopAllActions();
		selectRing->setVisible(false);
		selectRing->stopAllActions();
		setScale(scale);
	}
}

void GameScene::Piece::goThrough(vector<ushort>& path) {	
	select(false);
	this->path = path;
	curPathId = 1;
	stepOnce();
}

void GameScene::Piece::stepOnce() {
	if (curPathId == path.size()) {
		uint act = scene->backend->getAct(pos, path.back());
		pos = path.back();
		scene->runAction(Sequence::createWithTwoActions(DelayTime::create(0.2f),
			CallFunc::create(bind(&GameScene::onPieceArrived, scene, act))));
		return;
	}
	Vec2 target = scene->ptToScreen(path[curPathId]);
	++curPathId;

	if (curPathId == path.size()) {
		arriveRing->setVisible(true);
		arriveRing->runAction(Sequence::create(
			DelayTime::create(0.2f),
			arriveAni,
			CallFunc::create(bind([](Sprite* ring) {
				ring->setVisible(false);
			}, arriveRing)),
			nullptr
		));
	}
	runAction(Sequence::create(
		MoveTo::create(0.2f, target),
		DelayTime::create(0.1f),
		CallFunc::create(bind(&Piece::stepOnce, this)),
		nullptr
	));
}

GameScene* GameScene::createSimpleOffline() {
	vector<MCPlayerInfo*> playerInfos;

	playerInfos.push_back(MCPlayerInfo::create("", "", 0, 0, true, false, true));
	playerInfos.push_back(MCPlayerInfo::create("", "", 1, 0, true, true, true));

	MCSceneInfo* sceneInfo = MCSceneInfo::create(TAG_SCENE_GAME, "", true, MCScreenOrientationType::screen_orientation_landscape, false, false, 0);
	Backend::Rule rule;

	rule.aiLevel = 2;
	rule.dim = 5;
	rule.party = 2;

	Backend* backend = new Backend(&rule);

	GameScene* scene = GameScene::create();
	scene->init(backend, sceneInfo, playerInfos);
	return scene;
}

GameScene::~GameScene() {
}

void GameScene::initUI() {
	SpriteFrameCache::getInstance()->addSpriteFramesWithFile("images/game/legal_effect.plist");
	SpriteFrameCache::getInstance()->addSpriteFramesWithFile("images/game/arrive_effect.plist");

	court = Node::create();
	getRoot()->addChild(court);

	courtSize = MCUI::imageView("images/courts/00.jpg", SCREEN_CENTER, 0, 0, court)->getContentSize();
	setCourtBound();

	board = Node::create();
	board->setPosition(SCREEN_CENTER);
	court->addChild(board);
	
	headerBar = Scale9Sprite::create("images/game/rounded_oniskin.png");
	headerBar->setCapInsets(Rect(10.f, 10.f, 10.f, 10.f));
	headerBar->setAnchorPoint(Vec2::ANCHOR_MIDDLE_TOP);
	headerBar->setPosition(Vec2(_appSetting.screenWidth / 2, _appSetting.screenHeight - 10.f));
	headerBar->setContentSize(Size(_appSetting.screenWidth - 20.f, 110.f));
	getRoot()->addChild(headerBar);

	auto progBk = ImageView::create("mocos/black.png");
	progBk->setScale(45.f, 6.f);
	progBk->setOpacity(133);
	progBk->setAnchorPoint(Vec2::ANCHOR_TOP_LEFT);
	progBk->setPosition(Vec2(50.f, _appSetting.screenHeight - 118.f));
	getRoot()->addChild(progBk);

	progLabel = Text::create(createGStr("prog_text", 0, backend->getMaxGameLen()).c_str(), DOTUM, 22.f);	
	progLabel->setAnchorPoint(Vec2::ANCHOR_MIDDLE_LEFT);
	progLabel->setPosition(Vec2(60.f, _appSetting.screenHeight - 133.f));
	getRoot()->addChild(progLabel);
	
	auto logo = Sprite::create("images/game/game_logo.png");
	logo->setPosition(Vec2(_appSetting.screenWidth - 115.f, 17.f));
	headerBar->addChild(logo);

	auto gameLevelLabel = Text::create(createGStr("game_level", getGStr("level_%d", backend->getAiLevel()).c_str()), DOTUM, 22.f);
	gameLevelLabel->setAnchorPoint(Vec2::ANCHOR_MIDDLE_LEFT);
	gameLevelLabel->setPosition(Vec2(245.f, 85.f));
	headerBar->addChild(gameLevelLabel);

	undoCountLabel = Text::create(createGStr("undo_count", 0, getMaxUndoCount(backend->getAiLevel())), DOTUM, 22.f);
	undoCountLabel->setAnchorPoint(Vec2::ANCHOR_MIDDLE_LEFT);
	undoCountLabel->setPosition(Vec2(245.f, 55.f));
	headerBar->addChild(undoCountLabel);

	auto hintLegalsLabel = Text::create(createGStr("hint_legals", getGStr("enabled").c_str()), DOTUM, 22.f);
	hintLegalsLabel->setAnchorPoint(Vec2::ANCHOR_MIDDLE_LEFT);
	hintLegalsLabel->setPosition(Vec2(245.f, 25.f));
	headerBar->addChild(hintLegalsLabel);

	auto modeSp = Sprite::create("images/game/mode_general.png");
	modeSp->setPosition(Vec2(22.f, 54.f));
	headerBar->addChild(modeSp);

	auto undoButton = MCUI::button("images/game/undo.png", "", "", Vec2(_appSetting.screenWidth - 85.f, _appSetting.screenHeight - 200.f),
		0, Code::code_undo, 0);

	exitLabel = Text::create("", DOTUM, 22.f);
	exitLabel->setAnchorPoint(Vec2::ANCHOR_MIDDLE_RIGHT);
	exitLabel->setPosition(Vec2(_appSetting.screenWidth - 20.f, 30.f));
	getRoot()->addChild(exitLabel);

	auto mouseListener = EventListenerMouse::create();
	mouseListener->onMouseScroll = CC_CALLBACK_1(GameScene::onMouseScroll, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(mouseListener, getRoot());

	startMark = Sprite::create("images/game/start_pos.png");
	startMark->setOpacity(180);
	startMark->setVisible(false);
	board->addChild(startMark, 2);

	endMark = Sprite::create("images/game/end_pos.png");
	endMark->setOpacity(180);
	endMark->setVisible(false);
	board->addChild(endMark, 2);

	touchMe = MCUI::text(getGStr("touch_me"), DOTUM, 32.f, SCREEN_CENTER, Color3B::WHITE);
	touchMe->enableShadow(Color4B::BLACK);
	touchMe->runAction(RepeatForever::create(Sequence::createWithTwoActions(
		FadeTo::create(1.f, 110),
		FadeTo::create(1.f, 255)
	)));
	touchMe->setVisible(false);
}
	 
void GameScene::initPlayers() {
	for (char i = 0; i < backend->getParty(); ++i) {
		Player* p = Player::createPlayer(i, i);
		players[i] = p;
	}
}
	 
void GameScene::initState() {
	backend = (Backend*) getBackend();
	stage = Stage::stage_init;
	me = getBackTurn(0);

	touchable = false;
	isBoardInited = false;

	edgeLen = ORTH_LEN / ROOT_3 * 2;
	origPos = Vec2(0, V_CENTER_OFFSET - (backend->getDim() - 1) * 2 * ORTH_LEN);
	pieceRad = edgeLen * 0.465f;

	firstTouch = Vec2::ZERO;
	courtPos = Vec2::ZERO;
	boardScale = 1.f;
}

void GameScene::onInitialized() {
	CLScene::onInitialized();
}

void GameScene::initBoard() {
	if (isBoardInited) return;
	vector<uint> edges = backend->getEdges();

	for (vector<uint>::iterator it = edges.begin(); it != edges.end(); ++it) {
		Vec2 start = ptToScreen(backend->getSpt(*it));
		Vec2 end = ptToScreen(backend->getEpt(*it));

		ImageView* img = ImageView::create("images/edges/00.png");		
		img->setScaleX(start.distance(end) / img->getContentSize().width);
		img->setPosition((start + end) / 2);
		img->setRotation(-atan((end.y - start.y) / (end.x - start.x)) / M_PI * 180);
		board->addChild(img);
	}
#ifdef SHOW_VALUE_BOARD
	vector<ushort> validPts = backend->getValidPts();
	const char* valueBoard = backend->getValueBoard(0);

	for (vector<ushort>::iterator it = validPts.begin(); it != validPts.end(); ++it) {
		Vec2 pos = ptToScreen(*it) + Vec2(0, -15.f);
		Text* vt = MCUI::text(MCUtil::createStr("%d", valueBoard[*it]), "", 24.f, pos, Color3B::WHITE, board);
		vt->setLocalZOrder(3);
	}
#endif
	isBoardInited = true;
}

Vec2 GameScene::ptToScreen(ushort pt) {
	float rx = ((char) backend->getX(pt) - backend->getDim() + 1) * edgeLen;
	float ry = ((char) backend->getY(pt) - backend->getDim() + 1) * edgeLen;
	return origPos + Vec2((rx - ry) / 2, (rx + ry) * ROOT_3 / 2);
}

void GameScene::refreshBoard() {
	initBoard();
	destroyPieces();

	backend->getBoard(tempBoard, me);
	vector<ushort> validPts = backend->getValidPts();
	
	for (vector<ushort>::iterator it = validPts.begin(); it != validPts.end(); ++it) {
		ushort pt = *it;
		if (tempBoard[pt] < 0) continue;
		pieces.push_back(Piece::createPiece(pt, getBackTurn(tempBoard[pt])));
	}
}

void GameScene::destroyPieces() {
	for (vector<Piece*>::iterator it = pieces.begin(); it != pieces.end(); ++it) {		
		(*it)->removeFromParent();
	}
	pieces.clear();
}

void GameScene::showActMarks(bool show) {
	startMark->setVisible(false);
	endMark->setVisible(false);

	if (show && backend->getGameLen() > 0) {
		uint act = backend->getHistory(backend->getGameLen() - 1);
		
		startMark->setPosition(ptToScreen(backend->getSpt(act)));
		endMark->setPosition(ptToScreen(backend->getEpt(act)));
		startMark->setVisible(true);
		endMark->setVisible(true);
	}
}

Animate* GameScene::get2dEffect(const char* baseName, int count, float delay) {
	Vector<SpriteFrame*> frames(count);
	
	for (int i = 0; i < count; i++) {
		SpriteFrame* f = SpriteFrameCache::getInstance()->getSpriteFrameByName(
			__String::createWithFormat("%s%03d.png", baseName, i)->getCString());
		frames.pushBack(f);
	}
	Animate* animate = Animate::create(Animation::createWithSpriteFrames(frames, delay));
	return animate;
}

void GameScene::onPieceArrived(uint act) {
	selectedPiece = nullptr;
	showActMarks();
	progLabel->setString(createGStr("prog_text", backend->getGameLen(), backend->getMaxGameLen()).c_str());

	uchar ranks[MAX_PARTY];
	Backend::Exit exit = backend->checkExit(ranks);

	if (exit != Backend::Exit::none) {
		///////////////////////////////
#ifdef PY_THINK
		if (ranks[me] == 1) {
			FILE* file = fopen(MCUtil::getWritablePath(MCUtil::numToStr(rand() % 99999)).c_str(), "wb");
			
			for (ushort i = 0; i < backend->getGameLen(); ++i) {
				uint act = backend->getHistory(i);
				fwrite(&act, 4, 1, file);
			}
			fclose(file);
		}
#endif
		///////////////////////////////
		onGameOver(exit, ranks);
	} else {
		think();
	}
}

void GameScene::onGameOver(Backend::Exit exit, uchar* ranks) {
	stage = Stage::stage_game_over;
	touchable = true;
	enableRecv(true);

	if (ranks[me] == 1) {
		if (ranks[(me + 1) % 2] == 1) {
			exitLabel->setString(createGStr("exit_draw", getGStr("exit_%d", exit).c_str()).c_str());
		} else {
			exitLabel->setString(createGStr("exit_win", getGStr("exit_%d", exit).c_str()).c_str());
		}
	} else {
		exitLabel->setString(createGStr("exit_lose", getGStr("exit_%d", exit).c_str()).c_str());
	}
}

void GameScene::think() {
	selectedPiece = nullptr;
	stage = Stage::stage_play;
	enableRecv(true);

	players[getFrontTurn(backend->getTurn())]->think();
	uchar turn = backend->getTurn();

#ifdef PY_THINK
	if (!strcmp(&thinkConfig[turn][0], "human")) {
		touchable = true;
	} else if (!strcmp(&thinkConfig[turn][0], "local")) {
		runEngine(0);
	} else {
		pyThink(&thinkConfig[turn][0]);
	}
#else
	if (backend->getTurn() == me && !mPlayerInfo[me]->isBot()) {
		touchable = true;
	} else {
		runEngine(0);
	}
#endif
}

void GameScene::engineProc(int engineCode) {
	thought = backend->think();
}

void GameScene::onEngineCompleted(int engineCode, float dt) {
	mNotify.ibuff[0] = thought;
	notify();
}

void GameScene::go() {
	players[getFrontTurn(backend->getTurn())]->go();
	vector<ushort> path = backend->go(thought);

	thought = backend->backToFront(thought, me);
	path = backend->backToFront(&path, me);

	if (selectedPiece && (selectedPiece->pos != backend->getSpt(thought))) selectedPiece->select(false);

	if (!selectedPiece) {
		for (Piece* p : pieces) {
			if (p->pos == path[0]) {
				selectedPiece = p;
				p->select(true);
				break;
			}
		}
	}
	stage = Stage::stage_effect;
	touchable = false;
	enableRecv(false);
		
	selectedPiece->goThrough(path);
}

void GameScene::undo() {
	uint undoActs[MAX_PARTY];
	bool ok = backend->undo(undoActs);
	if (!ok) return;

	stage = Stage::stage_effect;
	enableRecv(false);
	touchable = false;

	if (selectedPiece) {
		selectedPiece->select(false);
		selectedPiece = nullptr;
	}
	hideLegalEffects();

	for (uchar i = 0; i < backend->getParty(); ++i) {
		uint act = backend->backToFront(undoActs[i], me);
		ushort spt = backend->getSpt(act);
		ushort ept = backend->getEpt(act);

		for (Piece* p : pieces) {
			if (p->pos == ept) {
				p->pos = spt;
				p->setPosition(ptToScreen(spt));
				break;
			}
		}
	}
	showActMarks();
	progLabel->setString(createGStr("prog_text", backend->getGameLen(), backend->getMaxGameLen()).c_str());

	if (backend->getTurn() == me) {
		undoCountLabel->setString(createGStr("undo_count", backend->getUndoCount(me), getMaxUndoCount(backend->getAiLevel())).c_str());
	}
	think();
}

void GameScene::hideLegalEffects() {
	for (vector<Sprite*>::iterator it = legalEffects.begin(); it != legalEffects.end(); ++it) {		
		(*it)->removeFromParent();
	}
	legalEffects.clear();
}

void GameScene::onClickListener(int code, int tag) {
	if (code == Code::code_undo) {
		if (!touchable) return;
		if (stage != Stage::stage_play) return;
		if (backend->getUndoCount(me) >= getMaxUndoCount(backend->getAiLevel())) return;

		mNotify.ibuff[0] = -1;
		notify();
	}
}

bool GameScene::onTouchBegan(Vec2 loc) {
	firstTouch = loc;
	if (!touchable) return true;

	switch (backend->getStage()) {
	case Backend::Stage::stage_new_game:
		firstTouch = Vec2::ZERO;

		touchMe->setVisible(false);
		touchable = false;

		mNotify.cbuff[0] = rand() % backend->getParty();
		notify();
		return true;
	case Backend::Stage::stage_game_over:
		touchable = false;
		//notify();
		return true;
	case Backend::Stage::stage_play:
		if (stage != Stage::stage_play) return true;
		if (backend->getTurn() != me) return true;

		hideLegalEffects();
		loc = board->convertToNodeSpace(convertToWorldSpace(loc));

		for (Piece* p : pieces) {
			if (p->player != me) continue;
			if (p->getPosition().distance(loc) <= pieceRad) {
				if (selectedPiece && (selectedPiece != p)) selectedPiece->select(false);

				p->select(!p->isSelected);
				selectedPiece = p->isSelected ? p : nullptr;

				if (p->isSelected) {
					vector<ushort> legals = backend->getLegalsFrom(backend->frontToBack(p->pos, me));
					legals = backend->backToFront(&legals, me);
					
					for (vector<ushort>::iterator it = legals.begin(); it != legals.end(); ++it) {
						Sprite* sp = Sprite::createWithSpriteFrameName("C000.png");
						sp->setScale(0.8f);
						sp->setPosition(ptToScreen(*it));
						board->addChild(sp, 3);

						Animate* ani = get2dEffect("C", 10, 0.07f);
						sp->runAction(RepeatForever::create(Sequence::createWithTwoActions(ani, ani->reverse())));
						legalEffects.push_back(sp);
					}
				}
				firstTouch = Vec2::ZERO;
				return true;
			}
		}
		if (!selectedPiece) return true;
		vector<ushort> validPts = backend->getValidPts();

		for (vector<ushort>::iterator it = validPts.begin(); it != validPts.end(); ++it) {
			if (ptToScreen(*it).distance(loc) <= pieceRad / 2.f) {
				uint act = backend->getAct(selectedPiece->pos, *it);
				act = backend->frontToBack(act, me);
				
				if (backend->isLegal(act)) {
					mNotify.ibuff[0] = act;
					notify();
					return true;
				}
				break;
			}
		}
		selectedPiece->select(false);
		selectedPiece = nullptr;
		return true;
	}
	return true;
}

void GameScene::onTouchMoved(Vec2 loc) {
	if (!firstTouch.equals(Vec2::ZERO)) court->setPosition(safeCourtPos(courtPos + loc - firstTouch));
}

void GameScene::onTouchEnded(Vec2 loc) {
	if (!firstTouch.equals(Vec2::ZERO)) {
		Vec2 newCourtPos = safeCourtPos(courtPos + loc - firstTouch);
		court->setPosition(newCourtPos);

		firstTouch = Vec2::ZERO;
		courtPos = newCourtPos;
	}
}

void GameScene::onKeyReleased(EventKeyboard::KeyCode keycode) {
}

void GameScene::onKeyPressed(EventKeyboard::KeyCode keycode) {
}

void GameScene::onMouseScroll(EventMouse* event) {
	float scrollY = event->getScrollY();

	if (scrollY < 0) {
		boardScale = MIN(boardScale + 0.1f, 1.5f);
	} else if (scrollY > 0) {
		boardScale = MAX(boardScale - 0.1f, 0.5f);
	}
	board->setScale(boardScale);
}

void GameScene::advance() {
	switch (backend->getStage()) {
	case Backend::Stage::stage_new_game:
		for (char i = 0; i < backend->getParty(); ++i) {
			players[i]->resetChair();
		}
		backend->newGame(mNotify.cbuff[0]);
		refreshBoard();
		think();
		break;
	case Backend::Stage::stage_game_over:
		replaceTo(nullptr, false, 0);
		break;
	case Backend::Stage::stage_play:
		thought = mNotify.ibuff[0];

		if (thought == 0xFFFFFFFF) {
			undo();
		} else {
			go();
		}		
	}
}

void GameScene::recover() {
	Backend::Stage backStage = backend->getStage();

	if (stage == Backend::Stage::stage_new_game) {
		if (me == 0) {
			runAction(Sequence::createWithTwoActions(
				DelayTime::create(0.3f),
				CallFunc::create([&]() {
					touchMe->setVisible(true);
					touchable = true;
				})
			));
		}
		return;
	}
}

Vec2 GameScene::safeCourtPos(Vec2 pos) {
	Vec2 safePos;
	safePos.x = MIN(MAX(pos.x, courtBound.getMinX()), courtBound.getMaxX());
	safePos.y = MIN(MAX(pos.y, courtBound.getMinY()), courtBound.getMaxY());
	return safePos;
}

void GameScene::setCourtBound() {
	courtBound.setRect(
		(_appSetting.screenWidth - courtSize.width) / 2,
		(_appSetting.screenHeight - courtSize.height) / 2,
		courtSize.width - _appSetting.screenWidth,		
		courtSize.height - _appSetting.screenHeight
	);
}

#ifdef PY_THINK
void GameScene::pyThink(char* serverAddress) {
	ushort boardSize = backend->getBoardSize();

	if (backend->getParty() == 2) {
		boardSize = (ushort) backend->getMiniBoardSize();
		backend->getMiniBoard(tempBoard, 0);
	} else {
		backend->getBoard(tempBoard, 0);
	}
	char url[2048];
	sprintf(url, "%s/sos/%d:%d:%d:", serverAddress, backend->getTurn(), backend->getGameLen(), backend->getLeader());

	for (ushort i = 0; i < boardSize; ++i) {
		if (i > 0) strcat(url, ",");
		strcat(url, MCUtil::createStr("%d", tempBoard[i]).c_str());
	}
	HttpRequest* pyRequest = new (std::nothrow) HttpRequest();
    pyRequest->setUrl(url);
    pyRequest->setRequestType(HttpRequest::Type::POST);
    pyRequest->setResponseCallback(CC_CALLBACK_2(GameScene::onPyThinkCompleted, this));    
    
	HttpClient::getInstance()->setTimeoutForConnect(1200);
	HttpClient::getInstance()->setTimeoutForRead(1200);

	HttpClient::getInstance()->send(pyRequest);
	pyRequest->release();
}

void GameScene::onPyThinkCompleted(HttpClient* sender, HttpResponse* response) {	
	if (!response || !response->isSucceed()) return;

    vector<char>* buffer = response->getResponseData();	
	string str;

	for (char i = 0; i < buffer->size(); ++i) {
		str += buffer->at(i);
	}
	if (backend->getParty() == 2) {
		uchar miniDim = (backend->getDim() - 1) * 2 + 3;
		uchar dim2 = backend->getDim() - 2;

		str = str.substr(18);
		ushort miniThought = (ushort) atoi(str.c_str());

		uchar miniSpt = miniThought & 0xFF;
		uchar miniEpt = miniThought >> 8;
		
		uchar miniSX = miniSpt / miniDim;
		uchar miniSY = miniSpt % miniDim;

		uchar miniEX = miniEpt / miniDim;
		uchar miniEY = miniEpt % miniDim;

		ushort spt = backend->getPt(miniSX + dim2, miniSY + dim2);
		ushort ept = backend->getPt(miniEX + dim2, miniEY + dim2);

		thought = backend->getAct(spt, ept);
	} else {
		thought = (uint) atoi(str.c_str());
	}
	onEngineCompleted(0, 0);
}
#endif
