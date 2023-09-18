
#include "koni.h"

namespace koni {

namespace global {

uchar party = 0;
bool only_mini_board = false;
ushort max_depth;
uchar dim = 0;
uchar dim_1 = 0;
uchar dim_2 = 0;
uchar dbl_dim = 0;
uchar tri_dim = 0;
uchar quad_dim = 0;
uchar penta_dim = 0;
uchar dec_dim = 0;
uchar fifteen_dim = 0;
uchar board_dim = 0;
ushort board_size = 0;
uchar mini_board_dim = 0;
uchar mini_board_size = 0;
short win_value = 0;
short init_value = 0;
uchar piece_count = 0;
uchar valid_pts_count = 0;
float inv_dec_dim = 0;
uint max_hop_tag = 0;
Orient seats[MAX_PARTY];
char vacant_board[MAX_PT];
char boot_board[MAX_PT];
char value_board[MAX_PARTY][MAX_PT];
char prison_board[MAX_PT];
ushort valid_pts[MAX_VALID];
ushort home_pts[MAX_PARTY][MAX_PIECE_COUNT];
Coord home_coords[MAX_PIECE_COUNT];
Coord valid_coords[MAX_VALID];
uchar walks_count[MAX_PT];
ushort walks[MAX_PT][MAX_ADJACENT];
uchar good_walks_count[MAX_PARTY][MAX_PT];
ushort good_walks[MAX_PARTY][MAX_PT][MAX_ADJACENT];
uchar hops_count[MAX_PT];
ushort walk_hops[MAX_PT][MAX_ADJACENT][2];
char rot_vmap[MAX_PARTY - 1][MAX_PARTY + 2];
ushort trace_buff[MAX_VALID];
char board_buff[MAX_PT];
uint hop_tags[MAX_PT];
bool inited = false;

}

bool init_koni(uchar party, uchar dim, bool only_mini_board) {
	finish_koni();

	if (party != 2 && party != 3) return false;
	if (dim < 3 || dim > 5) return false;

	global::party = party;
	global::only_mini_board = party == 2 && only_mini_board;
	global::max_depth = MAX_GO_LEN * party;
	global::dim = dim;
	
	reset_hop_tags();

	_init_geometry_();
	_init_valid_coords_();
	_init_basic_boards_();
	_init_value_board_();
	_init_walks_();
	_init_good_walks_();
	_init_walk_hops_();
	_init_rot_vmap_();
		
	global::inited = true;
	return true;
}

void finish_koni() {
	if (!global::inited) return;
	global::inited = false;
}

void _init_geometry_() {
	using namespace global;

	dim_1 = dim - 1;
	dim_2 = dim - 2;
	dbl_dim = dim_1 * 2;
	tri_dim = dim_1 * 3;
	quad_dim = dim_1 * 4;
	penta_dim = dim_1 * 5;
	dec_dim = dim_1 * 10;
	fifteen_dim = dim_1 * 15;
	inv_dec_dim = 1.f / dec_dim;

	board_dim = quad_dim + 1;
	board_size = (ushort) board_dim * board_dim;

	mini_board_dim = dbl_dim + 3;
	mini_board_size = mini_board_dim * mini_board_dim;
	
	seats[0] = Orient::north_west;

	if (party == 3) {
		seats[1] = Orient::east;
		seats[2] = Orient::south;
	} else {
		seats[1] = Orient::south_east;
	}
}

void _init_valid_coords_() {
	using namespace global;
		
	valid_pts_count = 0;

	for (char x = 0; x <= tri_dim; ++x) {
		for (char y = 0; y <= tri_dim - x; ++y) {
			char ox = dbl_dim - x;
			char oy = dbl_dim - y;

			bool exists = false;
			bool o_exists = false;

			for (uchar i = 0; i < valid_pts_count; ++i) {
				Coord* vc = &valid_coords[i];

				if (vc->x == x && vc->y == y) exists = true;
				if (vc->x == ox && vc->y == oy) o_exists = true;
			}
			if (x == ox && y == oy) o_exists = true;
			if (!exists) set_coord(&valid_coords[valid_pts_count++], x, y);
			if (!o_exists) set_coord(&valid_coords[valid_pts_count++], ox, oy);
		}
	}
}

void _init_basic_boards_() {
	using namespace global;

	memset(vacant_board, -2, MAX_PT);
	piece_count = 0;

	for (char x = 0; x < dim; ++x) {
		for (char y = 0; y < dim - x; ++y) {
			set_coord(&home_coords[piece_count++], x, y);
		}
	}
	for (uchar i = 0; i < valid_pts_count; ++i) {
		Coord* vc = &valid_coords[i];
		ushort pt = coord_to_pt(Orient::north_west, vc->x, vc->y);
		vacant_board[pt] = -1;
		valid_pts[i] = pt;
	}
	memset(prison_board, 0, MAX_PT);
	memcpy(boot_board, vacant_board, MAX_PT);

	for (uchar t = 0; t < party; ++t) {
		for (uchar i = 0; i < piece_count; ++i) {
			Coord* hc = &home_coords[i];
			ushort pt = coord_to_pt(seats[t], hc->x, hc->y);

			boot_board[pt] = (char) t;
			home_pts[t][i] = pt;

			if (hc->x + hc->y < dim_2) prison_board[pt] = 1;
		}
	}
}

void _init_value_board_() {
	using namespace global;

	memset(value_board, 0, MAX_PARTY * MAX_PT);

	for (uchar i = 0; i < valid_pts_count; ++i) {
		Coord* vc = &valid_coords[i];
		char x = vc->x;
		char y = vc->y;
		char v = x + y;

		char min_xy = MIN(x, y);
		char max_xy = MAX(x, y);

		if (min_xy < 0) {
			v += min_xy;
		} else if (max_xy > dbl_dim) {
			v += dbl_dim - max_xy;
		}
		for (uchar t = 0; t < party; ++t) {
			ushort pt = coord_to_pt(seats[t], x, y);
			value_board[t][pt] = v;
		}
	}
	init_value = win_value = 0;

	for (uchar i = 0; i < piece_count; ++i) {
		Coord* hc = &home_coords[i];
		ushort pt = coord_to_pt(Orient::north_west, hc->x, hc->y);
		init_value += value_board[0][pt];

		pt = coord_to_pt(Orient::south_east, hc->x, hc->y);
		win_value += value_board[0][pt];
	}
}

void _init_walks_() {
	using namespace global;

	char dx[MAX_ADJACENT] = {1, 0, -1, 1, 0, -1};
	char dy[MAX_ADJACENT] = {0, 1, 0, -1, -1, 1};

	for (uchar i = 0; i < valid_pts_count; ++i) {
		ushort pt = valid_pts[i];
		uchar wcount = 0;

		uchar x = PX(pt);
		uchar y = PY(pt);		

		for (uchar j = 0; j < MAX_ADJACENT; ++j) {
			char wx = x + dx[j];
			char wy = y + dy[j];

			if (!is_in_board(wx, wy)) continue;
			ushort wpt = PT(wx, wy);

			if (vacant_board[wpt] == -2) continue;
			walks[pt][wcount++] = wpt;
		}
		walks_count[pt] = wcount;
	}
}

void _init_good_walks_() {
	using namespace global;

	char good_dx[4] = {0, 1, 1, -1};
	char good_dy[4] = {1, 0, -1, 1};

	for (uchar i = 0; i < valid_pts_count; ++i) {
		Coord* vc = &valid_coords[i];
		ushort pt = coord_to_pt(Orient::north_west, vc->x, vc->y);
		uchar x = PX(pt);
		uchar y = PY(pt);

		vector<ushort> gw_vec;

		for (uchar j = 0; j < 4; ++j) {
			char gwx = x + good_dx[j];
			char gwy = y + good_dy[j];

			if (only_mini_board) {
				if (!is_in_mini_board(gwx, gwy)) continue;
			} else if (!is_in_board(gwx, gwy)) {
				continue;
			}
			ushort gwpt = PT(gwx, gwy);

			if (vacant_board[gwpt] == -2) continue;
			gw_vec.push_back(gwpt);
		}
		for (uchar t = 0; t < party; ++t) {
			ushort tpt = coord_to_pt(seats[t], vc->x, vc->y);
			uchar gwcount = (uchar) gw_vec.size();
			good_walks_count[t][tpt] = gwcount;

			for (uchar j = 0; j < gwcount; ++j) {
				ushort gwpt = rotate(gw_vec[j], t);
				good_walks[t][tpt][j] = gwpt;
			}
		}
	}
}

void _init_walk_hops_() {
	using namespace global;

	char dx[MAX_ADJACENT] = {1, 0, -1, 1, 0, -1};
	char dy[MAX_ADJACENT] = {0, 1, 0, -1, -1, 1};

	for (uchar i = 0; i < valid_pts_count; ++i) {
		ushort pt = valid_pts[i];
		uchar hcount = 0;

		uchar x = PX(pt);
		uchar y = PY(pt);

		for (uchar j = 0; j < MAX_ADJACENT; ++j) {
			char wx = x + dx[j]; char hx = x + 2 * dx[j];
			char wy = y + dy[j]; char hy = y + 2 * dy[j];

			if (!is_in_board(hx, hy)) continue;
			ushort hpt = PT(hx, hy);

			if (vacant_board[hpt] == -2) continue;
			if (only_mini_board && !is_in_mini_board(hx, hy)) continue;

			ushort wpt = PT(wx, wy);

			walk_hops[pt][hcount][0] = wpt;
			walk_hops[pt][hcount++][1] = hpt;
		}
		hops_count[pt] = hcount;
	}
}

void _init_rot_vmap_() {
	using namespace global;

	char vmap[] = {-2, -1, 1, 2, 0};

	if (party == 2) {
		memcpy(&rot_vmap[0][0], vmap, MAX_PARTY + 2);
		rot_vmap[0][3] = 0;
	} else {
		memcpy(&rot_vmap[0][0], vmap, MAX_PARTY + 2);
		memcpy(&rot_vmap[1][0], vmap, MAX_PARTY + 2);
		rot_vmap[1][2] = 2;
		rot_vmap[1][3] = 0;
		rot_vmap[1][4] = 1;
	}
}

void reset_hop_tags() {
	memset(global::hop_tags, 0, sizeof(uint) * MAX_PT);
	global::max_hop_tag = 0;
}

vector<ushort> get_hops_from_recursive(char* board, ushort pt, uchar trace) {
	vector<ushort> hops;
	ushort* walk_hops = &global::walk_hops[pt][0][0];

	for (uchar i = 0; i < global::hops_count[pt]; ++i, walk_hops += 2) {
		ushort wpt = *walk_hops;
		ushort hpt = walk_hops[1];

		if (board[wpt] < 0 || board[hpt] >= 0) continue;
		if (is_in_trace(hpt, trace)) continue;

		hops.push_back(hpt);
		global::trace_buff[trace] = pt;

		vector<ushort> conseq_hops = get_hops_from_recursive(board, hpt, trace + 1);
		hops.insert(hops.end(), conseq_hops.begin(), conseq_hops.end());
	}
	return hops;
}

bool is_independent(char* board, uchar turn) {
	rotate(global::board_buff, board, global::party - turn);
	uchar min_xy = 0;

	for (ushort* valid_pt = global::valid_pts; valid_pt < &global::valid_pts[global::valid_pts_count]; ++valid_pt) {
		ushort pt = *valid_pt;

		if (global::board_buff[pt] == 0) {
			uchar xy = PX(pt) + PY(pt);
			if (xy < min_xy) min_xy = xy;
		}
	}
	--min_xy;
	bool ok = true;

	for (ushort* valid_pt = global::valid_pts; valid_pt < &global::valid_pts[global::valid_pts_count]; ++valid_pt) {
		ushort pt = *valid_pt;
		uchar xy = PX(pt) + PY(pt);

		if (xy >= min_xy && global::board_buff[pt] > 0) {
			ok = false;
			break;
		}
	}
	return ok;
}

bool is_walk(uint act) {
	ushort spt = ASPT(act);
	ushort ept = AEPT(act);
	ushort* walks = &global::walks[spt][0];

	for (uchar i = 0; i < global::walks_count[spt]; ++i) {
		if (walks[i] == ept) return true;
	}
	return false;
}

vector<ushort> find_path(char* board, uint act) {
	if (act == 0) {
		vector<ushort> empty_path;
		return empty_path;
	}
	ushort spt = ASPT(act);
	ushort ept = AEPT(act);
	vector<ushort> walks = get_walks_from(board, 0, spt);

	for (vector<ushort>::iterator wpt = walks.begin(); wpt != walks.end(); ++wpt) {
		if (ept == *wpt) {
			vector<ushort> path;
			path.push_back(spt);
			path.push_back(ept);
			return path;
		}
	}
	return find_hop_path_recursive(board, spt, ept);
}

vector<ushort> find_hop_path_recursive(char* board, ushort spt, ushort ept, uchar trace) {
	vector<ushort> path;
	ushort* walk_hops = &global::walk_hops[spt][0][0];
	uchar min_len = MAX_VALID;

	for (uchar i = 0; i < global::hops_count[spt]; ++i, walk_hops += 2) {
		ushort wpt = *walk_hops;
		ushort hpt = walk_hops[1];

		if (board[wpt] < 0 || board[hpt] >= 0) continue;
		if (is_in_trace(hpt, trace)) continue;

		if (hpt == ept) {
			path.clear();
			path.push_back(spt);
			path.push_back(hpt);
			return path;
		}
		global::trace_buff[trace] = spt;
		vector<ushort> conseq_hops = find_hop_path_recursive(board, hpt, ept, trace + 1);

		if (!conseq_hops.empty() && conseq_hops.size() < min_len) {
			min_len = (uchar) conseq_hops.size();
			path = conseq_hops;
			path.insert(path.begin(), spt);
		}
	}
	return path;
}

bool is_legal(char* board, uchar turn, uint act) {
	vector<uint> legals = get_legals(board, turn);

	for (vector<uint>::iterator l = legals.begin(); l != legals.end(); ++l) {
		if (*l == act) return true;
	}
	return false;
}

vector<uint> get_edges() {
	vector<uint> edges;

	for (ushort* pt1 = global::valid_pts; pt1 < global::valid_pts + global::valid_pts_count; ++pt1) {
		for (ushort* pt2 = &global::walks[*pt1][0]; pt2 < &global::walks[*pt1][global::walks_count[*pt1]]; ++pt2) {
			if (*pt1 < *pt2) edges.push_back(ACT(*pt1, *pt2));
		}
	}
	return edges;
}

void get_random_board(char* board, bool in_mini) {
	memcpy(board, global::vacant_board, global::board_size);
	vector<ushort> pts;

	for (uchar i = 0; i < global::valid_pts_count; ++i) {
		ushort vpt = global::valid_pts[i];
		char x = PX(vpt);
		char y = PY(vpt);

		if (!in_mini || is_in_mini_board(x, y)) pts.push_back(vpt);
	}
	for (uchar i = 0; i < global::piece_count; ++i) {
		for (uchar t = 0; t < global::party; ++t) {
			int pos = rand() % pts.size();
			board[pts[pos]] = t;
			pts.erase(pts.begin() + pos);
		}
	}
}

#ifdef _WIN32
void print_board(char* board, bool is_scalar, uchar indent) {
	printf_s("\n");

	for (uchar x = 0; x < global::board_dim; ++x) {
		for (uchar i = 0; i < indent; ++i) {
			printf_s("\t");
		}
		for (uchar y = 0; y < global::board_dim; ++y) {
			ushort pt = PT(x, y);
			char v = board[pt];

			if (!is_scalar) {
				switch (v) {
				case -2:
					printf_s("  ");
					break;
				case -1:
					printf_s("O ");
					break;
				default:
					printf_s("%c ", 'A' + v);
					break;
				}
			} else {
				char tmp[8];
				sprintf_s(tmp, 8, "%d", v);				
				
				string str = string(tmp);
				uchar sl = (uchar) (3 - str.length());

				for (uchar i = 0; i < sl; ++i) {
					str = ' ' + str;
				}
				str += ' ';
				printf_s(str.c_str());
			}
		}
		printf_s("\n");
	}	
}

void interrupt_console(const char* msg) {
	if (msg) printf_s(msg);
	char ch;
	scanf_s("%c", &ch, 1);
}
#endif

}
