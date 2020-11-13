//
// Created by Administrator on 2020/11/13.
//

#ifndef AALIVE_IRTMPMUXER_H
#define AALIVE_IRTMPMUXER_H

#include <cstdint>
#include "LiveMuxerInfo.h"

class IRtmpMuxer{
public:
    virtual bool open(const LiveMuxerInfo& muxerInfo){};
    virtual bool isConnected(){};
    virtual bool close(){};
    virtual void release(){};

    bool writeVideoFrame(const uint8_t *buf, const int bufBytes, uint64_t dtsUS){};
    bool writeAudioFrame(const uint8_t *buf, const int bufBytes, uint64_t dtsUS){};
};
#endif //AALIVE_IRTMPMUXER_H
