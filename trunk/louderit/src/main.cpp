#include "precompiled.h"
#include "tray.h"
#include "volume_impl.h"
#include "volume_mx_impl.h"
#include "settings.h"
#include "lexical_cast.h"
#include "resource.h"

enum HotKeysIds
{
	kUpKey = 0,
	kDownKey,
	kMuteKey
};

// -----------------------------------------------------------------------------
// Данные, отвечающие за реализацию хука на мышь
// -----------------------------------------------------------------------------

POINT					last_tray_pos;		// Координаты мыши над треем

UINT					WM_TASKBARCREATED	= 0;
UINT					WM_LOADCONFIG		= 0;

//------------------------------------------------------------------------------
HINSTANCE				instance = NULL;
HWND					window = NULL;
bool					isWindowsXP = false;

vector<HICON>			icons;
int						icon_index = 0;
int						volume_level = 0;
int 					num_icons = 0;

// Для различия двойного и одиночного клика
bool					is_double_click = false;

int						hot_key = 0;
int						key_mod = 0;

IVolume					*volume = NULL;

TrayIcon				*trayicon = NULL;

HHOOK					hook;

int							tray_commands[] = {0,0,0};

UINT						device_number = 0;

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
	UnregisterHotKey(window, kUpKey);
	UnregisterHotKey(window, kDownKey);
	UnregisterHotKey(window, kMuteKey);
}
//------------------------------------------------------------------------------
// Установка горячих клавиш
//------------------------------------------------------------------------------
BOOL SetHotKey(const wstring& SKey, const wstring& SMod, int NumKey)
{
	hot_key = GetPrivateProfileInt(L"HotKeys", SKey.c_str(), 0, config_file);
	if (hot_key != 0)
	{
		key_mod = GetPrivateProfileInt(L"HotKeys", SMod.c_str(), 0, config_file);
		return RegisterHotKey(window, NumKey, key_mod, hot_key);
	}
	return false;
}

//------------------------------------------------------------------------------
// Загрузка настроек
//------------------------------------------------------------------------------
void LoadConfig() //FIXME! mus be move to settings.c
{
	getConfigFile();

	// [General]
	//...устройства
	GetPrivateProfileString(L"General", L"Device", 0, device_name, 256, config_file);
		
	
	// [General]
	int n = volume->GetNumDevice();

	for (int i = 0; n - 1 >= i; ++i)
	{
		// FIXME: ТУТ КРОЕТСЯ ОШИБКА!!!!!!!
		if (lstrcmp(device_name, volume->GetDeviceName(i).c_str()) == 0)
		{
			device_number = i;
			break;
		}
	}
	lstrcpyn(device_name, volume->GetDeviceName(device_number).c_str(),
		sizeof(device_name)/sizeof(device_name[0]));

	// INIT MIXER!
	// сюда перенести volume->Init(...);

	balance = GetPrivateProfileInt(L"General", L"Balance", 50, config_file);

	//...скорости регулирования
	steps = GetPrivateProfileInt(L"General", L"Steps", 5, config_file);

	GetPrivateProfileString(L"View", L"Skin", L"Classic.lsk", &skin_name[0], 1024, config_file);
	balloon_hint = GetPrivateProfileInt(L"View", L"BalloonHint", 0, config_file);

	scroll_with_tray = GetPrivateProfileInt(L"Mouse", L"Tray", 1, config_file);
	scroll_with_ctrl = GetPrivateProfileInt(L"Mouse", L"Ctrl", 0, config_file);
	scroll_with_alt = GetPrivateProfileInt(L"Mouse", L"Alt", 0, config_file);
	scroll_with_shift = GetPrivateProfileInt(L"Mouse", L"Shift", 0, config_file);

	//...действий над треем
	tray_commands[0] = GetPrivateProfileInt(L"Mouse", L"Click", 1, config_file);
	tray_commands[1] = GetPrivateProfileInt(L"Mouse", L"DClick", 2, config_file);
	tray_commands[2] = GetPrivateProfileInt(L"Mouse", L"MClick", 3, config_file);

	UnregHotKeys();
	SetHotKey(L"UpKey", L"UpMod", kUpKey);
	SetHotKey(L"DownKey", L"DownMod", kDownKey);
	SetHotKey(L"MuteKey", L"MuteMod", kMuteKey);
}
//------------------------------------------------------------------------------
// Загрузка иконок
//------------------------------------------------------------------------------

