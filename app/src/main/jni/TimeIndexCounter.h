//
// Created by Administrator on 2021/11/4.
//

#include <stdint.h>
#ifndef AALIVE_TIMEINDEXCOUNTER_H
#define AALIVE_TIMEINDEXCOUNTER_H

class TimeIndexCounter{
public:
    TimeIndexCounter():lastTimeUs(0),timeIndex(0){}

    void calcTotalTime(uint64_t currentTimeUs) {
        if (lastTimeUs <= 0) {
            lastTimeUs = currentTimeUs;
        }
        uint64_t delta = currentTimeUs - lastTimeUs;
        lastTimeUs = currentTimeUs;
        timeIndex += delta / 1000;
    }

     void reset() {
        lastTimeUs = 0;
        timeIndex = 0;
    }

    uint64_t getTimeIndex() {
        return timeIndex;
    }
private:
    uint64_t lastTimeUs;
    uint64_t timeIndex;
};

#endif //AALIVE_TIMEINDEXCOUNTER_H
