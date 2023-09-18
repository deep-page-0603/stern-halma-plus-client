
#include "mcjni.h"
#include "mcsock.h"

#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID

using namespace mocos2d;

extern "C" {

void JNICALL Java_com_mrb_mocos_Mocos2d_nativeOnBluetoothOpened(JNIEnv* env, jobject thiz, jboolean result) {
	MCSock::onBluetoothOpened(result);
}

void JNICALL Java_com_mrb_mocos_Mocos2d_nativeOnPairedWithServer(JNIEnv* env, jobject thiz) {
	MCSock::onPairedWithServer();
}

void JNICALL Java_com_mrb_mocos_Mocos2d_nativeOnPairingFailed(JNIEnv* env, jobject thiz) {
	MCSock::onPairingFailed();
}

void JNICALL Java_com_mrb_mocos_Mocos2d_nativeOnConnectingFailed(JNIEnv* env, jobject thiz) {
	MCSock::onConnectingFailed();
}

void JNICALL Java_com_mrb_mocos_Mocos2d_nativeOnDisconnected(JNIEnv* env, jobject thiz) {
	MCSock::onDisconnected();
}

void JNICALL Java_com_mrb_mocos_Mocos2d_nativeOnClientDisconnected(JNIEnv* env, jobject thiz, jint index) {
	MCSock::onClientDisconnected(index);
}

void JNICALL Java_com_mrb_mocos_Mocos2d_nativeOnRecv(JNIEnv* env, jobject thiz, jbyteArray byteArr, jint index) {
	jsize len = env->GetArrayLength(byteArr);
	jbyte* pb = env->GetByteArrayElements(byteArr, NULL);
	MCSock::onRecv((uchar*) pb, len, index);
	env->ReleaseByteArrayElements(byteArr, pb, 0);
}

}

