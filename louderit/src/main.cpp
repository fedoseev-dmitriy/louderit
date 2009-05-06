#include "precompiled.h"
#include "tray.h"
#include "volume_impl.h"
#include "volume_mx_impl.h"
#include "lexical_cast.h"
#include "resource.h"

enum hotkeys_ids
{
	HK_UPKEY = 0,
	HK_DOWNKEY,
	HK_MUTEKEY
};

// -----------------------------------------------------------------------------
// Данные, отвечающие за реализацию хука на мышь
// -----------------------------------------------------------------------------

bool	scrollWithTray	= 0;		// Вкл./выкл. управление скроллом над треем
bool	scrollWithCtrl	= 0;
bool	scrollWithAlt	= 0;
bool	scrollWithShift	= 0;
POINT	lastTrayPos;		// Координаты мыши над треем

UINT					WM_TASKBARCREATED	= 0;
UINT					WM_LOADCONFIG		= 0;

//------------------------------------------------------------------------------
HINSTANCE				hInst = NULL;
HWND					hwnd = NULL;
bool					isWindowsXP = false;
int						deviceNumber = 0;

vector<HICON>			hIcons;
int						iconIndex = 0;
int						volumeLevel = 0;
int 					numIcons = 0;

// Для различия двойного и одиночного клика
bool					isDblClick = false;

int						hotKey = 0;
int						keyMod = 0;

IVolume					*pVolume = NULL;

TrayIcon				*pTrayIcon = NULL;

HHOOK					hHook;


//------------------------------------------------------------------------------
// Settings
//------------------------------------------------------------------------------

bool						balloonHint = false;
int							steps = 0;
int							trayCommands[] = {0,0,0};
int							balance = 50;

wchar_t						config_file[MAX_PATH] = {0};
wchar_t						device_name[256] = {0};
wchar_t						skin[256] = {0};

//-----------------------------------------------------------------------------
bool Launch(HWND hwnd, const wchar_t* command_line, const wchar_t* parameters = NULL,
			unsigned int cmd_show = SW_SHOWNORMAL)
{
	HINSTANCE instance = ShellExecute(hwnd, NULL/*operation*/, 
		command_line, parameters, NULL/*dir*/, cmd_show);

	if ((int)instance <= 32)
	{
		return false;
	}
	return true;
}
//------------------------------------------------------------------------------
// Return true - XP, return false - Vista or later
//------------------------------------------------------------------------------
bool CheckXP()
{
	bool				preVista;
	OSVERSIONINFO		osvi;

	ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	GetVersionEx(&osvi);

	preVista = osvi.dwMajorVersion < 6;
	return preVista;
}

//------------------------------------------------------------------------------
// Выгрузка горячих клавиш
//------------------------------------------------------------------------------
void UnregHotKeys()
{
	UnregisterHotKey(hwnd, HK_UPKEY);
	UnregisterHotKey(hwnd, HK_DOWNKEY);
	UnregisterHotKey(hwnd, HK_MUTEKEY);
}
//------------------------------------------------------------------------------
// Установка горячих клавиш
//------------------------------------------------------------------------------
BOOL SetHotKey(const wstring& SKey, const wstring& SMod, int NumKey)
{
	hotKey = GetPrivateProfileInt(L"HotKeys", SKey.c_str(), 0, config_file);
	if (hotKey != 0)
	{
		keyMod = GetPrivateProfileInt(L"HotKeys", SMod.c_str(), 0, config_file);
		return RegisterHotKey(hwnd, NumKey, keyMod, hotKey);
	}
	return false;
}

//------------------------------------------------------------------------------
// Getting the application directory
//------------------------------------------------------------------------------
bool GetAppPath(wchar_t *path)
{
	wchar_t		path_buff[MAX_PATH] = {0};
	wchar_t		*path_name = 0;
		
	if ((!GetModuleFileName(NULL, path_buff, MAX_PATH)) ||
		(!GetFullPathName(path_buff, MAX_PATH, path, &path_name)))
	{
		*path = '\0';
		return false;
	}

	*path_name = '\0';
	return true;
}

