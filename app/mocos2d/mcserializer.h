
#pragma once

#include "mocos2d.h"

namespace mocos2d {

class MCSerializer {
public:
	virtual ~MCSerializer() {};

	virtual uint getSize() = 0;
	virtual uchar* save() = 0;
	virtual void load(uchar* buffer) = 0;

	void saveFile(string path);
	void loadFile(string path);
};

}