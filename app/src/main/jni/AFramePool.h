//
// Created by Administrator on 2021/11/5.
//
#include "codec/AFrame.h"
#include <vector>
#ifndef AALIVE_AFRAMEPOOL_H
#define AALIVE_AFRAMEPOOL_H

using namespace std;

class AFramePool {
public:
    AFramePool(int capacity);
    AFrame* acquireFrame();
    void releaseFrame(AFrame* pFrame);

private:
    vector<AFrame*> pool;
    int frameSize;
};

#endif //AALIVE_AFRAMEPOOL_H
