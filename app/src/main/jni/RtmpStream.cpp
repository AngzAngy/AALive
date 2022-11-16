/********************************************************************
 filename:   RTMPStream.cpp
 created:    2013-04-3
 author:     firehood
 purpose:    发送H264视频到RTMP Server，使用libRtmp库
 *********************************************************************/
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include "RtmpStream.h"
#include "TimeUtils.h"
#ifdef WIN32
#include <windows.h>
#endif

#ifdef WIN32
#pragma comment(lib,"WS2_32.lib")
#pragma comment(lib,"winmm.lib")
#endif
using namespace std;
enum {
	FLV_CODECID_H264 = 7,
	FLV_CODECID_AAC = 10
};
#define STREAM_CHANNEL_METADATA  0x03
#define STREAM_CHANNEL_VIDEO     0x04
#define STREAM_CHANNEL_AUDIO     0x05

int InitSockets() {
#ifdef WIN32
	WORD version;
	WSADATA wsaData;
	version = MAKEWORD(1, 1);
	return (WSAStartup(version, &wsaData) == 0);
#else
	return TRUE;
#endif
}

inline void CleanupSockets() {
#ifdef WIN32
	WSACleanup();
#endif
}

char * put_byte(char *output, uint8_t nVal) {
	output[0] = nVal;
	return output + 1;
}
char * put_be16(char *output, uint16_t nVal) {
	output[1] = nVal & 0xff;
	output[0] = nVal >> 8;
	return output + 2;
}
char * put_be24(char *output, uint32_t nVal) {
	output[2] = nVal & 0xff;
	output[1] = nVal >> 8;
	output[0] = nVal >> 16;
	return output + 3;
}
char * put_be32(char *output, uint32_t nVal) {
	output[3] = nVal & 0xff;
	output[2] = nVal >> 8;
	output[1] = nVal >> 16;
	output[0] = nVal >> 24;
	return output + 4;
}
char * put_be64(char *output, uint64_t nVal) {
	output = put_be32(output, nVal >> 32);
	output = put_be32(output, nVal);
	return output;
}
char * put_amf_string(char *c, const char *str) {
	uint16_t len = strlen(str);
	c = put_be16(c, len);
	memcpy(c, str, len);
	return c + len;
}
char * put_amf_double(char *c, double d) {
	*c++ = AMF_NUMBER;
	{
		/* type: Number */
		unsigned char *ci, *co;
		ci = (unsigned char *) &d;
		co = (unsigned char *) c;
		co[0] = ci[7];
		co[1] = ci[6];
		co[2] = ci[5];
		co[3] = ci[4];
		co[4] = ci[3];
		co[5] = ci[2];
		co[6] = ci[1];
		co[7] = ci[0];
	}
	return c + 8;
}

char * put_amf_boolean(char *c, bool d) {
	*c++ = AMF_BOOLEAN;
	{
		c[0] = d;
	}
	return c + 1;
}

CRtmpStream::CRtmpStream(void) :
		m_pRtmp(NULL), m_nFileBufSize(0), m_nCurPos(0) ,m_startTime(0),m_isSendAudio(false), m_streamId(0){
	InitSockets();
	m_pRtmp = RTMP_Alloc();
	RTMP_Init(m_pRtmp);
}

CRtmpStream::~CRtmpStream(void) {
	Close();
	//WSACleanup();
}

bool CRtmpStream::Connect(const char* url) {
	if (RTMP_SetupURL(m_pRtmp, (char*) url) < 0) {
		return FALSE;
	}
	RTMP_EnableWrite(m_pRtmp);
	if (RTMP_Connect(m_pRtmp, NULL) == FALSE) {
		return FALSE;
	}
	if (RTMP_ConnectStream(m_pRtmp, 0)  == FALSE) {
		return FALSE;
	}
	//初始化开始时间用于计算时间戳
	m_startTime = TimeUtils::currentTimeMillis();
	m_streamId = m_pRtmp->m_stream_id;
	return TRUE;
}

