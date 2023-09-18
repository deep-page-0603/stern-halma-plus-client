
#pragma once

#include "mcserializer.h"

namespace mocos2d {

class MCPlayerInfo final : public MCSerializer {
public:
	struct Serial {
		char address[MAX_PLAYER_ADDRESS];
		char name[MAX_PLAYER_NAME];		
		char character;
		char team;
		bool connected;
		bool bot;
		bool valid;
	};
	virtual ~MCPlayerInfo() {};

	static MCPlayerInfo* create(string address, string name, char character, char team, bool connected, bool bot, bool valid);
	static MCPlayerInfo* create(uchar* buffer);
	void init(string address, string name, char character, char team, bool connected, bool bot, bool valid);
	bool isEqual(MCPlayerInfo* pi);

	virtual uint getSize() override;
	virtual uchar* save() override;
	virtual void load(uchar* buffer) override;

protected:
	Serial mSerial;

public:
	string getAddress();
	string getName();
	void setName(string name);
	char getCharacter();
	char getTeam();
	void setTeam(char team);
	bool isConnected();
	bool isBot();
	void setConnected(bool connected);
	void setBot(bool bot);
	void setAddress(string address);
	void setCharacter(char character);
	bool isValid();
	void setValid(bool valid);
};

}