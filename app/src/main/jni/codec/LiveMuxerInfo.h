//
// Created on 2017/1/18.
//

#include <string>

#ifndef AALIVE_LIVEMUXERINFO_H
#define AALIVE_LIVEMUXERINFO_H

class LiveMuxerInfo{
public:
    LiveMuxerInfo();
    LiveMuxerInfo& operator = (const LiveMuxerInfo &);
    std::string muxerUri;

    int videoSrcWidth;
    int videoSrcHeight;
    int videoDstWidth;
    int videoDstHeight;
    int voideoBitrate;

    int audioSampleRate;
    int audioChannelNumber;
    int audioBytesPerSample;
    int audioBitrate;
};
#endif //AALIVE_LIVEMUXERINFO_H
