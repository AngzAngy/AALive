#ifndef __A_BUF_H_
#define __A_BUF_H_

class ABuffer{
public:
	ABuffer():buf(nullptr), sizeInBytes(0)/*, timeStamp(0)*/{}
	~ABuffer(){}
	void *buf;
	int sizeInBytes;
	/*int64_t timeStamp;
	int64_t timeEnd;*/
};
#endif//__A_BUF_H_
