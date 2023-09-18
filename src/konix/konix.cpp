
#include "konix.h"
#include "players.h"
#include "net.h"

using namespace dajnn;

namespace konix {

koni::Player* ai_player = nullptr;
uchar prev_ai_level = 0xFF;
koni::Net* neunet = nullptr;
bool inited = false;

bool init_konix(uchar party, uchar dim, const char* model_buff) {
	finish_konix();

	if (koni::init_koni(party, dim)) inited = true;
	dajnn::init_dajnn();

	ByteStream* stream = new ByteStream(model_buff);
	neunet = new koni::Net(stream);
	delete stream;

	//think(koni::global::boot_board, 0, 0, 1);
	return inited;
}

void finish_konix() {
	if (!inited) return;
	
	if (ai_player) {
		delete ai_player;
		ai_player = nullptr;
	}
	if (neunet) {
		delete neunet;
		neunet = nullptr;
	}
	dajnn::finish_dajnn();
	koni::finish_koni();	

	inited = false;
}

const char* get_boot_board() {
	return koni::global::boot_board;
}

const char* get_vacant_board() {
	return koni::global::vacant_board;
}

ushort get_board_size() {
	return koni::global::board_size;
}

uchar get_mini_board_size() {
	return koni::global::mini_board_size;
}

vector<uint> get_edges() {
	return koni::get_edges();
}

vector<uint> get_legals(char* board, uchar turn) {
	koni::reset_hop_tags();
	return koni::get_legals(board, turn);
}

vector<ushort> get_legals_from(char* board, ushort pt) {
	koni::reset_hop_tags();

	vector<ushort> legals = koni::get_walks_from(board, 0, pt);
	vector<ushort> hops = koni::get_hops_from(board, 0, pt);
	legals.insert(legals.end(), hops.begin(), hops.end());
	return legals;
}

void rotate(char* dst_board, char* src_board, uchar count) {
	koni::rotate(dst_board, src_board, count);
}

ushort rotate(ushort pt, uchar count) {
	return koni::rotate(pt, count);
}

void get_mini_board(char* dst_board, char* src_board) {
	koni::get_mini_board(dst_board, src_board);
}

const char* get_value_board(uchar turn) {
	return koni::global::value_board[turn];
}

void get_random_board(char* board, bool in_mini) {
	koni::get_random_board(board, in_mini);
}

vector<ushort> get_valid_pts() {
	vector<ushort> valid_pts;

	for (ushort* pt = koni::global::valid_pts; pt < koni::global::valid_pts + koni::global::valid_pts_count; ++pt) {
		valid_pts.push_back(*pt);
	}
	return valid_pts;
}

vector<ushort> go(char* board, uint act) {
	vector<ushort> path = koni::find_path(board, act);
	koni::go(koni::global::board_buff, board, act);

	memcpy(board, koni::global::board_buff, koni::global::board_size);
	return path;
}

uint think(char* board, uchar turn, ushort depth, uchar ai_level) {
	using namespace koni;

	if (prev_ai_level != ai_level || !ai_player) {
		if (ai_player) delete ai_player;
		ai_player = new MCTSPlayer(Player::ThinkMode::valid, 10, 0.16f, neunet);
		prev_ai_level = ai_level;
	}
	vector<uint> legals;
	uint act = ai_player->think(&legals, board, turn, depth);

	if (!act) {
		legals = get_legals(board, turn);
		act = legals[rand() % legals.size()];
	}
	return act;
}

bool is_end(char* board, uchar turn) {
	return koni::get_value(board, turn) == koni::global::win_value;
}

bool is_end_but_one(char* board, uchar turn) {
	vector<uint> legals = get_legals(board, turn);

	for (vector<uint>::iterator act = legals.begin(); act != legals.end(); ++act) {
		koni::go(koni::global::board_buff, board, *act);
		if (koni::get_value(koni::global::board_buff, turn) == koni::global::win_value) return true;
	}
	return false;
}

bool is_knockout(char* board, uchar turn) {
	short v1 = koni::get_value(board, turn);
	short v2 = koni::get_value(board, (turn + 1) % koni::global::party);
	return v1 - v2 > koni::global::fifteen_dim;
}

bool is_stocked(char* board, uchar turn) {
	vector<uint> legals = get_legals(board, turn);
	char* vb = &koni::global::value_board[turn][0];

	for (vector<uint>::iterator act = legals.begin(); act != legals.end(); ++act) {
		ushort spt = ASPT(*act);
		ushort ept = AEPT(*act);

		if (vb[ept] > vb[spt]) return false;
	}
	return true;
}

uchar get_piece_count() {
	return koni::global::piece_count;
}

ushort get_spt(uint act) {
	return ASPT(act);
}

ushort get_ept(uint act) {
	return AEPT(act);
}

uchar get_x(ushort pt) {
	return PX(pt);
}

uchar get_y(ushort pt) {
	return PY(pt);
}

ushort get_pt(uchar x, uchar y) {
	return PT(x, y);
}

uint get_act(ushort spt, ushort ept) {
	return ACT(spt, ept);
}

}
