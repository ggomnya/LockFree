#pragma comment(lib, "winmm.lib")
#include "LockfreeQueue.h"
#include "CCrashDump.h"
#include <Windows.h>
#include <stdio.h>
#include <process.h>
#include <conio.h>
#include "my_profile.h"


LONG CCrashDump::_DumpCount = 0;
CCrashDump _Dump;
#define dfTHREAD_NUM	8
#define dfMAXALLOC		3


struct st_DATA {
	LONG64 lData;
	LONG64 lCount;
	st_DATA() {
		lData = 0x0000000055555555;
		lCount = 0;
	}
};

CLockfreeQueue<st_DATA*> Queue;
LONG64 g_AllocCount = 0;
LONG64 g_EnqCount = 0;
LONG64 g_DeqCount = 0;
volatile LONG64 g_StackSize = 0;

unsigned int WINAPI WorkerThread(LPVOID lParam) {
	st_DATA** Data = new st_DATA * [dfMAXALLOC];
	InterlockedAdd64(&g_AllocCount, dfMAXALLOC);
	//int idx = 10000;
	while (1) {
		//idx--;
		//데이터 넣기
		for (int i = 0; i < dfMAXALLOC; i++) {
			Data[i] = new st_DATA;
			PRO_BEGIN(L"Enqueue");
			Queue.Enqueue(Data[i]);
			PRO_END(L"Enqueue");
			InterlockedIncrement64(&g_EnqCount);
		}

		//데이터 검사
		for (int i = 0; i < dfMAXALLOC; i++) {
			PRO_BEGIN(L"Dequeue");
			Queue.Dequeue(&Data[i]);
			PRO_END(L"Dequeue");
			InterlockedIncrement64(&g_DeqCount);
			if (Data[i]->lData != 0x0000000055555555) {
				CCrashDump::Crash();
			}
			if (Data[i]->lCount != 0) {
				CCrashDump::Crash();
			}
			InterlockedIncrement64(&Data[i]->lData);
			InterlockedIncrement64(&Data[i]->lCount);


			if (Data[i]->lData != 0x0000000055555556) {
				CCrashDump::Crash();
			}
			if (Data[i]->lCount != 1) {
				CCrashDump::Crash();
			}

			Data[i]->lData = 0x0000000055555555;
			Data[i]->lCount = 0;
		}

		for (int i = 0; i < dfMAXALLOC; i++) {
			PRO_BEGIN(L"Enqueue");
			Queue.Enqueue(Data[i]);
			PRO_END(L"Enqueue");
			InterlockedIncrement64(&g_EnqCount);
		}

		//데이터 뽑기
		for (int i = 0; i < dfMAXALLOC; i++) {
			st_DATA* PopData;
			PRO_BEGIN(L"Dequeue");
			Queue.Dequeue(&PopData);
			PRO_END(L"Dequeue");
			InterlockedIncrement64(&g_DeqCount);
			delete PopData;
		}

	}
	return 0;
}

int wmain() {
	timeBeginPeriod(1);
	DWORD curTime = timeGetTime();
	ProfileInit();
	for (int i = 0; i < dfTHREAD_NUM; i++) {
		HANDLE _hWorkerThread = (HANDLE)_beginthreadex(NULL, 0, WorkerThread, 0, 0, NULL);
		CloseHandle(_hWorkerThread);
	}

	while (1) {
		if (timeGetTime() - curTime >= 1000) {
			wprintf(L"[Alloc Count: %d]\n", g_AllocCount);
			wprintf(L"[Stack Size: %d]\n", Queue.Size());
			wprintf(L"[ObjectPool Alloc: %d]\n", Queue.AllocCount());
			wprintf(L"[ObjectPool UseCount: %d]\n", Queue.UseCount());
			wprintf(L"[EnqCount: %d]\n", g_EnqCount);
			wprintf(L"[DeqCount: %d]\n\n", g_DeqCount);
			g_EnqCount = 0;
			g_DeqCount = 0;
			curTime = timeGetTime();
			if (_kbhit()) {
				if (_getch() == L'a')
					break;
			}
		}
	}
	ProfileDataOutText();
}