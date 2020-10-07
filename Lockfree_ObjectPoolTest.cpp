#pragma comment(lib, "winmm.lib")
#include "Lockfree_ObjectPool.h"
#include "CCrashDump.h"
#include <Windows.h>
#include <stdio.h>
#include <process.h>

#define dfMAX_ALLOC	4000
LONG CCrashDump::_DumpCount = 0;
CCrashDump _Dump;



struct st_DATA {
	LONG64 lData;
	LONG64 lCount;
	st_DATA() {
		lData = 0x0000000055555555;
		lCount = 0;
	}
};

CObjectPool<st_DATA> Objectpool(dfMAX_ALLOC);

unsigned int WINAPI WorkerThread(LPVOID lParam) {
	//srand((unsigned int)lParam);
	st_DATA** Data = new st_DATA * [dfMAX_ALLOC/4];
	while (1) {
		//int iRand = rand() % 1000;
		//Sleep(0);
		//데이터 검사
		for (int i = 0; i < dfMAX_ALLOC/4; i++) {
			Data[i] = Objectpool.Alloc();
			if (Data[i]->lData != 0x0000000055555555) {
				CCrashDump::Crash();
			}
			if (Data[i]->lCount != 0) {
				CCrashDump::Crash();
			}
			InterlockedIncrement64(&Data[i]->lData);
			InterlockedIncrement64(&Data[i]->lCount);

			//Sleep(0);
			if (Data[i]->lData != 0x0000000055555556) {
				CCrashDump::Crash();
			}
			if (Data[i]->lCount != 1) {
				CCrashDump::Crash();
			}

			Data[i]->lData = 0x0000000055555555;
			Data[i]->lCount = 0;
		}
		for (int i = 0; i < dfMAX_ALLOC/4; i++) {
			Objectpool.Free(Data[i]);
		}
	}
}

int wmain() {
	timeBeginPeriod(1);
	DWORD curTime = timeGetTime();

	for (int i = 0; i < 4; i++) {
		HANDLE _hWorkerThread = (HANDLE)_beginthreadex(NULL, 0, WorkerThread, (LPVOID)i, 0, NULL);
		CloseHandle(_hWorkerThread);
	}

	while (1) {
		if (timeGetTime() - curTime >= 1000) {
			wprintf(L"[Alloc Count: %d]\n", Objectpool.GetAllocCount());
			wprintf(L"[Use Count: %d]\n", Objectpool.GetUseCount());
			if (Objectpool.GetAllocCount() > dfMAX_ALLOC)
				CCrashDump::Crash();
			if (Objectpool.GetUseCount() < 0)
				CCrashDump::Crash();
			curTime = timeGetTime();
		}
	}
}