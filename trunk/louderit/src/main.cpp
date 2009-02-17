#include "precompiled.h"
#include "volume_impl.h"
#include "volume_mx_impl.h"
#include "lexical_cast.h"

enum hotkeys_ids
{
	HK_UPKEY = 0,
	HK_DOWNKEY,
	HK_MUTEKEY
};

enum app_msg_ids
{
	WM_NOTIFYICON = WM_USER + 1
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
typedef struct
{
	bool	scrollWithTray;		// Вкл./выкл. управление скроллом над треем
	bool	scrollWithCtrl;
	bool	scrollWithAlt;
	bool	scrollWithShift;
	POINT	lastTrayPos;		// Координаты мыши над треем
} globalDLLData_t;

static HANDLE			hMapObject = NULL;  // handle to file mapping

UINT					WM_VOLCHANGE		= 0;
UINT					WM_TASKBARCREATED	= 0;
UINT					WM_LOADCONFIG		= 0;

//------------------------------------------------------------------------------
HINSTANCE				hInst = NULL;
HWND					hwnd = NULL;
bool					isWindowsXP = false;
int						deviceNumber = 0;

std::string				skin;
HICON					hIcons[NUM_ICONS-1];
int						iconIndex = 0;
int						volumeLevel = 0;

// Для различия двойного и одиночного клика
bool					isClickStart = false;
bool					isClickReady = false;
bool					isDblClick = false;

int						hotKey = 0;
int						keyMod = 0;

IVolumeControlPtr		pVolume;

extern "C" __declspec(dllexport) void SetHook(bool enable);
//------------------------------------------------------------------------------
// Setting
//------------------------------------------------------------------------------
static globalDLLData_t*		pSharedMem = NULL; // pointer to shared memory
bool						balloonHint = false;
int							steps = 0;
int							trayCommands[] = {0,0,0};
int							balance = 50;
std::string					deviceName;
std::string					configFile;
// TODO:	char configFile[MAX_PATH];
// ???		char deviceName[256];


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
	hotKey = GetPrivateProfileInt("HotKeys", SKey.c_str(), 0, configFile.c_str());
	if (hotKey != 0)
	{
		keyMod = GetPrivateProfileInt("HotKeys", SMod.c_str(), 0, configFile.c_str());
		return RegisterHotKey(hwnd, NumKey, keyMod, hotKey);
	}
	return false;
}

//------------------------------------------------------------------------------
// Загрузка настроек
//------------------------------------------------------------------------------
void LoadConfig()
{
	configFile.resize(MAX_PATH + 1);
	int j = GetCurrentDirectory(MAX_PATH, &configFile[0]);
	configFile.resize(j);
	configFile += "\\lconfig.ini";

	// [General]
	//...устройства
	deviceName.resize(MAX_PATH+1);
	GetPrivateProfileString("General", "Device", 0, &deviceName[0], 1024, configFile.c_str());
	int sz=deviceName.find(char(0)); 
	deviceName.resize((sz != -1) ? sz : 0);
	
	
	// [General]
	int n = pVolume->GetNumDevice();

	TraceA("GetNumDevice = %i", n);

	for (int i = 0; n - 1 >= i; i++)
	{
		if (deviceName.compare(pVolume->GetDevName(i)) == 0)
		{
			deviceNumber = i;
			break;
		}
	}
	deviceName = pVolume->GetDevName(deviceNumber);
	TraceA("Using device = %s", deviceName.c_str());

	// INIT MIXER!
	// сюда перенести pVolume->Init(...);


	// ..баланс
	balance = GetPrivateProfileInt("General", "Balance", 50, configFile.c_str());

	//...скорости регулирования
	steps = GetPrivateProfileInt("General", "Steps", 5, configFile.c_str());



	GetPrivateProfileString("View", "Skin", "Classic.lsk", &skin[0], 1024, configFile.c_str());
	balloonHint = GetPrivateProfileInt("View", "BalloonHint", 0, configFile.c_str());

	// [Mouse]
	//...колесика
	pSharedMem->scrollWithTray = GetPrivateProfileInt("Mouse", "Tray", 1, configFile.c_str());
	pSharedMem->scrollWithCtrl = GetPrivateProfileInt("Mouse", "Ctrl", 0, configFile.c_str());
	pSharedMem->scrollWithAlt = GetPrivateProfileInt("Mouse", "Alt", 0, configFile.c_str());
	pSharedMem->scrollWithShift = GetPrivateProfileInt("Mouse", "Shift", 0, configFile.c_str());

	//...действий над треем
	trayCommands[0] = GetPrivateProfileInt("Mouse", "Click", 1, configFile.c_str());
	trayCommands[1] = GetPrivateProfileInt("Mouse", "DClick", 2, configFile.c_str());
	trayCommands[2] = GetPrivateProfileInt("Mouse", "MClick", 3, configFile.c_str());

	// [HotKeys]
	//...горячих клавиш
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
	std::string		path;
	path.resize(MAX_PATH + 1);
	int n = GetCurrentDirectory(MAX_PATH, &path[0]);
	path.resize(n);
	path += "\\skins\\";
	path += skin.c_str();

	for (int i = 0; NUM_ICONS - 1 >= i; i++)
	{
		hIcons[i] = ExtractIcon(hInst, path.c_str(), i);
	}
}

