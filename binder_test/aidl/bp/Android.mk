#
# AIDL Bp test.
#
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := $(call all-subdir-java-files)
LOCAL_SRC_FILES += src/com/android/commands/bp/IRemoteService.aidl \
				   src/com/android/commands/bp/IRemoteServiceClient.aidl
LOCAL_MODULE := bp
include $(BUILD_JAVA_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := bp
LOCAL_SRC_FILES := bp
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_MODULE_TAGS := optional
include $(BUILD_PREBUILT)
