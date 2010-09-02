#include "precompiled.h"
#include "settings.h"
#include "resource.h"

//#include "WinHotkeyCtrl.h"

vector<wstring> settingsItems;

HWND hSettingsWnd = NULL;
HWND hSettingsListBox = NULL;

HWND hGeneralPage = NULL;
HWND hDevListBox = NULL;

HWND hViewPage = NULL;
HWND hMousePage = NULL;
HWND hHotkeysPage = NULL;
HWND hAboutPage = NULL;

HWND hCurrentPage = NULL;

wchar_t app_file[MAX_PATH] = {0};		// application fullpath
wchar_t config_name[] = L"lconfig.ini";	// config filename
wchar_t config_file[MAX_PATH] = {0};	// config fullpath

HKEY hk;

//------------------------------------------------------------------------------
// Settings
//------------------------------------------------------------------------------
wchar_t	device_name[MAX_PATH] = {0};
bool	autostart = false;
int		steps = 0;
wchar_t	skin_name[MAX_PATH] = {0};
int		balance = 50;
bool	balloon_hint = false;
bool	scroll_with_tray = false;
bool	scroll_with_ctrl = false;
bool	scroll_with_alt = false;
bool	scroll_with_shift = false;

IVolume	*volume = NULL;
int		nDev;
bool	isWindowsXP = false;

//------------------------------------------------------------------------------
// Getting the application directory
//------------------------------------------------------------------------------
bool getAppPath(wchar_t *path)
{
	//wchar_t		path_buff[MAX_PATH] = {0};
	wchar_t		*path_name = 0;
		
	if ((!GetModuleFileName(NULL, app_file, MAX_PATH)) ||
		(!GetFullPathName(app_file, MAX_PATH, path, &path_name)))
	{
		*path = '\0';
		return false;
	}

	*path_name = '\0';
	return true;
}

//------------------------------------------------------------------------------
// Getting the config file path
//------------------------------------------------------------------------------
void getConfigFile(void)
{
	getAppPath(config_file);
	_tcscat_s(config_file, config_name);
}

//-----------------------------------------------------------------------------
void saveConfig()
{
	// device
	LRESULT selected_device = ComboBox_GetCurSel(GetDlgItem(hGeneralPage, IDC_DEVLIST));
	lstrcpy(device_name, selected_device ? volume->GetDeviceName(selected_device-1).c_str() : L"");
	WritePrivateProfileString(L"General", L"Device",  device_name, config_file);

	// autostart
	RegCreateKey(HKEY_CURRENT_USER,L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", &hk);
	if (Button_GetCheck(GetDlgItem(hGeneralPage, IDC_AUTOSTART)) == BST_CHECKED)
	{
		RegSetValueEx(hk, L"LouderIt", 0, REG_SZ, (LPBYTE)app_file, MAX_PATH);
		autostart = true;
	}
	else
	{
		RegDeleteValue(hk, L"LouderIt");
		autostart = false;
	}
	RegCloseKey(hk);
	
	// hotkeys
	//WritePrivateProfileString(L"HotKeys", L"VolumeUp", L"0", config_file);
	//WritePrivateProfileString(L"HotKeys", L"VolumeDown", L"0", config_file);
	//WritePrivateProfileString(L"HotKeys", L"VolumeMute", L"0", config_file);
	//WritePrivateProfileString(L"HotKeys", L"ShowMixer", L"0", config_file);
}

//-----------------------------------------------------------------------------
void loadConfig()
{
	GetPrivateProfileString(L"General", L"Device", L"", device_name, MAX_PATH, config_file);
	
	DWORD dwBytes = MAX_PATH;
	wchar_t path[MAX_PATH];
	RegCreateKey(HKEY_CURRENT_USER,L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", &hk);
	if ((RegQueryValueEx(hk, L"LouderIt", NULL, NULL, (LPBYTE)path, &dwBytes) == ERROR_SUCCESS) &&
		(lstrcmp(path, app_file) == 0))
		autostart = true;
	RegCloseKey(hk);
}

INT_PTR CALLBACK GeneralPage_Proc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HWND hAutostarBox;
	switch (uMsg)
	{
		case WM_INITDIALOG:	// before a dialog is displayed
			
			loadConfig();
			
			// device
			hDevListBox = GetDlgItem(hwndDlg, IDC_DEVLIST);
			ComboBox_AddString(hDevListBox, L"Default");
			ComboBox_SetCurSel(hDevListBox, 0);
			for (int i = 0; nDev - 1 >= i; ++i)
			{
				ComboBox_AddString(hDevListBox, volume->GetDeviceName(i).c_str());
				if (lstrcmp(device_name, volume->GetDeviceName(i).c_str()) == 0)
					ComboBox_SetCurSel(hDevListBox, i+1);
			}
			
			// autostart
			if (autostart)
				Button_SetCheck(GetDlgItem(hwndDlg, IDC_AUTOSTART), BST_CHECKED);
						
			return TRUE;
		
		case WM_COMMAND:	// notification msgs from child controls
			return TRUE;
	}
	return FALSE;
}

INT_PTR CALLBACK ViewPage_Proc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:	// before a dialog is displayed
				
			return TRUE;
		
		case WM_COMMAND:	// notification msgs from child controls
			return TRUE;
	}
	return FALSE;
}

