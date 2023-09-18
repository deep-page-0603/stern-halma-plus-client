
#pragma once

#include "koni.h"

namespace koni {

enum VerboseLevel {
	vl_fastest, vl_estim, vl_full
};

extern float randf_unit;

FILE* safe_fopen(const char* path, const char* mode);
string get_turn_name(uchar turn);

inline float randf() {
	return rand()* randf_unit;
}

inline void scalar_mul(float* x, uchar len, float scalar) {
	for (float* v = x; v < x + len; ++v) {
		*v *= scalar;
	}
}

inline uchar random_choice(float* prob, uchar len) {
	float f = randf();

	for (uchar i = 0; i < len; ++i) {
		if (f <= prob[i]) return i;
		f -= prob[i];
	}
	return len - 1;
}

inline void softmax(float* x, uchar len) {
	float max_v = -1e10f;

	for (float* v = x; v < x + len; ++v) {
		if (max_v < *v) max_v = *v;
	}
	float sum_v = 0;

	for (float* v = x; v < x + len; ++v) {
		*v = expf(*v - max_v);
		sum_v += *v;
	}
	for (float* v = x; v < x + len; ++v) {
		*v /= sum_v;
	}
}

}
