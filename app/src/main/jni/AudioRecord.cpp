#include "AudioRecord.h"
#include "AALog.h"
#include <stdlib.h>
#include <fcntl.h>
/* Size of the recording buffer queue */
#define NB_BUFFERS_IN_QUEUE 1

/* Explicitly requesting SL_IID_ANDROIDSIMPLEBUFFERQUEUE and SL_IID_ANDROIDCONFIGURATION
 * on the AudioRecorder object */
#define NUM_EXPLICIT_INTERFACES_FOR_RECORDER 2

/* Size of the recording buffer queue */
//#define NB_BUFFERS_IN_QUEUE 1
/* Size of each buffer in the queue */
//#define BUFFER_SIZE_IN_SAMPLES 8192
//#define BUFFER_SIZE_IN_BYTES   (2 * BUFFER_SIZE_IN_SAMPLES)

/* Local storage for Audio data */
//int8_t pcmData[NB_BUFFERS_IN_QUEUE * BUFFER_SIZE_IN_BYTES];

/* Callback for recording buffer queue events */

//#define ORG_SOUND

#ifdef ORG_SOUND
#include <string>
static FILE *gorgfile = NULL;
#endif

//#define LOG_TIME
#ifdef LOG_TIME
	static struct timeval tv0;
	static struct timeval tv1;
#endif

using namespace std;
using namespace AA;

#define INIT_ABUF_NUM 2

#define TIMEVAL_2_STAMP(timev) (timev.tv_usec)/1000 + (timev.tv_sec)*1000

