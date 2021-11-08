//
// Created by Administrator on 2020/11/24.
//

#ifndef AALIVE_IENCODER_H
#define AALIVE_IENCODER_H

#include "LiveMuxerInfo.h"
#include "ATimestampBuffer.h"
class IEncoder {
public:
    virtual bool init(LiveMuxerInfo & muxerInfo){};
    virtual bool release(){};
    virtual bool start(){};
    virtual bool stop(){};
    virtual bool sendBuffer(ATimestampBuffer &buffer){};
    virtual bool receiveBuffer(ATimestampBuffer &buffer){};
};
#endif //AALIVE_IENCODER_H
