#include <stdint.h>
#include <stdlib.h>
#include "buffer/AAutoBuffer.h"
#ifndef COMMON_GLOBALDEF_H_H_H
#define COMMON_GLOBALDEF_H_H_H

#ifndef NULL
#define NULL 0
#endif
#define FALSE 0
#define TRUE 1
#define FALSE 0

uint64_t currentUsec();

enum SampleFormat {
	SAMPLE_FMT_NONE = -1,
	SAMPLE_FMT_U8,          ///< unsigned 8 bits
	SAMPLE_FMT_S16,         ///< signed 16 bits
	SAMPLE_FMT_S32,         ///< signed 32 bits
	SAMPLE_FMT_FLT,         ///< float
	SAMPLE_FMT_DBL,         ///< double

	SAMPLE_FMT_U8P,         ///< unsigned 8 bits, planar
	SAMPLE_FMT_S16P,        ///< signed 16 bits, planar
	SAMPLE_FMT_S32P,        ///< signed 32 bits, planar
	SAMPLE_FMT_FLTP,        ///< float, planar
	SAMPLE_FMT_DBLP,        ///< double, planar

	SAMPLE_FMT_NB           ///< Number of sample formats. DO NOT USE if linking dynamically
};

enum VideoPixelFormat {
	FMT_NONE = -1,
	FMT_YUV420P,   
	FMT_YUYV422,   
	FMT_RGB24,     
	FMT_BGR24,     
	FMT_YUV422P,   
	FMT_YUV444P,   
	FMT_NV12 = 25,
	FMT_NV21 = 26,  
};

class DefABufferAlloc : public ABufferAlloc{
public:
	void *alloc(int sizeInBytes){
		return malloc(sizeInBytes);
	}

	void free(void *bufAddr){
		if(bufAddr) {
			free(bufAddr);
		}
	}
};

static DefABufferAlloc gDefABufferAlloc;
#endif
