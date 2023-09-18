
#pragma once

#include "mocos2d.h"

namespace mocos2d {

enum MCPacketType {
	packet_invalid,
	packet_find, packet_server, packet_join, packet_allow, packet_refuse, packet_kick,
	packet_ping, packet_broadcast, packet_notify, packet_push, packet_request_push,
	packet_quit,
	packet_user
};

struct MCPacket {
	bool send;
	int len;
	char address[MAX_PLAYER_ADDRESS];
	uchar* buff;
};

struct MCPacketHeader {
	unsigned int meta;
	char address[MAX_PLAYER_ADDRESS];
	MCPacketType type;
	int code;
};

extern vector<MCPacket*> _mcOnlinePool;
extern bool _mcOnlineBusy;
extern thread _mcRecvThread;
extern bool _mcOnlineInited;
extern bool _mcOnlineOpened;
extern bool _mcOnlineListening;
extern bool _mcOnlineFinding;
extern vector<string> _mcDeviceIP;

class MCOnline {
public:
	static bool init();
	static void quit();

	static void open(string address = "");
	static void onOpened(bool ok);
	static void close();
	static void reset();
	static void clearPool();

	static void listen();
	static void find(int code);
	static void stopListening();

	static void requestJoin(string address, int code);
	static void allowJoin(string address);
	static void refuseJoin(string address);

	static void onConnected(string address, string myAddress);
	static void onDisconnected();
	static void onClientDisconnected(string address);
	static void onPairedWithServer();
	static void onPairingFailed();
	static void onConnectingFailed();
	
	static MCPacketType check(uchar* buffer, int len, int& code);
	static void read(uchar* dst, uchar* buffer, int len);
	
	static void addRecvPacket(uchar* packet, int len, string address = NULL);
	static void addSendPacket(string address, uchar* buffer, int len);	
	static bool addSendPacket(string address, uchar* buffer, int len, MCPacketType type, int code);

	static bool recv();
	static bool send(MCPacket* packet);

	static bool dispatchSend();
	static MCPacketType dispatchRecv(string& address, uchar* dst, int& len, int& code);

	static string getAddress();
	static string getOnlineName();
	static void setAddress(string address);
};

}