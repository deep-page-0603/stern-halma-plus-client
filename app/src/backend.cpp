
#include "backend.h"

Backend::Backend() {}

Backend::~Backend() {
	konix::finish_konix();
}

Backend::Backend(Backend::Rule* rule) {
	memset(&serial, 0, sizeof(Serial));
	memcpy(&serial.rule, rule, sizeof(Rule));

	serial.stage = Stage::stage_new_game;

#if CC_TARGET_PLATFORM == CC_PLATFORM_WIN32
	Data dat = FileUtils::getInstance()->getDataFromFile("models/koni_p2_d5_win.daj");
#elif CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
	Data dat = FileUtils::getInstance()->getDataFromFile("models/koni_p2_d5_paddle.daj");
#endif
	konix::init_konix(serial.rule.party, serial.rule.dim, (const char*) dat.getBytes());
}

uint Backend::getSize() {
	return sizeof(Backend::Serial);
}

uchar* Backend::save() {
	uchar* buffer = (uchar*) malloc(sizeof(Serial));
	memcpy(buffer, &serial, sizeof(Serial));
	return buffer;
}

void Backend::load(uchar* buffer) {
	memcpy(&serial, buffer, sizeof(Serial));
	konix::init_konix(serial.rule.dim, serial.rule.party, MCUtil::getWritablePath("koni_p2_d5.daj").c_str());
	setLegals();
}

void Backend::newGame(uchar leader) {	
	memcpy(serial.board, konix::get_boot_board(), konix::get_board_size());
	//konix::get_random_board(serial.board, true); //////////////////////

	serial.stage = Stage::stage_play;
	serial.leader = leader;
	serial.turn = serial.leader;
	serial.gameLen = 0;
	setLegals();
}

const char* Backend::getVacantBoard() {
	return konix::get_vacant_board();
}

const char* getBootBoard() {
	return konix::get_boot_board();
}

vector<uint> Backend::getEdges() {
	return konix::get_edges();
}

uchar Backend::getDim() {
	return serial.rule.dim;
}

uchar Backend::getLeader() {
	return serial.leader;
}

uchar Backend::getTurn() {
	return serial.turn;
}

void Backend::getBoard(char* board, uchar me) {
	konix::rotate(board, serial.board, serial.rule.party - me);
}

void Backend::getMiniBoard(char* board, uchar me) {
	char temp_board[MAX_PT];
	konix::rotate(temp_board, serial.board, serial.rule.party - me);
	konix::get_mini_board(board, temp_board);
}

const char* Backend::getValueBoard(uchar turn) {
	return konix::get_value_board(turn);
}

vector<ushort> Backend::getValidPts() {
	return konix::get_valid_pts();
}

ushort Backend::getBoardSize() {
	return konix::get_board_size();
}

uchar Backend::getMiniBoardSize() {
	return konix::get_mini_board_size();
}

uchar Backend::getAiLevel() {
	return serial.rule.aiLevel;
}

uint Backend::think() {
	return konix::think(serial.board, serial.turn, serial.gameLen, serial.rule.aiLevel);
}

vector<ushort> Backend::go(uint act) {
	vector<ushort> path = konix::go(serial.board, act);
	if (konix::is_end(serial.board, serial.turn)) serial.winner = serial.turn + 1;

	serial.turn = (serial.turn + 1) % serial.rule.party;
	serial.history[serial.gameLen++] = act;
	setLegals();
	return path;
}

bool Backend::undo(uint* acts) {
	if (serial.gameLen < serial.rule.party) return false;	
	++serial.undoCount[serial.turn];
	
	for (uchar i = 0; i < serial.rule.party; ++i) {
		uint act = serial.history[--serial.gameLen];
		acts[i] = act;

		ushort spt = konix::get_spt(act);
		ushort ept = konix::get_ept(act);
	
		act = konix::get_act(ept, spt);
		konix::go(serial.board, act);
	}
	setLegals();
	return true;
}

uchar Backend::getUndoCount(uchar turn) {
	return serial.undoCount[serial.turn];
}

ushort Backend::getMaxGameLen() {
	return MAX_GO_PER_PLAYER * serial.rule.party;
}

uint Backend::getHistory(ushort idx) {
	return serial.history[idx];
}

