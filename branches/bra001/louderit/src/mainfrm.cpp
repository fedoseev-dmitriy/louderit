#include "precompiled.h"
#include "mainfrm.h"

template <class REZ, class SRC> __forceinline
REZ cast(SRC CastFrom)
{
	union
	{
		REZ rez;
		SRC src;
	};
	src = CastFrom;
	return rez;
}

//------------------------------------------------------------------------------
bool CMainFrame::Launch(const wchar_t* command_line, 
						const wchar_t* parameters)
{
	HINSTANCE instance = ShellExecute(0, NULL, command_line, parameters,
		NULL, SW_SHOWNORMAL);

	if ((int)instance <= 32)
	{
		return false;
	}
	return true;
}
//------------------------------------------------------------------------------
// Return true - XP, return false - Vista or later
//------------------------------------------------------------------------------
bool CMainFrame::CheckXP()
{
	OSVERSIONINFO		osvi;

	ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osvi);
	return osvi.dwMajorVersion < 6;
}

//------------------------------------------------------------------------------
void CMainFrame::UnregHotKeys()
{
	UnregisterHotKey(m_hWnd, HK_UPKEY);
	UnregisterHotKey(m_hWnd, HK_DOWNKEY);
	UnregisterHotKey(m_hWnd, HK_MUTEKEY);
}

//------------------------------------------------------------------------------
bool CMainFrame::SetHotKey(const wchar_t* skey, const wchar_t* smod, int numkey)
{
	hotkey_ = GetPrivateProfileInt(L"HotKeys", skey, 0, config_file_);
	if (hotkey_ != 0)
	{
		keymod_ = GetPrivateProfileInt(L"HotKeys", smod, 0, config_file_);
		return RegisterHotKey(m_hWnd, numkey, keymod_, hotkey_);
	}
	return false;
}
//------------------------------------------------------------------------------
// get application directory
bool CMainFrame::GetAppPath(wchar_t* app_path)
{
	wchar_t		path_buff[MAX_PATH] = {0};
	wchar_t*	path_name = 0;

	if ((!GetModuleFileName(NULL, path_buff, MAX_PATH)) ||
		(!GetFullPathName(path_buff, MAX_PATH, app_path, &path_name)))
	{
		*app_path = '\0';
		return false;
	}

	*path_name = '\0';
	return true;
}

//------------------------------------------------------------------------------
// Загрузка настроек
//------------------------------------------------------------------------------
void CMainFrame::LoadConfig()
{
	wchar_t device_name_[256] = {0};
	GetAppPath(config_file_);
	_tcscat_s(config_file_, L"\\lconfig.ini");

	// [General]
	//...устройства
	GetPrivateProfileString(L"General", L"Device", 0, device_name_, 256, config_file_);


	// [General]
	int n = volume_->GetNumDevice();

	for (int i = 0; n - 1 >= i; ++i)
	{
		// FIXME: ТУТ КРОЕТСЯ ОШИБКА!!!!!!!
		if (lstrcmp(device_name_, volume_->GetDeviceName(i).c_str()) == 0)
		{
			device_number_ = i;
			break;
		}
	}
	_tcscpy_s(device_name_, volume_->GetDeviceName(device_number_).c_str());

	// INIT MIXER!
	// сюда перенести pVolume->Init(...);

	balance_ = GetPrivateProfileInt(L"General", L"Balance", 50, config_file_);

	//...скорости регулирования
	steps_ = GetPrivateProfileInt(L"General", L"Steps", 5, config_file_);

	GetPrivateProfileString(L"View", L"Skin", L"Classic.lsk", &skin_[0], 1024, config_file_);
	balloon_hint_ = GetPrivateProfileInt(L"View", L"BalloonHint", 0, config_file_);

	scroll_with_tray_ = GetPrivateProfileInt(L"Mouse", L"Tray", 1, config_file_);
	scroll_with_ctrl_ = GetPrivateProfileInt(L"Mouse", L"Ctrl", 0, config_file_);
	scroll_with_alt_ = GetPrivateProfileInt(L"Mouse", L"Alt", 0, config_file_);
	scroll_with_shift_ = GetPrivateProfileInt(L"Mouse", L"Shift", 0, config_file_);

	//...действий над треем
	tray_commands_[0] = GetPrivateProfileInt(L"Mouse", L"Click", 1, config_file_);
	tray_commands_[1] = GetPrivateProfileInt(L"Mouse", L"DClick", 2, config_file_);
	tray_commands_[2] = GetPrivateProfileInt(L"Mouse", L"MClick", 3, config_file_);

	UnregHotKeys();
	SetHotKey(L"UpKey", L"UpMod", HK_UPKEY);
	SetHotKey(L"DownKey", L"DownMod", HK_DOWNKEY);
	SetHotKey(L"MuteKey", L"MuteMod", HK_MUTEKEY);
}
//------------------------------------------------------------------------------
// Загрузка иконок
//------------------------------------------------------------------------------

