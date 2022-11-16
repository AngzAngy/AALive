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
    virtual bool open(const LiveMuxerInfo& muxerInfo){return false;};
    virtual bool writeFrame(AFrame* pFrame){return false;};
    virtual bool isConnected(){return false;};
    virtual bool close(){return false;};
    virtual void release(){};
};
#endif //AALIVE_IRTMPMUXER_H
