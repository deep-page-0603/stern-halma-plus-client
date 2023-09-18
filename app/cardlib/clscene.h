
#pragma once

#include "mcscene.h"
#include "mcutil.h"

using namespace mocos2d;

namespace cardlib {

class CLScene : public MCScene {
public:
	CREATE_FUNC(CLScene);

	virtual char getFrontTurn(char turn);
	virtual char getBackTurn(char turn);

	virtual void refreshStock(int change, bool effect) {};
	virtual void onInitialized() override;

protected:	
	virtual void onFinalized() override;

	virtual void initUI() {};
	virtual void initPlayers() {};
	virtual void initState() {};
	virtual void initCache() {};

	virtual void advance(uchar* notifyInfo, int len) override;
	virtual void advance() {};

	void notify();
	void _initCamera();	

public:
	struct NotifyInfo {
		union {
			char cbuff[60];
			short sbuff[30];
			int ibuff[15];
		};
	};

protected:
	NotifyInfo mNotify;
};

}