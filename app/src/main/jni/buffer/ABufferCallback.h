//
// Created by Administrator on 2020/11/27.
//

#ifndef AALIVE_ABUFFERCALLBACK_H
#define AALIVE_ABUFFERCALLBACK_H

template <class Buffer>
class ABufferCallback{
public:
    virtual void callback(Buffer buffer) = 0;
};
#endif //AALIVE_ABUFFERCALLBACK_H
