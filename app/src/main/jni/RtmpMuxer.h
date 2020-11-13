//
// Created by Administrator on 2020/11/13.
//

#ifndef AALIVE_RTMPMUXER_H
#define AALIVE_RTMPMUXER_H

#include "IRtmpMuxer.h"

class RtmpMuxer: public IRtmpMuxer{
public:
    RtmpMuxer();
    ~RtmpMuxer();

    bool open(const LiveMuxerInfo& muxerInfo);
    bool isConnected();
    bool close();
    void release();

    bool writeVideoFrame(const uint8_t *buf, const int bufBytes, uint64_t dtsUS);
    bool writeAudioFrame(const uint8_t *buf, const int bufBytes, uint64_t dtsUS);
private:
    LiveMuxerInfo mMuxerInfo;
    void * mRtmpHandler;
};
#endif //AALIVE_RTMPMUXER_H
