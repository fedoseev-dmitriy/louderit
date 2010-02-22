#pragma once

extern wchar_t config_file[MAX_PATH];	// config fullpath

//------------------------------------------------------------------------------
// Settings
//------------------------------------------------------------------------------
extern wchar_t device_name[MAX_PATH];
extern int steps;
extern wchar_t skin_name[MAX_PATH];
extern int balance;
extern bool balloon_hint;
extern bool scroll_with_tray;
extern bool scroll_with_ctrl;
extern bool scroll_with_alt;
extern bool scroll_with_shift;

extern bool getAppPath(wchar_t *path);
extern void getConfigFile(void);
extern bool ShowSettingsDlg(HWND hParentWnd);

//class SettingsPage
//{
//public:
//					SettingsPage(HWND hWnd);
//	virtual			~SettingsPage();
//
//	bool			SetTitle(wstring);
//	wstring			GetTitle();
//
//	INT_PTR CALLBACK PageDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
//
//protected:
//	wstring			m_Title;
//};