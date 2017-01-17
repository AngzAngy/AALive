#ifndef AA_LOG_H
#define AA_LOG_H

#include <android/log.h>
#define AA_MODULE "AALog"

#define LOG_TRACE(...) \
    __android_log_print(ANDROID_LOG_VERBOSE, AA_MODULE, __VA_ARGS__)
#define LOG_DEBUG(...) \
    __android_log_print(ANDROID_LOG_DEBUG, AA_MODULE, __VA_ARGS__)
#define LOG_INFO(...) \
    __android_log_print(ANDROID_LOG_INFO, AA_MODULE, __VA_ARGS__)

#define LOG_WARN(...) \
    __android_log_print(ANDROID_LOG_WARN, AA_MODULE, __VA_ARGS__)
#define LOG_ERROR(...) \
    __android_log_print(ANDROID_LOG_ERROR, AA_MODULE, __VA_ARGS__)

#endif