void CMainFrame::LoadIcons()
{
	wchar_t			path[MAX_PATH] = {0};

	GetAppPath(path);
	_tcscat_s(path, L"\\skins\\");
	_tcscat_s(path, skin_);

	//get number of icons contained in the skin 
	num_icons_ = ExtractIconEx(path, -1, NULL, NULL, 0);

	if (num_icons_ > 1)
	{
		for (int i = 0; i < num_icons_; ++i)
		{
			icons_.push_back(ExtractIcon(_Module.m_hInst, path, i));
		}
	}
	else
	{
		// FIXME!
		num_icons_ = 2;
		icons_.push_back(LoadIcon(NULL, IDI_ERROR));
		icons_.push_back(LoadIcon(NULL, IDI_ERROR));
	}
}

//------------------------------------------------------------------------------
// Обновление трея
//------------------------------------------------------------------------------
void CMainFrame::UpdateTrayIcon()
{
	volume_level_ = volume_->GetVolume();
	icon_index_	= volume_level_ * (num_icons_ / 2 - 1) / MAX_VOL;

	if (volume_->GetMute())
		icon_index_ = icon_index_ + (num_icons_ / 2);

	//pTrayIcon->Update(icons[icon_index], L"LouderIt");
	//trayicon_.SetIcon(icons_.at(icon_index_));
}

//------------------------------------------------------------------------------
wstring CMainFrame::GetMixerCmdLine()
{
	wstring str = L"-d " + lexical_cast<wstring>(device_number_);
	return str;
}

//------------------------------------------------------------------------------
void CMainFrame::TrayCommand(int flag)
{
	switch (flag)
	{
	case 1:
		{
			if (is_winxp_)
			{
				Launch(L"sndvol32.exe", L"-t");
			}
			else
			{
				Launch(L"sndvol.exe", L"-f 49349574");
			}
			break;
		}
	case 2:
		{
			if (is_winxp_)
			{
				Launch(L"sndvol32.exe", GetMixerCmdLine().c_str());
			}
			else
			{
				Launch(L"sndvol.exe");
			}
			break;
		}
	case 3:
		volume_->SetMute(!volume_->GetMute());
		break;
	case 4:
		Launch(L"LConfig.exe");
		break;
	}
}

//------------------------------------------------------------------------------
void CMainFrame::VolumeUp()
{
	int rightChannelVol	= 0;
	int leftChannelVol	= 0;

	volume_level_ = min((volume_level_ + steps_), MAX_VOL);

	if (balance_ == 50)
	{
		volume_->SetVolume(volume_level_);
		//pVolume->SetVolumeChannel(nVolumeLevel, nVolumeLevel);
	}
	else
	{
		if (balance_ > 50)
		{
			leftChannelVol = max(volume_level_ + (volume_level_ * (50 - balance_)) / 50, 0);
			rightChannelVol = volume_level_;
		}
		else
		{
			leftChannelVol = volume_level_;
			rightChannelVol = max(volume_level_ - (volume_level_ * (50 - balance_)) / 50, 0);
		}
		volume_->SetVolumeChannel(leftChannelVol, rightChannelVol);
	}
}

