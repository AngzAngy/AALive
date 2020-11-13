#include "org_angzangy_aalive_muxer_RtmpMuxer.h"
#include "AALog.h"
#include "Mutex.h"
#include "JNIUtils.h"
#include "FFRtmpMuxer.h"
#include "RtmpMuxer.h"

#define P_MUXER(jptr) RtmpMuxer * pMuxer = (RtmpMuxer*)jptr

static jmethodID gArrayID = 0;
static AA::Mutex gMutex;
static void getMethodID(JNIEnv *jniEnv) {
  if(!gArrayID) {
    gMutex.lock();
    if(!gArrayID) {
      jclass byteBufclazz = jniEnv->FindClass("java/nio/ByteBuffer");
      if (byteBufclazz) {
        gArrayID = jniEnv->GetMethodID(byteBufclazz, "array", "()[B");
        LOGD("%s arrayId: %ld", __FUNCTION__, (long)gArrayID);
      }
    }
    gMutex.unlock();
  }
}
/*
 * Class:     org_angzangy_aalive_muxer_RtmpMuxer
 * Method:    init
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_org_angzangy_aalive_muxer_RtmpMuxer_init
  (JNIEnv *jniEnv, jobject jobj){
  getMethodID(jniEnv);
//  IRtmpMuxer * pMuxer = new FFRtmpMuxer;
    RtmpMuxer * pMuxer = new RtmpMuxer;
  return (jlong)(pMuxer);
}

/*
 * Class:     org_angzangy_aalive_muxer_RtmpMuxer
 * Method:    open
 * Signature: (JLjava/lang/String;II)I
 */
JNIEXPORT jint JNICALL Java_org_angzangy_aalive_muxer_RtmpMuxer_open
  (JNIEnv *jniEnv, jobject jobj, jlong jptr, jstring jurl, jint jvWidth, jint jvHeight) {
  P_MUXER(jptr);
  if(pMuxer && jurl) {
    const char* url = jniEnv->GetStringUTFChars(jurl, JNI_FALSE);
    LiveMuxerInfo liveMuxerInfo;
    liveMuxerInfo.muxerUri = url;
    liveMuxerInfo.videoSrcWidth = liveMuxerInfo.videoDstWidth = jvWidth;
    liveMuxerInfo.videoSrcHeight = liveMuxerInfo.videoDstWidth = jvHeight;
    bool ret =  pMuxer->open(liveMuxerInfo);
    jniEnv->ReleaseStringUTFChars(jurl, url);
    return ret ? 1 : 0;
  }
  if(!pMuxer) {
    std::string msg = "";
    msg.append(__FUNCTION__).append(" Native RtmpMuxer Object is null");
    LOGE("%s", msg.c_str());
    JNIUtils::throwIllegalStateException(jniEnv, msg);
  }
  if(!jurl) {
    std::string msg = "";
    msg.append(__FUNCTION__).append(" url is null");
    LOGE("%s", msg.c_str());
    JNIUtils::throwNullPointerException(jniEnv, msg);
  }
  return 0;
}

/*
 * Class:     org_angzangy_aalive_muxer_RtmpMuxer
 * Method:    writeVideo
 * Signature: (J[BIIJ)I
 */
JNIEXPORT jint JNICALL Java_org_angzangy_aalive_muxer_RtmpMuxer_writeVideo
  (JNIEnv *jniEnv, jobject jobj, jlong jptr, jbyteArray jbuf, jint offset, jint len, jlong jtimestamp) {
  P_MUXER(jptr);
  if(pMuxer && jbuf) {
    jbyte * buf = jniEnv->GetByteArrayElements(jbuf, NULL);
    bool ret = pMuxer->writeVideoFrame((uint8_t *)(&buf[offset]), len, jtimestamp);
    jniEnv->ReleaseByteArrayElements(jbuf, buf, 0);
    return ret ? 1 : 0;
  }
  if(!pMuxer) {
//    std::string msg(__FUNCTION__);
//    msg += " Native RtmpMuxer Object is null";
//    LOGE("%s", msg.c_str());
//    JNIUtils::throwIllegalStateException(jniEnv, msg);
      LOGE("%s Native RtmpMuxer Object is null", __FUNCTION__);
  }
  if(!jbuf) {
//    std::string msg(__FUNCTION__);
//    msg += " byteArray is null";
//    LOGE("%s", msg.c_str());
//    JNIUtils::throwNullPointerException(jniEnv, msg);
    LOGE("%s Native byteArray is null", __FUNCTION__);
  }
  return 0;
}

