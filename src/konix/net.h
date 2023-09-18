
#pragma once

#include "koni.h"
#include "dajmodel.h"
#include "dajtensor.h"
#include "koni_p2_d5.h"

using namespace dajnn;

namespace koni {

class Net : public Model {
public:
	Net(ByteStream* stream);
	virtual ~Net();

public:
	void run(FTensor* states, FTensor** pi, FTensor** v);
	void board_to_state(float* state, char* board, ushort spt = 0xFFFF, vector<ushort>* epts = nullptr);

protected:
	FTensor* conv(FTensor* input, FTensor* weight, FTensor* bias, FTensor* gamma, FTensor* beta, bool apply_relu);
	FTensor* residual(FTensor* input, FTensor* weight1, FTensor* bias1, FTensor* gamma1, FTensor* beta1,
		FTensor* weight2, FTensor* bias2, FTensor* gamma2, FTensor* beta2);

protected:
	FTensor* vacant_board;
};

}
