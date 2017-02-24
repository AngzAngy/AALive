#include "org_angzangy_aalive_LiveTelecastNative.h"
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
 * Class:     org_angzangy_aalive_LiveTelecastNative
 * Method:    init
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_org_angzangy_aalive_LiveTelecastNative_init
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
    ptr->liveMuxerInfo.audioSampleRate = 44100;
    ptr->liveMuxerInfo.audioBytesPerSample = 2;
    ptr->liveMuxerInfo.audioChannelNumber = 1;
    ptr->liveMuxerInfo.audioBitrate = 128000;
    AudioRecord *pAudioRecord = new AudioRecord(ptr->liveMuxerInfo.audioSampleRate, ptr->liveMuxerInfo.audioBytesPerSample,
                                                ptr->liveMuxerInfo.audioChannelNumber, 4096);
    if(pAudioRecord){
        ptr->audioRecord = pAudioRecord;
    }
    setNativePrt(jniEnv, jobj, ptr);
}

/*
 * Class:     org_angzangy_aalive_LiveTelecastNative
 * Method:    release
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_org_angzangy_aalive_LiveTelecastNative_release
        (JNIEnv *jniEnv, jobject jobj){
    NativeContext * ptr = getNativePtr(jniEnv, jobj);
    if(ptr){
        ptr->liveMuxer.stop();
        ptr->liveMuxer.release();
        if(ptr->audioRecord){
            ptr->audioRecord->stop();
            delete (ptr->audioRecord);
        }
        delete(ptr);
    }
    setNativePrt(jniEnv, jobj, NULL);
}

/*
 * Class:     org_angzangy_aalive_LiveTelecastNative
 * Method:    release
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_org_angzangy_aalive_LiveTelecastNative_setRtmpUrl
        (JNIEnv *jniEnv, jobject jobj, jstring jurl){
    NativeContext * ptr = getNativePtr(jniEnv, jobj);
    if(ptr && jurl){
        const char *url = jniEnv->GetStringUTFChars(jurl, 0);
        ptr->liveMuxerInfo.muxerUri = url;
        jniEnv->ReleaseStringUTFChars(jurl, url);
    }
}

/*
 * Class:     org_angzangy_aalive_LiveTelecastNative
 * Method:    onSurfaceCreated
 * Signature: ()V
 */
//JNIEXPORT void JNICALL Java_org_angzangy_aalive_LiveTelecastNative_onSurfaceCreated
//        (JNIEnv *jniEnv, jobject jobj){
//}

/*
 * Class:     org_angzangy_aalive_LiveTelecastNative
 * Method:    onPreviewSizeChanged
 * Signature: (II)V
 */
JNIEXPORT void JNICALL Java_org_angzangy_aalive_LiveTelecastNative_onPreviewSizeChanged
        (JNIEnv *jniEnv, jobject jobj, jint jsurfaceWidth, jint jsurfaceHeight){
    NativeContext * ptr = getNativePtr(jniEnv, jobj);
    if(ptr){
        ptr->liveMuxerInfo.videoSrcWidth = jsurfaceWidth;
        ptr->liveMuxerInfo.videoSrcHeight = jsurfaceHeight;
        ptr->liveMuxerInfo.videoDstWidth = jsurfaceWidth;
        ptr->liveMuxerInfo.videoDstHeight = jsurfaceHeight;
        ptr->liveMuxerInfo.voideoBitrate = 400000;
        ptr->videoRawBuf = NULL;

        ptr->liveMuxer.setMuxerInfo((ptr->liveMuxerInfo));
        ptr->liveMuxer.start();
        if(ptr->audioRecord){
            ptr->audioRecord->setOnFrameCallback(LiveMuxer::audioFrameCallback, &(ptr->liveMuxer));
            ptr->audioRecord->start();
        }
    }
}
/*
 * Class:     org_angzangy_aalive_LiveTelecastNative
 * Method:    onDrawFrame
 * Signature: ()V
 */
//JNIEXPORT void JNICALL Java_org_angzangy_aalive_LiveTelecastNative_onDrawFrame
//  (JNIEnv *jniEnv, jobject jobj){
//    NativeContext * ptr = getNativePtr(jniEnv, jobj);
//    if(ptr){
//        GLsizei width = ptr->liveMuxerInfo.videoSrcWidth;
//        GLsizei height = ptr->liveMuxerInfo.videoSrcHeight;
//        if(NULL == ptr->videoRawBuf){
//            ptr->videoRawBuf = new char[width * height * 4];
//            ptr->videoRawBufBytes = width * height * 4;
//        }
//        GLvoid* pixels = (GLvoid*)ptr->videoRawBuf;
//        glReadPixels (0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
//        ptr->liveMuxer.queueVideoFrame(ptr->videoRawBuf, ptr->videoRawBufBytes);
//    }
//}

/*
 * Class:     org_angzangy_aalive_LiveTelecastNative
 * Method:    pushNV21Buffer
 * Signature: ([BII)V
 */
JNIEXPORT void JNICALL Java_org_angzangy_aalive_LiveTelecastNative_pushNV21Buffer
  (JNIEnv *jniEnv, jobject jobj, jbyteArray jbuffer, jint jw, jint jh){
    NativeContext * ptr = getNativePtr(jniEnv, jobj);
    if(ptr && jbuffer){
        jbyte* buffer = jniEnv->GetByteArrayElements(jbuffer, NULL);
        ptr->liveMuxer.queueVideoFrame((const char *)buffer, jw * jh * 3 / 2);
        jniEnv->ReleaseByteArrayElements(jbuffer, buffer, 0);
    }
 }

