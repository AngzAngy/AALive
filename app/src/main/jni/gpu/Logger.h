#ifndef __GPU_LOGGER_H_
#define __GPU_LOGGER_H_

 #include <android/log.h>

 #ifdef __cplusplus
 extern "C" {
 #endif

 #ifndef LOG_TAG
 #define LOG_TAG    "AA_GPU_LOG"
 #endif

 #define LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
 #define LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
 #define LOGW(...)  __android_log_print(ANDROID_LOG_WARN,LOG_TAG,__VA_ARGS__)
 #define LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
 #define LOGV(...)  __android_log_print(ANDROID_LOG_VERBOSE,LOG_TAG,__VA_ARGS__)

 #ifdef __cplusplus
 }
 #endif

 #endif /* __JNILOGGER_H_ */
