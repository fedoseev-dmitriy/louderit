#pragma once

//#define C_PAGES 3 
// 
//typedef struct tag_dlghdr { 
//    HWND hwndTab;       // tab control 
//    HWND hwndDisplay;   // current child dialog box 
//    RECT rcDisplay;     // display rectangle for the tab control 
//    DLGTEMPLATE *apRes[C_PAGES]; 
//} DLGHDR; 

bool ShowSettingsDlg(HWND hParentWnd);
INT_PTR CALLBACK SettingsDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);