#include "precompiled.h"
#include "tray.h"
#include "volume_impl.h"
#include "volume_mx_impl.h"
#include "lexical_cast.h"

enum hotkeys_ids
{
	HK_UPKEY = 0,
	HK_DOWNKEY,
	HK_MUTEKEY
};

enum app_ids
{
	NUM_ICONS = 30
};

enum menu_ids
{
	IDM_OPEN_VOLUME_CONTROL = 40001,
	IDM_SETTING_AUDIO_PARAMETR,
	IDM_SETTING_PROGRAM,	
	IDM_HELP,
	IDM_ABOUT,
	IDM_EXIT
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

HICON					hIcons[NUM_ICONS-1];
int						iconIndex = 0;
int						volumeLevel = 0;

// Для различия двойного и одиночного клика
bool					isClickStart = false;
bool					isClickReady = false;
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
//std::string					deviceName;
//std::string					configFile;
//std::string					skin;
TCHAR						configFile[MAX_PATH] = {0};
TCHAR						deviceName[256] = {0};
TCHAR						skin[256] = {0};


//------------------------------------------------------------------------------
// Return true - XP, return false - Vista or later
//------------------------------------------------------------------------------
bool checkXP()
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
BOOL SetHotKey(const std::string& SKey, const std::string& SMod, int NumKey)
{
	hotKey = GetPrivateProfileInt("HotKeys", SKey.c_str(), 0, configFile);
	if (hotKey != 0)
	{
		keyMod = GetPrivateProfileInt("HotKeys", SMod.c_str(), 0, configFile);
		return RegisterHotKey(hwnd, NumKey, keyMod, hotKey);
	}
	return false;
}

//------------------------------------------------------------------------------
// Загрузка настроек
//------------------------------------------------------------------------------
void LoadConfig()
{
	GetCurrentDirectory(MAX_PATH, configFile);
	strcat_s(configFile, _T("\\lconfig.ini"));

	// [General]
	//...устройства
	GetPrivateProfileString("General", "Device", 0, deviceName, 256, configFile);
		
	
	// [General]
	int n = pVolume->GetNumDevice();

	TraceA("GetNumDevice = %i", n);

	for (int i = 0; n - 1 >= i; i++)
	{
		// FIXME: ТУТ КРОЕТСЯ ОШИБКА!!!!!!!
		if (lstrcmp(deviceName, pVolume->GetDevName(i).c_str()) == 0)
		{
			deviceNumber = i;
			break;
		}
	}
	lstrcpyn(deviceName, pVolume->GetDevName(deviceNumber).c_str(),
		sizeof(deviceName)/sizeof(deviceName[0]));

	TraceA("Using device = %s", deviceName);

	// INIT MIXER!
	// сюда перенести pVolume->Init(...);

	balance = GetPrivateProfileInt("General", "Balance", 50, configFile);

	//...скорости регулирования
	steps = GetPrivateProfileInt("General", "Steps", 5, configFile);

	GetPrivateProfileString("View", "Skin", "Classic.lsk", &skin[0], 1024, configFile);
	balloonHint = GetPrivateProfileInt("View", "BalloonHint", 0, configFile);

	scrollWithTray = GetPrivateProfileInt("Mouse", "Tray", 1, configFile);
	scrollWithCtrl = GetPrivateProfileInt("Mouse", "Ctrl", 0, configFile);
	scrollWithAlt = GetPrivateProfileInt("Mouse", "Alt", 0, configFile);
	scrollWithShift = GetPrivateProfileInt("Mouse", "Shift", 0, configFile);

	//...действий над треем
	trayCommands[0] = GetPrivateProfileInt("Mouse", "Click", 1, configFile);
	trayCommands[1] = GetPrivateProfileInt("Mouse", "DClick", 2, configFile);
	trayCommands[2] = GetPrivateProfileInt("Mouse", "MClick", 3, configFile);

	UnregHotKeys();
	SetHotKey("UpKey", "UpMod", HK_UPKEY);
	SetHotKey("DownKey", "DownMod", HK_DOWNKEY);
	SetHotKey("MuteKey", "MuteMod", HK_MUTEKEY);
}
//------------------------------------------------------------------------------
// Загрузка иконок
//------------------------------------------------------------------------------
void LoadIcons()
{
	TCHAR			path[MAX_PATH] = {0};

	GetCurrentDirectory(MAX_PATH, path);
	strcat_s(path, "\\skins\\");
	strcat_s(path, skin);

	for (int i = 0; NUM_ICONS - 1 >= i; i++)
	{
		hIcons[i] = ExtractIcon(hInst, path, i);
	}
}

//------------------------------------------------------------------------------
// Обновление трея
//------------------------------------------------------------------------------
void UpdateTrayIcon()
{
	volumeLevel	= pVolume->GetVolume();
	iconIndex	= volumeLevel * 14 / MAX_VOL;
	
	if (pVolume->GetMute())
		iconIndex = iconIndex + 15;

	pTrayIcon->Update(hIcons[iconIndex]);
}

//------------------------------------------------------------------------------
bool ShowBalloon(BOOL flag, const std::string& balloonText,
				 const std::string& balloonTitle)
{
	NOTIFYICONDATA nid;

	ZeroMemory(&nid, sizeof(NOTIFYICONDATA));

	nid.cbSize	= sizeof(NOTIFYICONDATA);
	nid.hWnd	= hwnd;
	nid.uID		= 1;
	nid.uFlags	= NIF_INFO;
	//strcpy_s(nid.szTip, "");
	if (flag)
	{
		strcpy_s(nid.szInfo, balloonText.c_str());
		nid.uTimeout = 0;
		strcpy_s(nid.szInfoTitle, balloonTitle.c_str());
	}
	else
	{
		strcpy_s(nid.szInfo, "");
	}
	return Shell_NotifyIcon(NIM_MODIFY, &nid) != 0 ? true : false;
}

//-----------------------------------------------------------------------------
std::string GetMixerCmdLine()
{
	std::string str = "sndvol32.exe -d " + lexical_cast<std::string>(deviceNumber);
	return str;
}

//-----------------------------------------------------------------------------
void ProcessPopupMenu()
{
	HMENU	hMenu;
	POINT	cursorPos;
	UINT	command;

	hMenu	= CreatePopupMenu();
	AppendMenu(hMenu, MF_STRING, IDM_OPEN_VOLUME_CONTROL, _T("Открыть &регулятор громкости"));
	AppendMenu(hMenu, MF_STRING, IDM_SETTING_AUDIO_PARAMETR, _T("Настройка &аудиопараметров"));
	AppendMenu(hMenu, MF_SEPARATOR, 0, NULL) ;
	AppendMenu(hMenu, MF_STRING, IDM_SETTING_PROGRAM, _T("&Настройки программы"));
	AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
	//AppendMenu(hMenu, MF_STRING, IDM_ABOUT, _T("&О программе..."));
	AppendMenu(hMenu, MF_STRING, IDM_EXIT, _T("&Выход"));

	SetForegroundWindow(hwnd);
	GetCursorPos(&cursorPos);

	command = (UINT)TrackPopupMenuEx(hMenu, TPM_RETURNCMD + TPM_VERTICAL, 
		cursorPos.x, cursorPos.y, hwnd, NULL);

	PostMessage(hwnd, WM_NULL, 0, 0);

	switch (command)
	{
	case IDM_OPEN_VOLUME_CONTROL:
		{
			if (isWindowsXP)
			{
				WinExec(GetMixerCmdLine().c_str(), SW_SHOWNORMAL);
			}
			else
			{
				WinExec("sndvol.exe", SW_SHOWNORMAL);
			}
			break;
		}
	case IDM_SETTING_AUDIO_PARAMETR:
		WinExec("rundll32.exe shell32.dll,Control_RunDLL mmsys.cpl", SW_SHOWNORMAL);
		break;
	case IDM_SETTING_PROGRAM:
		WinExec("LConfig.exe", SW_SHOWNORMAL);
		break;
		//case IDM_ABOUT:
		//  WinExec("LConfig.exe -a", SW_SHOWNORMAL);
		//break;
	case IDM_EXIT:
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
				WinExec("sndvol32.exe -t", SW_SHOWNORMAL);
			}
			else
			{
				WinExec("sndvol.exe -f 49349574", SW_SHOWNORMAL);
			}
			break;
		}
	case 2:
		{
			if (isWindowsXP)
			{
				WinExec(GetMixerCmdLine().c_str(), SW_SHOWNORMAL);
			}
			else
			{
				WinExec("sndvol.exe", SW_SHOWNORMAL);
			}
			break;
		}
	case 3:
		pVolume->SetMute(!pVolume->GetMute());
		break;
	case 4:
		WinExec("LConfig.exe", SW_SHOWNORMAL);
		break;
	}
}

