//
// Created on 2017/1/18.
//
#include "LiveMuxer.h"
#include "AALog.h"
#include "CommonGlobaldef.h"
#include "libyuv.h"

static void rgba2I420(uint8* dsty, int ystride,
    uint8* dstu, int ustride,
    uint8* dstv, int vstride,
    const uint8* srcrgba, int rgbastride,
    int width, int height){

    unsigned char R,G,B;
    for(int h = 0; h < height; h ++){
        uint8* y = dsty + ystride * h;
        uint8* prgb = (uint8*)srcrgba + rgbastride * 4 * h;
        if(h%2){
            uint8* u = dstu + ustride * h / 2;
            uint8* v = dstv + vstride * h / 2;
            for(int w=0; w < width; w += 2){
                B = *prgb++;
                G = *prgb++;
                R = *prgb++;
                prgb ++;
                *y++ = ( (  66 * R + 129 * G +  25 * B + 128) >> 8) +  16 ;
/*                if(ADVANCEDFILTER_CAM_FMT_YUV_420_NV21==cameraFmt){
                    *u++ = ( ( -38 * R -  74 * G + 112 * B + 128) >> 8) + 128;
                    *v++ = ( ( 112 * R -  94 * G -  18 * B + 128) >> 8) + 128;
                }else{*/
                    *u++ = ( ( 112 * R -  94 * G -  18 * B + 128) >> 8) + 128;
                    *v++ = ( ( -38 * R -  74 * G + 112 * B + 128) >> 8) + 128;
               // }


                B = *prgb++;
                G = *prgb++;
                R = *prgb++;
                prgb ++;
                *y++ = ( (  66 * R + 129 * G +  25 * B + 128) >> 8) +  16 ;
            }
        }else {
            for(int w=0; w < width; w++){
                B = *prgb++;
                G = *prgb++;
                R = *prgb++;
                prgb ++;
                *y++ = ( (  66 * R + 129 * G +  25 * B + 128) >> 8) +  16 ;
            }
        }
    }
}

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
                       mVideoEncodedCount(0){
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
	mVideoEncoder.setSrcPixelFormat(AV_PIX_FMT_YUV420P);
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
    mAudioStream->time_base.num = 1;
    mAudioStream->time_base.den = mMuxerInfo.audioSampleRate;
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
    mVideoStream->time_base.num = 1;
    mVideoStream->time_base.den = 25;
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
        ////this should be attention for the sample count,
        ////aac should be 1024
        AVFrame *pAFrame = allocAudioFrame(1024);
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
    mVideoEncodedCount = 0;
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
        pFrame->format = mVideoEncoder.getSrcPixelFormat();//mVideoEncoder.getSrcPixelFormat();
        av_frame_get_buffer(pFrame, 16);
    }
    return pFrame;
}

void LiveMuxer::queueVideoFrame(const char* y, const char* vu,
                                 const int width, const int height){
    mVideoFramesMutex.lock();
    AVFrame *frame = mVideoFrames[mVideoWritePos];
    try {
        if (frame) {

            uint8_t *srcY = (uint8_t *) y;
            uint8_t *srcUV = (uint8_t *) vu;
            uint8_t *dstY = frame->data[0];
            uint8_t *dstUV = frame->data[1];
            for (int x = 0; x < frame->height; x++) {
                memcpy(dstY, srcY, frame->width);
                dstY += frame->linesize[0];
                srcY += width;

                if(x % 2 == 0){
                    memcpy(dstUV, srcUV, frame->width);
                    srcUV += width;
                    dstUV += frame->linesize[1];
                }
            }

            mVideoWritePos = (++mVideoWritePos) % mVideoFrames.size();
            mVideoFramesCount++;

            mVideoFramesCondition.signal();
        }
    }catch (...){
    }

    mVideoFramesMutex.unlock();
}

void LiveMuxer::queueVideoFrame(const char* rgba,
                         const int width, const int height){
    mVideoFramesMutex.lock();
    AVFrame *frame = mVideoFrames[mVideoWritePos];
    try {
        if (frame) {

            const uint8_t *src_frame = (const uint8_t *) rgba;
            uint8_t *dst_y = frame->data[0];
            uint8_t *dst_u = frame->data[1];
            uint8_t *dst_v = frame->data[2];
            libyuv::ABGRToI420(src_frame, width * 4,
                           dst_y, frame->linesize[0],
                           dst_u, frame->linesize[1],
                           dst_v, frame->linesize[2],
                           width, height);

            mVideoWritePos = (++mVideoWritePos) % mVideoFrames.size();
            mVideoFramesCount++;

            mVideoFramesCondition.signal();
 /*           LOGE("%s fbosize: %d x %d,,ystrid: %d,,ustrid: %d,,vstrid: %d,,frameSize:%d x %d",
                     __FUNCTION__,width, height, frame->linesize[0], frame->linesize[1],frame->linesize[2],
                     frame->width, frame->height);*/
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
        frame->pts = mVideoEncodedCount;
        mVideoReadPos = (++mVideoReadPos) % mVideoFrames.size();
        mVideoFramesCount--;

        ret = mVideoEncoder.encode(avpkt, frame);
        if(ret){
            mVideoEncodedCount++;
        }
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