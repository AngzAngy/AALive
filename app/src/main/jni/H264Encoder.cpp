#include "H264Encoder.h"
#include "AALog.h"
#include <stdio.h>

static size_t readYUVFrameFromFile(AVFrame *frame, FILE *f) {
	int y = 0;
	size_t r = 0;
	/* Y */
	uint8_t *buf = frame->data[0];
	int bufSize = frame->width;
	int bufline = frame->linesize[0];
	for (y = 0; y < frame->height; y++) {
		r = fread(buf, sizeof(uint8_t), bufSize, f);
		buf += bufline;
	}

	switch (frame->format) {
	case AV_PIX_FMT_YUV420P:
		/* U */
		buf = frame->data[1];
		bufSize = frame->width / 2;
		bufline = frame->linesize[1];
		for (y = 0; y < frame->height / 2; y++) {
			r = fread(buf, sizeof(uint8_t), bufSize, f);
			buf += bufline;
		}

		/* U */
		buf = frame->data[2];
		bufSize = frame->width / 2;
		bufline = frame->linesize[2];
		for (y = 0; y < frame->height / 2; y++) {
			r = fread(buf, sizeof(uint8_t), bufSize, f);
			buf += bufline;
		}
		break;
	case AV_PIX_FMT_NV21:
		/* VU */
		buf = frame->data[1];
		bufSize = frame->width;
		bufline = frame->linesize[1];
		for (y = 0; y < frame->height / 2; y++) {
			r = fread(buf, sizeof(uint8_t), bufSize, f);
			buf += bufline;
		}
		break;
	}

	return r;
}

H264Encoder::H264Encoder():
	mSrcWidth(640),
	mSrcHeight(480),
	mDstWidth(640),
	mDstHeight(480),
	mBitrate(400000),
	mSrcPixelFormat(AV_PIX_FMT_YUV420P),
	mCodec(NULL),
	mCodecContext(NULL),
	mSwsContext(NULL),
	mDstFrame(NULL){
}

H264Encoder::~H264Encoder(){
    release();
}

bool H264Encoder::init(){
    if(NULL != mCodecContext){
        return true;
    }
	uint8_t endcode[] = { 0, 0, 1, 0xb7 };

//	av_register_all();
//	avcodec_register_all();
	/* find the video encoder */
	mCodec = avcodec_find_encoder(AV_CODEC_ID_H264);
	if (!mCodec) {
		LOGE("%s Codec not found", __FUNCTION__);
		return false;
	}

	mCodecContext = avcodec_alloc_context3(mCodec);
	if (!mCodecContext) {
		LOGE("%s Could not allocate video codec context", __FUNCTION__);
		return false;
	}

	/* put sample parameters */
	mCodecContext->bit_rate = mBitrate;
	/* resolution must be a multiple of two */
	mCodecContext->width = mDstWidth;
	mCodecContext->height = mDstHeight;
	/* frames per second */
	mCodecContext->time_base.num = 1;
	mCodecContext->time_base.den= 25;
	/* emit one intra frame every ten frames
	* check frame pict_type before passing frame
	* to encoder, if frame->pict_type is AV_PICTURE_TYPE_I
	* then gop_size is ignored and the output of encoder
	* will always be I frame irrespective to gop_size
	*/
	mCodecContext->gop_size = 10;
	mCodecContext->max_b_frames = 1;
	mCodecContext->pix_fmt = AV_PIX_FMT_YUV420P;

	//h264 need
	av_opt_set(mCodecContext->priv_data, "preset", "slow", 0);

	/* open it */
	if (avcodec_open2(mCodecContext, mCodec, NULL) < 0) {
	    release();
		LOGE("%s Could not open codec",__FUNCTION__);
		return false;
	}

	if (mSrcPixelFormat != AV_PIX_FMT_YUV420P
	    || mSrcWidth != mDstWidth || mSrcHeight != mDstHeight) {
		/* create scaling context */
		mSwsContext = sws_getContext(mSrcWidth, mSrcHeight, mSrcPixelFormat,
			mDstWidth, mDstHeight, AV_PIX_FMT_YUV420P,
			SWS_BILINEAR, NULL, NULL, NULL);
		if (!mSwsContext) {
		    release();
			LOGE("%s Impossible to create scale context for the conversion fail",__FUNCTION__);
            return false;
		}
		mDstFrame = av_frame_alloc();
		if (!mDstFrame) {
		    release();
			LOGE("%s Could not allocate video frame",__FUNCTION__);
            return false;
        }
        mDstFrame->format = mCodecContext->pix_fmt;
        mDstFrame->width = mCodecContext->width;
        mDstFrame->height = mCodecContext->height;

        /* the image can be allocated by any means and av_image_alloc() is
        * just the most convenient way if av_malloc() is to be used */
        if (av_image_alloc(mDstFrame->data, mDstFrame->linesize,
            mCodecContext->width, mCodecContext->height,mCodecContext->pix_fmt, 16) < 0) {
		    release();
			LOGE("%s Could not allocate raw picture buffer",__FUNCTION__);
            return false;
        }
	}
	return true;
}

void H264Encoder::release(){
	if (mCodecContext) {
		avcodec_close(mCodecContext);
		av_free(mCodecContext);
		mCodecContext = NULL;
	}
	if (mSwsContext) {
		sws_freeContext(mSwsContext);
		mSwsContext = NULL;
	}
	if (mDstFrame) {
	    av_freep(&mDstFrame->data[0]);
	    av_frame_free(&mDstFrame);
	    mDstFrame = NULL;
    }
}

bool H264Encoder::encode(AVPacket *avpkt, const AVFrame *srcFrame) {
	int i = 0 , ret , got_output;
	if(!init()){
	    return false;
	}
	if(!avpkt || !srcFrame){
		LOGE("%s Error params",__FUNCTION__);
        return false;
	}
	if (mSwsContext != NULL) {
	    /* convert to destination format */
	    sws_scale(mSwsContext, srcFrame->data,
	    srcFrame->linesize, 0, srcFrame->height,
	    mDstFrame->data, mDstFrame->linesize);

	    mDstFrame->pts = srcFrame->pts;

	    ret = avcodec_encode_video2(mCodecContext, avpkt, mDstFrame, &got_output);
	}else{
		ret = avcodec_encode_video2(mCodecContext, avpkt, srcFrame, &got_output);
	}
	if (ret < 0) {
	    LOGE("%s Error encoding frame",__FUNCTION__);
	    return false;
	}

	/* get the delayed frames */
	for (got_output = 1; got_output; i++) {
		ret = avcodec_encode_video2(mCodecContext, avpkt, NULL, &got_output);
		if (ret < 0) {
			LOGE("%s Error encoding frame2",__FUNCTION__);
			return false;
		}
	}

	return true;
}