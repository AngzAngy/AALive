#include "org_angzangy_aalive_OpenglNative.h"
#include "AALog.h"
#include "NativeContext.h"
#include <GLES2/gl2.h>

typedef struct aa_fields{
    jfieldID nativePrt;
}fields;

static fields localfields={NULL};

static NativeContext * getNativePtr(JNIEnv *jniEnv, jobject jobj){
    NativeContext * ptr = NULL;
    if(localfields.nativePrt){
        ptr = reinterpret_cast<NativeContext *>(jniEnv->GetLongField(jobj, localfields.nativePrt));
    }
    return ptr;
}

static void setNativePrt(JNIEnv *jniEnv, jobject jobj, NativeContext *ptr){
    if(!localfields.nativePrt){
        return;
    }
    if(ptr){
        jniEnv->SetLongField(jobj, localfields.nativePrt, reinterpret_cast<jlong>(ptr));
    }else{
        jniEnv->SetLongField(jobj, localfields.nativePrt, 0);
    }
}
/*
 * Class:     org_angzangy_aalive_OpenglNative
 * Method:    init
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_org_angzangy_aalive_OpenglNative_init
        (JNIEnv *jniEnv, jobject jobj){
    jclass jclazz = jniEnv->GetObjectClass(jobj);
    localfields.nativePrt = jniEnv->GetFieldID(jclazz, "nativeObj", "J");
    if(!localfields.nativePrt){
        LOGE("%s no field (nativeObj J)", __FUNCTION__);
        return;
    }
    NativeContext *ptr = new NativeContext;
    if(!ptr){
        LOGE("%s new err", __FUNCTION__);
        return;
    }
    setNativePrt(jniEnv, jobj, ptr);
}

/*
 * Class:     org_angzangy_aalive_OpenglNative
 * Method:    release
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_org_angzangy_aalive_OpenglNative_release
        (JNIEnv *jniEnv, jobject jobj){
    NativeContext * ptr = getNativePtr(jniEnv, jobj);
    if(ptr){
        ptr->liveMuxer.stop();
        ptr->liveMuxer.release();
        delete(ptr);
    }
    setNativePrt(jniEnv, jobj, NULL);
}

/*
 * Class:     org_angzangy_aalive_OpenglNative
 * Method:    onSurfaceCreated
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_org_angzangy_aalive_OpenglNative_onSurfaceCreated
        (JNIEnv *jniEnv, jobject jobj){
}

/*
 * Class:     org_angzangy_aalive_OpenglNative
 * Method:    onSurfaceChanged
 * Signature: (II)V
 */
JNIEXPORT void JNICALL Java_org_angzangy_aalive_OpenglNative_onSurfaceChanged
        (JNIEnv *jniEnv, jobject jobj, jint jsurfaceWidth, jint jsurfaceHeight){
    NativeContext * ptr = getNativePtr(jniEnv, jobj);
    if(ptr){
        ptr->liveMuxerInfo.muxerUri = "rtmp://172.17.72.23:1935/myapp/test2";
        ptr->liveMuxerInfo.videoSrcWidth = jsurfaceWidth;
        ptr->liveMuxerInfo.videoSrcHeight = jsurfaceHeight;
        ptr->liveMuxerInfo.videoDstWidth = jsurfaceWidth;
        ptr->liveMuxerInfo.videoDstHeight = jsurfaceHeight;
        ptr->liveMuxerInfo.voideoBitrate = 400000;
        ptr->videoRawBuf = NULL;

        ptr->liveMuxer.setMuxerInfo((ptr->liveMuxerInfo));
        ptr->liveMuxer.start();
    }
}
/*
 * Class:     org_angzangy_aalive_OpenglNative
 * Method:    onDrawFrame
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_org_angzangy_aalive_OpenglNative_onDrawFrame
  (JNIEnv *jniEnv, jobject jobj){
    NativeContext * ptr = getNativePtr(jniEnv, jobj);
    if(ptr){
        GLsizei width = ptr->liveMuxerInfo.videoSrcWidth;
        GLsizei height = ptr->liveMuxerInfo.videoSrcHeight;
        if(NULL == ptr->videoRawBuf){
            ptr->videoRawBuf = new char[width * height * 4];
            ptr->videoRawBufBytes = width * height * 4;
        }
        GLvoid* pixels = (GLvoid*)ptr->videoRawBuf;
        glReadPixels (0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
        ptr->liveMuxer.queueVideoFrame(ptr->videoRawBuf, ptr->videoRawBufBytes);
    }
}

/*
 * Class:     org_angzangy_aalive_OpenglNative
 * Method:    pushNV21Buffer
 * Signature: ([BII)V
 */
JNIEXPORT void JNICALL Java_org_angzangy_aalive_OpenglNative_pushNV21Buffer
  (JNIEnv *jniEnv, jobject jobj, jbyteArray jbuffer, jint jw, jint jh){
    NativeContext * ptr = getNativePtr(jniEnv, jobj);
    if(ptr && jbuffer){
        jbyte* buffer = jniEnv->GetByteArrayElements(jbuffer, NULL);
        ptr->liveMuxer.queueVideoFrame((const char *)buffer, jw * jh * 3 / 2);
        jniEnv->ReleaseByteArrayElements(jbuffer, buffer, 0);
    }
 }

