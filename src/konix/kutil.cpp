
#include "kutil.h"

namespace koni {

float randf_unit = 1.f / RAND_MAX;

FILE* safe_fopen(const char* path, const char* mode) {
	FILE* file = nullptr;
#ifdef _WIN32
	::fopen_s(&file, path, mode);
#else
	file = fopen(path, mode);
#endif
	return file;
}

string get_turn_name(uchar turn) {
	if (turn == 0) return string("A");
	if (turn == 1) return string("B");
	if (turn == 2) return string("C");
	return "O";
}

}