//------------------------------------------------------------------------------
void VolumeUp()
{
	int rightChannelVol	= 0;
	int leftChannelVol	= 0;

	volumeLevel = std::min((volumeLevel + steps), MAX_VOL);

	if (balance == 50)
	{
		pVolume->SetVolume(volumeLevel);
		//pVolume->SetVolumeChannel(nVolumeLevel, nVolumeLevel);
	}
	else
	{
		if (balance > 50)
		{
			leftChannelVol = std::max(volumeLevel + (volumeLevel * (50 - balance)) / 50, 0);
			rightChannelVol = volumeLevel;
		}
		else
		{
			leftChannelVol = volumeLevel;
			rightChannelVol = std::max(volumeLevel - (volumeLevel * (50 - balance)) / 50, 0);
		}
		pVolume->SetVolumeChannel(leftChannelVol, rightChannelVol);
	}
}

//------------------------------------------------------------------------------
void VolumeDown()
{
	int rightChannelVol = 0;
	int leftChannelVol	= 0;
	
	volumeLevel = std::max((volumeLevel - steps), 0);

	if (balance == 50)
	{
		pVolume->SetVolume(volumeLevel);
		//pVolume->SetVolumeChannel(nVolumeLevel, nVolumeLevel);
	}
	else
	{
		if (balance > 50)
		{
			rightChannelVol = std::max(volumeLevel + (volumeLevel * (50 - balance)) / 50, 0);
			leftChannelVol = volumeLevel;
		}
		else
		{
			rightChannelVol = volumeLevel;
			leftChannelVol = std::max(volumeLevel - (volumeLevel * (50 - balance)) / 50, 0);
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
		pTrayIcon->Update(hIcons[iconIndex]);
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
				isClickStart = true;
				break;

			case WM_LBUTTONUP:
				if (isClickStart)
				{
					isClickReady = true;
					isClickStart = false;
				}
				break;

			case WM_LBUTTONDBLCLK:
				TrayCommand(trayCommands[1]);
				isClickReady = false;
				isDblClick = true;
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
				if (isClickReady)
				{
					isClickReady = false;
					TrayCommand(trayCommands[0]);
				}
			}
			isDblClick = false;
		}
		if (wParam == 2)
		{
			KillTimer(hWnd, 2);
			ShowBalloon(true, "", "");
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
				if (!pVolume->GetMute())
				{
					ShowBalloon(true, lexical_cast<std::string>(pVolume->GetVolume()) + "%", "Громкость:");
				}
				else
				{
					ShowBalloon(true, lexical_cast<std::string>(pVolume->GetVolume()) + "% (Выкл.)", "Громкость:");
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
				//WHEEL_UP
				VolumeUp();
			}
			else
			{
				//WHEEL_DOWN
				VolumeDown();
			}
			if (balloonHint)
			{
				if (!pVolume->GetMute())
				{
					ShowBalloon(true, lexical_cast<std::string>(pVolume->GetVolume()) + "%", "Громкость:");
				}
				else
				{
					ShowBalloon(true, lexical_cast<std::string>(pVolume->GetVolume()) + "% (Выкл.)", "Громкость:");
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
			MessageBox(hwnd, "SetWindowsHookEx", "Error!", MB_OK | MB_ICONERROR);
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
	CreateMutex(NULL, false, _T("louderit_mutex"));
	if (GetLastError() == ERROR_ALREADY_EXISTS)
		return 0;

	//----------------------------------------------------------------
	isWindowsXP = checkXP();
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
	wc.lpszClassName	= _T("LouderIt");

	if (RegisterClass(&wc) == 0)
		MessageBox(NULL, _T("Can't RegisterClassEx!"), _T("Error"), MB_OK);

	hwnd = CreateWindow(wc.lpszClassName, "", 0, 0, 0, 0, 0, 0, 0, hInstance, NULL);
	// -----------------------------------------------------------------------------
	// Инициализация программы
	// -----------------------------------------------------------------------------
	WM_LOADCONFIG		= RegisterWindowMessage("WM_LOADCONFIG");
	WM_TASKBARCREATED	= RegisterWindowMessage("TaskbarCreated");


	if (isWindowsXP)
	{
		pVolume = new CVolumeMxImpl;
	}
	else // Vista or Win7
	{
		pVolume = new CVolumeImpl;
	}
	

	
	SetHook(true); 
	LoadConfig();

	pVolume->Init(deviceNumber, hwnd);


	LoadIcons();
	
	pTrayIcon = new TrayIcon(hwnd, "LouderIt");
	pTrayIcon->Set(hIcons[0]);
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