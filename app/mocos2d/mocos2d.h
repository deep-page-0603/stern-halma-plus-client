
#pragma once

#include <time.h>
#include <stdio.h>
#include <math.h>
#include <random>
#include <vector>
#include <set>
#include <algorithm>
#include "cocos2d.h"
#include "cocostudio/cocostudio.h"
#include "ui/cocosgui.h"
#include "audio/include/audioengine.h"
#include "audio/include/simpleaudioengine.h"
#include "particle3d/ccparticlesystem3d.h"
#include "particle3d/pu/ccpuparticlesystem3d.h"

#if CC_TARGET_PLATFORM == CC_PLATFORM_WIN32
#include <winsock2.h>
#elif CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
#include <jni.h>
#include <android/log.h>
#include "platform/android/jni/jnihelper.h"
#endif

using namespace std; 
using namespace cocos2d;
using namespace cocos2d::ui;
using namespace cocostudio::timeline;
using namespace experimental;
using namespace CocosDenshion;

namespace mocos2d {

#define MAX_PLAYER_NAME 64
#define MAX_PLAYER_ADDRESS 16
#define MAX_PACKET 5120
#define MAX_Z_ORDER 1024
#define MAX_STR_LEN 256

#ifndef SOCKET
#define SOCKET int
#endif

#ifndef uint
#define uint unsigned int
#endif

#ifndef uchar
#define uchar unsigned char
#endif

#ifndef ushort
#define ushort unsigned short
#endif

enum MCScreenLayoutType {
	screen_layout_desktop, screen_layout_mobile
};

enum MCProductType {
	product_debug, product_release, product_final
};

struct AppSetting {
	char appName[MAX_STR_LEN];
	MCProductType productType;
	MCScreenLayoutType screenLayoutType;
	float screenWidth;
	float screenHeight;
	float windowWidth;
	float windowHeight;
	bool fullScreen;
	float updateFrequency;
	int randomSeed;
	float replaceSceneDuration;
	float loadingSceneDuration;
	int commPort;
	uint packetHeader;
	uint packetFooter;
	bool uiAssist;
	char maxPlayerCount;
	char defaultFont[MAX_STR_LEN];
	char logTag[MAX_STR_LEN];
};

extern AppSetting _appSetting;
extern int musicId;
extern int androidVer;

void initMocos2d();

}
