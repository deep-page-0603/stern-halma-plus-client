
#include "mcts.h"

namespace koni {

MCTS::MCTS(uint num_playouts, float p_uct, Net* net) {
	this->num_playouts = num_playouts;
	this->p_uct = p_uct;
	this->net = net;
	
	node_buff_len = 2 * num_playouts * (global::piece_count + MAX_LEGALS);
	node_buff = (Node*) malloc(sizeof(Node) * node_buff_len);
	node_buff_pos = 0;	
	trace_len = 0;
}

MCTS::~MCTS() {
	if (node_buff) free(node_buff);
}

string MCTS::get_info(Node* node) {
	char info[255];

#ifdef _WIN32
	sprintf_s(info, 255,
#else
	sprintf(info,
#endif
	"pt=%d,visits=%d,p=%.3f,q=%.3f,childs=%d", node->pt, node->visits, node->p, node->q, node->child_len);
	return string(info);
}

string MCTS::get_full_info(Node* node) {
	string info;

	if (node->child) {
		BEGIN_FOR_CHILD(node, child)
			info += get_info(child);
			info += "\n";

			if (child->child) {
				BEGIN_FOR_CHILD(child, grand_child)
					info += "\t";
					info += get_info(grand_child);
					info += "\n";
				END_FOR
			}
		END_FOR
	}
	return info;
}

MCTS::Node* MCTS::run(char* board, uchar turn, ushort depth, Node* spt_root) {
	short values[MAX_PARTY];
	uint playouts = num_playouts;

	if (!spt_root) {
		node_buff_pos = 0;
		root = create_node();
	} else {
		root = spt_root;
		playouts -= root->visits;
	}
	for (uchar t = 0; t < global::party; ++t) {
		values[t] = get_value(board, t);
	}
	char running_board[MAX_PT];
	short running_values[MAX_PARTY];

	for (uint i = 0; i < playouts; ++i) {
		memcpy(running_board, board, global::board_size);
		memcpy(running_values, values, 2 * global::party);

		playout(running_board, running_values, turn, depth);
	}
	return root;
}

MCTS::Node* MCTS::get_root() {
	return root;
}

}
