#pragma comment(lib, "winmm.lib")
#include "LockfreeStack.h"
#include "CCrashDump.h"
#include <Windows.h>
#include <stdio.h>
#include <process.h>


LONG CCrashDump::_DumpCount = 0;
CCrashDump _Dump;
#define dfTHREAD_NUM	4
#define dfMAXALLOC		10000


struct st_DATA {
	LONG64 lData;
	LONG64 lCount;
	st_DATA() {
		lData = 0x0000000055555555;
		lCount = 0;
	}
};

CLockfreeStack<st_DATA*> Stack;
LONG64 g_AllocCount = 0;
volatile LONG64 g_StackSize = 0;

unsigned int WINAPI WorkerThread(LPVOID lParam) {
	st_DATA** Data = new st_DATA*[dfMAXALLOC];
	InterlockedAdd64(&g_AllocCount, dfMAXALLOC);
	while (1) {
		//데이터 넣기
		for (int i = 0; i < dfMAXALLOC; i++) {
			Data[i] = new st_DATA;
			Stack.Push(Data[i]);
		}

		//데이터 검사
		for (int i = 0; i < dfMAXALLOC; i++) {
			Stack.Pop(&Data[i]);
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
			Stack.Push(Data[i]);
		}

		//데이터 뽑기
		for (int i = 0; i < dfMAXALLOC; i++) {
			st_DATA* PopData;
			Stack.Pop(&PopData);
			delete PopData;
		}

	}

}

int wmain() {
	timeBeginPeriod(1);
	DWORD curTime = timeGetTime();

	for (int i = 0; i < dfTHREAD_NUM; i++) {
		HANDLE _hWorkerThread = (HANDLE)_beginthreadex(NULL, 0, WorkerThread, 0, 0, NULL);
		CloseHandle(_hWorkerThread);
	}

	while (1) {
		if (timeGetTime() - curTime >= 1000) {
			wprintf(L"[Alloc Count: %d]\n", g_AllocCount);
			wprintf(L"[Stack Size: %d]\n", Stack.Size());
			wprintf(L"[ObjectPool Alloc: %d]\n", Stack.AllocCount());
			wprintf(L"[ObjectPool UseCount: %d]\n\n", Stack.UseCount());
			curTime = timeGetTime();
		}
	}
}