void CRtmpStream::Close() {
	AA::Lock lock(m_mutext);
	if (m_pRtmp) {
		RTMP_Close(m_pRtmp);
		RTMP_Free(m_pRtmp);
		m_pRtmp = NULL;
	}
}

int CRtmpStream::SendPacket(unsigned int nPacketType, int m_nChannel , unsigned char *data,
		unsigned int size, unsigned int nTimestamp) {
	if (m_pRtmp == NULL) {
		return FALSE;
	}

	RTMPPacket packet;
	RTMPPacket_Reset(&packet);
	RTMPPacket_Alloc(&packet, size);

	packet.m_packetType = nPacketType;
	packet.m_nChannel = m_nChannel;
	packet.m_headerType = RTMP_PACKET_SIZE_LARGE;
	packet.m_nTimeStamp = nTimestamp;
	packet.m_nInfoField2 = m_streamId;
	packet.m_nBodySize = size;
	memcpy(packet.m_body, data, size);
	int nRet = 0;
	{
		AA::Lock lock(m_mutext);
		if(m_pRtmp != NULL)nRet = RTMP_SendPacket(m_pRtmp, &packet, 0);
	}

	RTMPPacket_Free(&packet);

	return nRet;
}

bool CRtmpStream::SendMetadata(LPRTMPMetadata lpMetaData) {
	if (lpMetaData == NULL) {
		return false;
	}
	char body[1024] = { 0 };

	char * p = (char *) body;
	p = put_byte(p, AMF_STRING);
	p = put_amf_string(p, "@setDataFrame");

	p = put_byte(p, AMF_STRING);
	p = put_amf_string(p, "onMetaData");

	p = put_byte(p, AMF_OBJECT);
	p = put_amf_string(p, "copyright");
	p = put_byte(p, AMF_STRING);
	p = put_amf_string(p, "ithink");

	p = put_amf_string(p, "width");
	p = put_amf_double(p, lpMetaData->nWidth);

	p = put_amf_string(p, "height");
	p = put_amf_double(p, lpMetaData->nHeight);

	p = put_amf_string(p, "framerate");
	p = put_amf_double(p, lpMetaData->nFrameRate);

	p = put_amf_string(p, "videocodecid");
	p = put_amf_double(p, FLV_CODECID_H264);

	//音频
	if(lpMetaData->bHasAudio){
		p = put_amf_string(p, "audiosamplerate");
		p = put_amf_double(p, lpMetaData->nAudioSampleRate);

		p = put_amf_string(p, "audiosamplesize");
		p = put_amf_double(p, lpMetaData->nAudioSampleSize);

		p = put_amf_string(p, "stereo");
		p = put_amf_boolean(p, lpMetaData->nAudioChannels == 1?false:true);

		p = put_amf_string(p, "audiocodecid");
		p = put_amf_double(p, FLV_CODECID_AAC);
	}


	p = put_amf_string(p, "");
	p = put_byte(p, AMF_OBJECT_END);

	//int index = p-body;
	SendPacket(RTMP_PACKET_TYPE_INFO, STREAM_CHANNEL_METADATA ,(unsigned char*) body, p - body, 0);
	//视频同步包组装并发送
	int i = 0;
	body[i++] = 0x17; // 1:keyframe  7:AVC
	body[i++] = 0x00; // AVC sequence header

	body[i++] = 0x00;
	body[i++] = 0x00;
	body[i++] = 0x00; // fill in 0;

	// AVCDecoderConfigurationRecord.
	body[i++] = 0x01; // configurationVersion
	body[i++] = lpMetaData->Sps[1]; // AVCProfileIndication
	body[i++] = lpMetaData->Sps[2]; // profile_compatibility
	body[i++] = lpMetaData->Sps[3]; // AVCLevelIndication
	body[i++] = 0xff; // lengthSizeMinusOne

	// sps nums
	body[i++] = 0xE1; //&0x1f
	// sps data length
	body[i++] = lpMetaData->nSpsLen >> 8;
	body[i++] = lpMetaData->nSpsLen & 0xff;
	// sps data
	memcpy(&body[i], lpMetaData->Sps, lpMetaData->nSpsLen);
	i = i + lpMetaData->nSpsLen;

	// pps nums
	body[i++] = 0x01; //&0x1f
	// pps data length
	body[i++] = lpMetaData->nPpsLen >> 8;
	body[i++] = lpMetaData->nPpsLen & 0xff;
	// sps data
	memcpy(&body[i], lpMetaData->Pps, lpMetaData->nPpsLen);
	i = i + lpMetaData->nPpsLen;

	SendPacket(RTMP_PACKET_TYPE_VIDEO, STREAM_CHANNEL_VIDEO,(unsigned char*) body, i, 0);
	//音频同步包组装并发送
	if(lpMetaData->bHasAudio){
		m_isSendAudio = true;
		int i = 0;
		body[i++] = 0xAF; //
		body[i++] = 0x00; //0x00:发送的音频同步头,0x01:发送的音频数据
		if(lpMetaData->nAudioSampleRate == 16000 &&lpMetaData->nAudioChannels == 1 ){
			body[i++] = 0x14; //
			body[i++] = 0x08; //
		}else if(lpMetaData->nAudioSampleRate == 8000 &&lpMetaData->nAudioChannels == 1 ){
			body[i++] = 0x15; //
			body[i++] = 0x88; //
		}else{
			printf("ERROR! audio only support 16000 or 8000 sampleRate ,And support mono channel!\n ");
		}
		SendPacket(RTMP_PACKET_TYPE_AUDIO, STREAM_CHANNEL_AUDIO , (unsigned char*) body, i, 0);
	}
	return true;
}

