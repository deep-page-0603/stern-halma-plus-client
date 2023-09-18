
#include "appdelegate.h"
#include "gamescene.h"

AppDelegate::AppDelegate() {
	initialize();
}

AppDelegate::~AppDelegate() {
	finalize();
}

void AppDelegate::initGLContextAttrs() {
    GLContextAttrs glContextAttrs = {8, 8, 8, 8, 24, 8};
    GLView::setGLContextAttrs(glContextAttrs);
}

bool AppDelegate::applicationDidFinishLaunching() {
	MCUtil::verifyExecutePath();
	auto director = MCUtil::initDirector();

#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
	FileUtils::getInstance()->addSearchPath(MCUtil::getWritablePath("assets"));
	androidVer = MCJni::getAndroidVersion();
#endif
	gStr = MCXMLManager::createWithFile("values/string.xml");	
	initConfig();

	auto scene = GameScene::createSimpleOffline();
	director->runWithScene(scene);
	return true;
}

void AppDelegate::applicationDidEnterBackground() {
    Director::getInstance()->stopAnimation();
}

void AppDelegate::applicationWillEnterForeground() {
    Director::getInstance()->startAnimation();
}
