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
    int64_t timestamp;
};
#endif //AALIVE_ATIMESTAMPBUFFER_H
