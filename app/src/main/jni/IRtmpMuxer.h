//
// Created by Administrator on 2020/11/13.
//

#ifndef AALIVE_IRTMPMUXER_H
#define AALIVE_IRTMPMUXER_H

#include <cstdint>
#include "LiveMuxerInfo.h"
#include "AFrame.h"

class IRtmpMuxer{
public:
    virtual bool open(const LiveMuxerInfo& muxerInfo){};
    virtual bool writeFrame(AFrame* pFrame){};
    virtual bool isConnected(){};
    virtual bool close(){};
    virtual void release(){};
};
#endif //AALIVE_IRTMPMUXER_H