//------------------------------------------------------------------------------
// Установка трея
//------------------------------------------------------------------------------
bool SetTrayIcon(DWORD message, HICON hIcon)
{
	NOTIFYICONDATA	nid;

	ZeroMemory(&nid, sizeof(NOTIFYICONDATA));

	nid.cbSize				= sizeof(NOTIFYICONDATA);
	nid.hWnd				= hwnd;
	nid.uID					= 1;
	nid.uFlags				= NIF_ICON|NIF_MESSAGE|NIF_TIP;
	nid.uCallbackMessage	= WM_NOTIFYICON;
	nid.hIcon				= hIcon;

	strcpy_s(nid.szTip, "LouderIt");

	return Shell_NotifyIcon(message, &nid) != 0 ? true : false;
}

//------------------------------------------------------------------------------
// Обновление трея
//------------------------------------------------------------------------------
void UpdateTrayIcon()
{
	volumeLevel	= pVolume->GetVolume();
	iconIndex	= volumeLevel * 14 / MAX_VOL ;
	
	if (pVolume->GetMute())
		iconIndex = iconIndex + 15;
	SetTrayIcon(NIM_MODIFY, hIcons[iconIndex]);
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
			rightChannelVol = std::max(volumeLevel +
				(volumeLevel * (50 - balance)) / 50, 0);
			leftChannelVol = volumeLevel;
		}
		else
		{
			rightChannelVol = volumeLevel;
			leftChannelVol = std::max(volumeLevel - 
				(volumeLevel * (50 - balance)) / 50, 0);
		}
		pVolume->SetVolumeChannel(leftChannelVol, rightChannelVol);
	}
}



//------------------------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_VOLCHANGE)
	{
		if (lParam == (LPARAM)true)
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
				ShowBalloon(true, lexical_cast<std::string>(pVolume->GetVolume()) + "%", "Громкость:");
			}
			else
			{
				ShowBalloon(true, lexical_cast<std::string>(pVolume->GetVolume()) + "% (Выкл.)", "Громкость:");
			}
			SetTimer(hWnd, 2, 3000, NULL);
		}
	}

	else if (message == WM_LOADCONFIG)
	{
		LoadConfig();
		LoadIcons();
		SetTrayIcon(NIM_MODIFY, hIcons[iconIndex]);
	}

	else if (message == WM_TASKBARCREATED)
	{
		SetTrayIcon(NIM_ADD, hIcons[0]);
		UpdateTrayIcon();
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

	case WM_NOTIFYICON:
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
				GetCursorPos(&pSharedMem->lastTrayPos);
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
void SharedMemory(bool enable)
{
	if (enable)
	{
		hMapObject = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, 
			PAGE_READWRITE,	0, sizeof(globalDLLData_t), _T("LouderItMMF"));
		//hMapObject = OpenFileMapping(
		//	FILE_MAP_ALL_ACCESS,
		//	FALSE,
		//	TEXT("LouderItMMF"));

		if (hMapObject == NULL) 
			return; 

		pSharedMem =(globalDLLData_t*) MapViewOfFile(hMapObject, FILE_MAP_ALL_ACCESS,
			0, 0, 0);
		if (!pSharedMem)
		{
			CloseHandle(hMapObject);
		}
	}
	else
	{
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
	}

}
// -----------------------------------------------------------------------------
// Инициализация отраженного в памяти файла и установки хука на скролл мыши
// -----------------------------------------------------------------------------
void InitHookData()
{
	SharedMemory(true);
	WM_VOLCHANGE = RegisterWindowMessage("WM_VOLCHANGE");
	SetHook(true); 
}

// -----------------------------------------------------------------------------
// Закрытие отраженного в памяти файла и снятие хука на скролл мыши
// -----------------------------------------------------------------------------
void CloseHookData()
{
	SetHook(false);
	SharedMemory(false);
}

// -----------------------------------------------------------------------------
// Инициализация, регистрация и создание главного окна
// -----------------------------------------------------------------------------
int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, 
					 LPSTR lpCmdLine, int nShowCmd)
{
	// тут проверка на единственный запуск проги, через мютекс
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
		pVolume = CVolumeMxImpl::Create();
	}
	else
	{
		pVolume = CVolumeImpl::Create();
	}

	
	InitHookData();
	LoadConfig();

	pVolume->Init(deviceNumber, hwnd);


	LoadIcons();
	SetTrayIcon(NIM_ADD, hIcons[0]);
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
	SetTrayIcon(NIM_DELETE, 0);
	UnregHotKeys();
	CloseHookData();
	return 0;
}