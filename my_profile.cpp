/***********
	2020.02.10 (월)

	프로파일러 만들기

	txt로

	name	average		min		max		call

	형태의 결과 만들기

	출력은 application 입장에서 고려하기!

	매크로로 감싸기

	구조체
		이름
		startTime -> Begin에서 얻은 시간 값
		min -> max와 같이 함
		max -> 평균 구할때 빼고 구함 -> 보통 처음 실행 시 시간이 오래 걸리기 때문에 불필요한 값
		Time -> 원본 시간 그대로 누적
		totalCount

	자료구조는 배열로 해도 됨

	최대한 심플하게 짜야됨

	ProfileReset : 중간에 리셋 시키기 -> 초반 준비 시간을 제외하기 위해서



************/

#pragma comment(lib, "Winmm.lib")

#include "my_profile.h"
#include <Windows.h>
#include <tchar.h>
#include <stdio.h>
#include <strsafe.h>
#include <time.h>
#include <conio.h>


LARGE_INTEGER Freq;
PROFILE_THREAD gt_Profile[THREAD_NUM];
DWORD g_Index = -1;
DWORD g_tlsIndex;



//class CProfile {
//public:
//	WCHAR _TagName[64];
//	CProfile(LPCTSTR TagName) {
//		StringCchCopyW(_TagName, 64, TagName);
//		ProfileBegin(TagName);
//	}
//	~CProfile() {
//		ProfileEnd(_TagName);
//	}
//
//};

void ProfileInit() {
	memset(gt_Profile, 0, sizeof(PROFILE_THREAD) * THREAD_NUM);
	g_tlsIndex = TlsAlloc();
	QueryPerformanceFrequency(&Freq);
}

PROFILE_THREAD* SearchProfile(DWORD tlsIndex) {
	if (TlsGetValue(tlsIndex) == NULL) {
		DWORD retval = InterlockedIncrement(&g_Index);
		gt_Profile[retval].ThreadID = GetCurrentThreadId();
		TlsSetValue(tlsIndex, &gt_Profile[retval]);
	}
	return (PROFILE_THREAD*)TlsGetValue(tlsIndex);
}

BOOL GetSampleInfo(const WCHAR* TagName, PROFILE_THREAD* Profile, PROFILE_SAMPLE** ProfileSample) {
	for (DWORD i = 0; i < PRO_SIZE; i++) {
		if (_tcscmp(TagName, Profile->profile[i].szName) == 0) {
			*ProfileSample = &Profile->profile[i];
			return TRUE;
		}
	}

	return FALSE;
}

void ProfileBegin(const WCHAR* TagName) {
	PROFILE_THREAD* Profile = SearchProfile(g_tlsIndex);
	PROFILE_SAMPLE* ProfileSample= NULL;
	//Tag name이 일치하는 부분을 찾는다.
	if (GetSampleInfo(TagName, Profile, &ProfileSample)) {
		QueryPerformanceCounter(&ProfileSample->StartTime);
	}
	//일치한 부분이 없으면 새로 생성한다.
	else {
		for (DWORD i = 0; i < PRO_SIZE; i++) {
			if (Profile->profile[i].Flag == FALSE) {
				StringCchCopyW(Profile->profile[i].szName, 64, TagName);
				Profile->profile[i].Flag = TRUE;
				Profile->profile[i].Min[0] = 0x7fffffffffffffff;
				Profile->profile[i].Min[1] = 0x7fffffffffffffff;
				Profile->profile[i].Max[0] = 0;
				Profile->profile[i].Max[1] = 0;
				QueryPerformanceCounter(&Profile->profile[i].StartTime);
				break;
			}
		}
	}
}
void ProfileEnd(const WCHAR* TagName) {
	LARGE_INTEGER End;
	QueryPerformanceCounter(&End);
	PROFILE_THREAD* Profile = SearchProfile(g_tlsIndex);
	PROFILE_SAMPLE* ProfileSample=NULL;
	if (GetSampleInfo(TagName, Profile, &ProfileSample)) {
		ProfileSample->iTotalTime += (End.QuadPart - ProfileSample->StartTime.QuadPart);
		ProfileSample->Call += 1;
		//Min 채우기
		if (ProfileSample->Min[0] > (End.QuadPart - ProfileSample->StartTime.QuadPart)) {
			ProfileSample->Min[1] = ProfileSample->Min[0];
			ProfileSample->Min[0] = (End.QuadPart - ProfileSample->StartTime.QuadPart);
		}
		else if (ProfileSample->Min[1] > (End.QuadPart - ProfileSample->StartTime.QuadPart)) {
			ProfileSample->Min[1] = (End.QuadPart - ProfileSample->StartTime.QuadPart);
		}
		//Max 채우기
		if (ProfileSample->Max[0] < (End.QuadPart - ProfileSample->StartTime.QuadPart)) {
			ProfileSample->Max[1] = ProfileSample->Max[0];
			ProfileSample->Max[0] = (End.QuadPart - ProfileSample->StartTime.QuadPart);
		}
		else if (ProfileSample->Max[1] < (End.QuadPart - ProfileSample->StartTime.QuadPart)) {
			ProfileSample->Max[1] = (End.QuadPart - ProfileSample->StartTime.QuadPart);
		}
	}
	else {
		throw 1;
	}
}