//------------------------------------------------------------------------------
// Загрузка настроек
//------------------------------------------------------------------------------
void LoadConfig()
{
	GetAppPath(config_file);
	_tcscat_s(config_file, L"\\lconfig.ini");

	// [General]
	//...устройства
	GetPrivateProfileString(L"General", L"Device", 0, device_name, 256, config_file);
		
	
	// [General]
	int n = pVolume->GetNumDevice();

	for (int i = 0; n - 1 >= i; ++i)
	{
		// FIXME: ТУТ КРОЕТСЯ ОШИБКА!!!!!!!
		if (lstrcmp(device_name, pVolume->GetDeviceName(i).c_str()) == 0)
		{
			deviceNumber = i;
			break;
		}
	}
	lstrcpyn(device_name, pVolume->GetDeviceName(deviceNumber).c_str(),
		sizeof(device_name)/sizeof(device_name[0]));

	// INIT MIXER!
	// сюда перенести pVolume->Init(...);

	balance = GetPrivateProfileInt(L"General", L"Balance", 50, config_file);

	//...скорости регулирования
	steps = GetPrivateProfileInt(L"General", L"Steps", 5, config_file);

	GetPrivateProfileString(L"View", L"Skin", L"Classic.lsk", &skin[0], 1024, config_file);
	balloonHint = GetPrivateProfileInt(L"View", L"BalloonHint", 0, config_file);

	scrollWithTray = GetPrivateProfileInt(L"Mouse", L"Tray", 1, config_file);
	scrollWithCtrl = GetPrivateProfileInt(L"Mouse", L"Ctrl", 0, config_file);
	scrollWithAlt = GetPrivateProfileInt(L"Mouse", L"Alt", 0, config_file);
	scrollWithShift = GetPrivateProfileInt(L"Mouse", L"Shift", 0, config_file);

	//...действий над треем
	trayCommands[0] = GetPrivateProfileInt(L"Mouse", L"Click", 1, config_file);
	trayCommands[1] = GetPrivateProfileInt(L"Mouse", L"DClick", 2, config_file);
	trayCommands[2] = GetPrivateProfileInt(L"Mouse", L"MClick", 3, config_file);

	UnregHotKeys();
	SetHotKey(L"UpKey", L"UpMod", HK_UPKEY);
	SetHotKey(L"DownKey", L"DownMod", HK_DOWNKEY);
	SetHotKey(L"MuteKey", L"MuteMod", HK_MUTEKEY);
}
//------------------------------------------------------------------------------
// Загрузка иконок
//------------------------------------------------------------------------------

void LoadIcons()
{
	wchar_t			path[MAX_PATH] = {0};
    
	GetAppPath(path);
	_tcscat_s(path, L"\\skins\\");
	_tcscat_s(path, skin);

	//get number of icons contained in the skin 
	numIcons = ExtractIconEx(path, -1, NULL, NULL, 0);

	if (numIcons > 1)
	{
		for (int i = 0; i < numIcons; ++i)
		{
			hIcons.push_back(ExtractIcon(hInst, path, i));
		}
	}
	else
	{
		// FIXME!
		numIcons = 2;
		hIcons.push_back(LoadIcon(NULL, IDI_ERROR));
		hIcons.push_back(LoadIcon(NULL, IDI_ERROR));
	}
}

//------------------------------------------------------------------------------
// Обновление трея
//------------------------------------------------------------------------------
void UpdateTrayIcon()
{
	volumeLevel	= pVolume->GetVolume();
	iconIndex	= volumeLevel * (numIcons / 2 - 1) / MAX_VOL;
	
	if (pVolume->GetMute())
		iconIndex = iconIndex + (numIcons / 2);

	pTrayIcon->Update(hIcons[iconIndex], L"LouderIt");
}