//------------------------------------------------------------------------------
void CMainFrame::VolumeDown()
{
	int rightChannelVol = 0;
	int leftChannelVol	= 0;

	volume_level_ = max((volume_level_ - steps_), 0);

	if (balance_ == 50)
	{
		volume_->SetVolume(volume_level_);
		//pVolume->SetVolumeChannel(nVolumeLevel, nVolumeLevel);
	}
	else
	{
		if (balance_ > 50)
		{
			rightChannelVol = max(volume_level_ + (volume_level_ * (50 - balance_)) / 50, 0);
			leftChannelVol = volume_level_;
		}
		else
		{
			rightChannelVol = volume_level_;
			leftChannelVol = max(volume_level_ - (volume_level_ * (50 - balance_)) / 50, 0);
		}
		volume_->SetVolumeChannel(leftChannelVol, rightChannelVol);
	}
}

//------------------------------------------------------------------------------
bool CMainFrame::GetKeys()
{
	if (scroll_with_ctrl_ == false &&
		scroll_with_alt_ == false &&
		scroll_with_shift_ == false)
	{
		return false;
	}
	if (
		(!scroll_with_ctrl_ ^ GetKeyState(VK_LCONTROL) && 128 != 0) &&
		(!scroll_with_alt_ ^ GetKeyState(VK_MENU) && 128 != 0) &&
		(!scroll_with_shift_ ^ GetKeyState(VK_SHIFT) && 128 != 0) 
		)
		return true;
	return false;
}

//------------------------------------------------------------------------------
LRESULT CMainFrame::LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	HRESULT		hr;	
	CPoint		cursor_pos;

	if (nCode < 0)  // do not process the message 
		return CallNextHookEx(hook_, nCode, wParam, lParam);


	hr = CallNextHookEx(hook_, nCode, wParam, lParam);

	if (nCode == HC_ACTION && wParam == WM_MOUSEWHEEL) 
	{
		GetCursorPos(&cursor_pos);
		if (cursor_pos.x == last_tray_pos_.x && 
			cursor_pos.y == last_tray_pos_.y || GetKeys())
		{
			if ((short) HIWORD(((PMSLLHOOKSTRUCT) lParam)->mouseData) >= 0) // Delta of mouse wheel
			{
				VolumeUp();
			}
			else
			{
				VolumeDown();
			}
			if (balloon_hint_)
			{
				if (!volume_->GetMute())
				{
					//trayicon_.SetBalloonDetails((lexical_cast<wstring>(volume_->GetVolume()) + L"%").c_str(), 
						//L"Громкость:", CTrayNotifyIcon::None, 3000);

				}
				else
				{
					//trayicon_.SetBalloonDetails((lexical_cast<wstring>(volume_->GetVolume()) + L"% (Выкл.)").c_str(), 
						//L"Громкость:", CTrayNotifyIcon::None, 3000);
				}
			}
			hr = S_FALSE;
		}
	}
	return hr;
}

//------------------------------------------------------------------------------
void CMainFrame::SetHook(bool flag)
{
	if (flag) 
	{
		hook_ = SetWindowsHookEx(WH_MOUSE_LL, cast<HOOKPROC>(&CMainFrame::LowLevelMouseProc), _Module.m_hInst, 0);
		if (hook_ <= 0) 
		{
			MessageBox(L"SetWindowsHookEx", L"Error!", MB_OK | MB_ICONERROR);
			OnClose();
		}
	}
	else
		UnhookWindowsHookEx(hook_);
}

//------------------------------------------------------------------------------
//LRESULT CMainFrame::OnWmNotifyicontray(UINT uMsg, WPARAM wParam, LPARAM lParam)
//{
//	switch (lParam)
//	{
//
//	case WM_LBUTTONDOWN:
//		SetTimer(1, GetDoubleClickTime(), NULL);
//		break;
//
//	case WM_LBUTTONDBLCLK:
//		is_double_click_ = true;
//		TrayCommand(tray_commands_[1]);
//		break;
//
//	case WM_MBUTTONUP:
//		TrayCommand(tray_commands_[2]);
//		break;
//
//		//case WM_RBUTTONUP:
//		//	ProcessPopupMenu();
//		//	break;
//
//	case WM_MOUSEMOVE:
//		GetCursorPos(&last_tray_pos_);
//		break;
//	}
//	//trayicon_.OnTrayNotification(wParam, lParam);
//	return 0L;
//}