static int64_t getCurrentSystemTime(){
   struct timeval tv;
    gettimeofday(&tv,NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

void recCallback(SLRecordItf caller, void *pContext, SLuint32 event) {
	if (SL_RECORDEVENT_HEADATNEWPOS & event) {
		SLmillisecond pMsec = 0;
		(*caller)->GetPosition(caller, &pMsec);
//		LOGI("NEWPOS current position=%ums\n", pMsec);
	}

	if (SL_RECORDEVENT_HEADATMARKER & event) {
		SLmillisecond pMsec = 0;
		(*caller)->GetPosition(caller, &pMsec);
// 		LOGI("MARKER current position=%ums\n", pMsec);
	}
}

/* Callback for recording buffer queue events */
void AudioRecord::recBufferQueueCallback(SLAndroidSimpleBufferQueueItf queueItf, void *pContext) {

	AudioRecord *pCntxt = (AudioRecord*) pContext;
	if(pCntxt){
		pCntxt->doSamples(queueItf);
	}
}

void *AudioRecord::processThreadFunc(void *data){
	AudioRecord *pCntxt = (AudioRecord*)data;
	if(pCntxt){
		pCntxt->doProcess();
	}
	return NULL;
}

SLuint32 AudioRecord::convertSLSamplerate(int sampleRate){
     switch(sampleRate) {
     case 8000:
         return SL_SAMPLINGRATE_8;
         break;
     case 11025:
         return SL_SAMPLINGRATE_11_025;
         break;
     case 16000:
         return SL_SAMPLINGRATE_16;
         break;
     case 22050:
         return SL_SAMPLINGRATE_22_05;
         break;
     case 24000:
         return SL_SAMPLINGRATE_24;
         break;
     case 32000:
         return SL_SAMPLINGRATE_32;
         break;
     case 44100:
         return SL_SAMPLINGRATE_44_1;
         break;
     case 48000:
         return SL_SAMPLINGRATE_48;
         break;
     case 64000:
         return SL_SAMPLINGRATE_64;
         break;
     case 88200:
         return SL_SAMPLINGRATE_88_2;
         break;
     case 96000:
         return SL_SAMPLINGRATE_96;
         break;
     case 192000:
         return SL_SAMPLINGRATE_192;
         break;
     default:
         return -1;
     }
 }
AudioRecord::AudioRecord(int sampleRate, int bytesPerSample, int channelNumbre, int minBufferSize):
        mSampleRate(sampleRate),mBytesPerSample(bytesPerSample),mChannelNumber(channelNumbre),
        engineObject(NULL),engineEngine(NULL),recorderObject(NULL),
        recordItf(NULL),recBuffQueueItf(NULL),configItf(NULL),
        bufSize(0), audioState(AudioStateInit),
        mFrameCallback(NULL),mUserData(NULL),mStatue(NATIVE_AUDIORECORD_SUCCESS){
    SLresult result;
    SLEngineOption EngineOption[] = {
                {(SLuint32) SL_ENGINEOPTION_THREADSAFE, (SLuint32) SL_BOOLEAN_TRUE}};

    SLDataFormat_PCM format_pcm;
    format_pcm.formatType = SL_DATAFORMAT_PCM;
    format_pcm.numChannels = channelNumbre;
    format_pcm.samplesPerSec = convertSLSamplerate(sampleRate);
    format_pcm.bitsPerSample = SL_PCMSAMPLEFORMAT_FIXED_16;
    format_pcm.containerSize = SL_PCMSAMPLEFORMAT_FIXED_16;
    if(channelNumbre==2){
        format_pcm.channelMask = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
    }else{
        format_pcm.channelMask = SL_SPEAKER_FRONT_CENTER;
    }
    format_pcm.endianness = SL_BYTEORDER_LITTLEENDIAN;

#ifdef ORG_SOUND
    string orgfn(fileName);
    orgfn+=".org";
    gorgfile = fopen(orgfn.c_str(), "wb");
#endif
    result = slCreateEngine(&engineObject, 1, EngineOption, 0, NULL, NULL);
    if(SL_RESULT_SUCCESS != result){
    	mStatue = -2;
    	release();
    	return;
    }

     /* Realizing the SL Engine in synchronous mode. */
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
    if(SL_RESULT_SUCCESS != result){
    	mStatue = -3;
    	release();
    	return;
    }

    // get the engine interface, which is needed in order to create other objects
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);
    if(SL_RESULT_SUCCESS != result){
    	mStatue = -4;
    	release();
    	return;
    }

    /* setup the data source*/
        SLDataLocator_IODevice ioDevice = {
                SL_DATALOCATOR_IODEVICE,
                SL_IODEVICE_AUDIOINPUT,
                SL_DEFAULTDEVICEID_AUDIOINPUT,
                NULL
        };

        SLDataSource recSource = {&ioDevice, NULL};

        SLDataLocator_AndroidSimpleBufferQueue recBufferQueue = {
                SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,
                NB_BUFFERS_IN_QUEUE
        };

        SLDataSink dataSink = { &recBufferQueue, &format_pcm };
        SLInterfaceID iids[NUM_EXPLICIT_INTERFACES_FOR_RECORDER] = {SL_IID_ANDROIDSIMPLEBUFFERQUEUE, SL_IID_ANDROIDCONFIGURATION};
        SLboolean required[NUM_EXPLICIT_INTERFACES_FOR_RECORDER] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};

        /* Create the audio recorder */
        result = (*engineEngine)->CreateAudioRecorder(engineEngine, &recorderObject , &recSource, &dataSink,
                NUM_EXPLICIT_INTERFACES_FOR_RECORDER, iids, required);
        if(SL_RESULT_SUCCESS != result){
            recorderObject = NULL;
        	mStatue = -5;
        	release();
        	return;
        }

        /* get the android configuration interface*/
        result = (*recorderObject)->GetInterface(recorderObject, SL_IID_ANDROIDCONFIGURATION, &configItf);
        if(SL_RESULT_SUCCESS != result){
        	mStatue = -6;
        	release();
        	return;
        }

        /* Realize the recorder in synchronous mode. */
        result = (*recorderObject)->Realize(recorderObject, SL_BOOLEAN_FALSE);
        if(SL_RESULT_SUCCESS != result){
        	mStatue = -7;
        	release();
        	return;
        }

        /* Get the buffer queue interface which was explicitly requested */
        result = (*recorderObject)->GetInterface(recorderObject, SL_IID_ANDROIDSIMPLEBUFFERQUEUE, (void*) &recBuffQueueItf);
        if(SL_RESULT_SUCCESS != result){
        	mStatue = -8;
        	release();
        	return;
        }

        /* get the record interface */
        result = (*recorderObject)->GetInterface(recorderObject, SL_IID_RECORD, &recordItf);
        if(SL_RESULT_SUCCESS != result){
        	mStatue = -9;
        	release();
        	return;
        }

        /* Set up the recorder callback to get events during the recording */
        result = (*recordItf)->SetMarkerPosition(recordItf, 2000);
        if(SL_RESULT_SUCCESS != result){
        	mStatue = -10;
        	release();
        	return;
        }

        result = (*recordItf)->SetPositionUpdatePeriod(recordItf, 500);
        if(SL_RESULT_SUCCESS != result){
        	mStatue = -11;
        	release();
        	return;
        }

        result = (*recordItf)->SetCallbackEventsMask(recordItf,
                    SL_RECORDEVENT_HEADATMARKER | SL_RECORDEVENT_HEADATNEWPOS);
        if(SL_RESULT_SUCCESS != result){
        	mStatue = -12;
        	release();
        	return;
        }

        result = (*recordItf)->RegisterCallback(recordItf, recCallback, NULL);
        if(SL_RESULT_SUCCESS != result){
        	mStatue = -13;
        	release();
        	return;
        }

        /* Initialize the callback and its context for the recording buffer queue */
        bufSize = minBufferSize;

        for(int i=0;i<INIT_ABUF_NUM;i++){
        	ABuffer* abuf = createABuffer();
        	if(abuf){
        		sampleBufQ.push(abuf);
        	}
        }
        if(bufList.empty()){
        	mStatue = -14;
        	release();
        	return;
        }

        result = (*recBuffQueueItf)->RegisterCallback(recBuffQueueItf, recBufferQueueCallback, this);
        if(SL_RESULT_SUCCESS != result){
        	mStatue = -15;
        	release();
        }
}

