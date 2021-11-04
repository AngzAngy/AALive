//
// Created by Administrator on 2020/11/27.
//

#ifndef AALIVE_ATIMESTAMPBUFFER_H
#define AALIVE_ATIMESTAMPBUFFER_H

#include <cstdint>
#include "ABuffer.h"

class ATimestampBuffer : public ABuffer{
public:
    ATimestampBuffer():timestamp(0){
    }
    ATimestampBuffer(int sizeBytes):timestamp(0){
        buf = new uint8_t[sizeBytes];
        if(buf) {
            sizeInBytes = sizeBytes;
        }
    }
    ~ATimestampBuffer(){
        if(buf) {
            delete []buf;
            buf = nullptr;
            sizeInBytes = 0;
        }
    }
    uint64_t timestamp;
};
#endif //AALIVE_ATIMESTAMPBUFFER_H
