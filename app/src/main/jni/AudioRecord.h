#include <stdio.h>
#include <assert.h>
#include <queue>
#include <vector>

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

#include "ABuffer.h"
#include "Mutex.h"
#include "AudioRecordState.h"

#ifndef __AudioRecord_H_
#define __AudioRecord_H_

#define NATIVE_AUDIORECORD_SUCCESS 0

#define ALIGN_VAL(value) (((value) + 4 - 1) & (~(4 - 1)))

typedef  void (*onFrameCallback)(void *buf, int32_t size, void* userData);
class AudioRecord{
public:
    static int64_t getAlignValue(int64_t value){
        int64_t alignVal = ALIGN_VAL(value);
        return alignVal;
    }

    static int32_t getAlignValue(int32_t value){
        int32_t alignVal = ALIGN_VAL(value);
        return alignVal;
    }

    AudioRecord(int sampleRate, int bytesPerSample, int channelNumbre, int minBufferSize);
    ~AudioRecord();
    int getBufferSize(){
    	return bufSize;
    }
    void *getUserData(){
    	return mUserData;
    }
    void setOnFrameCallback(onFrameCallback cb, void* userData);
    void start();
    void pause();
    void stop();
    void release();
    int getStatue(){
    	return mStatue;
    }
private:
    static SLuint32 convertSLSamplerate(int sampleRate);
    static void recBufferQueueCallback(SLAndroidSimpleBufferQueueItf queueItf, void *pContext);
    static void *processThreadFunc(void *);

    void releaseEngine();
    void releaseRecorder();
    ABuffer* createABuffer();
    void releaseListABuffers();
    void doSamples(SLAndroidSimpleBufferQueueItf queueItf);
    bool aBufferEnqueue(SLAndroidSimpleBufferQueueItf queueItf, ABuffer *sampledBuf);
    void doProcess();

    // engine interfaces
    SLObjectItf engineObject;
    SLEngineItf engineEngine;

    //audio record interfaces
    SLObjectItf recorderObject;
    SLRecordItf recordItf;
    SLAndroidSimpleBufferQueueItf recBuffQueueItf;
    SLAndroidConfigurationItf configItf;

    int mSampleRate;
    int mBytesPerSample;
    int mChannelNumber;

	uint32_t  bufSize;
    onFrameCallback mFrameCallback;
    void *mUserData;

    int mStatue;

    EAudioState audioState;
    std::vector<ABuffer*> bufList;//manager all buffers
    std::queue<ABuffer*> sampleBufQ;//Opensl enqueue buffers
    AA::Mutex sampleMutex;
    std::queue<ABuffer*> processBufQ;//processed buffers,such as do denoise,write file.
    AA::Mutex processMutex;
    AA::Condition processCond;
};

#endif
