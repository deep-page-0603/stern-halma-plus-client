
#include "mocos2d.h"
#include "mcutil.h"

namespace mocos2d {

AppSetting _appSetting;
int musicId;
int androidVer = -1;

void initMocos2d() {
	musicId = -1;

	if ((_appSetting.randomSeed != -1) && (_appSetting.productType == product_debug)) {
		srand(_appSetting.randomSeed);
	} else {
		srand((unsigned int) time(NULL));
	}
#if CC_TARGET_PLATFORM == CC_PLATFORM_WIN32
	char path[MAX_STR_LEN] = {0};
	GetCurrentDirectoryA(MAX_STR_LEN - 1, path);
	string storePath = MCUtil::createStr("%s/save", path);
	
	if (!FileUtils::getInstance()->isDirectoryExist(storePath)) {
		FileUtils::getInstance()->createDirectory(storePath);
	}
#endif
}

}
