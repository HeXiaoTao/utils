LOCAL_PATH:= $(call my-dir)

#-----------------------------------------------------
include $(CLEAR_VARS)
LOCAL_CFLAGS := -O2 -Wall -Wextra -std=c99
LOCAL_PLATFORM_SRC_FILES := platform/sx1276-Hal.c

LOCAL_SRC_FILES := $(LOCAL_PLATFORM_SRC_FILES)
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../libloragw/inc \
    $(LOCAL_PATH)/inc

LOCAL_MODULE_TAGS := optional
LOCAL_STATIC_LIBRARIES := libcutils
LOCAL_MODULE:= liblora.platform
include $(BUILD_STATIC_LIBRARY)

#-----------------------------------------------------
include $(CLEAR_VARS)
LOCAL_CFLAGS := -O2 -Wall -Wextra -std=c99
LOCAL_RADIO_SRC_FILES := radio/radio.c \
    radio/sx1276.c \
    radio/sx1276-LoRa.c \
    radio/sx1276-LoRaMisc.c

LOCAL_SRC_FILES := $(LOCAL_RADIO_SRC_FILES)

LOCAL_C_INCLUDES := $(LOCAL_PATH)/inc/
LOCAL_MODULE_TAGS := optional
LOCAL_STATIC_LIBRARIES := libcutils
LOCAL_MODULE:= liblora.radio
include $(BUILD_STATIC_LIBRARY)

#-----------------------------------------------------
include $(CLEAR_VARS)
LOCAL_CFLAGS := -O2 -Wall -Wextra -std=c11
LOCAL_SRC_FILES := queue/fixed_queue.c \
    queue/allocation_tracker.c \
    queue/hash_functions.c \
    queue/hash_map.c \
    queue/hash_map_utils.c \
    queue/allocator.c \
    queue/list.c \
    queue/reactor.c \
    queue/semaphore.c

LOCAL_C_INCLUDES := $(LOCAL_PATH)/inc
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE:= libfixqueue
include $(BUILD_STATIC_LIBRARY)

#-----------------------------------------------------
include $(CLEAR_VARS)
LOCAL_CFLAGS := -O2 -Wall -Wextra -std=c99

LOCAL_SRC_FILES := crc/crc16.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/inc/crc

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE:= liblora.crc
include $(BUILD_STATIC_LIBRARY)

#-----------------------------------------------------
include $(CLEAR_VARS)

LOCAL_CFLAGS := -O2 -Wall -Wextra -Werror -std=c99
LOCAL_SRC_FILES := lora_mac.c

LOCAL_C_INCLUDES := $(LOCAL_PATH)/inc

LOCAL_REQUIRED_MODULES := libfixqueue liblora.radio liblora.platform libloragw liblora.crc
LOCAL_SHARED_LIBRARIES := libutils
LOCAL_STATIC_LIBRARIES := libcutils libfixqueue liblora.radio liblora.platform libloragw liblora.crc
LOCAL_LDLIBS += -ldl -lm -llog
LOCAL_MODULE := loraservice
LOCAL_MODULE_TAGS := optional
LOCAL_CERTIFICATE := platform
# LOCAL_MODULE_PATH := $(TARGET_OUT_OPTIONAL_EXECUTABLES)
include $(BUILD_EXECUTABLE)
#-----------------------------------------------------

#-----------------------------------------------------
include $(CLEAR_VARS)

LOCAL_CFLAGS := -O2 -Wall -Wextra -Werror -std=c99
LOCAL_SRC_FILES := test/lora_ping.c

LOCAL_C_INCLUDES := $(LOCAL_PATH)/inc

LOCAL_REQUIRED_MODULES := libfixqueue liblora.radio liblora.platform libloragw
LOCAL_SHARED_LIBRARIES := libutils
LOCAL_STATIC_LIBRARIES := libcutils libfixqueue liblora.radio liblora.platform libloragw
LOCAL_LDLIBS += -ldl -lm -llog
LOCAL_MODULE := lora_ping
LOCAL_MODULE_TAGS := optional
LOCAL_CERTIFICATE := platform
# LOCAL_MODULE_PATH := $(TARGET_OUT_OPTIONAL_EXECUTABLES)
include $(BUILD_EXECUTABLE)
#-----------------------------------------------------
