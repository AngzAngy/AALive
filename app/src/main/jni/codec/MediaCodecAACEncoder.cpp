//
// Created by Administrator on 2020/11/24.
//
#include "MediaCodecAACEncoder.h"
#include "LiveMuxerInfo.h"
#include "../AALog.h"
#include <media/NdkMediaFormat.h>

#define MIMETYPE_AUDIO_AAC "audio/mp4a-latm"
#define ADTS_HEADER_BYTES 7

int getADTSsamplerate(int samplerate){
    int rate = 4;
    switch (samplerate){
        case 96000:
            rate = 0;
            break;
        case 88200:
            rate = 1;
            break;
        case 64000:
            rate = 2;
            break;
        case 48000:
            rate = 3;
            break;
        case 44100:
            rate = 4;
            break;
        case 32000:
            rate = 5;
            break;
        case 24000:
            rate = 6;
            break;
        case 22050:
            rate = 7;
            break;
        case 16000:
            rate = 8;
            break;
        case 12000:
            rate = 9;
            break;
        case 11025:
            rate = 10;
            break;
        case 8000:
            rate = 11;
            break;
        case 7350:
            rate = 12;
            break;
    }
    return rate;
}

void addADtsHeader(uint8_t* packet, int packetLen, int samplerate, int channel) {
    int profile = 2; // AAC LC
    int freqIdx = samplerate; // samplerate
    int chanCfg = channel; // CPE

    packet[0] = 0xFF; // 0xFFF(12bit) 这里只取了8位，所以还差4位放到下一个里面
    packet[1] = 0xF1; // 第一个t位放F
    packet[2] = (uint8_t) (((profile - 1) << 6) + (freqIdx << 2) + (chanCfg >> 2));
    packet[3] = (uint8_t) (((chanCfg & 0x3) << 6) + (packetLen >> 11));
    packet[4] = (uint8_t) ((packetLen & 0x7FF) >> 3);
    packet[5] = (uint8_t) (((packetLen & 7) << 5) + 0x1F);
    packet[6] = (uint8_t) 0xFC;
}

MedaiCodecAACEncoder::MedaiCodecAACEncoder():pMediaCodec(nullptr) {

}

MedaiCodecAACEncoder::~MedaiCodecAACEncoder() {

}

bool MedaiCodecAACEncoder::init(LiveMuxerInfo & muxerInfo) {
    muxerParam = muxerInfo;
    AMediaFormat *pMediaFormat = AMediaFormat_new();
    AMediaFormat_setString(pMediaFormat, AMEDIAFORMAT_KEY_MIME, MIMETYPE_AUDIO_AAC);
    AMediaFormat_setInt32(pMediaFormat, AMEDIAFORMAT_KEY_BIT_RATE, muxerInfo.audioBitrate);
    AMediaFormat_setInt32(pMediaFormat, AMEDIAFORMAT_KEY_CHANNEL_COUNT, muxerInfo.audioChannelNumber);
    AMediaFormat_setInt32(pMediaFormat, AMEDIAFORMAT_KEY_SAMPLE_RATE, muxerInfo.audioSampleRate);
    AMediaFormat_setInt32(pMediaFormat, AMEDIAFORMAT_KEY_AAC_PROFILE, 2);

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

bool MedaiCodecAACEncoder::receiveBuffer(ATimestampBuffer &buffer) {
    if(nullptr != pMediaCodec) {
        AMediaCodecBufferInfo info;
        size_t out_size = 0;
        ssize_t idx= AMediaCodec_dequeueOutputBuffer(pMediaCodec, &info, 0);
        if(idx >= 0) {
            if((info.flags & AMEDIACODEC_BUFFER_FLAG_END_OF_STREAM) != 0) {
                return false;
            }
            uint8_t *out_buf = AMediaCodec_getOutputBuffer(pMediaCodec, idx, &out_size);
            int adtsFrameBytes = info.size + ADTS_HEADER_BYTES;
            if(buffer.capacityInBytes < adtsFrameBytes || !buffer.buf) {
                buffer.freeBuffer();
                buffer.allocBuffer(adtsFrameBytes);
            }
            if(buffer.buf) {
                buffer.sizeInBytes = adtsFrameBytes;
                buffer.timestamp = info.presentationTimeUs;

                uint8_t *in_buf = (uint8_t*)(buffer.buf);

                int adtsSamplerate = getADTSsamplerate(muxerParam.audioSampleRate);
                addADtsHeader(in_buf, buffer.sizeInBytes, adtsSamplerate, muxerParam.audioChannelNumber);

                LOGE("recvadts size: %d",buffer.sizeInBytes);

                memcpy(in_buf + ADTS_HEADER_BYTES, out_buf + info.offset, info.size);

//                LOGE("rtmpMuxerWriteAudio codec buf0: %02X, buf1: %02X", in_buf[0], in_buf[1]);
            }
            media_status_t ret = AMediaCodec_releaseOutputBuffer(pMediaCodec, idx, false);
            return (AMEDIA_OK == ret);
        }
    }
    return false;
}
