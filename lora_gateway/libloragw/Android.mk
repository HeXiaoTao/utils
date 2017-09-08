LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_CFLAGS := -O2 -Wall -Wextra -std=c99

LOCAL_SRC_FILES := src/loragw_aux.c	\
		src/loragw_fpga.c	\
		src/loragw_gps.c	\
		src/loragw_hal.c	\
		src/loragw_lbt.c	\
		src/loragw_radio.c	\
		src/loragw_reg.c	\
		src/loragw_spi.native.c

LOCAL_C_INCLUDES := $(LOCAL_PATH)/inc		\
		$(LOCAL_PATH)/src/agc_fw.var	\
		$(LOCAL_PATH)/src/arb_fw.var	\
		$(LOCAL_PATH)/src/cal_fw.var

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE:= libloragw

include $(BUILD_STATIC_LIBRARY)
