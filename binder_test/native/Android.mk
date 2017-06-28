#
# Binder native service.
#

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := libremote
LOCAL_CFLAGS := -Wall -Wextra -Werror -Wunused
LOCAL_SRC_FILES := \
	libremote/IRemoteService.cpp \
	libremote/IRemoteServiceClient.cpp
LOCAL_C_INCLUDES := $(LOCAL_PATH)/include/
LOCAL_SHARED_LIBRARIES := \
	libbinder \
	liblog \
	libutils \
	libkeystore_binder

include $(BUILD_SHARED_LIBRARY)


include $(CLEAR_VARS)
LOCAL_CFLAGS := -Wall -Wextra -Werror -Wunused
LOCAL_SRC_FILES := \
	bn/main_remote.cpp \
	bn/RemoteService.cpp
LOCAL_C_INCLUDES := $(LOCAL_PATH)/include/
LOCAL_MODULE := bn_native
LOCAL_REQUIRED_MODULES := libremote
LOCAL_SHARED_LIBRARIES := \
	libbinder \
	liblog \
	libutils \
	libkeystore_binder \
	libremote
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_CFLAGS := -Wall -Wextra -Werror -Wunused
LOCAL_SRC_FILES := \
	bp/main.cpp \
	bp/Remote.cpp
LOCAL_C_INCLUDES := $(LOCAL_PATH)/include/
LOCAL_MODULE := bp_native
LOCAL_REQUIRED_MODULES := libremote
LOCAL_SHARED_LIBRARIES := \
	libbinder \
	liblog \
	libutils \
	libkeystore_binder \
	libremote
include $(BUILD_EXECUTABLE)
