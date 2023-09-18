
#include "mconline.h"
#include "mcsock.h"
#include "mcscene.h"
#include "mcutil.h"

namespace mocos2d {

vector<MCPacket*> _mcOnlinePool;
bool _mcOnlineBusy = false;
thread _mcRecvThread;
bool _mcOnlineInited = false;
bool _mcOnlineOpened = false;
bool _mcOnlineListening = false;
bool _mcOnlineFinding = false;
vector<string> _mcDeviceIP;
int _mcFindCode = -1;

void procRecv() {
	while (true) MCOnline::recv();
}

bool MCOnline::init() {
	if (_mcOnlineInited) return true;
	clearPool();
	if (!MCSock::init()) return false;
	
	_mcOnlineInited = true;
	_mcOnlineOpened = false;
	_mcOnlineListening = _mcOnlineFinding = false;
	return true;
}

void MCOnline::quit() {
	if (!_mcOnlineInited) return;
	if (_mcOnlineOpened) close();
	MCSock::quit();
	_mcOnlineInited = false;
}

void MCOnline::open(string address) {
	if (_mcOnlineOpened) return;
	MCSock::open(address);
	_mcOnlineListening = _mcOnlineFinding = false;
}

void MCOnline::close() {
	if (!_mcOnlineOpened) return;
#if CC_TARGET_PLATFORM == CC_PLATFORM_WIN32
	_mcRecvThread.detach();
	if (_mcRecvThread.joinable()) _mcRecvThread.join();
#endif
	MCSock::close();
	clearPool();
	_mcOnlineOpened = false;
	_mcOnlineListening = _mcOnlineFinding = false;
}

void MCOnline::reset() {
	MCSock::reset();
	clearPool();
}

void MCOnline::onOpened(bool ok) {
#if CC_TARGET_PLATFORM == CC_PLATFORM_WIN32
	if (ok) _mcRecvThread = thread(procRecv);
#endif
	_mcOnlineOpened = ok;
	MCScene::getInstance()->onOnlineOpened(ok);
}

void MCOnline::find(int code) {
	if (_mcOnlineFinding) return;
	_mcOnlineFinding = true;
	_mcFindCode = code;
	MCSock::find(code);
}

void MCOnline::listen() {
	if (_mcOnlineListening) return;
	_mcOnlineListening = true;
	MCSock::listen();
}

void MCOnline::stopListening() {
	_mcOnlineListening = false;
	MCSock::stopListen();
}

void MCOnline::requestJoin(string address, int code) {
	MCScene::getInstance()->requestJoin(address, code);
}

void MCOnline::onConnected(string address, string myAddress) {
	_mcOnlineFinding = false;
	MCScene::getInstance()->onConnected(address, myAddress);
}

void MCOnline::onDisconnected() {
	MCScene::getInstance()->_onDisconnected();
}

void MCOnline::onClientDisconnected(string address) {
	MCScene::getInstance()->_onClientDisconnected(address);
}

void MCOnline::onPairedWithServer() {
	addSendPacket("0", NULL, 0, packet_find, _mcFindCode);
	MCScene::getInstance()->onPairedWithServer();
}

void MCOnline::onPairingFailed() {
	MCScene::getInstance()->onPairingFailed();
}

void MCOnline::onConnectingFailed() {
	MCScene::getInstance()->onConnectingFailed();
}

void MCOnline::addSendPacket(string address, uchar* buffer, int len) {
	while (_mcOnlineBusy);
	_mcOnlineBusy = true;

	MCPacket* packet = (MCPacket*) malloc(sizeof(MCPacket));
	memset(packet, 0, sizeof(MCPacket));
	packet->len = len;
	packet->buff = buffer;
	packet->send = true;
	strcpy(packet->address, address.c_str());
	_mcOnlinePool.push_back(packet);
	_mcOnlineBusy = false;
}

bool MCOnline::addSendPacket(string address, uchar* buffer, int size, MCPacketType type, int code) {
	if (!_mcOnlineOpened) return false;

	uchar* packet = (uchar*) malloc(MAX_PACKET);
	memset(packet, 0, MAX_PACKET);
	MCPacketHeader* header = (MCPacketHeader*) packet;
	header->meta = _appSetting.packetHeader;
	strcpy(header->address, address.c_str());
	header->type = type;
	header->code = code;
	if (buffer) memcpy(packet + sizeof(MCPacketHeader), buffer, size);
	memcpy(packet + size + sizeof(MCPacketHeader), &_appSetting.packetFooter, 4);
	addSendPacket(address, packet, size + sizeof(MCPacketHeader) + 4);
	return true;
}

void MCOnline::addRecvPacket(uchar* packet, int len, string address) {
	uchar* buff = (uchar*) malloc(len);
	memcpy(buff, packet, len);

	MCPacket* mcPacket = (MCPacket*) malloc(sizeof(MCPacket));
	mcPacket->len = len;
	mcPacket->buff = buff;
	mcPacket->send = false;
	strcpy(mcPacket->address, address.c_str());

	while (_mcOnlineBusy);
	_mcOnlineBusy = true;
	_mcOnlinePool.push_back(mcPacket);
	_mcOnlineBusy = false;
}

bool MCOnline::recv() {
	if (!_mcOnlineOpened) return false;
	return MCSock::recv();
}

bool MCOnline::send(MCPacket* packet) {
	return MCSock::send(packet->address, packet->buff, packet->len);
}

bool MCOnline::dispatchSend() {
	if (_mcOnlineBusy) return false;
	_mcOnlineBusy = true;

	for (int i = 0; i < _mcOnlinePool.size(); i++) {
		if (!_mcOnlinePool[i]->send) continue;				
		send(_mcOnlinePool[i]);

		free(_mcOnlinePool[i]->buff);
		free(_mcOnlinePool[i]);
		_mcOnlinePool.erase(_mcOnlinePool.begin() + i);
		_mcOnlineBusy = false;
		return true;
	}
	_mcOnlineBusy = false;
	return false;
}

MCPacketType MCOnline::dispatchRecv(string& address, uchar* dst, int& len, int& code) {
	if (_mcOnlineBusy) return packet_invalid;
	_mcOnlineBusy = true;

	for (int i = 0; i < _mcOnlinePool.size(); i++) {
		if (_mcOnlinePool[i]->send) continue;
		MCPacketType type = check(_mcOnlinePool[i]->buff, _mcOnlinePool[i]->len, code);

		if (type != packet_invalid) {
			read(dst, _mcOnlinePool[i]->buff, _mcOnlinePool[i]->len);
			len = _mcOnlinePool[i]->len - sizeof(MCPacketHeader) - 4;
			address = _mcOnlinePool[i]->address;
		}
		free(_mcOnlinePool[i]->buff);
		free(_mcOnlinePool[i]);
		_mcOnlinePool.erase(_mcOnlinePool.begin() + i);
		_mcOnlineBusy = false;
		return type;
	}
	_mcOnlineBusy = false;
	return packet_invalid;
}

MCPacketType MCOnline::check(uchar* buffer, int len, int& code) {
	if (len < sizeof(MCPacketHeader) + 4) return packet_invalid;
	MCPacketHeader* header = (MCPacketHeader*) buffer;

	if (header->meta != _appSetting.packetHeader) return packet_invalid;
	if (*((unsigned int*) (buffer + len - 4)) != _appSetting.packetFooter) return packet_invalid;
	int type = header->type;
	if ((type < 0) || (type > packet_user)) return packet_invalid;
	code = header->code;
	return (MCPacketType) type;
}

void MCOnline::read(uchar* dst, uchar* buffer, int len) {
	memcpy(dst, buffer + sizeof(MCPacketHeader), len - sizeof(MCPacketHeader) - 4);
}

void MCOnline::clearPool() {
	while (_mcOnlineBusy) {};
	_mcOnlineBusy = true;

	for (int i = 0; i < _mcOnlinePool.size(); i++) {
		free(_mcOnlinePool[i]->buff);
		free(_mcOnlinePool[i]);
	}
	_mcOnlinePool.clear();
	_mcOnlineBusy = false;
}

void MCOnline::allowJoin(string address) {
	MCOnline::addSendPacket(address, (uchar*) address.c_str(), (int) address.length() + 1, packet_allow, 0);
}

void MCOnline::refuseJoin(string address) {
	MCOnline::addSendPacket(address, NULL, 0, packet_refuse, 0);
}

string MCOnline::getAddress() {
	return _mcAddress.c_str();
}

void MCOnline::setAddress(string address) {
	_mcAddress = address;
}

string MCOnline::getOnlineName() {
	return MCSock::getName();
}

}