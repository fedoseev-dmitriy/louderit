#pragma once

#define HKComboBox_AddKey(hComboBox, vk, fExtended) ((HWND)(UINT)(BOOL)SNDMSG(hComboBox, CB_ADDSTRING, 0, (LPARAM)GetKeyName(vk, fExtended).c_str()));

extern wchar_t config_file[MAX_PATH];

extern bool GetAppPath(wchar_t *path);
extern void GetConfigFile(void);
extern bool ShowSettingsDlg(HWND hParentWnd);