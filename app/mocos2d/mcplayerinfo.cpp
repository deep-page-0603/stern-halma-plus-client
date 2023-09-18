
#include "mcplayerinfo.h"

namespace mocos2d {

MCPlayerInfo* MCPlayerInfo::create(string address, string name, char character, char team, bool connected, bool bot, bool valid) {
	MCPlayerInfo* playerInfo = new MCPlayerInfo;
	playerInfo->init(address, name, character, team, connected, bot, valid);
	return playerInfo;
}

MCPlayerInfo* MCPlayerInfo::create(uchar* buffer) {
	MCPlayerInfo* playerInfo = new MCPlayerInfo;
	playerInfo->load(buffer);
	return playerInfo;
}

void MCPlayerInfo::init(string address, string name, char character, char team, bool connected, bool bot, bool valid) {
	strcpy(mSerial.address, address.c_str());
	strcpy(mSerial.name, name.c_str());
	mSerial.character = character;
	mSerial.team = team;
	mSerial.connected = connected;
	mSerial.bot = bot;
	mSerial.valid = valid;
}

uint MCPlayerInfo::getSize() {
	return sizeof(Serial);
}

uchar* MCPlayerInfo::save() {
	uchar* buffer = (uchar*) malloc(sizeof(Serial));
	memcpy(buffer, &mSerial, sizeof(Serial));
	return buffer;
}

void MCPlayerInfo::load(uchar* buffer) {
	memcpy(&mSerial, buffer, sizeof(Serial));
}

void MCPlayerInfo::setConnected(bool connected) {
	mSerial.connected = connected;
}

string MCPlayerInfo::getAddress() {
	return string(mSerial.address);
}

string MCPlayerInfo::getName() {
	return string(mSerial.name);
}

void MCPlayerInfo::setName(string name) {
	strcpy(mSerial.name, name.c_str());
}

char MCPlayerInfo::getCharacter() {
	return mSerial.character;
}

char MCPlayerInfo::getTeam() {
	return mSerial.team;
}

void MCPlayerInfo::setTeam(char team) {
	mSerial.team = team;
}

bool MCPlayerInfo::isConnected() {
	return mSerial.connected;
}

bool MCPlayerInfo::isBot() {
	return mSerial.bot;
}

void MCPlayerInfo::setBot(bool bot) {
	mSerial.bot = bot;
}

void MCPlayerInfo::setAddress(string address) {
	strcpy(mSerial.address, address.c_str());
}

void MCPlayerInfo::setCharacter(char character) {
	mSerial.character = character;
}

bool MCPlayerInfo::isEqual(MCPlayerInfo* pi) {
	if (strcmp(mSerial.address, pi->getAddress().c_str()) != 0) return false;
	if (strcmp(mSerial.name, pi->getName().c_str()) != 0) return false;
	if (mSerial.bot != pi->isBot()) return false;
	if (mSerial.character != pi->getCharacter()) return false;
	if (mSerial.connected != pi->isConnected()) return false;
	if (mSerial.team != pi->getTeam()) return false;
	if (mSerial.valid != pi->isValid()) return false;
	return true;
}

bool MCPlayerInfo::isValid() {
	return mSerial.valid;
}

void MCPlayerInfo::setValid(bool valid) {
	mSerial.valid = valid;
}

}