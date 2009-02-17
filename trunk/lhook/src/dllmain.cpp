// dllmain.cpp : Defines the entry point for the DLL application.
#include "precompiled.h"
#include "Trace.h"

#pragma data_seg(".shared")
HINSTANCE	hInst = NULL;
HHOOK		hHook = NULL;
int			countRunning = 0;
bool		initialized = false; 
#pragma data_seg()
#pragma comment(linker, "/SECTION:.shared,RWS")

UINT		WM_VOLCHANGE;

typedef struct
{
	bool	scrollWithTray;		// Вкл./выкл. управление скроллом над треем
	bool	scrollWithCtrl;
	bool	scrollWithAlt;
	bool	scrollWithShift;
	POINT	lastTrayPos;		// Координаты мыши над треем
} globalDLLData_t;

static globalDLLData_t*		pSharedMem = NULL; // pointer to shared memory
static HANDLE				hMapObject = NULL;  // handle to file mapping


//-----------------------------------------------------------------------------
bool GetKeys()
{
	if (pSharedMem->scrollWithCtrl == false 
		&& pSharedMem->scrollWithAlt == false
		&& pSharedMem->scrollWithShift == false)
	{
		return false;
	}
	if (
		(!pSharedMem->scrollWithCtrl ^ GetKeyState(VK_LCONTROL) && 128 != 0)
		&&	(!pSharedMem->scrollWithAlt ^ GetKeyState(VK_MENU) && 128 != 0) 
		&& (!pSharedMem->scrollWithShift ^ GetKeyState(VK_SHIFT) && 128 != 0) 
		)
		return true;
	return false;
}

//-----------------------------------------------------------------------------
// Hook-function:
LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	POINT	cursorPos;

	if(nCode == HC_ACTION && ((MSG*)lParam)->message == WM_MOUSEWHEEL)
	{
		GetCursorPos(&cursorPos);

		if (pSharedMem->scrollWithTray && cursorPos.x == pSharedMem->lastTrayPos.x && 
			cursorPos.y == pSharedMem->lastTrayPos.y || GetKeys())
		{
			MSLLHOOKSTRUCT * st = (MSLLHOOKSTRUCT*)lParam; 
			HALF_PTR delta = HIWORD(st->mouseData);

			if (delta > 0)
			{
				if(! PostMessage(HWND_BROADCAST, WM_VOLCHANGE, MAKEWPARAM(cursorPos.x, cursorPos.y), TRUE))
				{
					TraceA("%s", "PostMessage ERROR");
				}
			}
			else
			{
				if(! PostMessage(HWND_BROADCAST, WM_VOLCHANGE, MAKEWPARAM(cursorPos.x, cursorPos.y), FALSE))
				{
					TraceA("%s", "PostMessage ERROR");
				}
			}
			
			((MSG*)lParam)->message = WM_NULL;

		}
	}
	return CallNextHookEx(hHook, nCode, wParam, lParam);
}
//-----------------------------------------------------------------------------
// Функция устанавливает/снимает хук в зависимости от enable
extern "C" __declspec(dllexport) void SetHook(bool enable) 
{
	if (enable)
	{
		hHook  = SetWindowsHookEx(WH_GETMESSAGE, (HOOKPROC)&MouseProc,
			(HINSTANCE)hInst, 0);
		if (hHook == 0)
		{
			TraceA("%s", "SetWindowsHookEx ERROR");
		}
		TraceA("%s", "SetWindowsHookEx - ok");

	}
	else
	{
		if (hHook)
		{
			UnhookWindowsHookEx(hHook);
			hHook = 0;
			TraceA("%s", "UnhookWindowsHookEx - ok");
		}
	}
}
//-----------------------------------------------------------------------------
bool APIENTRY DllMain(HMODULE hModule,
					  DWORD  ul_reason_for_call,
					  LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		{
			countRunning++;

			DisableThreadLibraryCalls(hModule);
			hInst = (HINSTANCE)hModule;
			WM_VOLCHANGE = RegisterWindowMessage((LPCSTR)"WM_VOLCHANGE");

			hMapObject = CreateFileMapping(
				INVALID_HANDLE_VALUE,   // use paging file
				NULL,                   // default security attributes
				PAGE_READWRITE,         // read/write access
				0,                      // size: high 32-bits
				sizeof(globalDLLData_t),      // size: low 32-bits
				TEXT("LouderItMMF")); // name of map object

			if (hMapObject == NULL) 
				return FALSE; 

			// The first process to attach initializes memory
			initialized = (GetLastError() != ERROR_ALREADY_EXISTS); 

			pSharedMem =(globalDLLData_t*) MapViewOfFile(
				hMapObject,     // object to map view of
				FILE_MAP_ALL_ACCESS, // read/write access
				0,              // high offset:  map from
				0,              // low offset:   beginning
				0);             // default: map entire file

			// Initialize memory if this is the first process
			if (initialized)
			{
				ZeroMemory(pSharedMem, sizeof(pSharedMem));
				TraceA("%s", "Clear shared memory - ok");
			}
  
			TraceA("Load LHook %i", countRunning);
			break;
		}
	case DLL_PROCESS_DETACH:
		{
			countRunning--;
			if (pSharedMem)
			{
				UnmapViewOfFile(pSharedMem);
				pSharedMem = NULL;
			}
			if (hMapObject)
			{
				CloseHandle(hMapObject);
				hMapObject = 0;
			}
			TraceA("UNload LHook %i", countRunning);
		}
		break;
	}
	return TRUE;
}

