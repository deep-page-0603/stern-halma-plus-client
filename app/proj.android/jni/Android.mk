
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := paddle
LOCAL_SRC_FILES := libpaddle_light_api_shared.so

include $(PREBUILT_SHARED_LIBRARY)
APP_ALLOW_MISSING_DEPS := true

$(call import-add-path,$(LOCAL_PATH)/../../cocos2d)
$(call import-add-path,$(LOCAL_PATH)/../../cocos2d/external)
$(call import-add-path,$(LOCAL_PATH)/../../cocos2d/cocos)

LOCAL_MODULE := cocos2dcpp_shared
LOCAL_MODULE_FILENAME := libcocos2dcpp

LOCAL_SRC_FILES := main.cpp \
	../../src/appdelegate.cpp \
	../../src/backend.cpp \
	../../src/gamescene.cpp \
	../../src/global.cpp \
	../../mocos2d/mcdialog.cpp \
	../../mocos2d/mcjni.cpp \
	../../mocos2d/mcloadingscene.cpp \
	../../mocos2d/mconline.cpp \
	../../mocos2d/mcplayer.cpp \
	../../mocos2d/mcplayerinfo.cpp \
	../../mocos2d/mcscene.cpp \
	../../mocos2d/mcsceneinfo.cpp \
	../../mocos2d/mcserializer.cpp \
	../../mocos2d/mcsock.cpp \
	../../mocos2d/mctoast.cpp \
	../../mocos2d/mcui.cpp \
	../../mocos2d/mcutil.cpp \
	../../mocos2d/mcxmlmanager.cpp \
	../../mocos2d/mocos2d.cpp \
	../../cardlib/cl2dplayer.cpp \
	../../cardlib/clscene.cpp \
	../../../src/dajnn/dajconv.cpp \
	../../../src/dajnn/dajdense.cpp \
	../../../src/dajnn/dajfunc.cpp \
	../../../src/dajnn/dajgemm.cpp \
	../../../src/dajnn/dajmodel.cpp \
	../../../src/dajnn/dajnn.cpp \
	../../../src/dajnn/dajnorm.cpp \
	../../../src/dajnn/dajtensor.cpp \
	../../../src/dajnn/dajutil.cpp \
	../../../src/konix/koni.cpp \
	../../../src/konix/konix.cpp \
	../../../src/konix/kutil.cpp \
	../../../src/konix/mcts.cpp \
	../../../src/konix/net.cpp \
	../../../src/konix/players.cpp

LOCAL_SHARED_LIBRARIES := paddle

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../src \
	$(LOCAL_PATH)/../../mocos2d \
	$(LOCAL_PATH)/../../cardlib \
	$(LOCAL_PATH)/../../../dajnn/paddle \
	$(LOCAL_PATH)/../../../src/konix \
	$(LOCAL_PATH)/../../../src/dajnn

# _COCOS_HEADER_ANDROID_BEGIN
# _COCOS_HEADER_ANDROID_END

LOCAL_STATIC_LIBRARIES := cocos2dx_static

# _COCOS_LIB_ANDROID_BEGIN
# _COCOS_LIB_ANDROID_END

LOCAL_CLANG_ASFLAGS_arm += -no-integrated-as
APP_ABI := armeabi-v7a

include $(BUILD_SHARED_LIBRARY)

$(call import-module,.)

# _COCOS_LIB_IMPORT_ANDROID_BEGIN
# _COCOS_LIB_IMPORT_ANDROID_END
