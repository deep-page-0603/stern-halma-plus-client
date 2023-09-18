
#pragma once

#include "mocos2d.h"

namespace mocos2d {

extern SOCKET _mcSocket;
extern string _mcAddress;
extern bool _mcPairedWithServer;

class MCSock {
public:
	static bool init();
	static void open(string address);	
	static void find(int code);
	static void listen();
	static void stopListen();
	static bool recv();
	static bool send(string address, uchar* packet, int len);
	static void kick(string address);
	static void close();
	static void quit();
	static void reset();
	static string getName();

	static void onBluetoothOpened(bool ok);
	static void onPairedWithServer();
	static void onPairingFailed();
	static void onConnectingFailed();
	static void onDisconnected();
	static void onClientDisconnected(int index);
	static void onRecv(uchar* buffer, int len, int index);

protected:
	static void _getDeviceIP();
};

}