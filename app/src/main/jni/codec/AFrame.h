//
// Created by Administrator on 2021/11/5.
//

#ifndef AALIVE_AFRAME_H
#define AALIVE_AFRAME_H

#include "ATimestampBuffer.h"
enum FrameType{
    VideoFrame,
    AudioFrame,
    UnknownFrame
};

class AFrame :public ATimestampBuffer{
public:
    static AFrame* allocFrame();
    static void freeFrame(AFrame* pFrame);

    AFrame();
    FrameType type;
};
#endif //AALIVE_AFRAME_H
