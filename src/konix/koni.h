
#pragma once

#include <stdio.h>
#include <math.h>
#include <time.h>
#include <vector>
#include <set>
#include <string>
#include <algorithm>
#include <random>

using namespace std;

namespace koni {

#define MAX_PT 289
#define MAX_PARTY 3
#define MAX_VALID 121
#define MAX_ADJACENT 6
#define MAX_PIECE_COUNT 15
#define MAX_LEGALS 150
#define MAX_GO_LEN 100
#define MAX_MINI_PT 121
#define CHECK_PRISON_DEPTH 100

#ifndef uchar
#define uchar unsigned char
#endif
#ifndef ushort
#define ushort unsigned short
#endif
#ifndef uint
#define uint unsigned int
#endif
#ifndef MIN
#define MIN(x, y) (((x) <= (y)) ? (x) : (y))
#endif
#ifndef MAX
#define MAX(x, y) (((x) >= (y)) ? (x) : (y))
#endif

#define PT(x, y) ((x) * koni::global::board_dim + (y))
#define PX(pt) ((uchar) ((pt) / koni::global::board_dim))
#define PY(pt) ((uchar) ((pt) % koni::global::board_dim))

#define ACT(spt, ept) (uint) (((uint) (spt)) | (((uint) (ept)) << 16))
#define ASPT(act) (ushort) ((act) & 0xFFFF)
#define AEPT(act) (ushort) ((act) >> 16)

struct Coord {
	char x;
	char y;
};

enum Orient {
	north_west, east, south_east, south
};

enum Exit {
	none, normal, knockout, prisoned, depthout, strange
};

namespace global {

extern uchar party;
extern bool only_mini_board;
extern ushort max_depth;
extern uchar dim;
extern uchar dim_1;
extern uchar dim_2;
extern uchar dbl_dim;
extern uchar tri_dim;
extern uchar quad_dim;
extern uchar penta_dim;
extern uchar dec_dim;
extern uchar fifteen_dim;
extern uchar board_dim;
extern ushort board_size;
extern uchar mini_board_dim;
extern uchar mini_board_size;
extern short win_value;
extern short init_value;
extern uchar piece_count;
extern uchar valid_pts_count;
extern float inv_dec_dim;
extern uint max_hop_tag;
extern Orient seats[MAX_PARTY];
extern char vacant_board[MAX_PT];
extern char boot_board[MAX_PT];
extern char value_board[MAX_PARTY][MAX_PT];
extern char prison_board[MAX_PT];
extern ushort valid_pts[MAX_VALID];
extern ushort home_pts[MAX_PARTY][MAX_PIECE_COUNT];
extern Coord home_coords[MAX_PIECE_COUNT];
extern Coord valid_coords[MAX_VALID];
extern uchar walks_count[MAX_PT];
extern ushort walks[MAX_PT][MAX_ADJACENT];
extern uchar good_walks_count[MAX_PARTY][MAX_PT];
extern ushort good_walks[MAX_PARTY][MAX_PT][MAX_ADJACENT];
extern uchar hops_count[MAX_PT];
extern ushort walk_hops[MAX_PT][MAX_ADJACENT][2];
extern char rot_vmap[MAX_PARTY - 1][MAX_PARTY + 2];
extern ushort trace_buff[MAX_VALID];
extern char board_buff[MAX_PT];
extern uint hop_tags[MAX_PT];
extern bool inited;

}

bool init_koni(uchar party, uchar dim, bool only_mini_board = false);
void finish_koni();

void _init_geometry_();
void _init_valid_coords_();
void _init_basic_boards_();
void _init_value_board_();
void _init_walks_();
void _init_good_walks_();
void _init_walk_hops_();
void _init_rot_vmap_();

void reset_hop_tags();
vector<ushort> get_hops_from_recursive(char* board, ushort pt, uchar trace = 0);
vector<uint> get_edges();
void get_random_board(char* board, bool in_mini = false);

vector<ushort> find_path(char* board, uint act);
vector<ushort> find_hop_path_recursive(char* board, ushort spt, ushort ept, uchar trace = 0);

bool is_independent(char* board, uchar turn);
bool is_legal(char* board, uchar turn, uint act);
bool is_walk(uint act);

#ifdef _WIN32
void print_board(char* board, bool is_scalar = false, uchar indent = 0);
void interrupt_console(const char* msg = nullptr);
#endif

inline void set_coord(Coord* dst, char x, char y) {
	dst->x = x;
	dst->y = y;
}

inline ushort coord_to_pt(Orient orient, char cx, char cy) {
	switch (orient) {
	case Orient::north_west: return PT(global::dim_1 + cx, global::dim_1 + cy);
	case Orient::east: return PT(global::dim_1 + cy, global::quad_dim - cx - cy);
	case Orient::south_east: return PT(global::tri_dim - cx, global::tri_dim - cy);
	case Orient::south: return PT(global::quad_dim - cx - cy, global::dim_1 + cx);
	}
	return 0;
}

inline void pt_to_coord(Coord* dst, Orient orient, ushort pt) {
	uchar x = PX(pt);
	uchar y = PY(pt);
	
	switch (orient) {
	case Orient::north_west:
		set_coord(dst, x - global::dim_1, y - global::dim_1);
		break;
	case Orient::east:
		set_coord(dst, global::penta_dim - x - y, x - global::dim_1);
		break;
	case Orient::south_east:
		set_coord(dst, global::tri_dim - x, global::tri_dim - y);
		break;
	case Orient::south:
		set_coord(dst, y - global::dim_1, global::penta_dim - x - y);
		break;
	}
}

inline ushort rotate(ushort pt, uchar count = 1) {
	if (count % global::party == 0) return pt;
	Coord coord;

	pt_to_coord(&coord, Orient::north_west, pt);
	return coord_to_pt(global::seats[count], coord.x, coord.y);
}

inline void rotate(char* dst_board, char* src_board, uchar count = 1) {
	memcpy(dst_board, src_board, global::board_size);
	if (count % global::party == 0) return;

	Orient orient = global::seats[count];
	char* vmap = &global::rot_vmap[count - 1][0];

	for (uchar i = 0; i < global::valid_pts_count; ++i) {
		Coord* vc = &global::valid_coords[i];
		ushort spt = coord_to_pt(Orient::north_west, vc->x, vc->y);
		ushort opt = coord_to_pt(orient, vc->x, vc->y);
		dst_board[opt] = vmap[src_board[spt] + 2];
	}
}

inline void go(char* dst_board, char* src_board, uint act, short* values = nullptr, uchar turn = 0) {
	if (dst_board != src_board) memcpy(dst_board, src_board, global::board_size);
	if (!act) return;

	ushort spt = ASPT(act);
	ushort ept = AEPT(act);

	dst_board[ept] = src_board[spt];
	dst_board[spt] = -1;

	if (values) {
		char* vb = &global::value_board[turn][0];
		values[turn] += vb[ept] - vb[spt];
	}
	return;
}

inline bool is_in_mini_board(char x, char y) {
	return x >= global::dim_2 && y >= global::dim_2 && x < global::board_dim - global::dim_2 && y < global::board_dim - global::dim_2;
}

inline bool is_in_board(char x, char y) {
	return x >= 0 && y >= 0 && x < global::board_dim && y < global::board_dim;
}

inline bool is_in_trace(ushort pt, uchar trace) {
	for (ushort* tpt = global::trace_buff; tpt < global::trace_buff + trace; ++tpt) {
		if (*tpt == pt) return true;
	}
	return false;
}

inline short get_value(char* board, uchar turn) {
	char* value_board = &global::value_board[turn][0];
	ushort* valid_pts = global::valid_pts;
	char tv = (char) turn;
	short value = 0;

	for (uchar i = 0; i < global::valid_pts_count; ++i, ++valid_pts) {
		ushort pt = *valid_pts;
		if (board[pt] == tv) value += value_board[pt];
	}
	return value;
}

inline vector<ushort> get_walks_from(char* board, uchar turn, ushort pt, bool drop_bads = false) {
	uchar wcount = drop_bads ? global::good_walks_count[turn][pt] : global::walks_count[pt];
	ushort* p_walks = drop_bads ? &global::good_walks[turn][pt][0] : &global::walks[pt][0];
	vector<ushort> walks;

	if (drop_bads) {
		char* value_board = &global::value_board[turn][0];
		char sv = value_board[pt];

		for (uchar i = 0; i < wcount; ++i, ++p_walks) {
			ushort wpt = *p_walks;
			if (board[wpt] < 0 && value_board[wpt] >= sv) walks.push_back(wpt);
		}
	} else {
		for (uchar i = 0; i < wcount; ++i, ++p_walks) {
			ushort wpt = *p_walks;
			if (board[wpt] < 0) walks.push_back(wpt);
		}
	}
	return walks;
}

inline vector<ushort> get_good_hops_from(char* board, uchar turn, ushort pt) {
	if (global::max_hop_tag == 0xFFFFFFFF) reset_hop_tags();

	uint tag = global::max_hop_tag + 1;
	global::max_hop_tag = tag;
	
	char* value_board = &global::value_board[turn][0];
	char sv = value_board[pt];

	vector<ushort> hop_roots;
	hop_roots.push_back(pt);
	global::hop_tags[pt] = tag;

	vector<ushort> hops;
	vector<ushort> strict_hops;

	while (!hop_roots.empty()) {
		vector<ushort> new_roots;

		for (vector<ushort>::iterator root = hop_roots.begin(); root != hop_roots.end(); ++root) {
			ushort* walk_hops = &global::walk_hops[*root][0][0];

			for (uchar i = 0; i < global::hops_count[*root]; ++i, walk_hops += 2) {
				ushort wpt = *walk_hops;
				ushort hpt = walk_hops[1];

				if (global::hop_tags[hpt] == tag) continue;
				if (board[wpt] < 0 || board[hpt] >= 0) continue;

				global::hop_tags[hpt] = tag;
				new_roots.push_back(hpt);

				char ev = value_board[hpt];

				if (ev >= sv) {
					hops.push_back(hpt);
					if (ev > sv) strict_hops.push_back(hpt);
				}
			}
		}
		hop_roots = new_roots;
	}
	if (!strict_hops.empty()) return strict_hops;
	return hops;
}

inline vector<ushort> get_hops_from(char* board, uchar turn, ushort pt, bool drop_bads = false) {
	vector<ushort> all_hops = get_hops_from_recursive(board, pt);	
	set<ushort> hops_set;
	vector<ushort> hops;

	hops_set.insert(all_hops.begin(), all_hops.end());

	if (drop_bads) {
		char* value_board = &global::value_board[turn][0];
		char sv = value_board[pt];
		vector<ushort> strict_hops;

		for (set<ushort>::iterator hpt = hops_set.begin(); hpt != hops_set.end(); ++hpt) {
			char ev = value_board[*hpt];

			if (ev >= sv) {
				hops.push_back(*hpt);
				if (ev > sv) strict_hops.push_back(*hpt);
			}
		}
		if (!strict_hops.empty()) hops = strict_hops;
	} else {
		hops.insert(hops.end(), hops_set.begin(), hops_set.end());
	}
	return hops;
}

inline vector<uint> get_legals(char* board, uchar turn, bool drop_bads = false) {
	vector<uint> legals;

	if (!drop_bads && get_value(board, turn) == global::win_value) {
		legals.push_back(0);
		return legals;
	}
	ushort* valid_pts = global::valid_pts;

	for (uchar i = 0; i < global::valid_pts_count; ++i, ++valid_pts) {
		ushort pt = *valid_pts;
		if (board[pt] != (char) turn) continue;

		vector<ushort> walks = get_walks_from(board, turn, pt, drop_bads);

		for (vector<ushort>::iterator wpt = walks.begin(); wpt != walks.end(); ++wpt) {
			legals.push_back(ACT(pt, *wpt));
		}
		vector<ushort> hops = drop_bads ? get_good_hops_from(board, turn, pt) : get_hops_from(board, turn, pt);
		
		for (vector<ushort>::iterator hpt = hops.begin(); hpt != hops.end(); ++hpt) {
			legals.push_back(ACT(pt, *hpt));
		}
	}
	return legals;
}

inline void get_scores(float* scores, short* values, bool clip = false) {
	if (global::party == 2) {
		float score_0 = (values[0] - values[1]) * global::inv_dec_dim;
		scores[0] = score_0; scores[1] = -score_0;
	} else {
		scores[0] = (values[0] - (values[1] + values[2]) / 2.f) * global::inv_dec_dim;
		scores[1] = (values[1] - (values[0] + values[2]) / 2.f) * global::inv_dec_dim;
		scores[2] = -(scores[0] + scores[1]);
	}
	if (clip) {
		for (float* v = scores; v < scores + global::party; ++v) {
			if (*v < -0.95f) {
				*v = -0.95f;
			} else if (*v > 0.95f) {
				*v = 0.95f;
			}
		}
	}
}

inline bool is_fully_prisoned(char* board, uchar turn) {
	ushort* home_pts = &global::home_pts[turn][0];

	for (ushort* pt = home_pts; pt < home_pts + global::piece_count; ++pt) {
		char v = board[*pt];

		if (v < 0) return false;
		if (v == (char) turn && !global::prison_board[*pt]) return false;
	}
	return true;
}

inline bool is_prisoned(char* board, uchar turn) {
	ushort* home_pts = &global::home_pts[turn][0];

	for (ushort* pt = home_pts; pt < home_pts + global::piece_count; ++pt) {
		if (board[*pt] == turn && global::prison_board[*pt]) return true;
	}
	return false;
}

inline Exit check_end(char* rewards, char* board, short* values, ushort depth = 0, bool no_legals = false, bool for_mcts = false) {
	uchar ends = 0;

	for (uchar t = 0; t < global::party; ++t) {
		if (values[t] == global::win_value) {
			++ends;
			rewards[t] = 1;
		} else {
			rewards[t] = -1;
		}
	}
	if (ends >= global::party - 1) return Exit::normal;

	if (global::party == 2) {
		if (!for_mcts) {
			if (values[0] > values[1] + global::fifteen_dim) {
				rewards[0] = 1; rewards[1] = -1; return Exit::knockout;
			}
			if (values[1] > values[0] + global::fifteen_dim) {
				rewards[1] = 1; rewards[0] = -1; return Exit::knockout;
			}
		}
		if (depth >= CHECK_PRISON_DEPTH || no_legals) {
			bool p0, p1;

			if (depth >= global::max_depth) {
				p0 = is_prisoned(board, 0); p1 = is_prisoned(board, 1);
			} else {
				p0 = is_fully_prisoned(board, 0); p1 = is_fully_prisoned(board, 1);
			}
			if (p0 && p1) {
				rewards[0] = rewards[1] = 0; return Exit::prisoned;
			} else if (p0) {
				rewards[1] = 1; rewards[0] = -1; return Exit::prisoned;
			} else if (p1) {
				rewards[0] = 1; rewards[1] = -1; return Exit::prisoned;
			} else if (no_legals) {
				rewards[0] = rewards[1] = 0; return Exit::strange;
			} else if (depth >= global::max_depth) {
				if (!for_mcts) {
					if (values[0] > values[1] + global::dim_1) {
						rewards[0] = 1; rewards[1] = -1; return Exit::depthout;
					}
					if (values[1] > values[0] + global::dim_1) {
						rewards[1] = 1; rewards[0] = -1; return Exit::depthout;
					}
				}
				rewards[0] = rewards[1] = 0; return Exit::depthout;
			}
		}
	}
	return Exit::none;
}

inline void get_mini_board(char* mini_board, char* board) {
	using namespace global;

	char* bp = board + board_dim * dim_2;
	char* mp = mini_board;

	for (uchar x = 0; x < mini_board_dim; ++x, bp += board_dim, mp += mini_board_dim) {
		memcpy(mp, bp + dim_2, mini_board_dim);
	}
}

inline uchar get_mini_pt(ushort pt) {
	uchar x = PX(pt) - global::dim_2;
	uchar y = PY(pt) - global::dim_2;
	return x * global::mini_board_dim + y;
}

inline ushort get_mini_act(uint act) {
	ushort spt = ASPT(act);
	ushort ept = AEPT(act);
	
	uchar mspt = get_mini_pt(spt);
	uchar mept = get_mini_pt(ept);

	return (ushort) (((ushort) mspt) | (((ushort) mept) << 8));
}

}
