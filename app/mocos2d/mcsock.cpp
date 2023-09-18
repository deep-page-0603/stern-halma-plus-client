
#include "mcsock.h"
#include "mconline.h"
#include "mcjni.h"
#include "mcutil.h"

namespace mocos2d {

SOCKET _mcSocket = 0;
string _mcAddress = "";
bool _mcPairedWithServer = false;

bool MCSock::init() {
#if CC_TARGET_PLATFORM == CC_PLATFORM_WIN32
	WSADATA wsaData;
	WORD wsaVer = MAKEWORD(2, 2);
	bool ok = (WSAStartup(wsaVer, &wsaData) == 0);
	if (ok) _getDeviceIP();
	return ok;
#endif
	return true;
}

void MCSock::open(string address) {
	_mcAddress = address;
#if CC_TARGET_PLATFORM == CC_PLATFORM_WIN32
	SOCKADDR_IN addrin;
	_mcSocket = socket(AF_INET, SOCK_DGRAM, 0);
	addrin.sin_family = AF_INET;
	addrin.sin_port = htons(_appSetting.commPort);
	PHOSTENT phe = gethostbyname(_mcAddress.c_str());
	::memcpy((char FAR*) &(addrin.sin_addr), phe->h_addr, phe->h_length);

	MCOnline::onOpened(::bind(_mcSocket, (SOCKADDR FAR*) &addrin, sizeof(addrin)) != SOCKET_ERROR);
#elif CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
	MCJni::openBluetooth();
#endif
}

void MCSock::_getDeviceIP() {
	_mcDeviceIP.clear();

#if CC_TARGET_PLATFORM == CC_PLATFORM_WIN32
	char hostName[MAX_STR_LEN];
	gethostname(hostName, MAX_STR_LEN);
	PHOSTENT ent = gethostbyname(hostName);
	if (!ent->h_addr_list) return;
	unsigned char* addr = (unsigned char*) ent->h_addr;
	
	while (addr) {
		if (strcmp((char*) addr, ent->h_name) == 0) break;
		string ip;

		for (char j = 0; j < 4; j++) {
			ip += MCUtil::numToStr(addr[j]);
			if (j != 3) ip += ".";
		}
		_mcDeviceIP.push_back(ip);
		addr += 4;
	}
#endif
}

void MCSock::find(int code) {
	_mcPairedWithServer = true;

#if CC_TARGET_PLATFORM == CC_PLATFORM_WIN32
	__Array* segments = __String::create(_mcAddress)->componentsSeparatedByString(".");
	string domain;

	for (char i = 0; i < 3; ++i) {
		domain += ((__String*) (segments->objectAtIndex(i)))->getCString();
		domain += ".";
	}
	for (int i = 1; i <= 255; i++) {
		MCOnline::addSendPacket(domain + MCUtil::numToStr(i), NULL, 0, packet_find, code);
	}
#else
	MCJni::findServer();
#endif
}

void MCSock::listen() {
#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
	MCJni::serverListen();
#endif
}

void MCSock::stopListen() {
#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
	MCJni::stopListen();
#endif
}

bool MCSock::recv() {
#if CC_TARGET_PLATFORM == CC_PLATFORM_WIN32
	uchar buff[MAX_PACKET];
	int len;
	SOCKADDR_IN addrin;	
	int size = sizeof(SOCKADDR_IN);

	len = recvfrom(_mcSocket, (char*) buff, MAX_PACKET, 0, (SOCKADDR FAR*) &addrin, &size);	
	if (len == SOCKET_ERROR) return false;
	MCOnline::addRecvPacket(buff, len, inet_ntoa(addrin.sin_addr));
	return true;
#endif
	return false;
}

bool MCSock::send(string address, uchar* packet, int len) {	
#if CC_TARGET_PLATFORM == CC_PLATFORM_WIN32
	SOCKADDR_IN toSock;
	toSock.sin_family = AF_INET;
	toSock.sin_port = htons(_appSetting.commPort);
	PHOSTENT phe = gethostbyname(address.c_str());
	memcpy((char FAR*) &(toSock.sin_addr), phe->h_addr, phe->h_length);

	return sendto(_mcSocket, (char*) packet, len, NULL, (SOCKADDR*) &toSock, sizeof(SOCKADDR_IN)) != SOCKET_ERROR;
#elif CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID	
	MCJni::sendTo(atoi(address.c_str()), packet, len);
	return true;
#endif
}

void MCSock::kick(string address) {
#if CC_TARGET_PLATFORM == CC_PLATFORM_WIN32
#elif CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID	
	MCJni::kick(atoi(address.c_str()));
#endif
}

void MCSock::close() {
#if CC_TARGET_PLATFORM == CC_PLATFORM_WIN32
	closesocket(_mcSocket);
#else
	MCJni::closeBluetooth();
#endif
}

void MCSock::reset() {
#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
	MCJni::resetBluetooth();
#endif
}

void MCSock::quit() {
#if CC_TARGET_PLATFORM == CC_PLATFORM_WIN32
	WSACleanup();
#endif
}

string MCSock::getName() {
#if CC_TARGET_PLATFORM == CC_PLATFORM_WIN32
	__Array* arr = __String::create(MCOnline::getAddress())->componentsSeparatedByString(".");
	return string(MCUtil::createStr("%s.%s", ((__String*) arr->objectAtIndex(2))->getCString(),
		((__String*) arr->objectAtIndex(3))->getCString()));
#else
	return MCJni::getBluetoothName();
#endif
}

void MCSock::onBluetoothOpened(bool ok) {
	Director::getInstance()->getRunningScene()->runAction(Sequence::createWithTwoActions(
		DelayTime::create(0.02f),
		CallFunc::create(bind(&MCOnline::onOpened, ok))));
}

void MCSock::onPairedWithServer() {
	_mcPairedWithServer = true;
	Director::getInstance()->getRunningScene()->runAction(Sequence::createWithTwoActions(
		DelayTime::create(0.02f),
		CallFunc::create(&MCOnline::onPairedWithServer)));
}

void MCSock::onPairingFailed() {
	_mcPairedWithServer = false;
	Director::getInstance()->getRunningScene()->runAction(Sequence::createWithTwoActions(
		DelayTime::create(0.02f),
		CallFunc::create(&MCOnline::onPairingFailed)));
}

void MCSock::onConnectingFailed() {
	Director::getInstance()->getRunningScene()->runAction(Sequence::createWithTwoActions(
		DelayTime::create(0.02f),
		CallFunc::create(&MCOnline::onConnectingFailed)));
}

void MCSock::onDisconnected() {
	Director::getInstance()->getRunningScene()->runAction(Sequence::createWithTwoActions(
		DelayTime::create(0.02f),
		CallFunc::create(&MCOnline::onDisconnected)));
}

void MCSock::onClientDisconnected(int index) {
	string address = MCUtil::numToStr(index);

	Director::getInstance()->getRunningScene()->runAction(Sequence::createWithTwoActions(
		DelayTime::create(0.02f),
		CallFunc::create(bind(&MCOnline::onClientDisconnected, address))));
}

void MCSock::onRecv(uchar* buffer, int len, int index) {
	uchar* pack = (uchar*) malloc(len);
	memcpy(pack, buffer, len);
	string address = MCUtil::numToStr(index);

	MCOnline::addRecvPacket(pack, len, address);
	free(pack);

	/*Director::getInstance()->getRunningScene()->runAction(Sequence::create(
		DelayTime::create(0.02f),
		CallFunc::create(bind(&MCOnline::addRecvPacket, pack, len, address)),
		CallFunc::create(bind([pack]() {
			free(pack);
		})),
		NULL
		));*/
}

}