INT_PTR CALLBACK MousePage_Proc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
		
	switch (uMsg)
	{
		case WM_INITDIALOG:	// before a dialog is displayed
			
			return TRUE;
		
		case WM_COMMAND:	// notification msgs from child controls
			return TRUE;
	}
	return FALSE;
}

INT_PTR CALLBACK HotkeysPage_Proc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:	// before a dialog is displayed
				
			return TRUE;
		
		case WM_COMMAND:	// notification msgs from child controls
			return TRUE;
	}
	return FALSE;
}
INT_PTR CALLBACK AboutPage_Proc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:	// before a dialog is displayed
				
			return TRUE;
		
		case WM_COMMAND:	// notification msgs from child controls
			return TRUE;
	}
	return FALSE;
}


void ChangePage(HWND hwndPage)
{
	ShowWindow(hCurrentPage, SW_HIDE);
	ShowWindow(hwndPage, SW_SHOW);
	hCurrentPage = hwndPage;
}

INT_PTR CALLBACK SettingsDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	UINT id = 0;
	
	switch (uMsg)
	{
		case WM_INITDIALOG:	// before a dialog is displayed
			
			// filling items in listbox
			if (settingsItems.size() == 0) {
				settingsItems.push_back(L"General"); // FIXME (need move this strings to res)
				settingsItems.push_back(L"View");
				settingsItems.push_back(L"Mouse");
				settingsItems.push_back(L"Hotkeys");
				settingsItems.push_back(L"About");
			}

			hSettingsListBox = GetDlgItem(hwndDlg, IDC_LIST);
			for(unsigned short i=0; i<settingsItems.size(); i++)
			{
				ListBox_AddString(hSettingsListBox, settingsItems[i].c_str());
			}
			ListBox_SetCurSel(hSettingsListBox, 0);
			
			hGeneralPage = CreateDialog(0, MAKEINTRESOURCE(IDD_GENERAL), hwndDlg, (DLGPROC)GeneralPage_Proc);
			hViewPage = CreateDialog(0, MAKEINTRESOURCE(IDD_VIEW), hwndDlg, (DLGPROC)ViewPage_Proc);
			hMousePage = CreateDialog(0, MAKEINTRESOURCE(IDD_MOUSE), hwndDlg, (DLGPROC)MousePage_Proc);
			hHotkeysPage = CreateDialog(0, MAKEINTRESOURCE(IDD_HOTKEYS), hwndDlg, (DLGPROC)HotkeysPage_Proc);
			hAboutPage = CreateDialog(0, MAKEINTRESOURCE(IDD_ABOUT), hwndDlg, (DLGPROC)AboutPage_Proc);
			hCurrentPage = hGeneralPage;
				
			return TRUE;
		
		case WM_COMMAND:	// notification msgs from child controls
			switch (LOWORD(wParam))
			{
				case IDC_LIST:
					switch (HIWORD(wParam))
					{
						case LBN_SELCHANGE:
							switch ListBox_GetCurSel(hSettingsListBox)
							{
								case 0:
									ChangePage(hGeneralPage);
									break;
								case 1:
									ChangePage(hViewPage);
									break;
								case 2:
									ChangePage(hMousePage);
									break;
								case 3:
									ChangePage(hHotkeysPage);
									break;
								case 4:
									ChangePage(hAboutPage);
									break;
								//default:
								//	ChangePage(hGeneralPage);
							}
							return TRUE;	
					}
					break;

				case IDOK: 
					saveConfig();
					DestroyWindow(hwndDlg); 
					hSettingsWnd = NULL; 
					return TRUE; 
				
				case IDCANCEL: 
					DestroyWindow(hwndDlg); 
					hSettingsWnd = NULL; 
					return TRUE; 
			}
	}
	return FALSE;
}

//-----------------------------------------------------------------------------
bool ShowSettingsDlg(HWND hParentWnd)
{
	if (hSettingsWnd = CreateDialog(0, MAKEINTRESOURCE(IDD_SETTINGS), hParentWnd, (DLGPROC)SettingsDlgProc))
		return true;
	else
		return false;
}