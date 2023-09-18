
#pragma once

#include "koni.h"
#include "kutil.h"
#include "net.h"

namespace koni {

#define BEGIN_FOR_CHILD(node, it) for (koni::MCTS::Node* it = node->child; it < node->child + node->child_len; ++it) {
#define END_FOR }

class MCTS {
public:
	enum NodeType : char {
		nt_none = 0, nt_spt, nt_ept
	};

	struct Node {
		NodeType type;
		ushort pt;
		uint visits;
		Node* child;
		uchar child_len;
		float p;
		float q;
		float leaf_q[MAX_PARTY];
		char terminated;
		char touched;
	};

private:
	uint num_playouts;
	float p_uct;
	Node* node_buff;
	uint node_buff_pos;
	uint node_buff_len;
	Node* trace[MAX_GO_LEN * MAX_PARTY];
	ushort trace_len;
	Node* root;
	Net* net;

public:
	MCTS(uint num_playouts, float p_uct, Net* net = nullptr);
	~MCTS();

	Node* run(char* board, uchar turn, ushort depth, Node* spt_root = nullptr);
	Node* get_root();

	string get_info(Node* node);
	string get_full_info(Node* node);

	inline Node* create_node(ushort pt = 0, float p = 0, NodeType type = NodeType::nt_none) {
		Node* node = &node_buff[node_buff_pos++];
		memset(node, 0, sizeof(Node));
		node->pt = pt;
		node->p = p;
		node->type = type;
		return node;
	}

	inline float get_uct(Node* node, float parent_factor) {
		return node->q + parent_factor * node->p / (1 + node->visits);
	}

	inline Node* select_child_to_search(Node* node) {
		float parent_factor = p_uct * node->child_len * sqrtf((float) node->visits);
		float max_uct = -1e10f;
		vector<Node*> candidates;

		BEGIN_FOR_CHILD(node, child)
			float uct = get_uct(child, parent_factor);
			
			if (uct > max_uct) {
				max_uct = uct;
				candidates.clear();
			}
			if (uct == max_uct) candidates.push_back(child);
		END_FOR

		Node* selected = candidates[rand() % candidates.size()];
		trace[trace_len++] = selected;
		return selected;
	}

	inline void update_node(Node* node, float leaf_q) {
		node->q += (leaf_q - node->q) / (++node->visits);
	}

	inline set<ushort> get_spts(vector<uint>* legals) {
		set<ushort> spts;

		for (vector<uint>::iterator act = legals->begin(); act != legals->end(); ++act) {
			spts.insert(ASPT(*act));
		}
		return spts;
	}

	inline vector<ushort> get_epts(vector<uint>* legals, ushort spt) {
		vector<ushort> epts;

		for (vector<uint>::iterator act = legals->begin(); act != legals->end(); ++act) {
			if (ASPT(*act) == spt) epts.push_back(AEPT(*act));
		}
		return epts;
	}

	inline void expand_node_deep(Node* node, vector<uint>* legals, char* board, uchar turn) {
		set<ushort> spts = get_spts(legals);
		node->child_len = (uchar) spts.size();
		node->child = &node_buff[node_buff_pos];

		FTensor* states = new FTensor(nullptr, false, node->child_len + 1, p2_d5_input_channels,
			global::mini_board_dim, global::mini_board_dim, END_DIM);
		
		int state_len = p2_d5_input_channels * global::mini_board_size;		
		int i = 0;

		vector<uchar> mini_spts;
		vector<vector<ushort>> all_epts;
		vector<vector<uchar>> all_mini_epts;

		if (turn == 0) {
			for (set<ushort>::iterator spt = spts.begin(); spt != spts.end(); ++spt) {
				vector<ushort> epts = get_epts(legals, *spt);
				all_epts.push_back(epts);

				mini_spts.push_back(get_mini_pt(*spt));
				vector<uchar> mini_epts;

				for (vector<ushort>::iterator ept = epts.begin(); ept != epts.end(); ++ept) {
					mini_epts.push_back(get_mini_pt(*ept));
				}
				all_mini_epts.push_back(mini_epts);
				net->board_to_state(&states->val[(i++) * state_len], board, *spt, &epts);
			}
			net->board_to_state(&states->val[i * state_len], board);
		} else {
			char temp_board[MAX_PT];
			rotate(temp_board, board);			
			
			for (set<ushort>::iterator spt = spts.begin(); spt != spts.end(); ++spt) {
				vector<ushort> epts = get_epts(legals, *spt);
				all_epts.push_back(epts);

				ushort rot_spt = rotate(*spt);
				mini_spts.push_back(get_mini_pt(rot_spt));

				vector<uchar> mini_epts;
				vector<ushort> temp_epts;

				for (vector<ushort>::iterator ept = epts.begin(); ept != epts.end(); ++ept) {
					ushort rot_ept = rotate(*ept);
					mini_epts.push_back(get_mini_pt(rot_ept));
					temp_epts.push_back(rot_ept);
				}
				all_mini_epts.push_back(mini_epts);
				net->board_to_state(&states->val[(i++) * state_len], temp_board, rot_spt, &temp_epts);
			}
			net->board_to_state(&states->val[i * state_len], temp_board);
		}
		FTensor *batch_pi, *batch_v;
		
		net->run(states, &batch_pi, &batch_v);		
		delete states;
		float v_0 = batch_v->val[i];
		
		if (turn == 0) {
			node->leaf_q[0] = -v_0;
			node->leaf_q[1] = v_0;
		} else {
			node->leaf_q[0] = v_0;
			node->leaf_q[1] = -v_0;
		}
		float* pi_0 = new float[node->child_len];
		float* bp = &batch_pi->val[i * global::mini_board_size];
		i = 0;

		for (vector<uchar>::iterator spt = mini_spts.begin(); spt != mini_spts.end(); ++spt) {
			pi_0[i++] = bp[*spt];
		}
		softmax(pi_0, i);
		i = 0;

		for (set<ushort>::iterator spt = spts.begin(); spt != spts.end(); ++spt) {
			create_node(*spt, pi_0[i++], NodeType::nt_spt);
		}
		delete[] pi_0;
		i = 0;

		BEGIN_FOR_CHILD(node, child)
			vector<ushort> epts = all_epts[i];
			vector<uchar> mini_epts = all_mini_epts[i];
			
			child->child_len = (uchar) epts.size();
			child->child = &node_buff[node_buff_pos];

			float v = batch_v->val[i];

			if (turn == 0) {
				child->leaf_q[0] = -v;
				child->leaf_q[1] = v;
			} else {
				child->leaf_q[0] = v;
				child->leaf_q[1] = -v;
			}
			float* pi = new float[epts.size()];						
			bp = &batch_pi->val[i * global::mini_board_size];
			int j = 0;

			for (vector<uchar>::iterator ept = mini_epts.begin(); ept != mini_epts.end(); ++ept) {
				pi[j++] = bp[*ept];
			}
			softmax(pi, j);
			j = 0;

			for (vector<ushort>::iterator ept = epts.begin(); ept != epts.end(); ++ept) {
				create_node(*ept, pi[j++], NodeType::nt_ept);
			}
			delete[] pi;
			++i;
		END_FOR

		delete batch_pi;
		delete batch_v;
	}