AudioRecord::~AudioRecord(){
    release();
}

void AudioRecord::start(){
    SLresult result;
    if (recordItf != NULL) {
        // in case already recording, stop recording and clear buffer queue
        result = (*recordItf)->SetRecordState(recordItf, SL_RECORDSTATE_STOPPED);
        result = (*recBuffQueueItf)->Clear(recBuffQueueItf);

        /* Enqueue buffers to map the region of memory allocated to store the recorded data */
        ABuffer *sampledBuf = NULL;
        if(sampleBufQ.empty()){
        	ABuffer* abuf = createABuffer();
        	if(abuf){
        		sampleBufQ.push(abuf);
        	}
        }
        if(!sampleBufQ.empty()){
        	sampledBuf = sampleBufQ.front();
        }
    	aBufferEnqueue(recBuffQueueItf, sampledBuf);
        assert(SL_RESULT_SUCCESS == result);

        result = (*recordItf)->SetRecordState(recordItf, SL_RECORDSTATE_RECORDING);

        SLuint32 pState = 0;
        (*recordItf)->GetRecordState(recordItf, &pState);
        assert(SL_RESULT_SUCCESS == result);
        LOGI("Start to record state: %ld", pState);

        audioState = AudioStateStarted;
        pthread_t tid;
        pthread_create(&tid, NULL, processThreadFunc, this);
    }

}
void AudioRecord::pause(){
    SLresult result;
    if (recordItf != NULL) {
        result = (*recordItf)->SetRecordState(recordItf, SL_RECORDSTATE_PAUSED);
        assert(SL_RESULT_SUCCESS == result);
        SLuint32 pState = 0;
        (*recordItf)->GetRecordState(recordItf, &pState);
        LOGI("pause to record state: %ld", pState);
    }
    audioState = AudioStatePaused;
    processCond.broadcast();
}
void AudioRecord::stop(){
    if (recordItf != NULL) {
        SLresult result = (*recordItf)->SetRecordState(recordItf, SL_RECORDSTATE_STOPPED);
        assert(SL_RESULT_SUCCESS == result);
        SLuint32 pState = 0;
        (*recordItf)->GetRecordState(recordItf, &pState);
        LOGI("Stop to record state:%ld",pState);
    }
    audioState = AudioStateStoped;
    processCond.broadcast();
}

void AudioRecord::setOnFrameCallback(onFrameCallback cb, void* userData){
	mFrameCallback = cb;
	mUserData = userData;
}

ABuffer* AudioRecord::createABuffer(){
	ABuffer *newBuf = new ABuffer;
	if(NULL == newBuf){
		return NULL;
	}
	newBuf->buf = new int8_t[bufSize];
	if(NULL != newBuf->buf){//avoid no memory
		newBuf->sizeInBytes = bufSize;
		bufList.push_back(newBuf);
		return newBuf;
	}else{
		delete newBuf;
		return NULL;
	}
}

