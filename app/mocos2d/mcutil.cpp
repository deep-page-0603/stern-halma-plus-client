
#include "mcutil.h"

namespace mocos2d {

void MCUtil::verifyExecutePath() {
#if CC_TARGET_PLATFORM == CC_PLATFORM_WIN32
	WCHAR wPath[MAX_STR_LEN] = {0};
	char aPath[MAX_STR_LEN] = {0};
	char path[MAX_STR_LEN] = {0};

    GetModuleFileNameW(NULL, wPath, MAX_STR_LEN - 1);        
	GetModuleFileNameA(NULL, path, MAX_STR_LEN - 1);
	WideCharToMultiByte(CP_UTF8, 0, wPath, wcslen(wPath) + 1, aPath, MAX_STR_LEN, NULL, NULL);

	if (strcmp(aPath, path) != 0) {
		MessageBox("Sorry, but execute path cannot contain multi-byte language such as East Asian languages.", _appSetting.appName);	
		exit(1);
	}
#endif
}

Director* MCUtil::initDirector() {
	Director* director = Director::getInstance();
    GLView* glview = director->getOpenGLView();
	
	float windowWid = _appSetting.windowWidth;
	float windowHei = _appSetting.windowHeight;

#if CC_TARGET_PLATFORM == CC_PLATFORM_WIN32
	windowWid = GetSystemMetrics(SM_CXSCREEN);
	windowHei = GetSystemMetrics(SM_CYSCREEN);
	
	if (!_appSetting.fullScreen) {
		windowHei *= 0.85f;
		windowWid = windowHei * _appSetting.screenWidth / _appSetting.screenHeight;
	}
#endif
    if (!glview) {
        glview = GLViewImpl::createWithRect(_appSetting.appName, Rect(0, 0, windowWid, windowHei));
        director->setOpenGLView(glview);
    }
    director->getOpenGLView()->setDesignResolutionSize(_appSetting.screenWidth, _appSetting.screenHeight, ResolutionPolicy::SHOW_ALL);
	director->setAnimationInterval(1 / _appSetting.updateFrequency);
	return director;
}

string MCUtil::createStr(const char* format, ...) {
	__String* str = __String::create("");
    va_list ap;
    va_start(ap, format);
    str->initWithFormatAndValist(format, ap);
    va_end(ap);
    return str->getCString();
}

string MCUtil::numToStr(int number, bool mili, const char* space) {
	if (!mili) return createStr("%d", number);

	string x = MCUtil::numToStr((number >= 0) ? number : -number);
	string result;
	char remains = x.length() % 3;

	if (remains > 0) result = x.substr(0, remains);
	char i = remains;

	while (i < x.length()) {
		if (result.length() > 0) result += space;
		result += x.substr(i, 3);
		i += 3;
	}
	return (number >= 0) ? result : ("-" + result);
}

string MCUtil::numToStr(float number) {
	return createStr("%f", number);
}

string MCUtil::getWritablePath(string path) {	
#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
	FileUtils::getInstance()->purgeCachedEntries();
	return __String::create(FileUtils::getInstance()->getWritablePath() + path)->getCString();
#else
	return __String::create(string("save/") + path)->getCString();
#endif
}

void MCUtil::setTexture(Sprite3D* sprite3D, string texture, string exceptMeshName, string exceptTexture) {
	Vector<Mesh*> meshes = sprite3D->getMeshes();

	for (Mesh* mesh : meshes) {
		if (mesh->getName().compare(exceptMeshName) != 0 && texture.compare("") != 0)
			mesh->setTexture(Director::getInstance()->getTextureCache()->addImage(texture));
		else if (exceptTexture.compare("") != 0)
			mesh->setTexture(Director::getInstance()->getTextureCache()->addImage(exceptTexture));
	}
}

bool MCUtil::isDescendant(Node* node, Node* ancestor) {
	Node* realParent = node->getParent();
	if (!realParent) return false;
	if (realParent == ancestor) return true;
	return isDescendant(realParent, ancestor);
}

void MCUtil::initCameras(Node* root, float width, float height) {
	auto camera = Camera::createOrthographic(width, height, 1, 1000);
	camera->setName("camera_1");
	camera->setPosition3D(Vec3(0, 0, 500));
	camera->setCameraFlag(CameraFlag::USER1);
	camera->setDepth(1);
	root->addChild(camera);

	camera = Camera::createOrthographic(width, height, 1, 1000);
	camera->setName("camera_2");
	camera->setPosition3D(Vec3(0, 0, 500));
	camera->setCameraFlag(CameraFlag::USER2);
	camera->setDepth(2);
	root->addChild(camera);

	camera = Camera::createOrthographic(width, height, 1, 1000);
	camera->setName("camera_3");
	camera->setPosition3D(Vec3(0, 0, 500));
	camera->setCameraFlag(CameraFlag::USER3);
	camera->setDepth(3);
	root->addChild(camera);

	camera = Camera::createOrthographic(width, height, 1, 1000);
	camera->setName("camera_4");
	camera->setPosition3D(Vec3(0, 0, 500));
	camera->setCameraFlag(CameraFlag::USER4);
	camera->setDepth(4);
	root->addChild(camera);

	camera = Camera::createOrthographic(width, height, 1, 1000);
	camera->setName("camera_5");
	camera->setPosition3D(Vec3(0, 0, 500));
	camera->setCameraFlag(CameraFlag::USER5);
	camera->setDepth(5);
	root->addChild(camera);

	camera = Camera::createOrthographic(width, height, 1, 1000);
	camera->setName("camera_6");
	camera->setPosition3D(Vec3(0, 0, 500));
	camera->setCameraFlag(CameraFlag::USER6);
	camera->setDepth(6);
	root->addChild(camera);

	camera = Camera::createOrthographic(width, height, 1, 1000);
	camera->setName("camera_7");
	camera->setPosition3D(Vec3(0, 0, 500));
	camera->setCameraFlag(CameraFlag::USER7);
	camera->setDepth(7);
	root->addChild(camera);

	camera = Camera::createOrthographic(width, height, 1, 1000);
	camera->setName("camera_8");
	camera->setPosition3D(Vec3(0, 0, 500));
	camera->setCameraFlag(CameraFlag::USER8);
	camera->setDepth(8);
	root->addChild(camera);

	auto amLight = AmbientLight::create(Color3B(255, 255, 255));
	amLight->setName("am_light_1");
	amLight->setIntensity(0.7f);
	amLight->setLightFlag(LightFlag::LIGHT1);
	root->addChild(amLight);

	amLight = AmbientLight::create(Color3B(255, 255, 255));
	amLight->setName("am_light_2");
	amLight->setIntensity(0.5f);
	amLight->setLightFlag(LightFlag::LIGHT2);
	root->addChild(amLight);

	auto dirLight = DirectionLight::create(Vec3(-0.3f, 0, -0.7f), Color3B(255, 255, 255));
	dirLight->setName("dir_light_1");
	dirLight->setIntensity(0.5f);
	dirLight->setLightFlag(LightFlag::LIGHT1);
	root->addChild(dirLight);

	dirLight = DirectionLight::create(Vec3(-0.3f, 0, -0.7f), Color3B(255, 255, 255));
	dirLight->setName("dir_light_2");
	dirLight->setIntensity(0.5f);
	dirLight->setLightFlag(LightFlag::LIGHT2);
	root->addChild(dirLight);
}

void MCUtil::mute(Node* node) {
	for (Node* child : node->getChildren()) {
		mute(child);
	}
	node->stopAllActions();
	node->unscheduleAllCallbacks();
	node->getEventDispatcher()->pauseEventListenersForTarget(node, true);
}

void MCUtil::muteActions(Node* node) {
	for (Node* child : node->getChildren()) {
		muteActions(child);
	}
	node->stopAllActions();
}

void MCUtil::runCocosEffect(Node* root, string csbPath, Vec2 pos, float speed, float delay, int times, int* countVar, string name) {
	string csbStr = csbPath;

	root->runAction(Sequence::createWithTwoActions(
		DelayTime::create(delay),
		CallFunc::create(bind([root, csbStr, pos, speed, times, countVar, name](){
			Node* effect = CSLoader::createNode(csbStr);
			effect->setPosition(pos);
			effect->setCameraMask((ushort) CameraFlag::USER5);
			effect->setName(name);
			root->addChild(effect);
			ActionTimeline* timeline = CSLoader::createTimeline(csbStr);
			effect->runAction(timeline);
			timeline->gotoFrameAndPlay(0, (times > 1) && countVar);
			timeline->setTimeSpeed(speed);
			if (countVar) *countVar = 0;
			timeline->addFrameEndCallFunc(timeline->getEndFrame(), "exit", [&, effect, times, countVar] {
				if (!countVar || (times < 1)) {
					effect->removeFromParent();
					return;
				}
				if (countVar) ++(*countVar);
				if (!countVar || (*countVar == times))
					effect->removeFromParent();
			});
			if ((times > 1) && countVar) {
				*countVar = 0;
				timeline->addFrameEndCallFunc(timeline->getEndFrame(), "exit", [&, effect, times, countVar] {
					++(*countVar);
					if (*countVar >= times)
						effect->removeFromParent();
				});
			} else {
				timeline->addFrameEndCallFunc(timeline->getEndFrame(), "exit", [&, effect] {
					effect->removeFromParent();
				});
			}
		}))
	));	
}

Sprite* MCUtil::runSpriteFrameEffect(Node* root, Vec2 pos, string format, float delay, bool removeSelf) {
	Sprite* sp = NULL;
	Vector<SpriteFrame*> frames;
	int id = 0;

	while (true) {
		string pngName = createStr(format.c_str(), id++);
		SpriteFrame* f = SpriteFrameCache::getInstance()->getSpriteFrameByName(pngName);
		if (!f && sp) break;
		if (!f) continue;
		if (!sp) sp = Sprite::createWithSpriteFrameName(pngName);
		frames.pushBack(f);
	}
	if (!sp) return NULL;
	
	sp->setName("frame_effect");
	sp->setPosition(pos);
	sp->setCameraMask((ushort) CameraFlag::USER7);
	root->addChild(sp);

	if (removeSelf)
		sp->runAction(Sequence::createWithTwoActions(
			Animate::create(Animation::createWithSpriteFrames(frames, delay)),
			RemoveSelf::create()
		));
	else
		sp->runAction(Animate::create(Animation::createWithSpriteFrames(frames, delay)));

	return sp;
}

string MCUtil::getOfflineSavePath(int tag) {
	return MCUtil::getWritablePath(MCUtil::createStr("offline_%d.sav", tag));
}

string MCUtil::getOnlineSavePath(int tag) {
	return MCUtil::getWritablePath(MCUtil::createStr("online_%d.sav", tag));
}

vector<Node*> MCUtil::getChildren(Node* root) {
	vector<Node*> children;

	for (Node* child : root->getChildren()) {
		vector<Node*> grandChildren = getChildren(child);
		children.insert(children.end(), grandChildren.begin(), grandChildren.end());
		children.push_back(child);
	}
	return children;
}

Rect MCUtil::getContentRect(Node* node) {
	Size size = node->getContentSize();
	Vec2 center = node->getPosition();
	return Rect(center.x - size.width / 2, center.y - size.height / 2, size.width, size.height);
}

Vec2 MCUtil::cvtToVec2(Vec3 vec3) {
	return Vec2(vec3.x, vec3.y);
}

Vec3 MCUtil::cvtToVec3(Vec2 vec2) {
	return Vec3(vec2.x, vec2.y, 0);
}

vector<string> MCUtil::split(string text, string delimeter) {
	__Array* components = __String::create(text)->componentsSeparatedByString(delimeter.c_str());
	vector<string> ret;

	for (int i = 0; i < components->count(); ++i) {
		ret.push_back(((__String*) components->getObjectAtIndex(i))->getCString());
	}
	return ret;
}

}