/*
 * Class:     org_angzangy_aalive_muxer_RtmpMuxer
 * Method:    writeNioVideo
 * Signature: (JLjava/nio/ByteBuffer;IIJ)I
 */
JNIEXPORT jint JNICALL Java_org_angzangy_aalive_muxer_RtmpMuxer_writeNioVideo
  (JNIEnv *jniEnv, jobject jobj, jlong jptr,
          jobject jBuf, jint joffset, jint jsize,
          jlong jtimestamp) {
  P_MUXER(jptr);
  if(pMuxer && jBuf) {
    jbyteArray jbyteArrObj = NULL;
    void *dst = jniEnv->GetDirectBufferAddress(jBuf);
    if(NULL == dst) {
      jbyteArrObj = (jbyteArray)jniEnv->CallObjectMethod(jBuf, gArrayID);
      if(NULL == jbyteArrObj){
        std::string msg(__FUNCTION__);
        msg += " ByteBuffer#array() null";
        LOGE("%s", msg.c_str());
        JNIUtils::throwIllegalStateException(jniEnv, msg);
        return 0;
      }
      dst = jniEnv->GetByteArrayElements(jbyteArrObj, NULL);
    }
    if(NULL == dst) {
      std::string msg(__FUNCTION__);
      msg += " ByteBuffer content null";
      LOGE("%s", msg.c_str());
      JNIUtils::throwIllegalStateException(jniEnv, msg);
      return 0;
    }
    uint8_t *buf = (uint8_t *)dst + joffset;
    bool ret = pMuxer->writeVideoFrame(buf, jsize, jtimestamp);
    if(NULL != jbyteArrObj) {
      jniEnv->ReleaseByteArrayElements(jbyteArrObj, (jbyte*)dst, 0);
    }
    return ret ? 1 : 0;
  }
  if(!pMuxer) {
    std::string msg(__FUNCTION__);
    msg += " Native RtmpMuxer Object is null";
    LOGE("%s", msg.c_str());
    JNIUtils::throwIllegalStateException(jniEnv, msg);
  }
  if(!jBuf) {
    std::string msg(__FUNCTION__);
    msg += " ByteBuffer is null";
    LOGE("%s", msg.c_str());
    JNIUtils::throwNullPointerException(jniEnv, msg);
  }
  return 0;
}

/*
 * Class:     org_angzangy_aalive_muxer_RtmpMuxer
 * Method:    writeAudio
 * Signature: (J[BIIJ)I
 */
JNIEXPORT jint JNICALL Java_org_angzangy_aalive_muxer_RtmpMuxer_writeAudio
  (JNIEnv *jniEnv, jobject jobj, jlong jptr, jbyteArray jbuf, jint offset, jint len, jlong jtimestamp) {
  P_MUXER(jptr);
  if(pMuxer && jbuf) {
    jbyte * buf = jniEnv->GetByteArrayElements(jbuf, NULL);
    bool ret = pMuxer->writeAudioFrame((uint8_t *)(&buf[offset]), len, jtimestamp);
    jniEnv->ReleaseByteArrayElements(jbuf, buf, 0);
    return ret ? 1 : 0;
  }
  if(!pMuxer) {
    std::string msg(__FUNCTION__);
    msg += " Native RtmpMuxer Object is null";
    LOGE("%s", msg.c_str());
    JNIUtils::throwIllegalStateException(jniEnv, msg);
  }
  if(!jbuf) {
    std::string msg(__FUNCTION__);
    msg += " byteArray is null";
    LOGE("%s", msg.c_str());
    JNIUtils::throwNullPointerException(jniEnv, msg);
  }
  return 0;
}

/*
 * Class:     org_angzangy_aalive_muxer_RtmpMuxer
 * Method:    writeNioAudio
 * Signature: (JLjava/nio/ByteBuffer;IIJ)I
 */
