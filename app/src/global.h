
#pragma once

#include "clscene.h"
#include "mcjni.h"
#include "mcui.h"
#include "mcxmlmanager.h"

#if CC_TARGET_PLATFORM == CC_PLATFORM_WIN32
#define WIDE_SCREEN

void initThinkConfig();
#endif

using namespace mocos2d;
using namespace cardlib;

#define DOTUM "fonts/dotum.ttf"
#define SCREEN_CENTER Vec2(_appSetting.screenWidth / 2, _appSetting.screenHeight / 2)

#define TAG_SCENE_GAME 0
#define TAG_SCENE_SPLASH 1
#define TAG_SCENE_MAIN 2
#define TAG_SCENE_SHOP 3

struct Config {
	char name[MAX_PLAYER_NAME];
	int stock;
	float musicVolume;
	float soundVolume;
	float speed;
	bool effect;	
};

extern MCXMLManager* gStr;
extern Config config;
extern char thinkConfig[2][64];

void initialize();
void finalize();

void initConfig();

void playSound(string path);
string getOnlineName();

string createGStr(const char* gStrKey, ...);
string getGStr(const char* format, ...);
string safeDivStr(int a, int b);

void saveConfig();
void loadConfig();

uchar getMaxUndoCount(uchar aiLevel);
