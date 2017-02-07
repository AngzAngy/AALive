//
// Created on 2017/1/19.
//
#include "LiveMuxer.h"
#include "LiveMuxerInfo.h"
#include "AudioRecord.h"

#ifndef AALIVE_NATIVECONTEXT_H
#define AALIVE_NATIVECONTEXT_H
class NativeContext{
public:
    NativeContext(){
        videoRawBuf = NULL;
        videoRawBufBytes = 0;
        audioRecord = NULL;
    }
    void allocVideoRawBuf(int bytes){
        videoRawBuf = new char[bytes];
        if(videoRawBuf){
            videoRawBufBytes = bytes;
        }
    }
    void releaseVideoRawBuf(){
        if(videoRawBuf){
            delete []videoRawBuf;
            videoRawBuf = NULL;
        }
        videoRawBufBytes = 0;
    }
    LiveMuxer liveMuxer;
    LiveMuxerInfo liveMuxerInfo;
    char *videoRawBuf;
    int videoRawBufBytes;
    AudioRecord *audioRecord;
};
#endif //AALIVE_NATIVECONTEXT_H
