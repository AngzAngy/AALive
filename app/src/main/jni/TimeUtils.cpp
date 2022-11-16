//
// Created by Administrator on 2022/11/11.
//
#include <sys/time.h>
#include "TimeUtils.h"

long long TimeUtils::currentTimeMicroSec() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000000 + tv.tv_usec;
}

long long TimeUtils::currentTimeMillis() {
    return currentTimeMicroSec() / 1000;
}