void LoadIcons() 
{
	wchar_t			path[MAX_PATH] = {0};
    
	getAppPath(path);
	_tcscat_s(path, L"\\skins\\");
	_tcscat_s(path, skin_name);

	//get number of icons contained in the skin 
	num_icons = ExtractIconEx(path, -1, NULL, NULL, 0);

	if (num_icons > 1)
	{
		for (int i = 0; i < num_icons; ++i)
		{
			icons.push_back(ExtractIcon(instance, path, i));
		}
	}
	else
	{
		// FIXME!
		num_icons = 2;
		if (isWindowsXP)
		{
			icons.push_back(LoadIcon(NULL, IDI_ERROR));
			icons.push_back(LoadIcon(NULL, IDI_ERROR));
		}
		else
		{
			icons.push_back(ExtractIcon(instance, L"SndVol.exe", 1));
			icons.push_back(ExtractIcon(instance, L"SndVol.exe", 2));
		}
	}
}

//------------------------------------------------------------------------------
// Обновление трея
//------------------------------------------------------------------------------
void UpdateTrayIcon()
{
	volume_level = volume->GetVolume();
	icon_index = volume_level * (num_icons / 2 - 1) / MAX_VOL;
	
	if (volume->GetMute())
		icon_index = icon_index + (num_icons / 2);

	trayicon->Update(icons[icon_index], L"LouderIt");
}

//-----------------------------------------------------------------------------
wstring GetMixerCmdLine()
{
	wstring str = L"-d " + lexical_cast<wstring>(device_number);
	return str;
}


//-----------------------------------------------------------------------------
void ProcessPopupMenu()
{
	HMENU	menu;
	POINT	cursor_pos;
	UINT	command;

	menu = GetSubMenu(LoadMenu(instance, MAKEINTRESOURCE(IDR_CONTEXT)), 0);

	SetForegroundWindow(window);
	GetCursorPos(&cursor_pos);

	command = (UINT)TrackPopupMenuEx(menu, TPM_RETURNCMD + TPM_VERTICAL, 
		cursor_pos.x, cursor_pos.y, window, NULL);

	PostMessage(window, WM_NULL, 0, 0);

	switch (command)
	{
	case ID_TRAYMENU_OPENMIXER:
		{
			if (isWindowsXP)
			{
				Launch(window, L"sndvol32.exe", GetMixerCmdLine().c_str());
			}
			else
			{
				Launch(window, L"sndvol.exe");
			}
			break;
		}
	case ID_TRAYMENU_AUDIOPROPERTIES:
		Launch(window, L"rundll32.exe", L"shell32.dll,Control_RunDLL mmsys.cpl");
		break;
	case ID_TRAYMENU_SETTINGS:
		//Launch(window, L"LConfig.exe");
		ShowSettingsDlg(window);
		break;
		//case IDM_ABOUT:
		//  Launch(L"LConfig.exe -a");
		//break;
	case ID_TRAYMENU_EXIT:	
		DestroyWindow(window);
		break;
	}
	DestroyMenu(menu);
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
				Launch(window, L"sndvol32.exe", L"-t");
			}
			else
			{
				Launch(window, L"sndvol.exe", L"-f 49349574");
			}
			break;
		}
	case 2:
		{
			if (isWindowsXP)
			{
				Launch(window, L"sndvol32.exe", GetMixerCmdLine().c_str());
			}
			else
			{
				Launch(window, L"sndvol.exe");
			}
			break;
		}
	case 3:
		volume->SetMute(!volume->GetMute());
		break;
	case 4:
		Launch(window, L"LConfig.exe");
		break;
	}
}

//------------------------------------------------------------------------------
void VolumeUp()
{
	int right_channel_vol	= 0;
	int left_channel_vol	= 0;

	volume_level = min((volume_level + steps), MAX_VOL);

	if (balance == 50)
	{
		volume->SetVolume(volume_level);
		//volume->SetVolumeChannel(nVolumeLevel, nVolumeLevel);
	}
	else
	{
		if (balance > 50)
		{
			left_channel_vol = max(volume_level + (volume_level * (50 - balance)) / 50, 0);
			right_channel_vol = volume_level;
		}
		else
		{
			left_channel_vol = volume_level;
			right_channel_vol = max(volume_level - (volume_level * (50 - balance)) / 50, 0);
		}
		volume->SetVolumeChannel(left_channel_vol, right_channel_vol);
	}
}

//------------------------------------------------------------------------------
void VolumeDown()
{
	int right_channel_vol = 0;
	int left_channel_vol	= 0;
	
	volume_level = max((volume_level - steps), 0);

	if (balance == 50)
	{
		volume->SetVolume(volume_level);
		//volume->SetVolumeChannel(nVolumeLevel, nVolumeLevel);
	}
	else
	{
		if (balance > 50)
		{
			right_channel_vol = max(volume_level + (volume_level * (50 - balance)) / 50, 0);
			left_channel_vol = volume_level;
		}
		else
		{
			right_channel_vol = volume_level;
			left_channel_vol = max(volume_level - (volume_level * (50 - balance)) / 50, 0);
		}
		volume->SetVolumeChannel(left_channel_vol, right_channel_vol);
	}
}