void ProfileDataOutText() {
	FILE* fp;
	WCHAR szFileName[64];
	SetFileName(szFileName);
	_tfopen_s(&fp, szFileName, _T("wb"));
	_ftprintf_s(fp, _T("ThreadID | Name            | Average        | Min1             | Min2            | Max1            | Max2             | Call\n"));

	for (DWORD i = 0; i <= g_Index; i++) {
		_ftprintf_s(fp, _T("------------------------------------------------------------------------------------------------------------------------------\n"));
		for (DWORD j = 0; j < PRO_SIZE; j++) {
			if (gt_Profile[i].profile[j].Flag == FALSE)
				continue;
			/*
			_ftprintf_s(fp, _T("%s\t | %.4f\t | %.4f\t | %.4f\t | %.4f\t | %.4f\t | %lld\t\n"), g_Profile[i].szName,
				(g_Profile[i].iTotalTime - g_Profile[i].Min[0] - g_Profile[i].Min[1] - g_Profile[i].Max[0] - g_Profile[i].Max[1]) / (g_Profile[i].Call - 4) / (double)Freq.QuadPart,
				g_Profile[i].Min[0] / (double)Freq.QuadPart, g_Profile[i].Min[1] / (double)Freq.QuadPart,
				g_Profile[i].Max[0] / (double)Freq.QuadPart, g_Profile[i].Max[1] / (double)Freq.QuadPart,
				g_Profile[i].Call);
			*/

			_ftprintf_s(fp, _T("%-8d | %-15s | %12.5f㎲ | %12.2f㎲ | %12.2f㎲ | %12.2f㎲ | %12.2f㎲ | %lld\n"), gt_Profile[i].ThreadID, gt_Profile[i].profile[j].szName,
				(gt_Profile[i].profile[j].iTotalTime - gt_Profile[i].profile[j].Min[0] - gt_Profile[i].profile[j].Min[1] - gt_Profile[i].profile[j].Max[0] - gt_Profile[i].profile[j].Max[1])
				/ (gt_Profile[i].profile[j].Call - 4) / (double)Freq.QuadPart * 1000000,
				gt_Profile[i].profile[j].Min[0] / (double)Freq.QuadPart * 1000000, gt_Profile[i].profile[j].Min[1] / (double)Freq.QuadPart * 1000000,
				gt_Profile[i].profile[j].Max[0] / (double)Freq.QuadPart * 1000000, gt_Profile[i].profile[j].Max[1] / (double)Freq.QuadPart * 1000000,
				gt_Profile[i].profile[j].Call);
		}
	}
	
	fclose(fp);
}
void SetFileName(WCHAR* szFileName) {
	tm TM;
	time_t t;
	time(&t);
	localtime_s(&TM, &t);
	StringCchPrintfW(szFileName, 64, _T("Profile_%04d%02d%02d_%02d%02d%02d.txt"), TM.tm_year + 1900, TM.tm_mon + 1, TM.tm_mday, TM.tm_hour, TM.tm_min, TM.tm_sec);
}



//void ProfileReset() {
//	if (_kbhit()) {
//		TCHAR key = _getch();
//		if(key == 'r')
//			ProfileInit();
//	}
//}

//void Sample() {
//	//CProfile CProfile(_T("Sample"));
//	PRO_BEGIN(L"Sample");
//	Sleep(10);
//	PRO_END(L"Sample");
//}
//
//void Sample2() {
//	//CProfile CProfile(_T("Sample2"));
//	time_t t = clock();
//	for (;;) {
//		if (clock() - t > 10)
//			break;
//	}
//}
//
//int _tmain() {
//	timeBeginPeriod(1);
//	ProfileInit(gt_Profile);
//	for (DWORD i = 0; i < 10; i++) {
//		Sample();
//		//Sample2();
//		//ProfileReset();
//	}
//	ProfileDataOutText();
//}