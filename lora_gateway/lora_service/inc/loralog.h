/*
 * Author: hexiaotao
 * Date: 2015.11.13
 * Version: V1.0
 *
 */
//-------------------------------------------------------------
#ifndef _LORALOG_H
#define _LORALOG_H

#include <cutils/log.h>
#include <stdlib.h>

#include <android/log.h>

//#############################################################
/* ==================== Android Log ======================== */
#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG "<LoRa-native> "
#endif

  
#define LOGD(fmt, args...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, fmt, ##args)

#define LOG_5(...) __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, __VA_ARGS__)
#define LOG_4(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
#define LOG_3(...)  __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOG_2(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define LOG_1(...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

#ifdef DEBUG
#define LOGI(...) ALOGI(__VA_ARGS__)
#define LOGE(...) ALOGE(__VA_ARGS__)
#define LOGW(...) ALOGW(__VA_ARGS__)
#else
#define LOGI(...) while(0){}
#define LOGE(...) while(0){}
#define LOGW(...) while(0){}
#endif

#define CHECK_VALID(V) 				\
	if(V == NULL){					\
		LOGE("%s is null.", #V);	\
		exit(-1);					\
	}else{							\
		LOGI("%s is %p.", #V, V);	\
	}								\

//#############################################################
/* ======================= Debug =========================== */ 
/* */
/*  Debug information verbosity: lower values indicate higher urgency */
/* */
#define DEBUG_OFF        0
#define DEBUG_ERROR      1
#define DEBUG_WARN       2
#define DEBUG_INFO       3
#define DEBUG_TRACE      4
#define DEBUG_LOUD       5

extern uint8_t DebugLevel;

#define DBGPRINT_RAW(Level, ...)		\
do{						\
    if (Level <= DebugLevel)			\
    {						\
        LOG_##Level(__VA_ARGS__);		\
    }						\
}while(0)

#define USE_PRINTF 1

#if USE_PRINTF
#define DBGPRINT(Level, ...)    {printf(__VA_ARGS__);printf("\n");}
#define LOG_ERROR(tag, ...)  {printf(__VA_ARGS__);printf("\n");}
#define LOG_VERBOSE(tag, ...)  {printf(__VA_ARGS__);printf("\n");}
#define LOG_INFO(tag, ...)  {printf(__VA_ARGS__);printf("\n");}
#else
#define DBGPRINT(Level, ...)    DBGPRINT_RAW(Level, __VA_ARGS__)
#define LOG_ERROR(tag, ...)  LOG_1(__VA_ARGS__)
#define LOG_VERBOSE(tag, ...)  LOG_5(__VA_ARGS__)
#define LOG_INFO(tag, ...)  LOG_3(__VA_ARGS__)
#endif

//##############################################################
#endif /* _LORALOG_H */
