//
// Created by Administrator on 2021/11/4.
//

#include <stdlib.h>
#include "ABuffer.h"
#ifndef __A_AUTOBUF_H_
#define __A_AUTOBUF_H_

class AAutoBuffer : public ABuffer{
public:
    AAutoBuffer():capacityInBytes(0){}
    ~AAutoBuffer(){}
    bool allocBuffer(int sizeBytes) {
        if(sizeBytes > 0) {
            buf = malloc(sizeBytes);
            if(buf) {
                capacityInBytes = sizeBytes;
                return true;
            }
        }
        return false;
    }
    void freeBuffer() {
        if(buf) {
            free(buf);
            buf = nullptr;
            capacityInBytes = 0;
            sizeInBytes = 0;
        }
    }
    int capacityInBytes;
};
#endif//__A_AUTOBUF_H_
