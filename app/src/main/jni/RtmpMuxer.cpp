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

void RtmpMuxer::doPublish(void *userdata) {
    RtmpMuxer *rtmpMuxer = (RtmpMuxer *)userdata;
    if(!rtmpMuxer) {
        return;
    }
    switch (rtmpMuxer->state) {
        case PREPARED:
            rtmpMuxer->doOpen();
            LOGD("doPublish doOpen");
            break;
        case OPENING:
            LOGD("mybug doWrite");
            rtmpMuxer->doWrite();
            LOGD("doPublish doWrite");
            break;
        case CLOSED:
            LOGD("doPublish doClose in");
            rtmpMuxer->doClose();
            LOGD("doPublish doClose out");
            break;
        case ERROR:
            LOGD("doPublish Error");
            break;
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
    mMuxerInfo = muxerInfo;
    mRtmpHandler = rtmp_sender_alloc();
    if(!mRtmpHandler) {
        LOGE("%s err handler null url:%s",__FUNCTION__, muxerInfo.muxerUri.c_str());
        return false;
    }
    state = PREPARED;

    ThreadCB cb = {RtmpMuxer::doPublish, this};
    thread.start(cb);
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
    LOGE("mybug writeFrame in");
    if(pFrame) {
        mFrameQMux.lock();
        mFrameQueue.push_back(pFrame);
        LOGE("mybug writeFrame size: %d",mFrameQueue.size());
        mFrameQMux.unlock();
        mFrameQCondition.signal();
        return true;
    }
    LOGE("mybug writeFrame null");
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
    LOGD("mybug 0");
    if(isConnected()) {
        LOGD("mybug 1");
        AFrame *pFrame = nullptr;
        mFrameQMux.lock();
        if(mFrameQueue.empty()) {
            LOGD("mybug 2");
//            mFrameQCondition.wait(mFrameQMux);
            return false;
        }
        LOGD("mybug 3");
        if(!mFrameQueue.empty()) {
            std::sort(mFrameQueue.begin(), mFrameQueue.end(), frameCompare);

            pFrame = mFrameQueue.front();
            mFrameQueue.pop_front();
        }
        mFrameQMux.unlock();

        LOGD("mybug 4");
        if(pFrame && pFrame->buf) {
            const uint8_t *buf = (const uint8_t *)pFrame->buf;
            const int bufBytes = (const int)pFrame->sizeInBytes;
            const uint64_t dtsUS = (const uint64_t)pFrame->timestamp;
            switch (pFrame->type){
                case AudioFrame:
                    writeAudioFrame(buf, bufBytes, dtsUS);
                    LOGD("mybug 5");
                    break;
                case VideoFrame:
                    writeVideoFrame(buf, bufBytes, dtsUS);
                    LOGD("mybug 6");
                    break;
                default:
                    break;
            }
            backFrame(pFrame);
        }
    }
}


bool RtmpMuxer::doClose() {
    if(state == CLOSED) {
        if(mRtmpHandler) {
            rtmp_close(mRtmpHandler);
        }
        state = IDEL;
        thread.stop();
    }
}

bool RtmpMuxer::writeVideoFrame(const uint8_t *buf, const int bufBytes, const uint64_t dtsUS) {
    if(mRtmpHandler) {
        return 0 == rtmp_sender_write_video_frame(mRtmpHandler, (uint8_t *)buf, bufBytes, dtsUS, 0, 0);
    }
    return false;
}

bool RtmpMuxer::writeAudioFrame(const uint8_t *buf, const int bufBytes, const uint64_t dtsUS) {
    if(mRtmpHandler) {
        return 0 == rtmp_sender_write_audio_frame(mRtmpHandler, (uint8_t *)buf, bufBytes, dtsUS, 0);
    }
    return false;
}