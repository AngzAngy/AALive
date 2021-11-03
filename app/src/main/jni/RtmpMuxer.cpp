//
// Created by Administrator on 2020/11/13.
//
#include "RtmpMuxer.h"
#include "AALog.h"
#include "codec/LiveMuxerInfo.h"

extern "C" {
#include "rtmp/flvmuxer/xiecc_rtmp.h"
}

RtmpMuxer::RtmpMuxer():mRtmpHandler(NULL){
    LOGD("%s Constructor",__FUNCTION__);
};

RtmpMuxer::~RtmpMuxer(){
    release();
};

bool RtmpMuxer::open(const LiveMuxerInfo& muxerInfo) {
    mMuxerInfo = muxerInfo;
    mRtmpHandler = rtmp_sender_alloc();
    if(!mRtmpHandler) {
        LOGD("%s err handler null url:%s",__FUNCTION__, muxerInfo.muxerUri.c_str());
        return false;
    }
    int ret = rtmp_open_for_write(mRtmpHandler, muxerInfo.muxerUri.c_str(), muxerInfo.videoDstWidth, muxerInfo.videoDstHeight);
    if(0 != ret) {
        LOGD("%s err(%d) url:%s",__FUNCTION__, ret, muxerInfo.muxerUri.c_str());
//        return false;
    }
    LOGD("%s success url:%s",__FUNCTION__, muxerInfo.muxerUri.c_str());
    return true;
}

bool RtmpMuxer::isConnected() {
    if(!mRtmpHandler) {
        return false;
    }
    return 1 == rtmp_is_connected(mRtmpHandler);
}

bool RtmpMuxer::close() {
    if(mRtmpHandler) {
        rtmp_close(&mRtmpHandler);
        mRtmpHandler = NULL;
    }
}

void RtmpMuxer::release() {
}

bool RtmpMuxer::writeVideoFrame(const uint8_t *buf, const int bufBytes, uint64_t dtsUS) {
    if(mRtmpHandler) {
        return 0 == rtmp_sender_write_video_frame(mRtmpHandler, (uint8_t *)buf, bufBytes, dtsUS, 0, 0);
    }
    return false;
}

bool RtmpMuxer::writeAudioFrame(const uint8_t *buf, const int bufBytes, uint64_t dtsUS) {
    if(mRtmpHandler) {
        return 0 == rtmp_sender_write_audio_frame(mRtmpHandler, (uint8_t *)buf, bufBytes, dtsUS, 0);
    }
    return false;
}