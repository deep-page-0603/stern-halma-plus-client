
#pragma once

#include "mocos2d.h"

namespace mocos2d {

#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, _appSetting.logTag, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, _appSetting.logTag, __VA_ARGS__)
#elif CC_TARGET_PLATFORM == CC_PLATFORM_WIN32
#define LOGD(f, ...) printf(f, __VA_ARGS__)
#define LOGE(f, ...) printf(f, __VA_ARGS__)
#endif

class MCUtil {
public:
	static void verifyExecutePath();
	static Director* initDirector();
	static string createStr(const char* format, ...);
	static string numToStr(int number, bool mili = false, const char* space = " ");
	static string numToStr(float number);
	static string getWritablePath(string path = "");
	static void setTexture(Sprite3D* sprite3D, string texture = "", string exceptMeshName = "", string exceptTexture = "");
	static void initCameras(Node* root, float width, float height);
	static void mute(Node* node);
	static void muteActions(Node* node);
	static bool isDescendant(Node* node, Node* ancestor);
	static void runCocosEffect(Node* root, string csbPath, Vec2 pos, float speed = 1.f, float delay = 0, int times = 1, int* countVar = NULL, string name = "");
	static Sprite* runSpriteFrameEffect(Node* root, Vec2 pos, string format, float delay, bool removeSelf = true);
	static string getOfflineSavePath(int tag);
	static string getOnlineSavePath(int tag);
	static vector<Node*> getChildren(Node* root);
	static Rect getContentRect(Node* node);
	static Vec2 cvtToVec2(Vec3 vec3);
	static Vec3 cvtToVec3(Vec2 vec2);
	static vector<string> split(string text, string delimeter);
};

}