//-----------------------------------------------------------------------------
wstring GetMixerCmdLine()
{
	wstring str = L"-d " + lexical_cast<wstring>(deviceNumber);
	return str;
}

//-----------------------------------------------------------------------------
void ProcessPopupMenu()
{
	HMENU	hMenu;
	POINT	cursorPos;
	UINT	command;

	hMenu = GetSubMenu(LoadMenu(hInst, MAKEINTRESOURCE(IDR_CONTEXT)), 0);

SetForegroundWindow(hwnd);
	GetCursorPos(&cursorPos);

	command = (UINT)TrackPopupMenuEx(hMenu, TPM_RETURNCMD + TPM_VERTICAL, 
		cursorPos.x, cursorPos.y, hwnd, NULL);

	PostMessage(hwnd, WM_NULL, 0, 0);

	switch (command)
	{
	case ID_TRAYMENU_OPENMIXER:
		{
			if (isWindowsXP)
			{
				Launch(hwnd, L"sndvol32.exe", GetMixerCmdLine().c_str());
			}
			else
			{
				Launch(hwnd, L"sndvol.exe");
			}
			break;
		}
	case ID_TRAYMENU_AUDIOPROPERTIES:
		Launch(hwnd, L"rundll32.exe", L"shell32.dll,Control_RunDLL mmsys.cpl");
		break;
	case ID_TRAYMENU_SETTINGS:
		Launch(hwnd, L"LConfig.exe");
		break;
		//case IDM_ABOUT:
		//  Launch(L"LConfig.exe -a");
		//break;
	case ID_TRAYMENU_EXIT:	
		DestroyWindow(hwnd);
		break;
	}
	DestroyMenu(hMenu);
}

//------------------------------------------------------------------------------
void TrayCommand(int flag)
{
	switch (flag)
	{
	case 1:
		{
			if (isWindowsXP)
			{
				Launch(hwnd, L"sndvol32.exe", L"-t");
			}
			else
			{
				Launch(hwnd, L"sndvol.exe", L"-f 49349574");
			}
			break;
		}
	case 2:
		{
			if (isWindowsXP)
			{
				Launch(hwnd, L"sndvol32.exe", GetMixerCmdLine().c_str());
			}
			else
			{
				Launch(hwnd, L"sndvol.exe");
			}
			break;
		}
	case 3:
		pVolume->SetMute(!pVolume->GetMute());
		break;
	case 4:
		Launch(hwnd, L"LConfig.exe");
		break;
	}
}

//------------------------------------------------------------------------------
void VolumeUp()
{
	int rightChannelVol	= 0;
	int leftChannelVol	= 0;

	volumeLevel = min((volumeLevel + steps), MAX_VOL);

	if (balance == 50)
	{
		pVolume->SetVolume(volumeLevel);
		//pVolume->SetVolumeChannel(nVolumeLevel, nVolumeLevel);
	}
	else
	{
		if (balance > 50)
		{
			leftChannelVol = max(volumeLevel + (volumeLevel * (50 - balance)) / 50, 0);
			rightChannelVol = volumeLevel;
		}
		else
		{
			leftChannelVol = volumeLevel;
			rightChannelVol = max(volumeLevel - (volumeLevel * (50 - balance)) / 50, 0);
		}
		pVolume->SetVolumeChannel(leftChannelVol, rightChannelVol);
	}
}

//------------------------------------------------------------------------------
void VolumeDown()
{
	int rightChannelVol = 0;
	int leftChannelVol	= 0;
	
	volumeLevel = max((volumeLevel - steps), 0);

	if (balance == 50)
	{
		pVolume->SetVolume(volumeLevel);
		//pVolume->SetVolumeChannel(nVolumeLevel, nVolumeLevel);
	}
	else
	{
		if (balance > 50)
		{
			rightChannelVol = max(volumeLevel + (volumeLevel * (50 - balance)) / 50, 0);
			leftChannelVol = volumeLevel;
		}
		else
		{
			rightChannelVol = volumeLevel;
			leftChannelVol = max(volumeLevel - (volumeLevel * (50 - balance)) / 50, 0);
		}
		pVolume->SetVolumeChannel(leftChannelVol, rightChannelVol);
	}
}



