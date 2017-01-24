//
// Created on 2017/1/18.
//
#include "LiveMuxer.h"
#include "AALog.h"

static int decode_interrupt_cb(void *pMuxer) {
    return false;
}

void LiveMuxer::aEncodeThreadCallback(void *pMuxer){
 /*   LiveMuxer *pLiveMuxer = (LiveMuxer*)pLiveMuxer;
    if(pLiveMuxer){

    }*/
}

void LiveMuxer::vEncodeThreadCallback(void *pMuxer){
	AVPacket output_packet;
	LiveMuxer *pLiveMuxer = NULL;
	pLiveMuxer = (LiveMuxer*)pMuxer;
	av_init_packet(&output_packet);
	output_packet.data = NULL;
	output_packet.size = 0;
	if (pLiveMuxer){
		if (pLiveMuxer->encodeVideoFrame(&output_packet)){
			pLiveMuxer->writeMuxerFrame(&output_packet, false);
        }
    }
	av_packet_unref(&output_packet);
}

LiveMuxer::LiveMuxer():mFormatContext(NULL),
                       mAudioStream(NULL),
                       mVideoStream(NULL),
                       mVideoWritePos(0),
                       mVideoReadPos(0),
                       mVideoFramesCount(0){
    for(int i=0; i < 2; i++){
        AVFrame *pFrame = allocVideoFrame();
        if(pFrame) {
            mVideoFrames.push_back(pFrame);
        }
    }
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
	av_log_set_level(AV_LOG_DEBUG);

    mAudioEncoder.setSampleRate(mMuxerInfo.audioSampleRate);
    mAudioEncoder.setChannelNumber(mMuxerInfo.audioChannelNumber);
    mAudioEncoder.setBytePerSample(mMuxerInfo.audioBytesPerSample);
    mAudioEncoder.setBitrate(mMuxerInfo.audioBitrate);
//    if(!mAudioEncoder.initEncoder()){
//        release();
//        LOGE("%s aencoder init err", __FUNCTION__);
//        return false;
//    }
    mVideoEncoder.setSrcVideoSize(mMuxerInfo.videoSrcWidth, mMuxerInfo.videoSrcHeight);
    mVideoEncoder.setDstVideoSize(mMuxerInfo.videoDstWidth, mMuxerInfo.videoDstHeight);
    mVideoEncoder.setBitrate(mMuxerInfo.voideoBitrate);
	mVideoEncoder.setSrcPixelFormat(AV_PIX_FMT_RGBA);
    if(!mVideoEncoder.initEncoder()){
        release();
        LOGE("%s vencoder init err", __FUNCTION__);
        return false;
    }

    if(avio_open(&avioContext, mMuxerInfo.muxerUri.c_str(), AVIO_FLAG_WRITE) < 0){
        release();
        LOGE("%s io open error: %s", __FUNCTION__, mMuxerInfo.muxerUri.c_str());
        return false;
    }

    //avformat_alloc_output_context2(&mFormatContext, NULL, "mpegts", mMuxerInfo.muxerUri.c_str());//UDP
    //RTMP
    if(avformat_alloc_output_context2(&mFormatContext, NULL,
                                   "flv", mMuxerInfo.muxerUri.c_str()) < 0){
        release();
        LOGE("%s fmt alloc err: %s", __FUNCTION__, mMuxerInfo.muxerUri.c_str());
        return false;
    }
    mFormatContext->interrupt_callback.callback = decode_interrupt_cb;
    mFormatContext->interrupt_callback.opaque = this;
    mFormatContext->pb = avioContext;
    memcpy(mFormatContext->filename, mMuxerInfo.muxerUri.c_str(), mMuxerInfo.muxerUri.size());

//    mAudioStream = avformat_new_stream(mFormatContext, mAudioEncoder.getAVCodec());
//    if(!mAudioStream){
//        release();
//        LOGE("%s new astream err", __FUNCTION__);
//        return false;
//    }
//    mAudioStream->time_base.den = mMuxerInfo.audioSampleRate;
//    mAudioStream->time_base.num = 1;
//    mAudioStream->codec->codec_tag = 0;
//    if (mFormatContext->oformat->flags & AVFMT_GLOBALHEADER) {
//        mAudioStream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
//    }

    mVideoStream = avformat_new_stream(mFormatContext, mVideoEncoder.getAVCodec());
    if(!mVideoStream){
        release();
        LOGE("%s new vstream err", __FUNCTION__);
        return false;
    }
    mVideoStream->time_base.den = 1000;
    mVideoStream->time_base.num = 1;
    mVideoStream->codec->codec_tag = 0;
    if (mFormatContext->oformat->flags & AVFMT_GLOBALHEADER) {
        mVideoStream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
    }
    if(!mVideoEncoder.initEncoderContext(mVideoStream->codec)){
        release();
        LOGE("%s init venctx err", __FUNCTION__);
        return false;
    }

    if(avformat_write_header(mFormatContext, NULL) < 0){
        release();
        LOGE("%s write header err", __FUNCTION__);
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
    mAudioEncoder.release();
    mVideoEncoder.release();
    if(mFormatContext){
        if(mFormatContext->pb) {
            avio_closep(&mFormatContext->pb);
            mFormatContext->pb = NULL;
        }
        avformat_free_context(mFormatContext);
        mFormatContext = NULL;
    }
    for(std::vector<AVFrame*>::iterator it = mVideoFrames.begin(); it != mVideoFrames.end(); ++it){
        AVFrame *frame = *it;
        if(frame){
            if (frame->data[0]) {
                av_free(frame->data[0]);
            }
            av_free(frame);
        }
    }
    mVideoFrames.clear();
}

bool LiveMuxer::writeMuxerFrame(AVPacket *pPacket, bool bIsAudio){
    AA::Lock lock(mMuxerMutex, true);
    if(!pPacket){
        LOGE("%s muxer err pkt null", __FUNCTION__);
        return false;
    }
    if(bIsAudio) {
        if(mAudioEncoder.getAVCodecContext() && mAudioStream) {
            pPacket->stream_index = mAudioStream->index;
            pPacket->pts = av_rescale_q_rnd(pPacket->pts,
                                            mAudioEncoder.getAVCodecContext()->time_base,
                                            mAudioStream->time_base,
                                            AVRounding(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
            pPacket->dts = av_rescale_q_rnd(pPacket->dts,
                                            mAudioEncoder.getAVCodecContext()->time_base,
                                            mAudioStream->time_base,
                                            AVRounding(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
            pPacket->duration = av_rescale_q(pPacket->duration,
                                             mAudioEncoder.getAVCodecContext()->time_base,
                                             mAudioStream->time_base);
        }
    } else {
        if(mVideoEncoder.getAVCodecContext() && mVideoStream) {
            pPacket->stream_index = mVideoStream->index;
            pPacket->pts = av_rescale_q_rnd(pPacket->pts,
                                            mVideoEncoder.getAVCodecContext()->time_base,
                                            mVideoStream->time_base,
                                            AVRounding(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
            pPacket->dts = av_rescale_q_rnd(pPacket->dts,
                                            mVideoEncoder.getAVCodecContext()->time_base,
                                            mVideoStream->time_base,
                                            AVRounding(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
            pPacket->duration = av_rescale_q(pPacket->duration,
                                             mVideoEncoder.getAVCodecContext()->time_base,
                                             mVideoStream->time_base);
        }
    }
    if(!mFormatContext || av_write_frame(mFormatContext, pPacket) < 0){
        LOGE("%s muxer write frame err", __FUNCTION__);
        return false;
    }
    return true;
}

AVFrame* LiveMuxer::allocVideoFrame(){
    AVFrame *pFrame = av_frame_alloc();
    if(pFrame){
        pFrame->width = mMuxerInfo.videoSrcWidth;
        pFrame->height = mMuxerInfo.videoSrcHeight;
        pFrame->format = AV_PIX_FMT_RGBA;
        av_frame_get_buffer(pFrame, 16);
    }
    return pFrame;
}

void LiveMuxer::queueVideoFrame(const char* rgbabuf, const int bufBytes){
    mVideoFramesMutex.lock();
    AVFrame *frame = mVideoFrames[mVideoWritePos];
    try {
        if (frame) {
            int ppb = (frame->width) * 4;
            uint8_t *src =(uint8_t *)rgbabuf;
            uint8_t *dst = frame->data[0];
            for(int x = 0; x < frame->height; x++){
                memcpy(dst, src, ppb);
                src += ppb;
                dst += frame->linesize[0];
            }
            mVideoWritePos = (++mVideoWritePos) % mVideoFrames.size();
            mVideoFramesCount++;

            mVideoFramesCondition.signal();
        }
    }catch (...){
    }

    mVideoFramesMutex.unlock();
}

bool LiveMuxer::encodeVideoFrame(AVPacket *avpkt){
    bool ret = false;
    mVideoFramesMutex.lock();

    if(mVideoFramesCount <= 0){
        mVideoFramesCondition.wait(mVideoFramesMutex);
    }

    try {
        AVFrame *frame = mVideoFrames[mVideoReadPos];
        mVideoReadPos = (++mVideoReadPos) % mVideoFrames.size();
        mVideoFramesCount--;

        ret = mVideoEncoder.encode(avpkt, frame);
    }catch(...){

    }

    mVideoFramesMutex.unlock();

    return ret;
}

void LiveMuxer::queueAudioFrame(){

}