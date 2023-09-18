
#include "global.h"
#include "konix.h"

class Backend : public MCSerializer {
public:
	enum Stage {
		stage_new_game, stage_play, stage_game_over
	};
	enum Exit {
		none, normal, defend_off, attack_off, knock_out, length_out
	};
	struct Rule {
		uchar aiLevel;
		uchar dim;
		uchar party;
	};
	struct Serial {
		Rule rule;
		Stage stage;
		char board[MAX_PT];
		uchar turn;
		uchar leader;
		uchar winner;
		ushort gameLen;
		uint history[MAX_PARTY * MAX_GO_PER_PLAYER];
		uchar undoCount[MAX_PARTY];
	};

private:
	Serial serial;
	vector<uint> legals;

public:
	Backend();
	Backend(Rule* rule);
	virtual ~Backend();

	virtual uint getSize() override;
	virtual uchar* save() override;
	virtual void load(uchar* buffer) override;

	void newGame(uchar leader);

	Stage getStage();
	uchar getLeader();
	uchar getTurn();	
	uchar getParty();
	uchar getDim();
	ushort getMaxGameLen();
	ushort getGameLen();
	uchar getAiLevel();

	void getBoard(char* board, uchar me);
	void getMiniBoard(char* board, uchar me);
	uint getHistory(ushort idx);

	const char* getVacantBoard();
	const char* getBootBoard();
	const char* getValueBoard(uchar turn);

	vector<uint> getEdges();
	vector<ushort> getValidPts();
	
	ushort getBoardSize();
	uchar getMiniBoardSize();
	
	uint think();
	vector<ushort> go(uint act);
	
	bool undo(uint* acts);
	uchar getUndoCount(uchar turn);
	Exit checkExit(uchar* ranks);
		
	bool isLegal(uint act);
	vector<ushort> getLegalsFrom(ushort pt);
	void setLegals();
	
	ushort backToFront(ushort pt, uchar me);
	ushort frontToBack(ushort pt, uchar me);
	
	uint backToFront(uint act, uchar me);
	uint frontToBack(uint act, uchar me);

	vector<ushort> backToFront(vector<ushort>* path, uchar me);
	vector<ushort> frontToBack(vector<ushort>* path, uchar me);

	uchar getX(ushort pt);
	uchar getY(ushort pt);

	ushort getSpt(uint act);
	ushort getEpt(uint act);

	ushort getPt(uchar x, uchar y);
	uint getAct(ushort spt, ushort ept);
};