//------------------------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_LOADCONFIG)
	{
		LoadConfig();
		LoadIcons();
		pTrayIcon->Update(hIcons[iconIndex], L"LouderIt");
	}

	else if (message == WM_TASKBARCREATED)
	{
		pTrayIcon->Restore();
	}

	//-----------------------------------------------------------------------------
	switch (message) 
	{
	case MM_MIXM_CONTROL_CHANGE:
		//if (lParam == mxVolCtrl.dwControlID  || lParam == mxMuteCtrl.dwControlID)
		//{
			UpdateTrayIcon();
		//}
		break;

	case WM_NOTIFYICONTRAY:
		{
			switch (lParam)
			{

			case WM_LBUTTONDOWN:
				SetTimer(hWnd, 1, GetDoubleClickTime(), NULL);
				break;

			case WM_LBUTTONDBLCLK:
				isDblClick = true;
				TrayCommand(trayCommands[1]);
				break;

			case WM_MBUTTONUP:
				TrayCommand(trayCommands[2]);
				break;

			case WM_RBUTTONUP:
				ProcessPopupMenu();
				break;

			case WM_MOUSEMOVE:
				GetCursorPos(&lastTrayPos);
				break;
			}
		}
		break;

	case WM_TIMER:
		if (wParam == 1)
		{
			KillTimer(hWnd, 1);
			if (!isDblClick)
			{
				TrayCommand(trayCommands[0]);
			}
			else
			{
				isDblClick = false;
			}
		}
		if (wParam == 2)
		{
			KillTimer(hWnd, 2);
			pTrayIcon->ShowBaloon(L"", L"");
		}
		break;

	case WM_HOTKEY:
		{
			switch (wParam)
			{
			case HK_UPKEY:
				VolumeUp();
				break;
			case HK_DOWNKEY:
				VolumeDown();
				break;
			case HK_MUTEKEY:
				pVolume->SetMute(!pVolume->GetMute());
				break;
			}
			if (balloonHint)
			{
				wstring str_on = lexical_cast<wstring>(pVolume->GetVolume()) + L"%";
				if (!pVolume->GetMute())
				{
					pTrayIcon->ShowBaloon((lexical_cast<wstring>(pVolume->GetVolume()) + L"%").c_str(), L"Громкость:");
				}
				else
				{
					pTrayIcon->ShowBaloon((lexical_cast<wstring>(pVolume->GetVolume()) + L"% (Выкл.)").c_str(), L"Громкость:");
				}
				SetTimer(hWnd, 2, 3000, NULL);
			}
			break;
		}

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		{
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	return 0;
}
//-----------------------------------------------------------------------------
bool GetKeys()
{
	if (scrollWithCtrl == false 
		&& scrollWithAlt == false
		&& scrollWithShift == false)
	{
		return false;
	}
	if (
		(!scrollWithCtrl ^ GetKeyState(VK_LCONTROL) && 128 != 0)
		&&	(!scrollWithAlt ^ GetKeyState(VK_MENU) && 128 != 0) 
		&& (!scrollWithShift ^ GetKeyState(VK_SHIFT) && 128 != 0) 
		)
		return true;
	return false;
}

LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	HRESULT		hr;	
	POINT		cursorPos;

	if (nCode < 0)  // do not process the message 
		return CallNextHookEx(hHook, nCode, wParam, lParam);



	hr = CallNextHookEx(hHook, nCode, wParam, lParam);

	if ((nCode == HC_ACTION) && (wParam == WM_MOUSEWHEEL)) 
	{
		GetCursorPos( &cursorPos );
		if (cursorPos.x == lastTrayPos.x && cursorPos.y == lastTrayPos.y || GetKeys())
		{

			if ((short) HIWORD(((PMSLLHOOKSTRUCT) lParam)->mouseData) >= 0) // Delta of mouse wheel
			{
				VolumeUp();
			}
			else
			{
				VolumeDown();
			}
			if (balloonHint)
			{
				if (!pVolume->GetMute())
				{
					pTrayIcon->ShowBaloon((lexical_cast<wstring>(pVolume->GetVolume()) + L"%").c_str(), L"Громкость:");

				}
				else
				{
					pTrayIcon->ShowBaloon((lexical_cast<wstring>(pVolume->GetVolume()) + L"% (Выкл.)").c_str(), L"Громкость:");
				}
				SetTimer(hwnd, 2, 3000, NULL);
			}

			hr = S_FALSE;
		}
	}

	return hr;
}

