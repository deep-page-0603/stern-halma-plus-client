
#include "net.h"
#include "dajdense.h"
#include "dajconv.h"
#include "dajnorm.h"
#include "dajfunc.h"
#include "dajutil.h"

namespace koni {

Net::Net(ByteStream* stream) : Model(stream) {
	char vb[MAX_MINI_PT];
	
	get_mini_board(vb, global::vacant_board);
	vacant_board = new FTensor(global::mini_board_size, END_DIM);

	for (uchar i = 0; i < global::mini_board_size; ++i) {
		vacant_board->val[i] = (float) (vb[i] + 2);
	}
}

Net::~Net() {
	if (vacant_board) delete vacant_board;
}

/*
	@param states: shape=(b, 8, md, md)
*/
void Net::run(FTensor* states, FTensor** pi, FTensor** v) {
	uint batch_size = states->shape[0];

	FTensor* init_out = conv(
		states,
		get_f(p2_d5_init_conv_layer_weight),
		get_f(p2_d5_init_conv_layer_bias),
		get_f(p2_d5_init_conv_bn_weight),
		get_f(p2_d5_init_conv_bn_bias),
		true
	);
	FTensor* res_input = init_out;

	for (uint i = 0; i < p2_d5_tower_height; ++i) {
		FTensor* res_out = residual(
			res_input,
			get_f(p2_d5_tower_0_conv1_layer_weight + 4 * i),
			get_f(p2_d5_tower_0_conv1_layer_bias + 4 * i),
			get_f(p2_d5_tower_0_conv1_bn_weight + 4 * i),
			get_f(p2_d5_tower_0_conv1_bn_bias + 4 * i),
			get_f(p2_d5_tower_0_conv2_layer_weight + 4 * i),
			get_f(p2_d5_tower_0_conv2_layer_bias + 4 * i),
			get_f(p2_d5_tower_0_conv2_bn_weight + 4 * i),
			get_f(p2_d5_tower_0_conv2_bn_bias + 4 * i)
		);
		delete res_input;
		res_input = res_out;
	}
	FTensor* tower_out = res_input;

	FTensor* v_conv_out = conv(
		tower_out,
		get_f(p2_d5_v_head_conv_layer_weight),
		get_f(p2_d5_v_head_conv_layer_bias),
		get_f(p2_d5_v_head_conv_bn_weight),
		get_f(p2_d5_v_head_conv_bn_bias),
		true
	);
	v_conv_out->reshape(batch_size, v_conv_out->span / batch_size, END_DIM);
	
	FTensor* v_hidden = dense::dense(
		v_conv_out,
		get_f(p2_d5_v_hidden_fc_layer_weight),
		get_f(p2_d5_v_hidden_fc_layer_bias)
	);	
	delete v_conv_out;
	func::relu(v_hidden);
	
	FTensor* v_out = dense::dense(
		v_hidden,
		get_f(p2_d5_v_fc_layer_weight),
		get_f(p2_d5_v_fc_layer_bias)
	);
	delete v_hidden;
	func::tanh(v_out);

	v_out->reshape(batch_size, END_DIM);
	*v = v_out;

	FTensor* pi_conv_out = conv(
		tower_out,
		get_f(p2_d5_pi_head_conv_layer_weight),
		get_f(p2_d5_pi_head_conv_layer_bias),
		get_f(p2_d5_pi_head_conv_bn_weight),
		get_f(p2_d5_pi_head_conv_bn_bias),
		true
	);
	delete tower_out;
	pi_conv_out->reshape(batch_size, pi_conv_out->span / batch_size, END_DIM);

	FTensor* pi_hidden = dense::dense(
		pi_conv_out,
		get_f(p2_d5_pi_hidden_fc_layer_weight),
		get_f(p2_d5_pi_hidden_fc_layer_bias)
	);
	delete pi_conv_out;
	func::relu(pi_hidden);

	FTensor* pi_out = dense::dense(
		pi_hidden,
		get_f(p2_d5_pi_fc_layer_weight),
		get_f(p2_d5_pi_fc_layer_bias)
	);
	delete pi_hidden;
	*pi = pi_out;
}

FTensor* Net::conv(FTensor* input, FTensor* weight, FTensor* bias, FTensor* gamma, FTensor* beta, bool apply_relu) {
	FTensor* out = conv::conv2d(input, weight, bias);	
	norm::batch_norm_with_precomputed(out, gamma, beta);
	if (apply_relu) func::relu(out);
	return out;
}

FTensor* Net::residual(FTensor* input, FTensor* weight1, FTensor* bias1, FTensor* gamma1, FTensor* beta1,
		FTensor* weight2, FTensor* bias2, FTensor* gamma2, FTensor* beta2) {
	FTensor* out_1 = conv(input, weight1, bias1, gamma1, beta1, true);
	FTensor* out_2 = conv(out_1, weight2, bias2, gamma2, beta2, false);
	delete out_1;

	func::add(out_2, input);
	func::relu(out_2);
	return out_2;
}

void Net::board_to_state(float* state, char* board, ushort spt, vector<ushort>* epts) {
	char mini_board[MAX_MINI_PT];
	get_mini_board(mini_board, board);	
	memset(state, 0, 4 * p2_d5_input_channels * global::mini_board_size);

	float* channel_0 = state;
	float* channel_1 = &state[global::mini_board_size];

	for (uchar pt = 0; pt < global::mini_board_size; ++pt) {
		char v = mini_board[pt];

		if (v == 0) {
			channel_0[pt] = 1.f;
		} else if (v == 1) {
			channel_1[pt] = 1.f;
		}		
	}
	if (spt != 0xFFFF) {
		state[2 * global::mini_board_size + get_mini_pt(spt)] = 1.f;
		channel_0 = &state[3 * global::mini_board_size];

		for (vector<ushort>::iterator pt = epts->begin(); pt != epts->end(); ++pt) {
			channel_0[get_mini_pt(*pt)] = 1.f;
		}
		channel_1 = &state[4 * global::mini_board_size];

		for (uchar pt = 0; pt < global::mini_board_size; ++pt) {
			channel_1[pt] = 1.f;
		}
	}
	channel_0 = &state[5 * global::mini_board_size];
	channel_1 = &state[6 * global::mini_board_size];

	memcpy(channel_0, vacant_board->val, 4 * global::mini_board_size);
	memcpy(channel_1, get_f(p2_d5_embedding)->val, 4 * 2 * global::mini_board_size);
}

}
