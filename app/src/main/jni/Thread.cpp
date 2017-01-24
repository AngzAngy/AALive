#include "Thread.h"

Thread::Thread() {
	mCB.callback = NULL;
	mCB.opaque = NULL;
	mRunning = false;
}

Thread::~Thread() {
}

void Thread::start(const ThreadCB &threadCB) {
	mCB.callback = threadCB.callback;
	mCB.opaque = threadCB.opaque;
	mRunning = true;
    pthread_create(&mThread, NULL, startThread, this);
}

void Thread::stop(){
	mRunning = false;
}

int Thread::join() {
	if(!mRunning) {
		return 0;
	}
    return pthread_join(mThread, NULL);
}

void* Thread::startThread(void* ptr) {
	Thread* thread = (Thread *) ptr;
	while(thread && thread->mRunning){
		try {
		    if(thread->mCB.callback != NULL){
			    thread->mCB.callback(thread->mCB.opaque);
			}
		}catch (...){
		}
	}
	thread->mRunning = false;
	return NULL;
}
