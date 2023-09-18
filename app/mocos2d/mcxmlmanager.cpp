
#include "mcxmlmanager.h"
#include "mcutil.h"

namespace mocos2d {

MCXMLManager* MCXMLManager::create(string subKey) {
	MCXMLManager* xmlManager = new MCXMLManager();
	xmlManager->_mSubKey = subKey;
	return xmlManager;
}

MCXMLManager* MCXMLManager::createWithData(uchar* xmlData, int dataLen, string subKey) {
	SAXParser saxParser;
	if (!saxParser.init("UTF-8")) return NULL;

	MCXMLManager* xmlManager = new MCXMLManager();
	xmlManager->_mSubKey = subKey;

	saxParser.setDelegator(xmlManager);
	saxParser.parse((const char*) xmlData, dataLen);
	return xmlManager;
}

MCXMLManager* MCXMLManager::createWithFile(string path, string subKey) {
	Data data = FileUtils::getInstance()->getDataFromFile(path);
	if (data.isNull()) return NULL;
	return MCXMLManager::createWithData((uchar*) data.getBytes(), (int) data.getSize(), subKey);
}

void MCXMLManager::startElement(void* ctx, const char* name, const char** atts) {
	if (!atts || !atts[0]) return;
	const char** pt = atts;

	while (pt[0]) {
		if (_mSubKey.compare(pt[0]) == 0) {
			_mStrVec.push_back(pair<string, string>(name, pt[1]));
		}
		pt += 2;
	}
}

string MCXMLManager::getStr(string key, string defaultStr) {
	for (int i = 0; i < _mStrVec.size(); ++i) {
		if (_mStrVec[i].first.compare(key) == 0) return _mStrVec[i].second.c_str();
	}
	return defaultStr;
}

string MCXMLManager::getStr(int id, string defaultStr) {
	if (id >= _mStrVec.size()) return defaultStr;
	return _mStrVec[id].second.c_str();
}

int MCXMLManager::getInt(string key, int defaultValue) {
	string str = getStr(key);
	if (str.compare("") == 0) return defaultValue;
	return atoi(str.c_str());
}

float MCXMLManager::getFloat(string key, float defaultValue) {
	string str = getStr(key);
	if (str.compare("") == 0) return defaultValue;
	return (float) (utils::atof(str.c_str()));
}

bool MCXMLManager::getBool(string key, bool defaultValue) {
	string str = getStr(key);
	if (str.compare("") == 0) return defaultValue;
	return strcmp(str.c_str(), "true") == 0;
}

void MCXMLManager::insertStr(string key, string value) {
	_mStrVec.push_back(pair<string, string>(key, value));
}

void MCXMLManager::insertInt(string key, int value) {
	_mStrVec.push_back(pair<string, string>(key, MCUtil::numToStr(value)));
}

void MCXMLManager::insertFloat(string key, float value) {
	_mStrVec.push_back(pair<string, string>(key, MCUtil::numToStr(value)));
}

void MCXMLManager::insertBool(string key, bool value) {
	_mStrVec.push_back(pair<string, string>(key, value ? "true" : "false"));
}

string MCXMLManager::exportXMLData() {
	__String* str = __String::create("<?xml version = \"1.0\" encoding = \"UTF-8\"?>");

	for (pair<string, string> pair : _mStrVec) {
		str->append("<" + pair.first + " ");
		str->append(_mSubKey + "=\"");
		str->append(pair.second);
		str->append("\"/>");
	}
	return str->getCString();
}

void MCXMLManager::exportFile(string path) {
	string xmlData = exportXMLData();
	FileUtils::getInstance()->writeStringToFile(xmlData, MCUtil::getWritablePath(path));
}

}