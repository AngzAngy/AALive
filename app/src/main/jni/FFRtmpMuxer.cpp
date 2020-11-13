//
// Created on 2017/1/18.
//
#include <sstream>
#include "FFRtmpMuxer.h"
#include "AALog.h"
#include "CommonGlobaldef.h"

static int getStartCodeBytes(const uint8_t* buf, const int bufBytes) {
    if(bufBytes < 3) {
        return 0;
    }
    if(buf[0] == 0 && buf[1] == 0) {
        if (buf[2] == 1) {
            return 3;
        } else if (bufBytes > 3 && buf[2] == 0 && buf[3] == 1) {
            return 4;
        }
    }
    return 0;
}

static Nalu *getNalu(const uint8_t* buf, const int bufBytes) {
    Nalu *nalu = NULL;
    int startCodeBytes = getStartCodeBytes(buf, bufBytes);
    if(startCodeBytes > 0) {
        nalu = new Nalu;
        nalu->startCodeBytes = startCodeBytes;
        nalu->buf = const_cast<uint8_t *>(buf + startCodeBytes);
        nalu->type = (nalu->buf[0]) & 0x1f;
        int pos = nalu->startCodeBytes;
        while (pos < bufBytes) {
            startCodeBytes = getStartCodeBytes(&buf[pos], bufBytes - pos);
            if(startCodeBytes > 0) {
                break;
            }
            pos++;
        }
        nalu->bytes = pos - nalu->startCodeBytes;
    }
    return nalu;
}

static int decode_interrupt_cb(void *pMuxer) {
    return false;
}

/* check that a given sample format is supported by the encoder */
static int check_sample_fmt(AVCodec *codec, enum AVSampleFormat sample_fmt){
    const enum AVSampleFormat *p = codec->sample_fmts;
    LOGE("%s srcFmt %d", __FUNCTION__, (int)sample_fmt);

    while (*p != AV_SAMPLE_FMT_NONE) {
        if (*p == sample_fmt)
            return 1;
        LOGE("%s sopported Fmt %d", __FUNCTION__, (int)(*p));
        p++;
    }
    return 0;
}

/* check that a given sample format is supported by the encoder */
static AVSampleFormat get_sample_fmt(AVCodec *codec){
    const enum AVSampleFormat *p = codec->sample_fmts;
    return *p;
}

/* just pick the highest supported samplerate */
static int select_sample_rate(AVCodec *codec, int desireSampleRate){
    const int *p;
    int best_samplerate = 0;

    if (!codec->supported_samplerates)
        return desireSampleRate;

    p = codec->supported_samplerates;
    while (*p) {
        best_samplerate = FFMAX(*p, best_samplerate);
        if (*p == desireSampleRate) {
            return *p;
        }
        p++;
    }
    return best_samplerate;
}

bool static initAudioCodecContext(AVCodec *codec, AVCodecContext *codecContext, LiveMuxerInfo &liveMuxerInfo) {
    if (!codecContext) {
        LOGE("%s Could not allocate audio codec context", __FUNCTION__);
        return false;
    }

    codecContext->bit_rate = liveMuxerInfo.audioBitrate;
    codecContext->sample_fmt = AV_SAMPLE_FMT_S16;
    if (!check_sample_fmt(codec, codecContext->sample_fmt)) {
        LOGE("%s Src Fmt %d no used", __FUNCTION__, (int) (codecContext->sample_fmt));
        codecContext->sample_fmt = get_sample_fmt(codec);
    }

    /* select other audio parameters supported by the encoder */
    codecContext->sample_rate = select_sample_rate(codec, liveMuxerInfo.audioSampleRate);
    switch (liveMuxerInfo.audioChannelNumber) {
        case 1:
            codecContext->channel_layout = AV_CH_LAYOUT_MONO;
            break;
        case 2:
            codecContext->channel_layout = AV_CH_LAYOUT_STEREO;
            break;
        default:
            break;
    }
    codecContext->channels = av_get_channel_layout_nb_channels(codecContext->channel_layout);
    return true;
}

