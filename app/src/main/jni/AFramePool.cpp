//
// Created by Administrator on 2021/11/5.
//
#include "AFramePool.h"

AFramePool::AFramePool(int capacity):pool(capacity),frameSize(0) {
}

AFrame* AFramePool::acquireFrame() {
    AFrame* frame = nullptr;
    if(frameSize > 0 && frameSize <= pool.size()) {
        int lastPoolIndex = frameSize - 1;
        frame = pool[lastPoolIndex];
        pool[lastPoolIndex] = nullptr;
        frameSize--;
    }
    if(!frame) {
        frame = AFrame::allocFrame();
    }
    return frame;
}

void AFramePool::releaseFrame(AFrame* pFrame) {
    for(int i = 0; i < frameSize; i++) {
        if(pFrame == pool[i]){
            return;
        }
    }
    if(frameSize < pool.size()) {
        pool[frameSize] = pFrame;
        frameSize++;
    } else {
//        AFrame::freeFrame(pFrame);
    }
}
