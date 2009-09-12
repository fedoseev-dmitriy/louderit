#pragma once

#define HKComboBox_AddKey(hComboBox, vk, fExtended) ((HWND)(UINT)(BOOL)SNDMSG(hComboBox, CB_ADDSTRING, 0, (LPARAM)GetKeyName(vk, fExtended).c_str()));

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

extern bool GetAppPath(wchar_t *path);
extern void GetConfigFile(void);
extern bool ShowSettingsDlg(HWND hParentWnd);