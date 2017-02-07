#ifndef __AudioRecordStat_H_
#define __AudioRecordStat_H_
typedef enum EAudioState{
	AudioStateInit,
	AudioStateStarted,
	AudioStatePaused,
	AudioStateStoped,
	AudioStateReleased
} EAudioState;
#endif//__AudioRecordStat_H_
