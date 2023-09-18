
#pragma once

#include "mocos2d.h"

#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID

namespace mocos2d {

class MCJni {
public:
	static void openBluetooth();
	static void closeBluetooth();
	static void resetBluetooth();

	static void findServer();
	static void serverListen();
	static void stopListen();
	static void sendTo(int index, uchar* packet, int len);
	static void kick(int index);
	static string getBluetoothName();

	static string getSim();
	static __String* getImei();
	
	static bool isFileExists(string path);
	static bool createFile(string path, string content);

	static long long getCurrentTime();
	static int getAndroidVersion();

	static void saveLog(string log);
};

}

#endif
