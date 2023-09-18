
#include "mcserializer.h"
#include "mcutil.h"

namespace mocos2d {

void MCSerializer::saveFile(string path) {
	uchar* buffer = save();
	FILE* f = fopen(MCUtil::getWritablePath(path).c_str(), "wb");
	fwrite(buffer, getSize(), 1, f);
	fclose(f);
	free(buffer);
}

void MCSerializer::loadFile(string path) {
	uint size = getSize();
	uchar* buffer = (uchar*) malloc(size);
	FILE* f = fopen(MCUtil::getWritablePath(path).c_str(), "rb");
	fread(buffer, size, 1, f);
	fclose(f);
	load(buffer);
	free(buffer);
}

}