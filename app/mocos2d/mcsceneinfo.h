
#pragma once

#include "mcserializer.h"

namespace mocos2d {

enum MCScreenOrientationType {
	screen_orientation_landscape, screen_orientation_portrait
};

class MCSceneInfo final : public MCSerializer {
public:
	struct Serial {
		char tag;
		char csb[32];
		bool episode;
		MCScreenOrientationType screenOrientation;
		bool online;
		bool challenge;
		char me;
		char savePath[32];
	};
	virtual ~MCSceneInfo() {};

	static MCSceneInfo* create(char tag, string csb, bool episode, MCScreenOrientationType screenOrientation, bool online, bool challenge, char me, string savePath = "");
	static MCSceneInfo* create(uchar* buffer);
	void init(char tag, string csb, bool episode, MCScreenOrientationType screenOrientation, bool online, bool challenge, char me, string savePath);
	
	virtual uint getSize() override;
	virtual uchar* save() override;
	virtual void load(uchar* buffer) override;

protected:
	Serial mSerial;

public:
	char getTag();
	bool isEpisode();
	MCScreenOrientationType getScreenOrientation();
	bool isOnline();
	bool isChallenge();
	char getMe();
	char* getCsb();
	void setMe(char me);
	char* getSavePath();
	void setSavePath(string savePath);
};

}