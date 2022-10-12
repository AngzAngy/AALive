//
// Created by Administrator on 2020/11/13.
//

#ifndef AALIVE_RTMPMUXER_H
#define AALIVE_RTMPMUXER_H

#include "codec/LiveMuxerInfo.h"
#include "IRtmpMuxer.h"
#include "AFramePool.h"
#include "Mutex.h"
#include <pthread.h>
#include <vector>
enum MuxerState {
    IDEL,
    PREPARED,
    OPENING,
    ERROR,
    CLOSED
};

class RtmpMuxer: public IRtmpMuxer{
public:
    RtmpMuxer();
    ~RtmpMuxer();

    bool open(const LiveMuxerInfo& muxerInfo);
    bool writeFrame(AFrame* pFrame);
    bool isConnected();
    bool close();
    void release();

    AFrame* obtionFrame();
    void backFrame(AFrame* pFrame);

private:
    static void* doPublish(void *userdata);
    bool doOpen();
    bool doWrite();
    bool doClose();
    bool writeVideoFrame(uint8_t *buf, int bufBytes, uint64_t dtsUS);
    bool writeAudioFrame(uint8_t *buf, int bufBytes, uint64_t dtsUS);
    LiveMuxerInfo mMuxerInfo;
    void * mRtmpHandler;
    AFramePool *mFramePool;
    AA::Mutex mPoolMux;
    vector<AFrame*> mFrameQueue;
    AA::Mutex mFrameQMux;
    AA::Condition mFrameQCondition;
    pthread_t  mThread;
    MuxerState state;
};
#endif //AALIVE_RTMPMUXER_H
