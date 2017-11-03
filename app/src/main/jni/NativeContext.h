//
// Created on 2017/1/19.
//
#include "LiveMuxer.h"
#include "LiveMuxerInfo.h"
#include "AudioRecord.h"
#include "YuvConverter.h"

#ifndef AALIVE_NATIVECONTEXT_H
#define AALIVE_NATIVECONTEXT_H
class NativeContext{
public:
    NativeContext(){
        videoRawBuf = NULL;
        videoRawBufBytes = 0;
        audioRecord = NULL;
        I420Convert = NULL;
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
    YuvConverter *I420Convert;
};
#endif //AALIVE_NATIVECONTEXT_H
