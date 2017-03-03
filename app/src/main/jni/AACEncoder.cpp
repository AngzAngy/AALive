#include "AACEncoder.h"
#include "AALog.h"
#include <stdio.h>

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

AACEncoder::AACEncoder():
	mSampleRate(44100),
	mChannelNumber(1),
	mBytesPerSample(2),
	mBitrate(128000),
	mCodec(NULL),
	mCodecContext(NULL),
	mSrcFormat(AV_SAMPLE_FMT_S16),
	mSwrCtx(NULL),
	mDstFrame(NULL){
}

AACEncoder::~AACEncoder(){
    release();
}

bool AACEncoder::initEncoder(){
	/* find the AAC encoder */
	mCodec = avcodec_find_encoder_by_name("libfaac");//avcodec_find_encoder(AV_CODEC_ID_AAC);
	if (!mCodec) {
		LOGE("%s AAC Codec not found", __FUNCTION__);
		return false;
	}
	return true;
}

bool AACEncoder::initEncoderContext(AVCodecContext *codecContext){
	mCodecContext = codecContext;
	if (!mCodecContext) {
		LOGE("%s Could not allocate audio codec context", __FUNCTION__);
		return false;
	}

	mCodecContext->bit_rate = mBitrate;
	mCodecContext->sample_fmt = mSrcFormat;
	if (!check_sample_fmt(mCodec, mSrcFormat)) {
		LOGE("%s Src Fmt %d no used", __FUNCTION__, (int)(mSrcFormat));
		mCodecContext->sample_fmt = get_sample_fmt(mCodec);
	}

	/* select other audio parameters supported by the encoder */
	mCodecContext->sample_rate = select_sample_rate(mCodec, mSampleRate);
	switch (mChannelNumber){
		case 1:
			mCodecContext->channel_layout = AV_CH_LAYOUT_MONO;
			break;
		case 2:
			mCodecContext->channel_layout = AV_CH_LAYOUT_STEREO;
			break;
		default:
			release();
			return false;
	}

	mCodecContext->channels = av_get_channel_layout_nb_channels(mCodecContext->channel_layout);

	/* open it */
	if (avcodec_open2(mCodecContext, mCodec, NULL) < 0) {
		release();
		LOGE("%s Could not open aac codec", __FUNCTION__);
		return false;
	}

	if(mSampleRate != mCodecContext->sample_rate || mSrcFormat != mCodecContext->sample_fmt){
		mSwrCtx = swr_alloc();
		if (!mSwrCtx) {
			release();
			LOGE("%s Could not allocate resampler context", __FUNCTION__);
			return false;
		}
		/* set options */
		av_opt_set_int(mSwrCtx, "in_channel_layout",    mCodecContext->channel_layout, 0);
		av_opt_set_int(mSwrCtx, "in_sample_rate",       mSampleRate, 0);
		av_opt_set_sample_fmt(mSwrCtx, "in_sample_fmt", mSrcFormat, 0);

		av_opt_set_int(mSwrCtx, "out_channel_layout",    mCodecContext->channel_layout, 0);
		av_opt_set_int(mSwrCtx, "out_sample_rate",       mCodecContext->sample_rate, 0);
		av_opt_set_sample_fmt(mSwrCtx, "out_sample_fmt", mCodecContext->sample_fmt, 0);

		/* initialize the resampling context */
		if (swr_init(mSwrCtx) < 0) {
			release();
			LOGE("%s Failed to initialize the resampling context", __FUNCTION__);
			return false;
		}
		mDstFrame = av_frame_alloc();
		mDstFrame->nb_samples = mCodecContext->frame_number;
		mDstFrame->sample_rate = mCodecContext->sample_rate;
		mDstFrame->format = mCodecContext->sample_fmt;
		mDstFrame->channel_layout = mCodecContext->channel_layout;
		av_frame_get_buffer(mDstFrame, 16);
	}

	return true;
}

void AACEncoder::release(){
    if(mCodecContext){
	    avcodec_close(mCodecContext);
	    av_free(mCodecContext);
	    mCodecContext = NULL;
	}
	if(mSwrCtx) {
		swr_free(&mSwrCtx);
		mSwrCtx = NULL;
	}
	if(mDstFrame){
		av_free(mDstFrame);
		mDstFrame = NULL;
	}
}

bool AACEncoder::encode(AVPacket *avpkt, const AVFrame *srcFrame){
    int i = 0 , ret = 0  , got_output = 0 ;

	if(!avpkt || !srcFrame){
		LOGE("%s Error params",__FUNCTION__);
        return false;
	}
	/* encode the samples */
	if(mSwrCtx && mDstFrame){
		uint8_t **src_data = srcFrame->extended_data;
		uint8_t **dst_data = mDstFrame->data;
		swr_convert(mSwrCtx, dst_data, mDstFrame->nb_samples, (const uint8_t **)src_data, srcFrame->nb_samples);
		ret = avcodec_encode_audio2(mCodecContext, avpkt, mDstFrame, &got_output);
	}else {
		ret = avcodec_encode_audio2(mCodecContext, avpkt, srcFrame, &got_output);
	}
	if (ret < 0) {
	    LOGE("%s Error encoding audio frame",__FUNCTION__);
	    return false;
	}

	/* get the delayed frames */
//	for (got_output = 1; got_output; i++) {
//		ret = avcodec_encode_audio2(mCodecContext, avpkt, NULL, &got_output);
//		if (ret < 0) {
//			LOGE("%s Error encoding audio frame2",__FUNCTION__);
//			return false;
//		}
//	}

	if(got_output){
		return true;
	}
	return false;
}