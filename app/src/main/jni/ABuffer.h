#ifndef __A_BUF_H_
#define __A_BUF_H_
#include <stdlib.h>
#include <stdint.h>
class ABuffer{
public:
	ABuffer():buf(NULL), sizeInBytes(0)/*, timeStamp(0)*/{}
	~ABuffer(){}
	void *buf;
	int sizeInBytes;
	/*int64_t timeStamp;
	int64_t timeEnd;*/
};
#endif//__A_BUF_H_
