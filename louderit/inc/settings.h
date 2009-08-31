#pragma once

#define HKComboBox_AddKey(hComboBox, vk, fExtended) ((HWND)(UINT)(BOOL)SNDMSG(hComboBox, CB_ADDSTRING, 0, (LPARAM)getKeyName(vk, fExtended).c_str()));

bool ShowSettingsDlg(HWND hParentWnd);
INT_PTR CALLBACK SettingsDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);