//
// Created by Administrator on 2020/11/13.
//
#include "RtmpMuxer.h"
#include "AALog.h"
#include "codec/LiveMuxerInfo.h"
#include <algorithm>

extern "C" {
#include "rtmp/flvmuxer/xiecc_rtmp.h"
}

static bool frameCompare(AFrame* left, AFrame* right) {
    if(left && right) {
        return left->timestamp < right->timestamp;
    }
    return false;
}

void* RtmpMuxer::doPublish(void *userdata) {
    while(true) {
        RtmpMuxer *rtmpMuxer = (RtmpMuxer *)userdata;
        if(!rtmpMuxer) {
            LOGE("doPublish rtmpMuxer null");
            return nullptr;
        }
        LOGD("doPublish state: %d", rtmpMuxer->state);
        switch (rtmpMuxer->state) {
            case PREPARED:
                LOGD("doPublish doOpen");
                rtmpMuxer->doOpen();
                break;
            case OPENING:
                LOGD("doPublish doWrite");
                rtmpMuxer->doWrite();
                break;
//            case CLOSED:
//                rtmpMuxer->doClose();
//                LOGD("doPublish doClose out");
//                return nullptr;
//            case ERROR:
//                LOGD("doPublish Error");
//                return nullptr;
//            case IDEL:
//                LOGD("doPublish Idel");
//                return nullptr;
        }
    }
}

RtmpMuxer::RtmpMuxer():mRtmpHandler(nullptr),mFramePool(nullptr),state(IDEL){
    LOGD("%s Constructor",__FUNCTION__);
    mFramePool = new AFramePool(8);
};

RtmpMuxer::~RtmpMuxer(){
    release();
};

bool RtmpMuxer::open(const LiveMuxerInfo& muxerInfo) {
    LOGD("%s",__FUNCTION__);
    if(state == PREPARED || state == OPENING) {
        LOGD("%s has opened",__FUNCTION__);
        return true;
    }
    mMuxerInfo = muxerInfo;
    mRtmpHandler = rtmp_sender_alloc();
    if(!mRtmpHandler) {
        LOGE("%s err handler null url:%s",__FUNCTION__, muxerInfo.muxerUri.c_str());
        return false;
    }
    state = PREPARED;

    pthread_create(&mThread, NULL, RtmpMuxer::doPublish, this);
    return true;
}

bool RtmpMuxer::isConnected() {
    if(mRtmpHandler && state == OPENING) {
        return 1 == rtmp_is_connected(mRtmpHandler);
    }
    return false;
}

bool RtmpMuxer::close() {
    LOGD("call close doClose");
    state = CLOSED;
    mFrameQCondition.signal();
}

void RtmpMuxer::release() {
    if(mRtmpHandler) {
        rtmp_sender_free(mRtmpHandler);
        mRtmpHandler = nullptr;
    }
    AA::Lock(mPoolMux, true);
    delete mFramePool;
    mFramePool = nullptr;
}

AFrame* RtmpMuxer::obtionFrame() {
//    AA::Lock(mPoolMux, true);
//    if(mFramePool) {
//        return mFramePool->acquireFrame();
//    }
//    return nullptr;
    return AFrame::allocFrame();
}

void RtmpMuxer::backFrame(AFrame* pFrame) {
//    AA::Lock(mPoolMux, true);
//    if(mFramePool) {
//        mFramePool->releaseFrame(pFrame);
//    }
    if(pFrame) {
        AFrame::freeFrame(pFrame);
    }
}

bool RtmpMuxer::writeFrame(AFrame* pFrame) {
    AA::Lock lock(mFrameQMux);
    if(pFrame) {
        mFrameQueue.push_back(pFrame);
        LOGD("writeFrame size: %d",mFrameQueue.size());
//        mFrameQCondition.signal();
        return true;
    }
    return false;
}

bool RtmpMuxer::doOpen() {
    if(state == PREPARED && mRtmpHandler) {
        int ret = rtmp_open_for_write(mRtmpHandler, mMuxerInfo.muxerUri.c_str(),
                                      mMuxerInfo.videoDstWidth, mMuxerInfo.videoDstHeight);
        if (ret < 0) {
            state = ERROR;
            LOGE("%s err(%d) url:%s,videosize(%d x %d)",__FUNCTION__, ret, mMuxerInfo.muxerUri.c_str(),mMuxerInfo.videoDstWidth, mMuxerInfo.videoDstHeight);
            return false;
        }
        state = OPENING;
        LOGD("%s success url:%s", __FUNCTION__, mMuxerInfo.muxerUri.c_str());
        return true;
    }
    return false;
}

bool RtmpMuxer::doWrite() {
//    AA::Lock lock(mFrameQMux);
//    if(isConnected()) {
//        AFrame *pFrame = nullptr;
//        if(mFrameQueue.empty()) {
//            mFrameQCondition.wait(mFrameQMux);
//        }
//        if(!mFrameQueue.empty()) {
//            std::sort(mFrameQueue.begin(), mFrameQueue.end(), frameCompare);

//            pFrame = mFrameQueue[0];
//            mFrameQueue.erase(mFrameQueue.begin(), mFrameQueue.begin() + 1);
//        }

//        if(pFrame && pFrame->buf) {
//            LOGD("doWrite frameSize: %d", pFrame->sizeInBytes);
//            switch (pFrame->type){
//                case AudioFrame:
//                    LOGD("mybug 50");
//                    writeAudioFrame((uint8_t *)pFrame->buf, pFrame->sizeInBytes, pFrame->timestamp);
//                    LOGD("mybug 5");
//                    break;
//                case VideoFrame:
//                    LOGD("mybug 60");
//                    writeVideoFrame((uint8_t *)pFrame->buf, pFrame->sizeInBytes, pFrame->timestamp);
//                    LOGD("mybug 6");
//                    break;
//                default:
//                    break;
//            }
//        }
//        backFrame(pFrame);
//    }
}


bool RtmpMuxer::doClose() {
    if(state == CLOSED) {
        if(mRtmpHandler) {
            rtmp_close(mRtmpHandler);
        }
        state = IDEL;
    }
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