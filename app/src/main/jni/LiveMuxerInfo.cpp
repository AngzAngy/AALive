//
// Created on 2017/1/18.
//

#include "LiveMuxerInfo.h"

LiveMuxerInfo::LiveMuxerInfo():videoSrcWidth(0),videoSrcHeight(0),
videoDstWidth(0),videoDstHeight(0), voideoBitrate(400000),
audioSampleRate(44100), audioChannelNumber(1), audioBytesPerSample(2),
audioBitrate(128000){
}

LiveMuxerInfo& LiveMuxerInfo::operator = (const LiveMuxerInfo &other){
    this->muxerUri = other.muxerUri;
    this->videoSrcWidth = other.videoSrcWidth;
    this->videoSrcHeight = other.videoSrcHeight;
    this->videoDstWidth = other.videoDstWidth;
    this->videoDstHeight = other.videoDstHeight;
    this->voideoBitrate = other.voideoBitrate;
    this->audioSampleRate = other.audioSampleRate;
    this->audioChannelNumber = other.audioChannelNumber;
    this->audioBytesPerSample = other.audioBytesPerSample;
    this->audioBitrate = other.audioBitrate;
    return *this;
}