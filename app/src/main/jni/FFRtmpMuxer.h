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
#include "LiveMuxerInfo.h"
#include "IRtmpMuxer.h"

#ifndef AAFF_RTMP_MUXER_H
#define AAFF_RTMP_MUXER_H

class Nalu{
public:
    Nalu():startCodeBytes(0),buf(NULL),bytes(0),type(0){}
    int startCodeBytes;
    uint8_t *buf;
    int bytes;
    int type;
    int totalBytes() {
        return bytes + startCodeBytes;
    }
};

class FFRtmpMuxer: public IRtmpMuxer{
public:
    FFRtmpMuxer();
    ~FFRtmpMuxer();

    bool open(const LiveMuxerInfo& muxerInfo);
    bool isConnected();
    bool close();
    void release();

    bool writeVideoFrame(const uint8_t *buf, const int bufBytes, uint64_t dtsUS);
    bool writeAudioFrame(const uint8_t *buf, const int bufBytes, uint64_t dtsUS);
private:

    bool writeMuxerFrame(AVPacket *pPacket, bool bIsAudio);
    LiveMuxerInfo mMuxerInfo;
    AVFormatContext *mFormatContext;
    AVStream *mAudioStream;
    AVStream *mVideoStream;
    bool hasSpsNalu;
    bool connected;
};

#endif //AAFF_RTMP_MUXER_H
