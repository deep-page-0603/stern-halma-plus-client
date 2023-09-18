
#include "players.h"
#include "kutil.h"

namespace koni {

Player::Player(Player::ThinkMode mode) {
	this->mode = mode;
}

template<typename T> T Player::inner_think(vector<T>* legals, float* pi, ushort depth) {
	if (legals->empty()) return 0;
	uchar len = (uchar) legals->size();

	float running_pi[MAX_LEGALS];
	memcpy(running_pi, pi, 4 * len);

	if (mode == ThinkMode::train) {
		if (depth > WARM_DEPTH) scalar_mul(running_pi, len, INV_COLD_TEMP);
		
		softmax(running_pi, len);
		scalar_mul(running_pi, len, 1 - TRAIN_PI_NOISE);

		uchar noise_pos = (uchar) (rand() % len);
		running_pi[noise_pos] += TRAIN_PI_NOISE;
	} else {
		scalar_mul(running_pi, len, INV_COLD_TEMP);
		softmax(running_pi, len);
	}
	uchar idx = random_choice(running_pi, len);
	return legals->at(idx);
}

uint RandomPlayer::think(vector<uint>* legals, char* board, uchar turn, ushort depth) {
	*legals = get_legals(board, turn, true);
	
	float pi[MAX_LEGALS];
	memset(pi, 0, MAX_LEGALS * 4);

	return inner_think(legals, pi, depth);
}

MCTSPlayer::MCTSPlayer(ThinkMode mode, uint num_playouts, float p_uct, Net* net) : Player(mode) {
	mcts = new MCTS(num_playouts, p_uct, net);
	this->num_playouts = num_playouts;
}

MCTSPlayer::~MCTSPlayer() {
	if (mcts) delete mcts;
}

uint MCTSPlayer::think(vector<uint>* legals, char* board, uchar turn, ushort depth) {
	MCTS::Node* root = mcts->run(board, turn, depth);

	legal_spts.clear();
	legal_epts.clear();

	uchar i = 0;
	legals->clear();

	BEGIN_FOR_CHILD(root, child)
		ushort spt = child->pt;
		uint visits = child->visits;

		legal_spts.push_back(spt);
		pi_1[i++] = logf((float) visits + 1e-10f);

		BEGIN_FOR_CHILD(child, grand_child)
			legals->push_back(ACT(spt, grand_child->pt));
		END_FOR
	END_FOR

	selected_spt = inner_think(&legal_spts, pi_1, depth);
	
	BEGIN_FOR_CHILD(root, child)
		if (child->pt == selected_spt) {
			mcts->run(board, turn, depth, child);
			i = 0;

			BEGIN_FOR_CHILD(child, grand_child)
				legal_epts.push_back(grand_child->pt);
				pi_2[i++] = logf((float) grand_child->visits + 1e-10f);
			END_FOR

			selected_ept = inner_think(&legal_epts, pi_2, depth);
			return ACT(selected_spt, selected_ept);
		}
	END_FOR
	return 0;
}

void MCTSPlayer::get_think_data(uchar* spt, char* ept_board, float* mini_pi_1, float* mini_pi_2) {
	memset(mini_pi_1, 0, 4 * global::mini_board_size);
	memset(mini_pi_2, 0, 4 * global::mini_board_size);

	softmax(pi_1, (uchar) legal_spts.size());
	softmax(pi_2, (uchar) legal_epts.size());

	uchar i = 0;
	*spt = get_mini_pt(selected_spt);

	for (vector<ushort>::iterator spt = legal_spts.begin(); spt != legal_spts.end(); ++spt) {
		ushort mini_spt = get_mini_pt(*spt);
		mini_pi_1[mini_spt] = pi_1[i++];
	}
	memset(ept_board, 0, global::mini_board_size);
	i = 0;

	for (vector<ushort>::iterator ept = legal_epts.begin(); ept != legal_epts.end(); ++ept) {
		ushort mini_ept = get_mini_pt(*ept);
		mini_pi_2[mini_ept] = pi_2[i++];
		ept_board[mini_ept] = 1;
	}
}

}
