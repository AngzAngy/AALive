//
// Created by Administrator on 2020/11/13.
//
#include "RtmpMuxer.h"
#include "AALog.h"
#include "codec/LiveMuxerInfo.h"
#include "rtmp/flvmuxer/xiecc_rtmp.h"

RtmpMuxer::RtmpMuxer():mRtmpHandler(nullptr){
    LOGD("%s Constructor",__FUNCTION__);
};

RtmpMuxer::~RtmpMuxer(){
    release();
};

bool RtmpMuxer::open(const LiveMuxerInfo& muxerInfo) {
    mMuxerInfo = muxerInfo;
    mRtmpHandler = rtmp_sender_alloc();
    if(!mRtmpHandler) {
        LOGE("%s err handler null url:%s",__FUNCTION__, muxerInfo.muxerUri.c_str());
        return false;
    }

    int ret = rtmp_open_for_write(mRtmpHandler, mMuxerInfo.muxerUri.c_str(),
                                  mMuxerInfo.videoDstWidth, mMuxerInfo.videoDstHeight);
    if (ret < 0) {
        LOGE("%s err(%d) url:%s,videosize(%d x %d)",__FUNCTION__, ret, mMuxerInfo.muxerUri.c_str(),mMuxerInfo.videoDstWidth, mMuxerInfo.videoDstHeight);
        return false;
    }
    LOGD("%s url:%s,videosize(%d x %d)",__FUNCTION__, mMuxerInfo.muxerUri.c_str(),mMuxerInfo.videoDstWidth, mMuxerInfo.videoDstHeight);
    return true;
}

bool RtmpMuxer::isConnected() {
    if(mRtmpHandler) {
        return 1 == rtmp_is_connected(mRtmpHandler);
    }
    return false;
}

bool RtmpMuxer::close() {
    LOGD("%s",__FUNCTION__);
    if(mRtmpHandler) {
        rtmp_close(mRtmpHandler);
    }
}

void RtmpMuxer::release() {
    if(mRtmpHandler) {
        rtmp_sender_free(mRtmpHandler);
        mRtmpHandler = nullptr;
    }
}

bool RtmpMuxer::writeFrame(AFrame* pFrame) {
    if(isConnected()) {
       if(pFrame && pFrame->buf) {
            LOGD("doWrite frameSize: %d", pFrame->sizeInBytes);
            switch (pFrame->type){
                case AudioFrame:
                    return writeAudioFrame((uint8_t *)pFrame->buf, pFrame->sizeInBytes, pFrame->timestamp);
                case VideoFrame:
                    return writeVideoFrame((uint8_t *)pFrame->buf, pFrame->sizeInBytes, pFrame->timestamp);
                default:
                    break;
            }
        }
    }
    return false;
}

bool RtmpMuxer::writeVideoFrame(uint8_t *buf, int bufBytes, uint64_t dtsUS) {
    if(mRtmpHandler) {
        return 0 == rtmp_sender_write_video_frame(mRtmpHandler, buf, bufBytes, dtsUS, 0, 0);
    }
    return false;
}

bool RtmpMuxer::writeAudioFrame(uint8_t *buf, int bufBytes, uint64_t dtsUS) {
    if(mRtmpHandler) {
        return 0 == rtmp_sender_write_audio_frame(mRtmpHandler, buf, bufBytes, dtsUS, 0);
    }
    return false;
}