//
// Created by Administrator on 2020/11/13.
//
#include "RtmpMuxer.h"
#include "AALog.h"
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
    mRtmpHandler = rtmp_sender_alloc(muxerInfo.muxerUri.c_str());
    if(!mRtmpHandler) {
        LOGD("%s err handler null url:%s",__FUNCTION__, muxerInfo.muxerUri.c_str());
        return false;
    }
    int ret = rtmp_sender_start_publish(mRtmpHandler, RTMP_STREAM_PROPERTY_PUBLIC, 0);
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
    return 1 == rtmp_sender_is_connected(mRtmpHandler);
}

bool RtmpMuxer::close() {
    if(mRtmpHandler) {
        rtmp_sender_stop_publish(mRtmpHandler);
    }
}

void RtmpMuxer::release() {
    if(mRtmpHandler) {
        rtmp_sender_free(mRtmpHandler);
        mRtmpHandler = NULL;
    }
}

bool RtmpMuxer::writeVideoFrame(const uint8_t *buf, const int bufBytes, uint64_t dtsUS) {
    if(mRtmpHandler) {
        return 0 == rtmp_sender_write_video_frame(mRtmpHandler,
                                          (uint8_t *)buf, bufBytes, dtsUS, 0, 0);
    }
    return false;
}

bool RtmpMuxer::writeAudioFrame(const uint8_t *buf, const int bufBytes, uint64_t dtsUS) {
    if(mRtmpHandler) {
        return 0 == rtmp_sender_write_audio_frame(mRtmpHandler,
                            (uint8_t *)buf, bufBytes, dtsUS, 0);
    }
    return false;
}