#include "precompiled.h"
#include "settings.h"
#include "resource.h"

HWND hSettingsWnd = NULL;
HWND hActionComboBox = NULL;
HWND hHKComboBox = NULL;

wchar_t config_name[] = L"lconfig.ini";	// config filename
wchar_t config_file[MAX_PATH] = {0};	// config fullpath

//------------------------------------------------------------------------------
// Settings
//------------------------------------------------------------------------------
wchar_t device_name[MAX_PATH] = {0};
int steps = 0;
wchar_t skin_name[MAX_PATH] = {0};
int balance = 50;
bool balloonHint = false;
bool scrollWithTray = false;
bool scrollWithCtrl = false;
bool scrollWithAlt = false;
bool scrollWithShift = false;

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
// Getting the config file path
//------------------------------------------------------------------------------
void GetConfigFile(void)
{
	GetAppPath(config_file);
	_tcscat_s(config_file, config_name);
}

//-----------------------------------------------------------------------------
wstring GetKeyName(UINT key)
{
	wchar_t keyName[64];

	UINT scanCode = MapVirtualKey(key, 0) << 16;
	switch(key)
	{
		case VK_INSERT:
		case VK_DELETE:
		case VK_HOME:
		case VK_END:
		case VK_NEXT:
		case VK_PRIOR:
		case VK_LEFT:
		case VK_RIGHT:
		case VK_UP:
		case VK_DOWN:
			scanCode |= 0x01000000; // add extendet flag
	}
	
	GetKeyNameText(scanCode, keyName, sizeof(keyName));
	return keyName;
}

//-----------------------------------------------------------------------------
void SaveConfig()
{
	// FIXME!
	WritePrivateProfileString(L"HotKeys", L"VolumeUp", L"0", config_file);
	WritePrivateProfileString(L"HotKeys", L"VolumeDown", L"0", config_file);
	WritePrivateProfileString(L"HotKeys", L"VolumeMute", L"0", config_file);
	WritePrivateProfileString(L"HotKeys", L"ShowMixer", L"0", config_file);

}

//-----------------------------------------------------------------------------
INT_PTR CALLBACK SettingsDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	UINT id = 0;
	
	switch (uMsg)
	{
		case WM_INITDIALOG:	// before a dialog is displayed
			hActionComboBox = GetDlgItem(hwndDlg, IDC_ACTCOMBO);
			ComboBox_AddString(hActionComboBox, L"Volume Up");
			ComboBox_AddString(hActionComboBox, L"Volume Down");
			ComboBox_AddString(hActionComboBox, L"Volume Mute");
			ComboBox_AddString(hActionComboBox, L"Show Mixer");
			ComboBox_SetCurSel(hActionComboBox, 0);
			
			hHKComboBox = GetDlgItem(hwndDlg, IDC_HKCOMBO);
			ComboBox_AddString(hHKComboBox, L"None");
			// adding hokeys names to list
			HKComboBox_AddKey(hHKComboBox, VK_BACK);
			HKComboBox_AddKey(hHKComboBox, VK_TAB);
			HKComboBox_AddKey(hHKComboBox, VK_RETURN);
			HKComboBox_AddKey(hHKComboBox, VK_CAPITAL);
			HKComboBox_AddKey(hHKComboBox, VK_ESCAPE);
			HKComboBox_AddKey(hHKComboBox, VK_SPACE);
			for (id=VK_PRIOR; id<=VK_DOWN; id++)
				HKComboBox_AddKey(hHKComboBox, id);
			for (id=VK_INSERT; id<=VK_DELETE; id++)
				HKComboBox_AddKey(hHKComboBox, id);
			for (id=0x30; id<=0x39; id++)				//numbers
				HKComboBox_AddKey(hHKComboBox, id);
			for (id=0x41; id<=0x5A; id++)				//words
				HKComboBox_AddKey(hHKComboBox, id);
			for (id=VK_NUMPAD0; id<=VK_ADD; id++)		//numpad
				HKComboBox_AddKey(hHKComboBox, id);
			for (id=VK_SUBTRACT; id<=VK_F12; id++)		//numpad and F1-F12
				HKComboBox_AddKey(hHKComboBox, id);
			HKComboBox_AddKey(hHKComboBox, VK_SNAPSHOT);
			HKComboBox_AddKey(hHKComboBox, VK_SCROLL);
			ComboBox_SetCurSel(hHKComboBox, 0);
							
			return TRUE;
		
		case WM_COMMAND:	// notification msgs from child controls
			switch (LOWORD(wParam)) 
			{ 
				case IDOK: 
					SaveConfig();
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
