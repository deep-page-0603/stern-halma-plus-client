
#include "mcsceneinfo.h"
#include "mcutil.h"

namespace mocos2d {

MCSceneInfo* MCSceneInfo::create(char tag, string csb, bool episode, MCScreenOrientationType screenOrientation, bool online, bool challenge, char me, string savePath) {
	MCSceneInfo* sceneInfo = new MCSceneInfo;
	sceneInfo->init(tag, csb, episode, screenOrientation, online, challenge, me, savePath);
	return sceneInfo;
}

MCSceneInfo* MCSceneInfo::create(uchar* buffer) {
	MCSceneInfo* sceneInfo = new MCSceneInfo;
	sceneInfo->load(buffer);
	return sceneInfo;
}

void MCSceneInfo::init(char tag, string csb, bool episode, MCScreenOrientationType screenOrientation, bool online, bool challenge, char me, string savePath) {
	mSerial.tag = tag;
	strcpy(mSerial.csb, csb.c_str());
	mSerial.episode = episode;
	mSerial.screenOrientation = screenOrientation;
	mSerial.online = online;
	mSerial.challenge = challenge;
	mSerial.me = me;
	
	if (savePath.compare("") == 0) {
		if (!online) {
			savePath = MCUtil::createStr("offline_%d.sav", tag);
		} else {
			savePath = MCUtil::createStr("online_%d.sav", tag);
		}
	}
	strcpy(mSerial.savePath, savePath.c_str());
}

uint MCSceneInfo::getSize() {
	return sizeof(Serial);
}

uchar* MCSceneInfo::save() {
	uchar* buffer = (uchar*) malloc(sizeof(Serial));
	memcpy(buffer, &mSerial, sizeof(Serial));
	return buffer;
}

void MCSceneInfo::load(uchar* buffer) {
	memcpy(&mSerial, buffer, sizeof(Serial));
}

char MCSceneInfo::getTag() {
	return mSerial.tag;
}

bool MCSceneInfo::isEpisode() {
	return mSerial.episode;
}

MCScreenOrientationType MCSceneInfo::getScreenOrientation() {
	return mSerial.screenOrientation;
}

bool MCSceneInfo::isOnline() {
	return mSerial.online;
}

bool MCSceneInfo::isChallenge() {
	return mSerial.challenge;
}

char MCSceneInfo::getMe() {
	return mSerial.me;
}

void MCSceneInfo::setMe(char me) {
	mSerial.me = me;
}

char* MCSceneInfo::getCsb() {
	return mSerial.csb;
}

char* MCSceneInfo::getSavePath() {
	return mSerial.savePath;
}

void MCSceneInfo::setSavePath(string savePath) {
	strcpy(mSerial.savePath, savePath.c_str());
}

}