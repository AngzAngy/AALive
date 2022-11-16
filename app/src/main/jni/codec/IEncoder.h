//
// Created by Administrator on 2020/11/24.
//

#ifndef AALIVE_IENCODER_H
#define AALIVE_IENCODER_H

#include "LiveMuxerInfo.h"
#include "ATimestampBuffer.h"
class IEncoder {
public:
    virtual bool init(LiveMuxerInfo & muxerInfo){return false;};
    virtual bool release(){return false;};
    virtual bool start(){return false;};
    virtual bool stop(){return false;};
    virtual bool sendBuffer(ATimestampBuffer &buffer){return false;};
    virtual bool receiveBuffer(ATimestampBuffer &buffer){return false;};
};
#endif //AALIVE_IENCODER_H
