//
// Created by Administrator on 2020/11/13.
//

#ifndef AALIVE_RTMPMUXER_H
#define AALIVE_RTMPMUXER_H

#include "codec/LiveMuxerInfo.h"
#include "IRtmpMuxer.h"

class RtmpMuxer: public IRtmpMuxer{
public:
    RtmpMuxer();
    ~RtmpMuxer();

    bool open(const LiveMuxerInfo& muxerInfo);
    bool writeFrame(AFrame* pFrame);
    bool isConnected();
    bool close();
    void release();

private:
    bool writeVideoFrame(uint8_t *buf, int bufBytes, uint64_t dtsUS);
    bool writeAudioFrame(uint8_t *buf, int bufBytes, uint64_t dtsUS);
    LiveMuxerInfo mMuxerInfo;
    void * mRtmpHandler;
};
#endif //AALIVE_RTMPMUXER_H
