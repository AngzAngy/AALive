//
// Created by Administrator on 2020/11/27.
//

#ifndef AALIVE_ATIMESTAMPBUFFER_H
#define AALIVE_ATIMESTAMPBUFFER_H

#include <cstdint>
#include "AAutoBuffer.h"

class ATimestampBuffer : public AAutoBuffer{
public:
    ATimestampBuffer():timestamp(0){
    }
    ~ATimestampBuffer(){
    }
    uint64_t timestamp;
};
#endif //AALIVE_ATIMESTAMPBUFFER_H