bool static initVideoEncoderContext(AVCodecContext *codecContext, LiveMuxerInfo &liveMuxerInfo) {
    if (!codecContext) {
        LOGE("%s Could not allocate video codec context", __FUNCTION__);
        return false;
    }

    /* put sample parameters */
    codecContext->bit_rate = liveMuxerInfo.voideoBitrate;
    /* resolution must be a multiple of two */
    codecContext->width = liveMuxerInfo.videoSrcWidth;
    codecContext->height = liveMuxerInfo.videoSrcHeight;
    /* frames per second */
    codecContext->time_base.num = 1;
    codecContext->time_base.den = 25;
    /* emit one intra frame every ten frames
    * check frame pict_type before passing frame
    * to encoder, if frame->pict_type is AV_PICTURE_TYPE_I
    * then gop_size is ignored and the output of encoder
    * will always be I frame irrespective to gop_size
    */
    codecContext->gop_size = 25;
    codecContext->max_b_frames = 1;
    codecContext->pix_fmt = AV_PIX_FMT_YUV420P;

    //h264 need
    av_opt_set(codecContext->priv_data, "preset", "slow", 0);
    return true;
}

FFRtmpMuxer::FFRtmpMuxer():mFormatContext(NULL),
                       mAudioStream(NULL),
                       mVideoStream(NULL),
                       hasSpsNalu(false),
                       connected(false){
  LOGE("%s FFRtmpMuxer Constructor", __FUNCTION__);
}

FFRtmpMuxer::~FFRtmpMuxer() {
    release();
}

bool FFRtmpMuxer::open(const LiveMuxerInfo& muxerInfo) {
    mMuxerInfo = muxerInfo;

    AVIOContext * avioContext = NULL;

    av_register_all();
    avcodec_register_all();
    avformat_network_init();
	av_log_set_level(AV_LOG_DEBUG);

    if(avio_open(&avioContext, mMuxerInfo.muxerUri.c_str(), AVIO_FLAG_WRITE) < 0){
        release();
        LOGE("%s io open error: %s", __FUNCTION__, mMuxerInfo.muxerUri.c_str());
        return false;
    }
    connected = true;

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

    AVCodec * aacCodec = avcodec_find_encoder(AV_CODEC_ID_AAC);
    mAudioStream = avformat_new_stream(mFormatContext, aacCodec);
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
    initAudioCodecContext(aacCodec, mAudioStream->codec, mMuxerInfo);

    AVCodec *videoCodec = avcodec_find_encoder(AV_CODEC_ID_H264);
    mVideoStream = avformat_new_stream(mFormatContext, videoCodec);
    if(!mVideoStream){
        release();
        LOGE("%s new vstream err", __FUNCTION__);
        return false;
    }
    mVideoStream->time_base.num = 1;
    mVideoStream->time_base.den = 25;
    mVideoStream->codec->codec_tag = 0;
//    if (mFormatContext->oformat->flags & AVFMT_GLOBALHEADER) {
        mVideoStream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
//    }
    initVideoEncoderContext(mVideoStream->codec, mMuxerInfo);

//    if(avformat_write_header(mFormatContext, NULL) < 0){
//        release();
//        LOGE("%s write header err", __FUNCTION__);
//        return false;
//    }

    return true;
}

bool FFRtmpMuxer::isConnected() {
    if(mFormatContext){
        return connected;
    }
    return false;
}

bool FFRtmpMuxer::close(){
    if(mFormatContext){
       av_write_trailer(mFormatContext);
       if(mFormatContext->pb) {
           avio_closep(&mFormatContext->pb);
           mFormatContext->pb = NULL;
        }
        connected = false;
        return true;
    }
    connected = false;
    return false;
}