//------------------------------------------------------------------------------

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_LOADCONFIG)
	{
		LoadConfig();
		LoadIcons();
		trayicon->Update(icons[icon_index], L"LouderIt");
	}

	else if (message == WM_TASKBARCREATED)
	{
		trayicon->Restore();
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
				is_double_click = true;
				TrayCommand(tray_commands[1]);
				break;

			case WM_MBUTTONUP:
				TrayCommand(tray_commands[2]);
				break;

			case WM_RBUTTONUP:
				ProcessPopupMenu();
				break;

			case WM_MOUSEMOVE:
				GetCursorPos(&last_tray_pos);
				break;
			}
		}
		break;

	case WM_TIMER:
		if (wParam == 1)
		{
			KillTimer(hWnd, 1);
			if (!is_double_click)
			{
				TrayCommand(tray_commands[0]);
			}
			else
			{
				is_double_click = false;
			}
		}
		if (wParam == 2)
		{
			KillTimer(hWnd, 2);
			trayicon->ShowBaloon(L"", L"");
		}
		break;

	case WM_HOTKEY:
		{
			switch (wParam)
			{
			case kUpKey:
				VolumeUp();
				break;
			case kDownKey:
				VolumeDown();
				break;
			case kMuteKey:
				volume->SetMute(!volume->GetMute());
				break;
			}
			if (balloon_hint)
			{
				wstring str_on = lexical_cast<wstring>(volume->GetVolume()) + L"%";
				if (!volume->GetMute())
				{
					trayicon->ShowBaloon((lexical_cast<wstring>(volume->GetVolume()) + L"%").c_str(), L"Громкость:");
				}
				else
				{
					trayicon->ShowBaloon((lexical_cast<wstring>(volume->GetVolume()) + L"% (Выкл.)").c_str(), L"Громкость:");
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
	if (scroll_with_ctrl == false 
		&& scroll_with_alt == false
		&& scroll_with_shift == false)
	{
		return false;
	}
	if (
		(!scroll_with_ctrl ^ GetKeyState(VK_LCONTROL) && 128 != 0)
		&&	(!scroll_with_alt ^ GetKeyState(VK_MENU) && 128 != 0) 
		&& (!scroll_with_shift ^ GetKeyState(VK_SHIFT) && 128 != 0) 
		)
		return true;
	return false;
}

LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	HRESULT		hr;	
	POINT		cursor_pos;

	if (nCode < 0)  // do not process the message 
		return CallNextHookEx(hook, nCode, wParam, lParam);



	hr = CallNextHookEx(hook, nCode, wParam, lParam);

	if ((nCode == HC_ACTION) && (wParam == WM_MOUSEWHEEL)) 
	{
		GetCursorPos( &cursor_pos );
		if (cursor_pos.x == last_tray_pos.x && cursor_pos.y == last_tray_pos.y || GetKeys())
		{

			if ((short) HIWORD(((PMSLLHOOKSTRUCT) lParam)->mouseData) >= 0) // Delta of mouse wheel
			{
				VolumeUp();
			}
			else
			{
				VolumeDown();
			}
			if (balloon_hint)
			{
				if (!volume->GetMute())
				{
					trayicon->ShowBaloon((lexical_cast<wstring>(volume->GetVolume()) + L"%").c_str(), L"Громкость:");

				}
				else
				{
					trayicon->ShowBaloon((lexical_cast<wstring>(volume->GetVolume()) + L"% (Выкл.)").c_str(), L"Громкость:");
				}
				SetTimer(window, 2, 3000, NULL);
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
		hook = SetWindowsHookEx(WH_MOUSE_LL, LowLevelMouseProc, instance, 0);
		if (hook <= 0) 
		{
			MessageBox(window, L"SetWindowsHookEx", L"Error!", MB_OK | MB_ICONERROR);
			DestroyWindow(window);
		}

	}
	else
		UnhookWindowsHookEx(hook);
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
	instance = hInstance;

	WNDCLASS wc;

	wc.style			= 0;
	wc.lpfnWndProc		= (WNDPROC)WndProc;
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= 0;
	wc.hInstance		= hInstance;
	wc.hIcon			= NULL;
	wc.hCursor			= NULL;
	wc.hbrBackground	= NULL;
	wc.lpszMenuName		= NULL;
	wc.lpszClassName	= L"LouderIt";

	RegisterClass(&wc);
	window = CreateWindow(wc.lpszClassName, L"", 0, 0, 0, 0, 0, 0, 0, hInstance, NULL);
	if (!window)
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
		volume = new VolumeMxImpl;
	}
	else // Vista or Win7
	{
		volume = new VolumeImpl;
	}
	
	SetHook(true); 
	LoadConfig();

	volume->Init(device_number, window);

	LoadIcons();
	
	trayicon = new TrayIcon(window);
	trayicon->Add(icons[0], L"LouderIt");
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
	volume->Shutdown();
	trayicon->Remove();
	delete trayicon;
	UnregHotKeys();
	SetHook(false);
	delete volume;
	return 0;
}