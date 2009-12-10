#pragma once

extern wchar_t config_file[MAX_PATH];	// config fullpath

//------------------------------------------------------------------------------
// Settings
//------------------------------------------------------------------------------
extern wchar_t device_name[MAX_PATH];
extern int steps;
extern wchar_t skin_name[MAX_PATH];
extern int balance;
extern bool balloonHint;
extern bool scrollWithTray;
extern bool scrollWithCtrl;
extern bool scrollWithAlt;
extern bool scrollWithShift;

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