bool CRtmpStream::SendH264Packet(unsigned char *data, unsigned int size,
		bool bIsKeyFrame) {
	if (data == NULL && size < 11) {
		return false;
	}
	unsigned int nTimeStamp = TimeUtils::currentTimeMillis() - m_startTime;
	unsigned char *body = new unsigned char[size + 9];

	int i = 0;
	if (bIsKeyFrame) {
		body[i++] = 0x17;	// 1:Iframe  7:AVC
	} else {
		body[i++] = 0x27;	// 2:Pframe  7:AVC
	}
	body[i++] = 0x01;	// AVC NALU
	body[i++] = 0x00;
	body[i++] = 0x00;
	body[i++] = 0x00;

	// NALU size
	body[i++] = size >> 24;
	body[i++] = size >> 16;
	body[i++] = size >> 8;
	body[i++] = size & 0xff;

	// NALU data
	memcpy(&body[i], data, size);

	bool bRet = SendPacket(RTMP_PACKET_TYPE_VIDEO, STREAM_CHANNEL_VIDEO, body, i + size, nTimeStamp);

	delete[] body;

	return bRet;
}

bool CRtmpStream::SendAACPacket(unsigned char *data,unsigned int size){
	if (data == NULL && size < 7 ) {
		return false;
	}
	if(!m_isSendAudio){
		return false;
	}

	unsigned int nTimeStamp = TimeUtils::currentTimeMillis() - m_startTime;
	unsigned char *body = new unsigned char[size + 2];

	int i = 0;
	//AAC format 48000Hz
	body[i++] = 0xAF;
	body[i++] = 0x01;//0x00:发送的音频同步头,0x01:发送的音频数据
	// NALU data
	memcpy(&body[i], data, size);

	bool bRet = SendPacket(RTMP_PACKET_TYPE_AUDIO, STREAM_CHANNEL_AUDIO , body, i + size, nTimeStamp);

	delete[] body;

	return bRet;
}