void AudioRecord::releaseListABuffers(){
	LOGI("in func %s,,AbufNum:%d",__FUNCTION__,bufList.size());
	sampleMutex.lock();
	while(!sampleBufQ.empty()){
		sampleBufQ.pop();
	}
	sampleMutex.unlock();

	processMutex.lock();
	while(!processBufQ.empty()){
		processBufQ.pop();
	}
	processMutex.unlock();
	processCond.broadcast();

	if(bufList.size()>0){
		for(int i=0; i<bufList.size(); ++i){
			ABuffer *abuf = bufList[i];
			if(abuf){
				if(abuf->buf){
					delete [](int8_t*)(abuf->buf);
				}
				delete abuf;
			}
		}
		bufList.clear();
	}
	LOGI("out func %s",__FUNCTION__);
}

void AudioRecord::doSamples(SLAndroidSimpleBufferQueueItf queueItf){
	ABuffer *sampledBuf = NULL;
	sampleMutex.lock();
	if(!sampleBufQ.empty()){
		sampledBuf = sampleBufQ.front();
		sampleBufQ.pop();
	}
	sampleMutex.unlock();

	if(sampledBuf && sampledBuf->buf && sampledBuf->sizeInBytes > 0){
		processMutex.lock();
		processBufQ.push(sampledBuf);
		processMutex.unlock();
		processCond.signal();
	}

	sampledBuf = NULL;
	sampleMutex.lock();
	if(sampleBufQ.empty()){
		ABuffer *newBuf = createABuffer();
		if(NULL != newBuf){//avoid no memory
			sampleBufQ.push(newBuf);
		}
	}
	if(!sampleBufQ.empty()){
		sampledBuf = sampleBufQ.front();
	}
	sampleMutex.unlock();
	aBufferEnqueue(queueItf, sampledBuf);
}

void AudioRecord::doProcess(){
	while(audioState == AudioStateStarted){
		ABuffer *processingBuf = NULL;

		processingBuf = NULL;
		processMutex.lock();
		if(processBufQ.empty()){
			processCond.wait(processMutex);
		}
		if(!processBufQ.empty() && (audioState == AudioStateStarted)){
			processingBuf = processBufQ.front();
			processBufQ.pop();
		}
		processMutex.unlock();

		if(audioState == AudioStateStarted){
            if(processingBuf && processingBuf->buf && processingBuf->sizeInBytes > 0){
            #ifdef ORG_SOUND
            		if(gorgfile){
            			fwrite(processingBuf->buf, processingBuf->sizeInBytes, sizeof(int8_t), gorgfile);
            		}
            #endif
		        if(mFrameCallback){
                    mFrameCallback(processingBuf->buf, (processingBuf->sizeInBytes)*sizeof(int8_t), mUserData);
                }
			    sampleMutex.lock();
			    sampleBufQ.push(processingBuf);
			    sampleMutex.unlock();
			}
		}
	}
}

bool AudioRecord::aBufferEnqueue(SLAndroidSimpleBufferQueueItf queueItf, ABuffer *sampledBuf){
	if(queueItf && sampledBuf && sampledBuf->buf && sampledBuf->sizeInBytes > 0){
		(*queueItf)->Enqueue(queueItf, sampledBuf->buf, sampledBuf->sizeInBytes);
		return true;
	}
	return false;
}

void AudioRecord::releaseEngine(){
    // destroy engine object, and invalidate all associated interfaces
	LOGI("in func %s",__FUNCTION__);
    if (engineObject != NULL) {
        (*engineObject)->Destroy(engineObject);
        engineObject = NULL;
        engineEngine = NULL;
    }
    LOGI("out func %s",__FUNCTION__);
}

void AudioRecord::releaseRecorder(){
    //destroy recorder object , and invlidate all associated interfaces
	LOGI("in func %s",__FUNCTION__);
    if (recorderObject != NULL) {
        (*recorderObject)->Destroy(recorderObject);
        recorderObject = NULL;
        recordItf = NULL;
        recBuffQueueItf = NULL;
        configItf = NULL;
    }
    LOGI("out func %s",__FUNCTION__);
}

void AudioRecord::release(){
	audioState = AudioStateReleased;
	releaseRecorder();
    releaseEngine();

#ifdef ORG_SOUND
    if(gorgfile != NULL){
    	fclose(gorgfile);
    	gorgfile = NULL;
    }
#endif

    releaseListABuffers();

    LOGI("release record");
}