void FFRtmpMuxer::release() {
    connected = false;
    hasSpsNalu = false;
    if(mFormatContext){
//        if(mFormatContext->pb) {
//            avio_closep(&mFormatContext->pb);
//            mFormatContext->pb = NULL;
//        }
        avformat_free_context(mFormatContext);
        mFormatContext = NULL;
    }
}

bool FFRtmpMuxer::writeMuxerFrame(AVPacket *pPacket, bool bIsAudio){
    if(!pPacket){
        LOGE("%s muxer err pkt null", __FUNCTION__);
        return false;
    }
    if(bIsAudio) {
        if(mAudioStream && mAudioStream->codec) {
            pPacket->stream_index = mAudioStream->index;
            pPacket->pts = av_rescale_q_rnd(pPacket->pts,
                                            mAudioStream->codec->time_base,
                                            mAudioStream->time_base,
                                            AVRounding(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
            pPacket->dts = av_rescale_q_rnd(pPacket->dts,
                                            mAudioStream->codec->time_base,
                                            mAudioStream->time_base,
                                            AVRounding(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
            pPacket->duration = av_rescale_q(pPacket->duration,
                                             mAudioStream->codec->time_base,
                                             mAudioStream->time_base);
        }
    } else {
        if(mVideoStream && mVideoStream->codec) {
            pPacket->stream_index = mVideoStream->index;
            pPacket->pts = av_rescale_q_rnd(pPacket->pts,
                                            mVideoStream->codec->time_base,
                                            mVideoStream->time_base,
                                            AVRounding(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
            pPacket->dts = av_rescale_q_rnd(pPacket->dts,
                                            mVideoStream->codec->time_base,
                                            mVideoStream->time_base,
                                            AVRounding(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
            pPacket->duration = av_rescale_q(pPacket->duration,
                                             mVideoStream->codec->time_base,
                                             mVideoStream->time_base);
        }
    }
    if(!mFormatContext) {
        LOGE("%s write frame err (FormatContext is NULL)", __FUNCTION__);
        return false;
    }
    int ret = av_interleaved_write_frame(mFormatContext, pPacket);
    if(ret < 0){
        LOGE("%s write frame err: %d", __FUNCTION__, AVERROR(ret));
        return false;
    }
    return true;
}

bool FFRtmpMuxer::writeAudioFrame(const uint8_t *buf, const int bufBytes, uint64_t dtsUS) {
    if( NULL != mFormatContext){
        return false;
    }
}

bool FFRtmpMuxer::writeVideoFrame(const uint8_t *buf, const int bufBytes, uint64_t dtsUS) {
    if (!connected
    || NULL == mFormatContext
    || NULL == mVideoStream
    || NULL == mVideoStream->codec) {
        return false;
    }
    if(!hasSpsNalu) {
        Nalu * nalu = getNalu(buf, bufBytes);
        if(nalu != NULL && nalu->type == 7) {
            Nalu *sps = nalu;

            std::ostringstream oss;
            oss << "[totalLen:" << bufBytes << "]";
            oss << "SPS[type:" << sps->type <<",strlen:" << sps->startCodeBytes << ",totalLen:" << sps->totalBytes() << "]";

            if(sps->totalBytes() < bufBytes) {
                nalu = getNalu(buf + sps->totalBytes(), bufBytes - sps->totalBytes());
                if(nalu != NULL && nalu->type == 8) {
                    Nalu *pps = nalu;

                    oss << "PPS[type:" << pps->type << ",strlen:" << pps->startCodeBytes << ",totalLen:" << pps->totalBytes() << "]";
                    LOGE("mybug %s", oss.str().c_str());

                    uint32_t spsFrameLen = sps->bytes;
                    uint8_t * spsFrame = sps->buf;

                    uint32_t ppsFrameLen = pps->bytes;
                    uint8_t * ppsFrame = pps->buf;

                    AVCodecContext *c = mVideoStream->codec;

                    int extradata_len = 8 + spsFrameLen - 4 + 1 + 2 + ppsFrameLen - 4;
                    c->extradata = (uint8_t*) av_mallocz(extradata_len);
                    c->extradata_size = extradata_len;
                    c->extradata[0] = 0x01;
                    c->extradata[1] = spsFrame[4 + 1];
                    c->extradata[2] = spsFrame[4 + 2];
                    c->extradata[3] = spsFrame[4 + 3];
                    c->extradata[4] = 0xFC | 3;
                    c->extradata[5] = 0xE0 | 1;
                    int tmp = spsFrameLen - 4;
                    c->extradata[6] = (tmp >> 8) & 0x00ff;
                    c->extradata[7] = tmp & 0x00ff;
                    int i = 0;
                    for (i = 0; i < tmp; i++)
                        c->extradata[8 + i] = spsFrame[4 + i];
                    c->extradata[8 + tmp] = 0x01;
                    int tmp2 = ppsFrameLen - 4;
                    c->extradata[8 + tmp + 1] = (tmp2 >> 8) & 0x00ff;
                    c->extradata[8 + tmp + 2] = tmp2 & 0x00ff;
                    for (i = 0; i < tmp2; i++)
                        c->extradata[8 + tmp + 3 + i] = ppsFrame[4 + i];

                    avformat_write_header(mFormatContext, NULL);

                    hasSpsNalu = true;
                    return true;
                }
            }
        }
    } else {
        AVPacket pkt = {0};
        av_init_packet(&pkt);
        pkt.data = (uint8_t *)buf;
        pkt.size = bufBytes;
        pkt.dts = dtsUS;
        pkt.pts = dtsUS;
        pkt.flags = 0;
        if(pkt.data[0] == 0x00 && pkt.data[1] == 0x00 && pkt.data[2] == 0x00 && pkt.data[3] == 0x01){
            uint32_t naluBytes = bufBytes - 4;
            pkt.data[0] = ((naluBytes) >> 24) & 0x00ff;
            pkt.data[1] = ((naluBytes) >> 16) & 0x00ff;
            pkt.data[2] = ((naluBytes) >> 8) & 0x00ff;
            pkt.data[3] = ((naluBytes)) & 0x00ff;
            if((pkt.data[4] & 0x1f) == 5) {
                pkt.flags = AV_PKT_FLAG_KEY;
                LOGE("%s isKeyFrame frameNumber:%d", __FUNCTION__, mVideoStream->codec->frame_number);
            }
        }
        mVideoStream->codec->frame_number ++;
        return writeMuxerFrame(&pkt, false);
    }
/*    int type = -1;
    if(buf[0] == 0x00 && buf[1] == 0x00 && buf[2] == 0x00 && buf[3] == 0x01){
        type = (buf[4] & 0x1f);
        if(type == 7) {
            mVideoStream->codec->extradata = (uint8_t *)av_mallocz(bufBytes + FF_INPUT_BUFFER_PADDING_SIZE);
            mVideoStream->codec->extradata_size = bufBytes;
            memcpy(mVideoStream->codec->extradata, buf, bufBytes);
            avformat_write_header(mFormatContext, NULL);
            LOGE("%s configFrame write header", __FUNCTION__);
            return true;
        }
    }
    AVPacket pkt = {0};
    av_init_packet(&pkt);
    pkt.data = (uint8_t *)buf;
    pkt.size = bufBytes;
    pkt.dts = dtsUS;
    pkt.pts = dtsUS;
    pkt.pos = -1;
    if(type == 5) {
        uint32_t naluBytes = bufBytes - 4;
        pkt.data[0] = ((naluBytes) >> 24) & 0x00ff;
        pkt.data[1] = ((naluBytes) >> 16) & 0x00ff;
        pkt.data[2] = ((naluBytes) >> 8) & 0x00ff;
        pkt.data[3] = ((naluBytes)) & 0x00ff;
        pkt.flags = AV_PKT_FLAG_KEY;
        LOGE("%s isKeyFrame", __FUNCTION__);
    }
    return writeMuxerFrame(&pkt, false);*/
}