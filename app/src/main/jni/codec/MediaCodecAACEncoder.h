//
// Created by Administrator on 2020/11/24.
//

#ifndef AALIVE_MEDIACODECAACENCODER_H
#define AALIVE_MEDIACODECAACENCODER_H

#include "IEncoder.h"
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
    bool receiveBuffer(ABufferCallback<ATimestampBuffer> *callback);

private:
    AMediaCodec* pMediaCodec;
};
#endif //AALIVE_MEDIACODECAACENCODER_H