Backend::Exit Backend::checkExit(uchar* ranks) {
	Stage oldStage = serial.stage;
	serial.stage = Stage::stage_game_over;

	char end_flags[MAX_PARTY] = {0, 0, 0};
	uchar ends = 0;

	for (uchar t = 0; t < serial.rule.party; ++t) {
		if (konix::is_end(serial.board, t)) {
			end_flags[t] = 1;
			++ends;
		}
	}
	if (ends == serial.rule.party - 1) {
		uchar last_count = (serial.rule.party - (serial.gameLen % serial.rule.party)) % serial.rule.party;

		for (uchar i = 0; i < last_count; ++i) {
			uchar t = (serial.turn + i) % serial.rule.party;
			if (!end_flags[t] && konix::is_end_but_one(serial.board, t)) end_flags[t] = 1;			
		}
		for (uchar t = 0; t < serial.rule.party; ++t) {
			ranks[t] = end_flags[t] ? 1 : 2;
			
			if ((serial.rule.party > 2) && serial.winner) {
				if (serial.winner != t + 1) ++ranks[t];
			}
		}
		return Exit::normal;
	}	
	if (serial.rule.party == 2) {
		uchar piece_count = konix::get_piece_count();
		const char* boot_board = konix::get_boot_board();
		vector<ushort> vpts = konix::get_valid_pts();
		
		uchar stat[2][2];
		memset(stat, 0, 4);

		for (vector<ushort>::iterator pt = vpts.begin(); pt != vpts.end(); ++pt) {
			char t = boot_board[*pt];

			if (t >= 0) {				
				if (serial.board[*pt] == t) {
					++stat[t][0];
				} else if (serial.board[*pt] >= 0) {
					++stat[(t + 1) % 2][1];
				}
			}
		}
		for (uchar t = 0; t < 2; ++t) {
			uchar o = (t + 1) % 2;

			if ((stat[o][1] > 0) && (stat[t][0] + stat[o][1] == piece_count)) {
				ranks[o] = 1; ranks[t] = 2;
				return Exit::defend_off;
			}
			if ((stat[t][0] > 0) && (stat[t][1] > 0) && (stat[t][0] + stat[t][1] == piece_count) && konix::is_stocked(serial.board, t)) {
				ranks[t] = 1; ranks[o] = 2;
				return Exit::attack_off;
			}
		}
		if (serial.gameLen >= getMaxGameLen() / 2) {
			for (uchar t = 0; t < 2; ++t) {
				if (konix::is_knockout(serial.board, t)) {
					ranks[t] = 1; ranks[(t + 1) % 2] = 2;
					return Exit::knock_out;
				}
			}
		}
	}
	if (serial.gameLen >= getMaxGameLen()) {
		for (uchar t = 0; t < serial.rule.party; ++t) {
			if (serial.winner) {
				ranks[t] = (serial.winner == t + 1) ? 1 : 2;
			} else {
				ranks[t] = 1;
			}
		}
		return Exit::length_out;
	}
	serial.stage = oldStage;
	return Exit::none;
}

bool Backend::isLegal(uint act) {
	return find(legals.begin(), legals.end(), act) != legals.end();
}

vector<ushort> Backend::getLegalsFrom(ushort pt) {
	return konix::get_legals_from(serial.board, pt);
}

void Backend::setLegals() {
	legals = konix::get_legals(serial.board, serial.turn);
}

Backend::Stage Backend::getStage() {
	return serial.stage;
}

uchar Backend::getParty() {
	return serial.rule.party;
}

ushort Backend::getGameLen() {
	return serial.gameLen;
}

ushort Backend::backToFront(ushort pt, uchar me) {
	return konix::rotate(pt, me);
}

ushort Backend::frontToBack(ushort pt, uchar me) {
	return konix::rotate(pt, serial.rule.party - me);
}

uint Backend::backToFront(uint act, uchar me) {
	ushort spt = konix::get_spt(act);
	ushort ept = konix::get_ept(act);

	spt = backToFront(spt, me);
	ept = backToFront(ept, me);

	return konix::get_act(spt, ept);
}

uint Backend::frontToBack(uint act, uchar me) {
	ushort spt = konix::get_spt(act);
	ushort ept = konix::get_ept(act);

	spt = frontToBack(spt, me);
	ept = frontToBack(ept, me);

	return konix::get_act(spt, ept);
}

vector<ushort> Backend::backToFront(vector<ushort>* path, uchar me) {
	vector<ushort> new_path;

	for (vector<ushort>::iterator pt = path->begin(); pt != path->end(); ++pt) {
		new_path.push_back(backToFront(*pt, me));
	}
	return new_path;
}

vector<ushort> Backend::frontToBack(vector<ushort>* path, uchar me) {
	vector<ushort> new_path;

	for (vector<ushort>::iterator pt = path->begin(); pt != path->end(); ++pt) {
		new_path.push_back(frontToBack(*pt, me));
	}
	return new_path;
}

uchar Backend::getX(ushort pt) {
	return konix::get_x(pt);
}

uchar Backend::getY(ushort pt) {
	return konix::get_y(pt);
}

ushort Backend::getSpt(uint act) {
	return konix::get_spt(act);
}

ushort Backend::getEpt(uint act) {
	return konix::get_ept(act);
}

ushort Backend::getPt(uchar x, uchar y) {
	return konix::get_pt(x, y);
}

uint Backend::getAct(ushort spt, ushort ept) {
	return konix::get_act(spt, ept);
}
