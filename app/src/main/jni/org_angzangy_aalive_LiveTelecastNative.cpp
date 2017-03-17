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
    int audioBytesPreSample = 2;
    int audioChannel = 1;
    ////this should be attention for the sample count,
    ////aac should be 1024
    int bytespreframe = 1024 * audioBytesPreSample * audioChannel;
    ptr->liveMuxerInfo.audioSampleRate = 44100;
    ptr->liveMuxerInfo.audioBytesPerSample = audioBytesPreSample;
    ptr->liveMuxerInfo.audioChannelNumber = audioChannel;
    ptr->liveMuxerInfo.audioBitrate = 32000;
    AudioRecord *pAudioRecord = new AudioRecord(ptr->liveMuxerInfo.audioSampleRate, ptr->liveMuxerInfo.audioBytesPerSample,
                                                ptr->liveMuxerInfo.audioChannelNumber, bytespreframe);
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
        ptr->releaseVideoRawBuf();
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
        (JNIEnv *jniEnv, jobject jobj, jint jpreviewWidth, jint jpreviewHeight){
    NativeContext * ptr = getNativePtr(jniEnv, jobj);
    if(ptr){
        //we need rotate preview
        ptr->liveMuxerInfo.videoSrcWidth = jpreviewWidth;
        ptr->liveMuxerInfo.videoSrcHeight = jpreviewHeight;
        ptr->liveMuxerInfo.videoDstWidth = jpreviewWidth;
        ptr->liveMuxerInfo.videoDstHeight = jpreviewHeight;
        ptr->liveMuxerInfo.voideoBitrate = 128 * 1024;
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
JNIEXPORT void JNICALL Java_org_angzangy_aalive_LiveTelecastNative_pushNV21Buffer___3BII
  (JNIEnv *jniEnv, jobject jobj, jbyteArray jbuffer, jint jw, jint jh){
    NativeContext * ptr = getNativePtr(jniEnv, jobj);
    if(ptr && jbuffer){
        jbyte* buffer = jniEnv->GetByteArrayElements(jbuffer, NULL);
        ptr->liveMuxer.queueVideoFrame((const char *)buffer, (const char *)buffer + jw * jh, jw, jh);
        jniEnv->ReleaseByteArrayElements(jbuffer, buffer, 0);
    }
 }

 /*
  * Class:     org_angzangy_aalive_LiveTelecastNative
  * Method:    pushNV21Buffer
  * Signature: (Ljava/nio/ByteBuffer;Ljava/nio/ByteBuffer;II)V
  */
 JNIEXPORT void JNICALL Java_org_angzangy_aalive_LiveTelecastNative_pushNV21Buffer__Ljava_nio_ByteBuffer_2Ljava_nio_ByteBuffer_2II
   (JNIEnv *jniEnv, jobject jobj, jobject ybufobj, jobject vubufobj, jint jw, jint jh){
    NativeContext * ptr = getNativePtr(jniEnv, jobj);
    if(ptr && ybufobj && vubufobj){
        char * y = (char *)jniEnv->GetDirectBufferAddress(ybufobj);
        char * vu = (char *)jniEnv->GetDirectBufferAddress(vubufobj);
        ptr->liveMuxer.queueVideoFrame(y, vu, jw, jh);
    }
}

  /*
   * Class:     org_angzangy_aalive_LiveTelecastNative
   * Method:    readFbo
   * Signature: (II)V
   */
  JNIEXPORT void JNICALL Java_org_angzangy_aalive_LiveTelecastNative_readFbo
    (JNIEnv *jniEnv, jobject jobj, jint jfboWidth, jint jfboHeight){

    NativeContext * ptr = getNativePtr(jniEnv, jobj);
    if(ptr){
        int rgbabufsize = jfboWidth * jfboHeight * 4;
        if(ptr->videoRawBuf == NULL || ptr->videoRawBufBytes != rgbabufsize){
            ptr->releaseVideoRawBuf();
            ptr->allocVideoRawBuf(rgbabufsize);
        }
        char *pixels = ptr->videoRawBuf;
        glReadPixels (0, 0, (GLsizei)jfboWidth, (GLsizei)jfboHeight, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)pixels);
        ptr->liveMuxer.queueVideoFrame((const char*)pixels,jfboWidth, jfboHeight);
    }
}
