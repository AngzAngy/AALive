//
// Created on 2017/1/18.
//
#include "LiveMuxer.h"
#include "AALog.h"
#include "CommonGlobaldef.h"

static int decode_interrupt_cb(void *pMuxer) {
    return false;
}

void LiveMuxer::aEncodeThreadCallback(void *pMuxer){
    LiveMuxer *pLiveMuxer = (LiveMuxer*)pMuxer;
    AVPacket outputPkt;
    av_init_packet(&outputPkt);
    outputPkt.data = NULL;
    outputPkt.size = 0;
    if(pLiveMuxer){
        if(pLiveMuxer->encodeAudioFrame(&outputPkt)){
            pLiveMuxer->writeMuxerFrame(&outputPkt, true);
        }
    }
    av_packet_unref(&outputPkt);
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

void LiveMuxer::audioFrameCallback(void *buf, int32_t size, void* userData){
    LiveMuxer * pLiveMuxer = (LiveMuxer*)userData;
    if(pLiveMuxer){
        pLiveMuxer->queueAudioFrame((const char*) buf, (const int) size);
    }
}

LiveMuxer::LiveMuxer():mFormatContext(NULL),
                       mAudioStream(NULL),
                       mVideoStream(NULL),
                       mAudioWritePos(0),
                       mAudioReadPos(0),
                       mAudioFramesCount(0),
                       mAudioArrivedTime(-1),
                       mAudioBeginTime(-1),
                       mVideoWritePos(0),
                       mVideoReadPos(0),
                       mVideoFramesCount(0),
                       mVideoArrivedTime(-1),
                       mVideoBeginTime(-1){
  LOGE("%s LiveMuxer Constructor", __FUNCTION__);
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
    if(!mAudioEncoder.initEncoder()){
        release();
        LOGE("%s aencoder init err", __FUNCTION__);
        return false;
    }
    mVideoEncoder.setSrcVideoSize(mMuxerInfo.videoSrcWidth, mMuxerInfo.videoSrcHeight);
    mVideoEncoder.setDstVideoSize(mMuxerInfo.videoDstWidth, mMuxerInfo.videoDstHeight);
    mVideoEncoder.setBitrate(mMuxerInfo.voideoBitrate);
	mVideoEncoder.setSrcPixelFormat(AV_PIX_FMT_NV21);
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

    mAudioStream = avformat_new_stream(mFormatContext, mAudioEncoder.getAVCodec());
    if(!mAudioStream){
        release();
        LOGE("%s new astream err", __FUNCTION__);
        return false;
    }
    mAudioStream->time_base.den = mMuxerInfo.audioSampleRate;
    mAudioStream->time_base.num = 1;
    mAudioStream->codec->codec_tag = 0;
    if (mFormatContext->oformat->flags & AVFMT_GLOBALHEADER) {
        mAudioStream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
    }
    if(!mAudioEncoder.initEncoderContext(mAudioStream->codec)){
        release();
        LOGE("%s init aenctx err", __FUNCTION__);
        return false;
    }

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

    for(int i=0; i < 2; i++){
        AVFrame *pVFrame = allocVideoFrame();
        if(pVFrame) {
            mVideoFrames.push_back(pVFrame);
        }
        AVFrame *pAFrame = allocAudioFrame(mAudioStream->codec->frame_number);
        if(pAFrame){
            mAudioFrames.push_back(pAFrame);
        }
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
    for(std::vector<AVFrame*>::iterator it = mAudioFrames.begin(); it != mAudioFrames.end(); ++it){
        AVFrame *frame = *it;
        if(frame){
            av_free(frame);
        }
    }
    mAudioFrames.clear();
    mAudioBeginTime = -1;
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
    mVideoBeginTime = -1;
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

AVFrame* LiveMuxer::allocAudioFrame(int frameSize){
    AVFrame *pFrame = av_frame_alloc();
    if(pFrame){
        pFrame->nb_samples = frameSize;
        pFrame->sample_rate = mMuxerInfo.audioSampleRate;
        pFrame->format = AV_SAMPLE_FMT_S16;
        switch(mMuxerInfo.audioChannelNumber){
            case 1:
                pFrame->channel_layout = AV_CH_LAYOUT_MONO;
                break;
            case 2:
                pFrame->channel_layout = AV_CH_LAYOUT_STEREO;
                break;
            default:
                pFrame->channel_layout = AV_CH_LAYOUT_MONO;
                break;
        }
        av_frame_get_buffer(pFrame, 16);
    }
    return pFrame;
}

AVFrame* LiveMuxer::allocVideoFrame(){
    AVFrame *pFrame = av_frame_alloc();
    if(pFrame){
        pFrame->width = mMuxerInfo.videoSrcWidth;
        pFrame->height = mMuxerInfo.videoSrcHeight;
        pFrame->format = AV_PIX_FMT_NV21;
        av_frame_get_buffer(pFrame, 16);
    }
    return pFrame;
}

void LiveMuxer::queueVideoFrame(const char* rgbabuf, const int bufBytes){
    mVideoFramesMutex.lock();
    AVFrame *frame = mVideoFrames[mVideoWritePos];
    try {
        if (frame) {
            mVideoArrivedTime = currentUsec();
            if(mVideoBeginTime == -1){
                mVideoBeginTime = mVideoArrivedTime;
            }
            int64_t videoDifferTime = mVideoArrivedTime - mVideoBeginTime;
            if(frame->format == AV_PIX_FMT_RGBA) {
                int ppb = (frame->width) * 4;
                uint8_t *src = (uint8_t *) rgbabuf;
                uint8_t *dst = frame->data[0];
                for (int x = 0; x < frame->height; x++) {
                    memcpy(dst, src, ppb);
                    src += ppb;
                    dst += frame->linesize[0];
                }
            } else if(frame->format == AV_PIX_FMT_NV21){
                int frameSize = frame->width * frame->height;
                uint8_t *srcY = (uint8_t *) rgbabuf;
                uint8_t *srcUV = srcY + frameSize;
                uint8_t *dstY = frame->data[0];
                uint8_t *dstUV = frame->data[1];
                for (int x = 0; x < frame->height; x++) {
                    memcpy(dstY, srcY, frame->width);
                    dstY += frame->linesize[0];
                    srcY += frame->width;

                    if(x % 2 == 0){
                        memcpy(dstUV, srcUV, frame->width);
                        srcUV += frame->width;
                        dstUV += frame->linesize[1];
                    }
                }
            }

            frame->pts = videoDifferTime / 1000;

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

    while(mVideoFramesCount <= 0){
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

void LiveMuxer::queueAudioFrame(const char* buf, const int bufBytes){
    mAudioFramesMutex.lock();
    AVFrame *frame = mAudioFrames[mAudioWritePos];
    try {
        if (frame) {
            mAudioArrivedTime = currentUsec();
            if(mAudioBeginTime == -1){
                mAudioBeginTime = mAudioArrivedTime;
            }
            int64_t audioDifferTime = mAudioArrivedTime - mAudioBeginTime;
            if(frame->extended_data[0]){
                memcpy(frame->extended_data[0], buf, bufBytes);
            }
            frame->pts = audioDifferTime / 1000;

            mAudioWritePos = (++mAudioWritePos) % mAudioFrames.size();
            mAudioFramesCount++;

            mAudioFramesCondition.signal();
        }
    }catch (...){
    }

    mAudioFramesMutex.unlock();
}

bool LiveMuxer::encodeAudioFrame(AVPacket *avpkt){
    bool ret = false;
    mAudioFramesMutex.lock();

    while(mAudioFramesCount <= 0){
        mAudioFramesCondition.wait(mAudioFramesMutex);
    }

    try {
        AVFrame *frame = mAudioFrames[mAudioReadPos];
        mAudioReadPos = (++mAudioReadPos) % mAudioFrames.size();
        mAudioFramesCount--;

        ret = mAudioEncoder.encode(avpkt, frame);
    }catch(...){

    }

    mAudioFramesMutex.unlock();

    return ret;
}