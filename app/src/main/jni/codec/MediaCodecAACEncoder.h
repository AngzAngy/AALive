//
// Created by Administrator on 2020/11/24.
//

#ifndef AALIVE_MEDIACODECAACENCODER_H
#define AALIVE_MEDIACODECAACENCODER_H

#include "IEncoder.h"
#include "../buffer/ATimestampBuffer.h"
#include <media/NdkMediaCodec.h>

class MedaiCodecAACEncoder : public IEncoder{
public:
    MedaiCodecAACEncoder();
    ~MedaiCodecAACEncoder();
    bool init(LiveMuxerInfo & muxerInfo);
    bool release();
    bool start();
    bool stop();
    bool sendBuffer(ATimestampBuffer &buffer);
    bool receiveBuffer(ATimestampBuffer &buffer);

private:
    LiveMuxerInfo muxerParam;
    AMediaCodec* pMediaCodec;
};
#endif //AALIVE_MEDIACODECAACENCODER_H
