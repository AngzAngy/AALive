//
// Created by Administrator on 2021/11/4.
//

#include "ABuffer.h"
#ifndef __A_AUTOBUF_H_
#define __A_AUTOBUF_H_

class ABufferAlloc{
public:
    virtual void* alloc(int sizeInBytes) = 0;
    virtual void free(void* bufAddr) = 0;
};

class AAutoBuffer : public ABuffer{
public:
    AAutoBuffer():capacityInBytes(0), bufferAlloc(nullptr){}
    ~AAutoBuffer(){}
    bool alloc(int sizeBytes) {
        if(bufferAlloc && sizeBytes > 0) {
            buf = bufferAlloc->alloc(sizeBytes);
            if(buf) {
                capacityInBytes = sizeBytes;
                return true;
            }
        }
        return false;
    }
    void free() {
        if(buf && bufferAlloc) {
            bufferAlloc->free(buf);
            buf = nullptr;
            capacityInBytes = 0;
            sizeInBytes = 0;
        }
    }
    int capacityInBytes;
    ABufferAlloc *bufferAlloc;
};
#endif//__A_AUTOBUF_H_
