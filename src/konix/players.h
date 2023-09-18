
#pragma once

#include "mcts.h"

namespace koni {

#define INV_COLD_TEMP 1000.f
#define TRAIN_PI_NOISE 0.25f
#define WARM_DEPTH 30

class Player {
public:
	enum ThinkMode {
		train, valid
	};

public:
	Player(ThinkMode mode);
	virtual uint think(vector<uint>* legals, char* board, uchar turn, ushort depth) = 0;
	
protected:
	template<typename T> T inner_think(vector<T>* legals, float* pi, ushort depth);

protected:
	ThinkMode mode;
};

class RandomPlayer : public Player {
public:
	RandomPlayer(ThinkMode mode) : Player(mode) {};

	virtual uint think(vector<uint>* legals, char* board, uchar turn, ushort depth) override;
};

class MCTSPlayer : public Player {
public:
	MCTSPlayer(ThinkMode mode, uint num_playouts, float p_uct, Net* net = nullptr);
	virtual ~MCTSPlayer();

	virtual uint think(vector<uint>* legals, char* board, uchar turn, ushort depth) override;
	void get_think_data(uchar* spt, char* ept_board, float* mini_pi_1, float* mini_pi_2);

protected:
	MCTS* mcts;
	uint num_playouts;

	ushort selected_spt;
	ushort selected_ept;

	vector<ushort> legal_spts;
	vector<ushort> legal_epts;

	float pi_1[MAX_PIECE_COUNT];
	float pi_2[MAX_LEGALS];
};

}
