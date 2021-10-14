//
// Created by Administrator on 2020/11/24.
//
#include "MediaCodecAACEncoder.h"
#include "LiveMuxerInfo.h"
#include "ATimestampBuffer.h"
#include <media/NdkMediaFormat.h>

#define MIMETYPE_AUDIO_AAC "audio/mp4a-latm"

MedaiCodecAACEncoder::MedaiCodecAACEncoder():pMediaCodec(nullptr) {

}

MedaiCodecAACEncoder::~MedaiCodecAACEncoder() {

}

bool MedaiCodecAACEncoder::init(LiveMuxerInfo & muxerInfo) {
    AMediaFormat *pMediaFormat = AMediaFormat_new();
    AMediaFormat_setString(pMediaFormat, AMEDIAFORMAT_KEY_MIME, MIMETYPE_AUDIO_AAC);
    AMediaFormat_setInt32(pMediaFormat, AMEDIAFORMAT_KEY_BIT_RATE, muxerInfo.audioBitrate);
    AMediaFormat_setInt32(pMediaFormat, AMEDIAFORMAT_KEY_CHANNEL_COUNT, muxerInfo.audioChannelNumber);
    AMediaFormat_setInt32(pMediaFormat, AMEDIAFORMAT_KEY_SAMPLE_RATE, muxerInfo.audioSampleRate);

    pMediaCodec = AMediaCodec_createEncoderByType(MIMETYPE_AUDIO_AAC);

    media_status_t status = AMediaCodec_configure(pMediaCodec, pMediaFormat, nullptr, nullptr, AMEDIACODEC_CONFIGURE_FLAG_ENCODE);

    AMediaFormat_delete(pMediaFormat);
    if(AMEDIA_OK == status) {
        return true;
    }
    return false;
}

bool MedaiCodecAACEncoder::release() {
    if(nullptr != pMediaCodec) {
        AMediaCodec_delete(pMediaCodec);
        pMediaCodec = nullptr;
    }
    return true;
}

bool MedaiCodecAACEncoder::start() {
    if(nullptr != pMediaCodec) {
        media_status_t ret = AMediaCodec_start(pMediaCodec);
        return (AMEDIA_OK == ret);
    }
    return false;
}

bool MedaiCodecAACEncoder::stop(){
    if(nullptr != pMediaCodec) {
        media_status_t ret = AMediaCodec_stop(pMediaCodec);
        return (AMEDIA_OK == ret);
    }
    return false;
}

bool MedaiCodecAACEncoder::sendBuffer(ATimestampBuffer &buffer) {
    if(nullptr != pMediaCodec && nullptr != buffer.buf) {
        int in_size = buffer.sizeInBytes;
        uint8_t* in_buf = (uint8_t*)buffer.buf;
        while(in_size > 0) {
            ssize_t idx = AMediaCodec_dequeueInputBuffer(pMediaCodec, -1);
            if(idx >= 0) {
                size_t out_size = 0;
                uint8_t* out_buf = AMediaCodec_getInputBuffer(pMediaCodec, idx, &out_size);
                if(out_buf && out_size > 0) {
                    if(in_size <= out_size) {
                        memcpy(out_buf, in_buf, in_size);
                        media_status_t ret = AMediaCodec_queueInputBuffer(pMediaCodec, idx, 0, in_size, buffer.timestamp, 0);
                        return (AMEDIA_OK == ret);
                    } else {
                        memcpy(out_buf, in_buf, out_size);

                        in_size -= out_size;
                        in_buf += out_size;

                        media_status_t ret = AMediaCodec_queueInputBuffer(pMediaCodec, idx, 0, out_size, buffer.timestamp, 0);
                        if(AMEDIA_OK != ret) {
                            return false;
                        }
                    }
                }
            }
        }
    }
    return false;
}

bool MedaiCodecAACEncoder::receiveBuffer(ABufferCallback<ATimestampBuffer> *callbackPtr) {
    if(nullptr != pMediaCodec && nullptr != callbackPtr) {
        AMediaCodecBufferInfo info;
        size_t out_size = 0;
        ssize_t idx= AMediaCodec_dequeueOutputBuffer(pMediaCodec, &info, 0);
        if(idx >= 0) {
            if((info.flags & AMEDIACODEC_BUFFER_FLAG_END_OF_STREAM) != 0) {

            }
            uint8_t *out_buf = AMediaCodec_getOutputBuffer(pMediaCodec, idx, &out_size);
            ATimestampBuffer aTimestampBuffer;
            aTimestampBuffer.timestamp = info.presentationTimeUs;
            aTimestampBuffer.sizeInBytes = info.size;
            aTimestampBuffer.buf = (out_buf + info.offset);
            callbackPtr->callback(aTimestampBuffer);
            media_status_t ret = AMediaCodec_releaseOutputBuffer(pMediaCodec, idx, false);
            return (AMEDIA_OK == ret);
        } else {
            ATimestampBuffer aTimestampBuffer;
            aTimestampBuffer.timestamp = 0;
            aTimestampBuffer.sizeInBytes = 0;
            aTimestampBuffer.buf = nullptr;
            callbackPtr->callback(aTimestampBuffer);
            return false;
        }
    }
    return false;
}
