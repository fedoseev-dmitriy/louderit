#include "precompiled.h"
#include "settings.h"
#include "resource.h"
#include "WinHotkeyCtrl.h"

vector<wstring> settingsItems;

HWND hSettingsWnd = NULL;
HWND hSettingsListBox = NULL;

HWND hGeneralPage = NULL;
HWND hHotkeysPage = NULL;

wchar_t config_name[] = L"lconfig.ini";	// config filename
wchar_t config_file[MAX_PATH] = {0};	// config fullpath

//------------------------------------------------------------------------------
// Settings
//------------------------------------------------------------------------------
wchar_t device_name[MAX_PATH] = {0};
int steps = 0;
wchar_t skin_name[MAX_PATH] = {0};
int balance = 50;
bool balloon_hint = false;
bool scroll_with_tray = false;
bool scroll_with_ctrl = false;
bool scroll_with_alt = false;
bool scroll_with_shift = false;

static WNDPROC OldEditProc;
static DWORD tempModifiers;
static DWORD modifiers;
static DWORD virtualKey;
static TCHAR *keySeparator = _T(" + ");

//------------------------------------------------------------------------------
// Getting the application directory
//------------------------------------------------------------------------------
bool getAppPath(wchar_t *path)
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
void getConfigFile(void)
{
	getAppPath(config_file);
	_tcscat_s(config_file, config_name);
}

////-----------------------------------------------------------------------------
//static const TCHAR* getKeyName(DWORD key) {
//	static TCHAR keyName[64];
//	int nameLen = 0;
//	ZeroMemory(keyName, sizeof(keyName));
//	if (key & MOD_CONTROL) {
//		GetKeyNameText(MAKELPARAM(0, MapVirtualKey(VK_CONTROL, 0)), keyName, 64);
//		_tcscat(keyName, keySeparator);
//		nameLen = _tcslen(keyName);
//	}
//	if (key & MOD_SHIFT) {
//		GetKeyNameText(MAKELPARAM(0, MapVirtualKey(VK_SHIFT, 0)), &keyName[nameLen], 64 - nameLen);
//		_tcscat(keyName, keySeparator);
//		nameLen = _tcslen(keyName);
//	}
//	if (key & MOD_ALT) {
//		GetKeyNameText(MAKELPARAM(0, MapVirtualKey(VK_MENU, 0)), &keyName[nameLen], 64 - nameLen);
//		_tcscat(keyName, keySeparator);
//		nameLen = _tcslen(keyName);
//	}
//	if ((key & 0xFFFF) != 0) {
//		DWORD scanCode = MapVirtualKey(key & 0xFFFF, 0);
//		switch(key & 0xFFFF) {
//		case VK_INSERT:
//		case VK_DELETE:
//		case VK_HOME:
//		case VK_END:
//		case VK_NEXT:
//		case VK_PRIOR:
//		case VK_LEFT:
//		case VK_RIGHT:
//		case VK_UP:
//		case VK_DOWN:
//			scanCode |= 0x100; // Add extended bit
//		}
//		GetKeyNameText(MAKELPARAM(0, scanCode), &keyName[nameLen], 64 - nameLen);
//		nameLen = _tcslen(keyName);
//	}
//	return keyName;
//}

//-----------------------------------------------------------------------------
void saveConfig()
{
	// FIXME!
	WritePrivateProfileString(L"HotKeys", L"VolumeUp", L"0", config_file);
	WritePrivateProfileString(L"HotKeys", L"VolumeDown", L"0", config_file);
	WritePrivateProfileString(L"HotKeys", L"VolumeMute", L"0", config_file);
	WritePrivateProfileString(L"HotKeys", L"ShowMixer", L"0", config_file);

}

//static void refreshPreview(HWND hwnd) 
//{
//	SetWindowText(hwnd, getKeyName(virtualKey | modifiers));
//}
//
//static LRESULT CALLBACK HKEditProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
//{
//	switch (msg) {
//		case WM_CREATE:
//			virtualKey = 0;
//			break;
//		case WM_SYSKEYDOWN:
//		case WM_KEYDOWN:
//			if (virtualKey != 0) {
//				virtualKey = 0;
//			}
//		    switch (wParam)
//			{
//				case VK_CONTROL:
//				case VK_LCONTROL:
//				case VK_RCONTROL: tempModifiers = MOD_CONTROL; break;
//				case VK_MENU:
//				case VK_LMENU: 
//				case VK_RMENU: tempModifiers = MOD_ALT; break;
//				case VK_SHIFT:
//				case VK_LSHIFT:
//				case VK_RSHIFT: tempModifiers = MOD_SHIFT; break;
//				
//				default:
//					virtualKey = wParam;
//					break;
//			}
//			modifiers = tempModifiers;
//			refreshPreview(hwnd);
//			return 0;
//		case WM_KEYUP:
//		case WM_SYSKEYUP:
//			switch (wParam)
//			{
//				case VK_SHIFT:
//					tempModifiers &= ~MOD_SHIFT;
//					break;
//				case VK_CONTROL:
//					tempModifiers &= ~MOD_CONTROL;
//					break;
//				case VK_MENU:
//					tempModifiers &= ~MOD_ALT;
//					break;
//				default:
//					break;
//			}
//			if (virtualKey == 0) {
//				modifiers = tempModifiers;
//				refreshPreview(hwnd);
//			}
//		case WM_CHAR:
//		case WM_PASTE:
//			return 0;
//		case WM_SETFOCUS:
//			modifiers = 0;
//			tempModifiers = 0;
//			virtualKey = 0;
//			refreshPreview(hwnd);
//			break;
//		case WM_GETDLGCODE:
//			return DLGC_WANTARROWS|DLGC_WANTALLKEYS| DLGC_WANTTAB;
//	}
//	return CallWindowProc(OldEditProc, hwnd, msg, wParam, lParam);
//}
//
//-----------------------------------------------------------------------------
INT_PTR CALLBACK GeneralPage_Proc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
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

INT_PTR CALLBACK SettingsDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	UINT id = 0;
	
	switch (uMsg)
	{
		case WM_INITDIALOG:	// before a dialog is displayed
			
			settingsItems.push_back(L"General");
			settingsItems.push_back(L"View");
			settingsItems.push_back(L"Mouse");
			settingsItems.push_back(L"Hotkeys");
			settingsItems.push_back(L"About");
			hSettingsListBox = GetDlgItem(hwndDlg, IDC_LIST);
			for(unsigned short i=0; i<settingsItems.size(); i++)
			{
				ListBox_AddString(hSettingsListBox, settingsItems[i].c_str());
			}
			ListBox_SetCurSel(hSettingsListBox, 0);
			hGeneralPage = CreateDialog(0, MAKEINTRESOURCE(IDD_GENERAL), hwndDlg, (DLGPROC)GeneralPage_Proc);
			hHotkeysPage = CreateDialog(0, MAKEINTRESOURCE(IDD_HOTKEYS), hwndDlg, (DLGPROC)HotkeysPage_Proc);

			
			//SubClassWinHotkeyCtrl(GetDlgItem(hwndDlg, IDC_EDIT));
			//OldEditProc = (WNDPROC) SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_EDIT), GWLP_WNDPROC, (LONG_PTR) HKEditProc);
				
			return TRUE;
		
		case WM_COMMAND:	// notification msgs from child controls
			switch (LOWORD(wParam))
			{
				case IDC_LIST:
					switch (HIWORD(wParam))
					{
						case LBN_SELCHANGE:
							MessageBox(hwndDlg, L"Current page = visible", L"!!!!", MB_OK | MB_ICONERROR); // fixme
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