#ifndef FFMPEG_THREAD_H
#define FFMPEG_THREAD_H

#include <pthread.h>

typedef struct ThreadCB {
	void (*callback)(void*);
	void *opaque;
} ThreadCB;

class Thread {
public:
	Thread();
	~Thread();

    void						start(const ThreadCB &threadCB);
    int							join();
	void                        stop();

protected:
    bool						mRunning;
	
private:
    pthread_t                   mThread;
	ThreadCB                    mCB;
	static void*				startThread(void* ptr);
};

#endif //FFMPEG_DECODER_H
