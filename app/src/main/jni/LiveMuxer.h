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

#include <vector>
#include "Thread.h"
#include "Mutex.h"
#include "LiveMuxerInfo.h"
#include "AACEncoder.h"
#include "H264Encoder.h"
#include "libyuv.h"

#ifndef AALIVE_LIVEMUXER_H
#define AALIVE_LIVEMUXER_H

class LiveMuxer{
public:
    static void audioFrameCallback(void *buf, int32_t size, void* userData);
    LiveMuxer();
    ~LiveMuxer();
    void setMuxerInfo(const LiveMuxerInfo& muxerInfo);
    void queueVideoFrame(const char* y, const char* vu,
                         const int width, const int height);
    void queueVideoFrame(const char* rgba,
                         const int width, const int height);
    void queueAudioFrame(const char* buf, const int bufBytes);

    bool start();
    bool stop();
    void release();
private:
    static void aEncodeThreadCallback(void *pMuxer);
    static void vEncodeThreadCallback(void *pMuxer);

    bool encodeAudioFrame(AVPacket *avpkt);
    bool encodeVideoFrame(AVPacket *avpkt);
    bool writeMuxerFrame(AVPacket *pPacket, bool bIsAudio);
    AVFrame* allocAudioFrame(int frameSize);
    AVFrame* allocVideoFrame();
    LiveMuxerInfo mMuxerInfo;
    AVFormatContext *mFormatContext;
    AVStream *mAudioStream;
    AVStream *mVideoStream;

    AACEncoder mAudioEncoder;
    H264Encoder mVideoEncoder;

    Thread mAEncoderThread;
    Thread mVEncoderThread;

    AA::Mutex mMuxerMutex;

    std::vector<AVFrame*> mAudioFrames;
    int mAudioWritePos;
    int mAudioReadPos;
    int mAudioFramesCount;
    int64_t mAudioArrivedTime;
    int64_t	mAudioBeginTime;
    AA::Mutex mAudioFramesMutex;
    AA::Condition mAudioFramesCondition;
    std::vector<AVFrame*> mVideoFrames;
    int mVideoWritePos;
    int mVideoReadPos;
    int mVideoFramesCount;
    int64_t mVideoEncodedCount;
    AA::Mutex mVideoFramesMutex;
    AA::Condition mVideoFramesCondition;
};

#endif //AALIVE_LIVEMUXER_H
