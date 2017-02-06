#include "CommonGlobaldef.h"
#include <sys/time.h>
int64_t currentUsec(){
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (int64_t)tv.tv_sec * 1000000 + tv.tv_usec;
}