	inline void expand_node_zero(Node* node, vector<uint>* legals) {
		set<ushort> spts = get_spts(legals);
		node->child_len = (uchar) spts.size();
		
		float sp = 1.f / node->child_len;
		node->child = &node_buff[node_buff_pos];

		for (set<ushort>::iterator spt = spts.begin(); spt != spts.end(); ++spt) {
			create_node(*spt, sp, NodeType::nt_spt);
		}
		BEGIN_FOR_CHILD(node, child)
			ushort spt = child->pt;
			vector<ushort> epts = get_epts(legals, spt);
			child->child_len = (uchar) epts.size();

			float ep = 1.f / child->child_len;
			child->child = &node_buff[node_buff_pos];

			for (vector<ushort>::iterator ept = epts.begin(); ept != epts.end(); ++ept) {
				create_node(*ept, ep, NodeType::nt_ept);
			}
		END_FOR
	}

	inline void playout(char* board, short* values, uchar turn, ushort depth) {
		Node* node = root;
		ushort i = (node->type == NodeType::nt_spt) ? 1 : 0;
		ushort spt = node->pt;

		trace_len = 0;
		trace[trace_len++] = node;

		while (node->child && node->touched) {
			node = select_child_to_search(node);
			++i;

			if (i % 2 == 0) {
				uint act = ACT(spt, node->pt);
				go(board, board, act, values, turn);

				++depth;
				turn = (turn + 1) % global::party;
			} else {
				spt = node->pt;
			}
		}
		float leaf_q[MAX_PARTY];
		bool is_update = true;

		if (!node->terminated) {
			if (node->type != NodeType::nt_spt) {
				vector<uint> legals = get_legals(board, turn, true);
				char rewards[MAX_PARTY];
				Exit exit = check_end(rewards, board, values, depth, legals.empty(), true);

				if (exit == Exit::none) {
					if (!net) {
						get_scores(leaf_q, values, true);
						expand_node_zero(node, &legals);
					} else {
						expand_node_deep(node, &legals, board, turn);
						memcpy(leaf_q, node->leaf_q, 4 * global::party);
					}
				} else {
					for (uchar t = 0; t < global::party; ++t) {
						leaf_q[t] = node->leaf_q[t] = (float) rewards[t];
					}
					node->terminated = 1;
				}
			} else {
				if (net) {
					memcpy(leaf_q, node->leaf_q, 4 * global::party);
				} else {
					is_update = false;
				}
			}
			node->touched = 1;
		} else {
			memcpy(leaf_q, node->leaf_q, 4 * global::party);
		}
		if (is_update) {
			for (short i = (short) trace_len - 1; i >= 0; i -= 2) {
				turn = (global::party + turn - 1) % global::party;
				float q = leaf_q[turn];

				update_node(trace[i], q);
				if (i > 0) update_node(trace[i - 1], q);
			}
		} else {
			for (short i = (short) trace_len - 1; i >= 0; --i) {
				++trace[i]->visits;
			}
		}
	}
};

}