namespace mocos2d {

#define MOCOS_CLASS_PATH "com/mrb/mocos/Mocos2d"

void MCJni::openBluetooth() {
	JniMethodInfo jminfo;

	if (JniHelper::getStaticMethodInfo(jminfo, MOCOS_CLASS_PATH, "openBluetooth", "()V")) {
		jminfo.env->CallStaticVoidMethod(jminfo.classID, jminfo.methodID);
	}
}

void MCJni::findServer() {
	JniMethodInfo jminfo;	
	if (JniHelper::getStaticMethodInfo(jminfo, MOCOS_CLASS_PATH, "findServer", "()V")) {
		jminfo.env->CallStaticVoidMethod(jminfo.classID, jminfo.methodID);
	}
}

void MCJni::serverListen() {
	JniMethodInfo jminfo;	
	if (JniHelper::getStaticMethodInfo(jminfo, MOCOS_CLASS_PATH, "serverListen", "()V")) {
		jminfo.env->CallStaticVoidMethod(jminfo.classID, jminfo.methodID);
	}
}

void MCJni::stopListen() {
	JniMethodInfo jminfo;	
	if (JniHelper::getStaticMethodInfo(jminfo, MOCOS_CLASS_PATH, "stopListen", "()V")) {
		jminfo.env->CallStaticVoidMethod(jminfo.classID, jminfo.methodID);
	}
}

void MCJni::sendTo(int index, uchar* packet, int len) {	
	JniMethodInfo jminfo;

	if (JniHelper::getStaticMethodInfo(jminfo, MOCOS_CLASS_PATH, "sendTo", "([BI)V")) {
		jbyteArray byteArr = jminfo.env->NewByteArray(len);
		jminfo.env->SetByteArrayRegion(byteArr, 0, len, (const jbyte*) packet);
		jminfo.env->CallStaticVoidMethod(jminfo.classID, jminfo.methodID, byteArr, index);
	}
}

void MCJni::kick(int index) {	
	JniMethodInfo jminfo;

	if (JniHelper::getStaticMethodInfo(jminfo, MOCOS_CLASS_PATH, "kick", "(I)V")) {
		jminfo.env->CallStaticVoidMethod(jminfo.classID, jminfo.methodID, index);
	}
}

void MCJni::closeBluetooth() {
	JniMethodInfo jminfo;
	
	if (JniHelper::getStaticMethodInfo(jminfo, MOCOS_CLASS_PATH, "closeBluetooth", "()V")) {
		jminfo.env->CallStaticVoidMethod(jminfo.classID, jminfo.methodID);
	}
}

void MCJni::resetBluetooth() {
	JniMethodInfo jminfo;
	
	if (JniHelper::getStaticMethodInfo(jminfo, MOCOS_CLASS_PATH, "resetBluetooth", "()V")) {
		jminfo.env->CallStaticVoidMethod(jminfo.classID, jminfo.methodID);
	}
}

string MCJni::getBluetoothName() {
	JniMethodInfo jminfo;
	
	if (JniHelper::getStaticMethodInfo(jminfo, MOCOS_CLASS_PATH, "getBluetoothName", "()Ljava/lang/String;")) {
		jstring key = (jstring) jminfo.env->CallStaticObjectMethod(jminfo.classID, jminfo.methodID);
		return string(__String::create(JniHelper::jstring2string(key))->getCString());
	}
	return string("");
}

bool MCJni::isFileExists(string path) {
	JniMethodInfo jminfo;
	
	if (JniHelper::getStaticMethodInfo(jminfo, MOCOS_CLASS_PATH, "isFileExists", "(Ljava/lang/String;)I")) {
		jstring strPath = jminfo.env->NewStringUTF(path.c_str());
		int ret = (jint) jminfo.env->CallStaticIntMethod(jminfo.classID, jminfo.methodID, strPath);
		return ret > 0;
	}
	return false;
}

int MCJni::getAndroidVersion() {
	JniMethodInfo jminfo;

	if (JniHelper::getStaticMethodInfo(jminfo, MOCOS_CLASS_PATH, "getAndroidVersion", "()I")) {
		int ret = (jint) jminfo.env->CallStaticIntMethod(jminfo.classID, jminfo.methodID);
		return ret;
	}
	return -1;
}

bool MCJni::createFile(string path, string content) {
	JniMethodInfo jminfo;
	
	if (JniHelper::getStaticMethodInfo(jminfo, MOCOS_CLASS_PATH, "createFile", "(Ljava/lang/String;Ljava/lang/String;)I")) {
		jstring strPath = jminfo.env->NewStringUTF(path.c_str());
		jstring strContent = jminfo.env->NewStringUTF(content.c_str());

		int ret = (jint) jminfo.env->CallStaticIntMethod(jminfo.classID, jminfo.methodID, strPath, strContent);
		return ret > 0;
	}
	return false;
}

long long MCJni::getCurrentTime() {
	JniMethodInfo jminfo;

	if (JniHelper::getStaticMethodInfo(jminfo, MOCOS_CLASS_PATH, "getCurrentTime", "()J")) {		
		long long ret = (jlong) jminfo.env->CallStaticLongMethod(jminfo.classID, jminfo.methodID);
		return ret;
	}
	return 0;
}

string MCJni::getSim() {
	JniMethodInfo jminfo;
	__String* strRet;

	jstring _string;
	if (JniHelper::getStaticMethodInfo(jminfo, MOCOS_CLASS_PATH, "getSim", "()Ljava/lang/String;")) {
		_string = (jstring)jminfo.env->CallStaticObjectMethod(jminfo.classID, jminfo.methodID);
		strRet = __String::create(JniHelper::jstring2string(_string));
	}
	return strRet->getCString();
}

__String* MCJni::getImei() {
	JniMethodInfo jminfo;
	
	if (JniHelper::getStaticMethodInfo(jminfo, MOCOS_CLASS_PATH, "getImei", "()Ljava/lang/String;")) {
		jstring key = (jstring) jminfo.env->CallStaticObjectMethod(jminfo.classID, jminfo.methodID);
		return String::create(JniHelper::jstring2string(key));
	}
	return String::create("000000000000000");
}

void MCJni::saveLog(string log) {
	JniMethodInfo jminfo;

	if (JniHelper::getStaticMethodInfo(jminfo, MOCOS_CLASS_PATH, "saveLog", "(Ljava/lang/String;)V")) {
		jstring strLog = jminfo.env->NewStringUTF(log.c_str());
		jminfo.env->CallStaticVoidMethod(jminfo.classID, jminfo.methodID, strLog);
	}
}

}

#endif