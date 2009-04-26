#include "precompiled.h"
#include "mainfrm.h"

CAppModule	_Module;

//------------------------------------------------------------------------------
int Run(LPTSTR /*lpstrCmdLine*/ = NULL, int nCmdShow = SW_SHOWDEFAULT)
{
	CMessageLoop theLoop;
	_Module.AddMessageLoop(&theLoop);

	CMainFrame wndMain;

	if(wndMain.CreateEx() == NULL)
	{
		ATLTRACE(_T("Main window creation failed!\n"));
		return 0;
	}

	wndMain.ShowWindow(nCmdShow);

	int nRet = theLoop.Run();

	_Module.RemoveMessageLoop();
	return nRet;
}

// -----------------------------------------------------------------------------
int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, 
					 LPTSTR lpstrCmdLine, int nCmdShow)

{
	CreateMutex(NULL, false, L"louderit_mutex");
	if (GetLastError() == ERROR_ALREADY_EXISTS)
		return 0;

	HRESULT hr = CoInitialize(NULL);
	ATLASSERT(SUCCEEDED(hr));

	hr = _Module.Init(NULL, hInstance);

	int ret = Run(lpstrCmdLine, SW_HIDE);
	ATLASSERT(SUCCEEDED(ret));

	_Module.Term();
	CoUninitialize();
	return 0;
}