void SetHook(BOOL flag)
{
	if (flag) 
	{
		hHook = SetWindowsHookEx(WH_MOUSE_LL, LowLevelMouseProc, hInst, 0);
		if (hHook <= 0) 
		{
			MessageBox(hwnd, L"SetWindowsHookEx", L"Error!", MB_OK | MB_ICONERROR);
			DestroyWindow(hwnd);
		}

	}
	else
		UnhookWindowsHookEx(hHook);
}

// -----------------------------------------------------------------------------
// Инициализация, регистрация и создание главного окна
// -----------------------------------------------------------------------------
int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
					 LPSTR lpCmdLine, int nShowCmd)
{
	CreateMutex(NULL, false, L"louderit_mutex");
	if (GetLastError() == ERROR_ALREADY_EXISTS)
		return 0;

	//----------------------------------------------------------------
	isWindowsXP = CheckXP();
	hInst = hInstance;

	WNDCLASS wc;

	wc.style			= 0;
	wc.lpfnWndProc		= (WNDPROC)WndProc;
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= 0;
	wc.hInstance		= 0;
	wc.hIcon			= NULL;
	wc.hCursor			= 0;
	wc.hbrBackground	= 0;
	wc.lpszMenuName		= NULL;
	wc.lpszClassName	= L"LouderIt";

	RegisterClass(&wc);
	hwnd = CreateWindow(wc.lpszClassName, L"", 0, 0, 0, 0, 0, 0, 0, hInstance, NULL);
	if (!hwnd)
	{
		return 0;
	}
	// -----------------------------------------------------------------------------
	// Инициализация программы
	// -----------------------------------------------------------------------------
	WM_LOADCONFIG		= RegisterWindowMessage(L"WM_LOADCONFIG");
	WM_TASKBARCREATED	= RegisterWindowMessage(L"TaskbarCreated");

	if (isWindowsXP)
	{
		pVolume = new VolumeMxImpl;
	}
	else // Vista or Win7
	{
		pVolume = new VolumeImpl;
	}
	
	SetHook(true); 
	LoadConfig();

	pVolume->Init(deviceNumber, hwnd);

	LoadIcons();
	
	pTrayIcon = new TrayIcon(hwnd);
	pTrayIcon->Add(hIcons[0], L"LouderIt");
	UpdateTrayIcon();

	// Обработка сообщений программы
	int		retCode;
	MSG		msg;
	while ((retCode = GetMessage(&msg, NULL, 0, 0)) != 0)
	{ 
		if (retCode == -1)
		{
			return false;
		}
		else
		{
			TranslateMessage(&msg); 
			DispatchMessage(&msg); 
		}
	} 
	// -----------------------------------------------------------------------------
	// Подготовка к закрытию программы
	// -----------------------------------------------------------------------------
	pVolume->Shutdown();
	pTrayIcon->Remove();
	delete pTrayIcon;
	UnregHotKeys();
	SetHook(false);
	delete pVolume;
	return 0;
}