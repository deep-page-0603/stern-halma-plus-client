
#pragma once

#include <stdio.h>
#include <vector>
#include <algorithm>

using namespace std;

namespace konix {

#ifndef uchar
#define uchar unsigned char
#endif
#ifndef ushort
#define ushort unsigned short
#endif
#ifndef uint
#define uint unsigned int
#endif

#ifndef MAX_PARTY
#define MAX_PARTY 3
#endif

#ifndef MAX_PT
#define MAX_PT 289
#endif

#define MAX_GO_PER_PLAYER 100

#define PT_INVALID -2
#define PT_VACANT -1
#define PT_0 0
#define PT_1 1
#define PT_2 2

bool init_konix(uchar party, uchar dim, const char* model_buff);
void finish_konix();

const char* get_boot_board();
const char* get_vacant_board();
const char* get_value_board(uchar turn);
void get_random_board(char* board, bool in_mini = false);

vector<ushort> get_valid_pts();
ushort get_board_size();
uchar get_mini_board_size();
uchar get_piece_count();

vector<uint> get_edges();
vector<uint> get_legals(char* board, uchar turn);
vector<ushort> get_legals_from(char* board, ushort pt);

void get_mini_board(char* dst_board, char* src_board);
void rotate(char* dst_board, char* src_board, uchar count = 1);
ushort rotate(ushort pt, uchar count = 1);

vector<ushort> go(char* board, uint act);
uint think(char* board, uchar turn, ushort depth, uchar ai_level);

bool is_end(char* board, uchar turn);
bool is_end_but_one(char* board, uchar turn);
bool is_knockout(char* board, uchar turn);
bool is_stocked(char* board, uchar turn);

ushort get_spt(uint act);
ushort get_ept(uint act);

uchar get_x(ushort pt);
uchar get_y(ushort pt);

ushort get_pt(uchar x, uchar y);
uint get_act(ushort spt, ushort ept);

}
