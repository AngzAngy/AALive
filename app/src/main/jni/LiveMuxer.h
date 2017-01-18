//
// Created on 2017/1/18.
//
extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavutil/channel_layout.h>
#include <libavutil/common.h>
#include <libavutil/imgutils.h>
#include <libavutil/mathematics.h>
#include <libavutil/samplefmt.h>
#include <libswscale/swscale.h>
}

#include "Thread.h"
#include "Mutex.h"
#include "LiveMuxerInfo.h"
#include "AACEncoder.h"
#include "H264Encoder.h"

#ifndef AALIVE_LIVEMUXER_H
#define AALIVE_LIVEMUXER_H

class LiveMuxer{
public:
    LiveMuxer();
    ~LiveMuxer();
    void setMuxerInfo(const LiveMuxerInfo& muxerInfo);

    bool start();
    bool stop();
    void release();
private:
    bool writeMuxerFrame(AVPacket *pPacket, bool bIsAudio);
    LiveMuxerInfo mMuxerInfo;
    AVFormatContext *mFormatContext;
    AVStream *mAudioStream;
    AVStream *mVideoStream;

    AACEncoder audioEncoder;
    H264Encoder videoEncoder;

    Thread mAEncoderThread;
    Thread mVEncoderThread;

    AA::Mutex mMuxerMutex;
};

#endif //AALIVE_LIVEMUXER_H
