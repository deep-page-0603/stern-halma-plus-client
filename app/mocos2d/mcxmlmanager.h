
#pragma once

#include "mocos2d.h"

namespace mocos2d {

class MCXMLManager : public Ref, public SAXDelegator {
public:
	static MCXMLManager* create(string subKey);
	static MCXMLManager* createWithFile(string path, string subKey = "text");
	static MCXMLManager* createWithData(uchar* xmlData, int dataLen, string subKey = "text");
	
	string getStr(string key, string defaultStr = "");
	string getStr(int id, string defaultStr = "");
	int getInt(string key, int defaultValue = 0);
	float getFloat(string key, float defaultValue = 0);
	bool getBool(string key, bool defaultValue = false);
	
	void insertStr(string key, string value);
	void insertInt(string key, int value);
	void insertFloat(string key, float value);
	void insertBool(string key, bool value);

	string exportXMLData();
	void exportFile(string path);

protected:
	void startElement(void* ctx, const char* name, const char** atts);
	void endElement(void* ctx, const char* name) {};
	void textHandler(void* ctx, const char* ch, int len) {};
	
protected:
	vector<pair<string, string>> _mStrVec;
	string _mSubKey;
};

}