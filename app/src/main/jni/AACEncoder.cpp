#include "AACEncoder.h"
#include "AALog.h"
#include <stdio.h>

/* check that a given sample format is supported by the encoder */
static int check_sample_fmt(AVCodec *codec, enum AVSampleFormat sample_fmt){
	const enum AVSampleFormat *p = codec->sample_fmts;

	while (*p != AV_SAMPLE_FMT_NONE) {
		if (*p == sample_fmt)
			return 1;
		p++;
	}
	return 0;
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
	mSampleRate(0),
	mChannelNumber(0),
	mBytesPerSample(0),
	mBitrate(0),
	mCodec(NULL),
	mCodecContext(NULL){
}

AACEncoder::~AACEncoder(){
    release();
}

bool AACEncoder::init(){
    if(NULL != mCodecContext){
        return true;
    }

//	av_register_all();
//	avcodec_register_all();

	/* find the AAC encoder */
	mCodec = avcodec_find_encoder(AV_CODEC_ID_AAC);
	if (!mCodec) {
		LOG_ERROR("%s AAC Codec not found", __FUNCTION__);
		return false;
	}

	mCodecContext = avcodec_alloc_context3(mCodec);
	if (!mCodecContext) {
		LOG_ERROR("%s Could not allocate audio codec context", __FUNCTION__);
		return false;
	}

	/* put sample parameters */
	mCodecContext->bit_rate = mBitrate;

	/* check that the encoder supports s16 pcm input */
	mCodecContext->sample_fmt = AV_SAMPLE_FMT_S16;
	if (!check_sample_fmt(mCodec, mCodecContext->sample_fmt)) {
		release();

		LOG_ERROR("%s Encoder does not support sample format %s",
		 __FUNCTION__, av_get_sample_fmt_name(mCodecContext->sample_fmt));
		return false;
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
	    LOG_ERROR("%s Could not open aac codec", __FUNCTION__);
		return false;
	}

	return true;
}

void AACEncoder::release(){
    if(mCodecContext){
	    avcodec_close(mCodecContext);
	    av_free(mCodecContext);
	    mCodecContext = NULL;
	}
}

bool AACEncoder::encode(AVPacket *avpkt, const AVFrame *srcFrame){
    int i = 0 , ret , got_output;
	if(!init()){
	    return false;
	}

	if(!avpkt || !srcFrame){
		LOG_ERROR("%s Error params",__FUNCTION__);
        return false;
	}
	/* encode the samples */
	ret = avcodec_encode_audio2(mCodecContext, avpkt, srcFrame, &got_output);
	if (ret < 0) {
	    LOG_ERROR("%s Error encoding audio frame",__FUNCTION__);
	    return false;
	}

	/* get the delayed frames */
	for (got_output = 1; got_output; i++) {
		ret = avcodec_encode_audio2(mCodecContext, avpkt, NULL, &got_output);
		if (ret < 0) {
			LOG_ERROR("%s Error encoding audio frame2",__FUNCTION__);
			return false;
		}
	}

	return true;
}