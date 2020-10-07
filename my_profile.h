#ifndef MY_PROFILE_H
#define MY_PROFILE_H

#include <Windows.h>
#include <tchar.h>

#define PRO_BEGIN(TagName)	ProfileBegin(TagName)
#define PRO_END(TagName)	ProfileEnd(TagName)

#define PRO_SIZE	20
#define THREAD_NUM	4

typedef struct {
	BOOL Flag;
	WCHAR szName[64];

	LARGE_INTEGER StartTime;

	__int64 iTotalTime;
	__int64 Min[2];
	__int64 Max[2];

	__int64 Call;

} PROFILE_SAMPLE;

struct PROFILE_THREAD {
	PROFILE_SAMPLE profile[PRO_SIZE];
	DWORD ThreadID;
};

void ProfileInit();
void ProfileBegin(const WCHAR* TagName);
void ProfileEnd(const WCHAR* TagName);
void ProfileDataOutText();
void SetFileName(WCHAR* szFileName);
void ProfileReset();
PROFILE_THREAD* SearchProfile(DWORD tlsIndex);


BOOL GetSampleInfo(const WCHAR* TagName, PROFILE_THREAD* Profile, PROFILE_SAMPLE** ProfileSample);

#endif