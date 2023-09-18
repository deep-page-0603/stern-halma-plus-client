
#include "global.h"

Config config;
MCXMLManager* gStr = nullptr;
char thinkConfig[2][64];

void initialize() {
	strcpy(_appSetting.appName, "Koni");

	_appSetting.productType = product_debug;
	_appSetting.screenLayoutType = screen_layout_desktop;
#ifdef WIDE_SCREEN
	_appSetting.screenWidth = 1280.f;
	_appSetting.windowWidth = 672.f;
#else
	_appSetting.screenWidth = 720.f;
	_appSetting.windowWidth = 378.f;
#endif
	_appSetting.screenHeight = 1280.f;	
	_appSetting.windowHeight = 672.f;
	_appSetting.fullScreen = false;
	_appSetting.updateFrequency = 50.f;
	_appSetting.randomSeed = -1;
	_appSetting.replaceSceneDuration = 0.3f;
	_appSetting.loadingSceneDuration = 1.f;
	_appSetting.commPort = 1989;
	_appSetting.packetHeader = 0xE3DC067F;
	_appSetting.packetFooter = 0x5F4D9C2A;
	_appSetting.uiAssist = false;
	_appSetting.maxPlayerCount = 3;
	
	strcpy(_appSetting.defaultFont, DOTUM);
	initMocos2d();
}

void finalize() {
	if (gStr) {
		delete gStr;
		gStr = nullptr;
	}
}

void playSound(string path) {
	if (config.soundVolume == 0) return;
	if (androidVer < 28) {
		AudioEngine::play2d(path, false, config.soundVolume);
	} else {
		SimpleAudioEngine::getInstance()->playEffect(path.c_str(), false, 1.f, 0, config.soundVolume);
	}
}

string getOnlineName() {
	if (strcmp(config.name, "") != 0) return string(config.name);
	return MCOnline::getOnlineName();
}

string createGStr(const char* gStrKey, ...) {
	__String* str = __String::create("");
	va_list ap;
	va_start(ap, gStrKey);
	str->initWithFormatAndValist(gStr->getStr(gStrKey).c_str(), ap);
	va_end(ap);
	return str->getCString();
}

string getGStr(const char* format, ...) {
	__String* str = __String::create("");
	va_list ap;
	va_start(ap, format);
	str->initWithFormatAndValist(format, ap);
	va_end(ap);
	return gStr->getStr(str->getCString());
}

string safeDivStr(int a, int b) {
	if (b == 0) return string("0");
	if (a % b == 0) return MCUtil::numToStr(a / b);
	return MCUtil::createStr("%.1f", 1.f * a / b);
}

void saveConfig() {
	FILE* f = fopen(MCUtil::getWritablePath("global.config").c_str(), "wb");
	fwrite(&config, sizeof(Config), 1, f);
	fclose(f);
}

void loadConfig() {
	FILE* f = fopen(MCUtil::getWritablePath("global.config").c_str(), "rb");
	fread(&config, sizeof(Config), 1, f);
	fclose(f);
}

void initConfig() {
	if (FileUtils::getInstance()->isFileExist(MCUtil::getWritablePath("global.config"))) {
		loadConfig();
	} else {
		config.stock = 500;
		config.musicVolume = 1.f;
		config.soundVolume = 1.f;
		config.speed = 0.5f;
		config.effect = true;
		strcpy(config.name, "");
		saveConfig();
	}
#if CC_TARGET_PLATFORM == CC_PLATFORM_WIN32
	initThinkConfig();
#else
	strcpy(&thinkConfig[0][0], "human");
	strcpy(&thinkConfig[1][0], "local");
#endif
}

uchar getMaxUndoCount(uchar aiLevel) {
	if (aiLevel >= 4) return 0;
	if (aiLevel == 3) return 1;
	if (aiLevel == 2) return 3;
	if (aiLevel == 1) return 5;
	return 10;
}

void initThinkConfig() {
	string content = FileUtils::getInstance()->getStringFromFile("think.config");
	vector<string> segs = MCUtil::split(content, "\n");
	
	segs[0] = MCUtil::split(segs[0], "\r")[0];
	segs[1] = MCUtil::split(segs[1], "\r")[0];

	strcpy(&thinkConfig[0][0], segs[0].c_str());
	strcpy(&thinkConfig[1][0], segs[1].c_str());	
}
