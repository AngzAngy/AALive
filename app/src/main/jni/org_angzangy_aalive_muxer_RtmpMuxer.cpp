#include "org_angzangy_aalive_muxer_RtmpMuxer.h"
#include "AALog.h"
#include "Mutex.h"
#include "JNIUtils.h"
#include "FFRtmpMuxer.h"
#include "RtmpMuxer.h"
#include "buffer/ATimestampBuffer.h"
#include "buffer/ABufferCallback.h"
#include "codec/LiveMuxerInfo.h"
#include "codec/MediaCodecAACEncoder.h"
#include "AudioRecord.h"
#include "CommonGlobaldef.h"
#include "TimeIndexCounter.h"
#include <cstdint>
#include <fstream>

//#define DUMP_ADTS 1

#define P_MUXER(jptr) RtmpContext * pMuxer = (RtmpContext*)jptr

class RtmpContext{
public:
    static void onPCMCallback(void *buf, int32_t size, void* userData);
    RtmpContext();
    ~RtmpContext();
    bool open(LiveMuxerInfo &muxerInfo);
    bool receiveEncoderBuffer(void *buf, int sizeBytes, uint64_t timestamp, FrameType type);
    void receiveAdtsFrame();
    MedaiCodecAACEncoder aacEncoder;
    TimeIndexCounter aacDtsCounter;
    Thread aacEncoderThread;
    AudioRecord *audioRecord;
    RtmpMuxer rtmpMuxer;
#ifdef DUMP_ADTS
    std::fstream ofs;
#endif
};

void RtmpContext::onPCMCallback(void *buf, int32_t size, void* userData) {
    LOGD("%s : audio record", __FUNCTION__);
    RtmpContext *pContext = (RtmpContext *)userData;
    if(pContext) {
        ATimestampBuffer buffer;
        buffer.timestamp = currentUsec();
        buffer.sizeInBytes = size;
        buffer.buf = buf;
        pContext->aacEncoder.sendBuffer(buffer);

        pContext->receiveAdtsFrame();
    }
}

RtmpContext::RtmpContext():audioRecord(nullptr){
#ifdef DUMP_ADTS
    ofs.open("/sdcard/DCIM/dump.aac", std::ios::out | std::ios::binary);
#endif
}

bool RtmpContext::open(LiveMuxerInfo &muxerInfo) {
    if(rtmpMuxer.open(muxerInfo)) {

        aacEncoder.init(muxerInfo);
        aacEncoder.start();

        audioRecord = new AudioRecord(muxerInfo.audioSampleRate, muxerInfo.audioBytesPerSample,
                                      muxerInfo.audioChannelNumber, 1024);

        audioRecord->setOnFrameCallback(RtmpContext::onPCMCallback, this);
        audioRecord->start();

        return true;
    }
    return false;
}

RtmpContext::~RtmpContext() {
    aacEncoderThread.stop();
    aacEncoder.release();
    if(audioRecord) {
        audioRecord->release();
        delete audioRecord;
        audioRecord = nullptr;
    }
    rtmpMuxer.release();
}

void RtmpContext::receiveAdtsFrame() {
  AFrame *pFrame = rtmpMuxer.obtionFrame();
  LOGE("mybug receiveAdtsFrame Frame: %p", pFrame);
  if(pFrame) {
    pFrame->type = AudioFrame;

    pFrame->bufferAlloc = &gDefABufferAlloc;

    aacEncoder.receiveBuffer(*pFrame);

    LOGE("mybug receiveAdtsFrame 0");
    if (pFrame->sizeInBytes > 0 && pFrame->buf) {
        aacDtsCounter.calcTotalTime(pFrame->timestamp);
        pFrame->timestamp = aacDtsCounter.getTimeIndex();
#ifdef DUMP_ADTS
      ofs.write((char*)(pFrame->buf), pFrame->sizeInBytes);
#endif

        bool ret = rtmpMuxer.writeFrame(pFrame);
//        LOGD("callback rtmpMuxerWriteAudio dts: %lld", dtsUS);
//        LOGD("callback rtmpMuxerWriteAudio buf0: %02X, buf1: %02X", buf[0], buf[1]);
    }
  }
}

bool RtmpContext::receiveEncoderBuffer(void *buf, int sizeBytes, uint64_t timestamp, FrameType type) {
    AFrame *pF = rtmpMuxer.obtionFrame();
  LOGE("mybug receiveEncoderBuffer Frame: %p", pF);
    if(pF) {
      pF->type = type;
      pF->timestamp = timestamp;
      pF->bufferAlloc = &gDefABufferAlloc;

      if(pF->capacityInBytes < sizeBytes || !pF->buf) {
          pF->free();
          pF->alloc(sizeBytes);
      }
      if(pF->buf) {
        LOGE("mybug receiveEncoderBuffer 0");
          pF->sizeInBytes = sizeBytes;
          memcpy((pF->buf), buf, sizeBytes);
          return rtmpMuxer.writeFrame(pF);
      }
    }
    return false;
}

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
RtmpContext * pMuxer = new RtmpContext;
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
    liveMuxerInfo.videoSrcHeight = liveMuxerInfo.videoDstHeight = jvHeight;
    liveMuxerInfo.audioSampleRate = 44100;
    liveMuxerInfo.audioChannelNumber = 1;
    liveMuxerInfo.audioBytesPerSample = 2;
    liveMuxerInfo.audioBitrate=128000;
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
    bool ret = pMuxer->receiveEncoderBuffer((void *)(&buf[offset]), len, jtimestamp, VideoFrame);
    jniEnv->ReleaseByteArrayElements(jbuf, buf, 0);
    return ret ? 1 : 0;
  }
  if(!pMuxer) {
    std::string msg(__FUNCTION__);
    msg += " Native RtmpMuxer Object is null";
    LOGE("%s", msg.c_str());
    JNIUtils::throwIllegalStateException(jniEnv, msg);
//      LOGE("%s Native RtmpMuxer Object is null", __FUNCTION__);
  }
  if(!jbuf) {
    std::string msg(__FUNCTION__);
    msg += " byteArray is null";
    LOGE("%s", msg.c_str());
    JNIUtils::throwNullPointerException(jniEnv, msg);
//    LOGE("%s Native byteArray is null", __FUNCTION__);
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
    bool ret = pMuxer->receiveEncoderBuffer((void *)buf, jsize, jtimestamp, VideoFrame);
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
    bool ret = pMuxer->receiveEncoderBuffer((void *)(&buf[offset]), len, jtimestamp, AudioFrame);
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
    bool ret = pMuxer->receiveEncoderBuffer((void *)buf, size, jtimestamp, AudioFrame);
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
#ifdef DUMP_ADTS
      pMuxer->ofs.close();
#endif
    return pMuxer->rtmpMuxer.close() ? 1 : 0;
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
    return pMuxer->rtmpMuxer.isConnected() ? JNI_TRUE : JNI_FALSE;
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
    delete pMuxer;
    LOGE("%s success!", __FUNCTION__);
  } else {
    std::string msg(__FUNCTION__);
    msg += " Native RtmpMuxer Object is null";
    LOGE("%s", msg.c_str());
    JNIUtils::throwIllegalStateException(jniEnv, msg);
  }
}