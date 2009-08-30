#include "precompiled.h"
#include "settings.h"
#include "lexical_cast.h"
#include "resource.h"

HWND hSettingsWnd = NULL;
HWND hUpCombo = NULL;
HWND hDownCombo = NULL;
wchar_t *wc = NULL;

//-----------------------------------------------------------------------------
bool ShowSettingsDlg(HWND hParentWnd)
{
	if (hSettingsWnd = CreateDialog(0, MAKEINTRESOURCE(IDD_SETTINGS), hParentWnd, (DLGPROC)SettingsDlgProc))
		return true;
	else
		return false;
}


//-----------------------------------------------------------------------------
wstring getKeyName(UINT vk, BOOL fExtended)
{
	LONG lScan = MapVirtualKey(vk, 0) << 16;

	// if it's an extended key, add the extended flag
	if (fExtended)
		lScan |= 0x01000000L;
	int nBufferLen = 64;

	wstring str;
	int nLen;
	do
	{
		nBufferLen *= 2;
		str.resize(nBufferLen);
		nLen = GetKeyNameText(lScan, &str[0], nBufferLen);
	}
	while (nLen == nBufferLen);
	return str;
}

//-----------------------------------------------------------------------------
INT_PTR CALLBACK SettingsDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:	// before a dialog is displayed
			hUpCombo = GetDlgItem(hwndDlg, IDC_UPCOMBO);
			SendMessage(hUpCombo, CB_ADDSTRING, 0, (LPARAM)L"None");
			
			SendMessage(hUpCombo, CB_SETCURSEL, 0, 0);
			

			return TRUE;
		case WM_COMMAND:	// notification msgs from child controls
			switch (LOWORD(wParam)) 
			{ 
				case IDOK: 
					MessageBox(hwndDlg, L"OK", L"Error!", MB_OK | MB_ICONERROR);
					return TRUE; 
				case IDCANCEL: 
					DestroyWindow(hwndDlg); 
					hSettingsWnd = NULL; 
					return TRUE; 
			}
	}
	return FALSE;
}