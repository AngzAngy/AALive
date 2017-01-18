//
// Created on 2017/1/18.
//
#include "LiveMuxer.h"
#include "AALog.h"

static int decode_interrupt_cb(void *pMuxer) {
    return false;
}

static void aEncodeThreadCallback(void *pMuxer){
    LiveMuxer *pLiveMuxer = (LiveMuxer*)pLiveMuxer;
    if(pLiveMuxer){

    }
}

static void vEncodeThreadCallback(void *pMuxer){
    LiveMuxer *pLiveMuxer = (LiveMuxer*)pLiveMuxer;
    if(pLiveMuxer){

    }
}

LiveMuxer::LiveMuxer():mFormatContext(NULL),
                       mAudioStream(NULL),
                       mVideoStream(NULL){
}

LiveMuxer::~LiveMuxer() {
    release();
}

void LiveMuxer::setMuxerInfo(const LiveMuxerInfo& muxerInfo){
    mMuxerInfo = muxerInfo;
}

bool LiveMuxer::start() {
    if( NULL != mFormatContext){
        return true;
    }
    AVIOContext * avioContext = NULL;

    av_register_all();
    avcodec_register_all();
    avformat_network_init();

    if(!audioEncoder.init()){
        release();
        LOG_ERROR("%s aencoder init err", __FUNCTION__);
        return false;
    }
    if(!videoEncoder.init()){
        release();
        LOG_ERROR("%s vencoder init err", __FUNCTION__);
        return false;
    }

    if(avio_open(&avioContext, mMuxerInfo.muxerUri.c_str(), AVIO_FLAG_WRITE) < 0){
        release();
        LOG_ERROR("%s io open error: %s", __FUNCTION__, mMuxerInfo.muxerUri.c_str());
        return false;
    }

    //avformat_alloc_output_context2(&mFormatContext, NULL, "mpegts", mMuxerInfo.muxerUri.c_str());//UDP
    //RTMP
    if(avformat_alloc_output_context2(&mFormatContext, NULL,
                                   "flv", mMuxerInfo.muxerUri.c_str()) < 0){
        release();
        LOG_ERROR("%s fmt alloc err: %s", __FUNCTION__, mMuxerInfo.muxerUri.c_str());
        return false;
    }
    mFormatContext->interrupt_callback.callback = decode_interrupt_cb;
    mFormatContext->interrupt_callback.opaque = this;
    mFormatContext->pb = avioContext;
    memcpy(mFormatContext->filename, mMuxerInfo.muxerUri.c_str(), mMuxerInfo.muxerUri.size());

    mAudioStream = avformat_new_stream(mFormatContext, audioEncoder.getAVCodec());
    if(!mAudioStream){
        release();
        LOG_ERROR("%s new astream err", __FUNCTION__);
        return false;
    }
    mAudioStream->time_base.den = mMuxerInfo.audioSampleRate;
    mAudioStream->time_base.num = 1;
    mAudioStream->codec->codec_tag = 0;
    if (mFormatContext->oformat->flags & AVFMT_GLOBALHEADER) {
        mAudioStream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
    }

    mVideoStream = avformat_new_stream(mFormatContext, videoEncoder.getAVCodec());
    if(!mVideoStream){
        release();
        LOG_ERROR("%s new vstream err", __FUNCTION__);
        return false;
    }
    mVideoStream->time_base.den = 1000;
    mVideoStream->time_base.num = 1;
    mVideoStream->codec->codec_tag = 0;
    if (mFormatContext->oformat->flags & AVFMT_GLOBALHEADER) {
        mVideoStream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
    }

    if(avformat_write_header(mFormatContext, NULL) < 0){
        release();
        LOG_ERROR("%s write header err", __FUNCTION__);
        return false;
    }
    ThreadCB aThreadCB;
    aThreadCB.callback = aEncodeThreadCallback;
    aThreadCB.opaque = this;
    mAEncoderThread.start(aThreadCB);

    ThreadCB vThreadCB;
    vThreadCB.callback = vEncodeThreadCallback;
    vThreadCB.opaque = this;
    mVEncoderThread.start(vThreadCB);

    return true;
}

bool LiveMuxer::stop(){
    if(mFormatContext){
       av_write_trailer(mFormatContext);
        return true;
    }
    return false;
}

void LiveMuxer::release() {
    audioEncoder.release();
    videoEncoder.release();
    if(mFormatContext){
        if(mFormatContext->pb) {
            avio_closep(&mFormatContext->pb);
            mFormatContext->pb = NULL;
        }
        avformat_free_context(mFormatContext);
        mFormatContext = NULL;
    }
}

bool LiveMuxer::writeMuxerFrame(AVPacket *pPacket, bool bIsAudio){
    AA::Lock lock(mMuxerMutex, true);
    if(!pPacket){
        LOG_ERROR("%s muxer err pkt null", __FUNCTION__);
        return false;
    }
    if(bIsAudio) {
        if(audioEncoder.getAVCodecContext() && mAudioStream) {
            pPacket->stream_index = mAudioStream->index;
            pPacket->pts = av_rescale_q_rnd(pPacket->pts,
                                            audioEncoder.getAVCodecContext()->time_base,
                                            mAudioStream->time_base,
                                            AVRounding(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
            pPacket->dts = av_rescale_q_rnd(pPacket->dts,
                                            audioEncoder.getAVCodecContext()->time_base,
                                            mAudioStream->time_base,
                                            AVRounding(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
            pPacket->duration = av_rescale_q(pPacket->duration,
                                             audioEncoder.getAVCodecContext()->time_base,
                                             mAudioStream->time_base);
        }
    } else {
        if(videoEncoder.getAVCodecContext() && mVideoStream) {
            pPacket->stream_index = mVideoStream->index;
            pPacket->pts = av_rescale_q_rnd(pPacket->pts,
                                            videoEncoder.getAVCodecContext()->time_base,
                                            mVideoStream->time_base,
                                            AVRounding(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
            pPacket->dts = av_rescale_q_rnd(pPacket->dts,
                                            videoEncoder.getAVCodecContext()->time_base,
                                            mVideoStream->time_base,
                                            AVRounding(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
            pPacket->duration = av_rescale_q(pPacket->duration,
                                             videoEncoder.getAVCodecContext()->time_base,
                                             mVideoStream->time_base);
        }
    }
    if(!mFormatContext || av_write_frame(mFormatContext, pPacket) < 0){
        LOG_ERROR("%s muxer write frame err", __FUNCTION__);
        return false;
    }
    return true;
}