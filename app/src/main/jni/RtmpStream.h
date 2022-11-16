/********************************************************************
filename:   RTMPStream.h
created:    2013-04-3
author:     firehood
purpose:    发送H264视频到RTMP Server，使用libRtmp库
*********************************************************************/
#ifndef ITK_RTMPSTREAM_H_2014
#define ITK_RTMPSTREAM_H_2014

#include "librtmp/rtmp.h"
#include "librtmp/rtmp_sys.h"
#include "librtmp/amf.h"
#include "Mutex.h"
#include <stdio.h>



typedef struct _RTMPMetadata
{
	// video, must be h264 type
	unsigned int	nWidth;
	unsigned int	nHeight;
	unsigned int	nFrameRate;		// fps
	unsigned int	nVideoDataRate;	// bps
	unsigned int	nSpsLen;
	unsigned char	Sps[1024];
	unsigned int	nPpsLen;
	unsigned char	Pps[1024];

	// audio, must be aac type
	bool	        bHasAudio;
	unsigned int	nAudioSampleRate;
	unsigned int	nAudioSampleSize;
	unsigned int	nAudioChannels;
	char		    pAudioSpecCfg;
	unsigned int	nAudioSpecCfgLen;

} RTMPMetadata,*LPRTMPMetadata;


class CRtmpStream
{
public:
	CRtmpStream(void);
	~CRtmpStream(void);
public:
	// 连接到RTMP Server
	bool Connect(const char* url);
    // 断开连接
	void Close();
    // 发送MetaData
	bool SendMetadata(LPRTMPMetadata lpMetaData);
//    // 发送H264数据帧
//	bool SendH264Packet(unsigned char *data,unsigned int size,bool bIsKeyFrame,unsigned int nTimeStamp);
//    // 发送AAC数据帧
//	bool SendAACPacket(unsigned char *data,unsigned int size,unsigned int nTimeStamp);
	//为了音视频同步改为统一给时间戳
    // 发送H264数据帧
	bool SendH264Packet(unsigned char *data,unsigned int size,bool bIsKeyFrame);
    // 发送AAC数据帧
	bool SendAACPacket(unsigned char *data,unsigned int size);
private:
	// 发送数据
	int SendPacket(unsigned int nPacketType,int m_nChannel ,unsigned char *data,unsigned int size,unsigned int nTimestamp);
private:
	RTMP* m_pRtmp;
	int m_streamId;
	unsigned int  m_nFileBufSize;
	unsigned int  m_nCurPos;
	long long m_startTime;
	bool m_isSendAudio;
	AA::Mutex m_mutext;

};
#endif /* ITK_RTMPSTREAM_H_2014 */