JNIEXPORT jint JNICALL Java_org_angzangy_aalive_muxer_RtmpMuxer_writeNioAudio
  (JNIEnv *jniEnv, jobject jobj, jlong jptr,
          jobject jBuf, jint joffset, jint size,
          jlong jtimestamp) {
  P_MUXER(jptr);
  if(pMuxer && jBuf) {
    jbyteArray jbyteArrObj = NULL;
    void *dst = jniEnv->GetDirectBufferAddress(jBuf);
    if(NULL == dst) {
      jbyteArrObj = (jbyteArray)jniEnv->CallObjectMethod(jBuf, gArrayID);
      if(NULL == jbyteArrObj){
        std::string msg(__FUNCTION__);
        msg += " ByteBuffer#array() null";
        LOGE("%s", msg.c_str());
        JNIUtils::throwIllegalStateException(jniEnv, msg);
          return 0;
      }
      dst = jniEnv->GetByteArrayElements(jbyteArrObj, NULL);
    }
    if(NULL == dst) {
      std::string msg(__FUNCTION__);
      msg += " ByteBuffer content null";
      LOGE("%s", msg.c_str());
      JNIUtils::throwIllegalStateException(jniEnv, msg);
      return 0;
    }
    uint8_t *buf = (uint8_t *)dst + joffset;
    bool ret = pMuxer->writeAudioFrame(buf, size, jtimestamp);
    if(NULL != jbyteArrObj) {
      jniEnv->ReleaseByteArrayElements(jbyteArrObj, (jbyte*)dst, 0);
    }
    return ret ? 1 : 0;
  }
  if(!pMuxer) {
    std::string msg(__FUNCTION__);
    msg += " Native RtmpMuxer Object is null";
    LOGE("%s", msg.c_str());
    JNIUtils::throwIllegalStateException(jniEnv, msg);
  }
  if(!jBuf) {
    std::string msg(__FUNCTION__);
    msg += " ByteBuffer is null";
    LOGE("%s", msg.c_str());
    JNIUtils::throwNullPointerException(jniEnv, msg);
  }
  return 0;
}

/*
 * Class:     org_angzangy_aalive_muxer_RtmpMuxer
 * Method:    read
 * Signature: (J[BII)I
 */
JNIEXPORT jint JNICALL Java_org_angzangy_aalive_muxer_RtmpMuxer_read
  (JNIEnv *jniEnv, jobject jobj, jlong jptr, jbyteArray jbuf, jint offset, jint size){
  return 0;
}

/*
 * Class:     org_angzangy_aalive_muxer_RtmpMuxer
 * Method:    close
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_org_angzangy_aalive_muxer_RtmpMuxer_close
  (JNIEnv *jniEnv, jobject jobj, jlong jptr) {
  P_MUXER(jptr);
  if(pMuxer) {
    return pMuxer->close() ? 1 : 0;
  }
  if(!pMuxer) {
    std::string msg(__FUNCTION__);
    msg += " Native RtmpMuxer Object is null";
    LOGE("%s", msg.c_str());
    JNIUtils::throwIllegalStateException(jniEnv, msg);
  }
  return 0;
}

/*
 * Class:     org_angzangy_aalive_muxer_RtmpMuxer
 * Method:    isConnected
 * Signature: (J)Z
 */
JNIEXPORT jboolean JNICALL Java_org_angzangy_aalive_muxer_RtmpMuxer_isConnected
  (JNIEnv *jniEnv, jobject jobj, jlong jptr){
  P_MUXER(jptr);
  if(pMuxer) {
    return pMuxer->isConnected() ? JNI_TRUE : JNI_FALSE;
  } else {
    std::string msg(__FUNCTION__);
    msg += " Native RtmpMuxer Object is null";
    LOGE("%s", msg.c_str());
    JNIUtils::throwIllegalStateException(jniEnv, msg);
    return JNI_FALSE;
  }
}

/*
 * Class:     org_angzangy_aalive_muxer_RtmpMuxer
 * Method:    release
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_org_angzangy_aalive_muxer_RtmpMuxer_release
  (JNIEnv *jniEnv, jobject jobj, jlong jptr) {
  P_MUXER(jptr);
  if(pMuxer) {
    pMuxer->release();
    delete pMuxer;
    LOGE("%s success!", __FUNCTION__);
  } else {
    std::string msg(__FUNCTION__);
    msg += " Native RtmpMuxer Object is null";
    LOGE("%s", msg.c_str());
    JNIUtils::throwIllegalStateException(jniEnv, msg);
  }
}