//
// Created by Administrator on 2020/10/27.
//

#ifndef AALIVE_JNIUTILS_H
#define AALIVE_JNIUTILS_H

#include <jni.h>
#include <string>

class JNIUtils {
public:
    static jint jniThrowException(JNIEnv *env, const char *classname, const char *message) {
        jclass jclazz = env->FindClass(classname);
        return env->ThrowNew(jclazz, message);
    }

    static jint jniThrowException(JNIEnv *env, const char *classname, const std::string message) {
        return jniThrowException(env, classname, message.c_str());
    }

    static jint throwIllegalStateException (JNIEnv* env, const char* message){
        return jniThrowException(env, "java/lang/IllegalStateException", message);
    }

    static jint throwIllegalStateException (JNIEnv* env, const std::string message){
        return throwIllegalStateException(env, message.c_str());
    }

    static jint throwNullPointerException (JNIEnv* env, const char* message){
        return jniThrowException(env, "java/lang/NullPointerException", message);
    }

    static jint throwNullPointerException (JNIEnv* env, const std::string message){
        return throwNullPointerException(env, message.c_str());
    }
};
#endif //AALIVE_JNIUTILS_H
