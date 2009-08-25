#include "precompiled.h"
#include "settings.h"
#include "resource.h"

HWND hSettingsWnd = NULL;
HWND hTabControl = NULL;

//-----------------------------------------------------------------------------
bool ShowSettingsDlg(HWND hParentWnd)
{
	if (hSettingsWnd = CreateDialog(0, MAKEINTRESOURCE(IDD_PROPDLG), hParentWnd, (DLGPROC)SettingsDlgProc))
		return true;
	else
		return false;
}

//-----------------------------------------------------------------------------
INT_PTR CALLBACK SettingsDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:	// before a dialog is displayed
			hTabControl = GetDlgItem(hwndDlg, IDC_TAB);
			
			//DLGHDR *pHdr = (DLGHDR *) LocalAlloc(LPTR, sizeof(DLGHDR)); 
			//TCITEM tie;
			//SetWindowLong(hwndDlg, GWL_USERDATA, (LONG) pHdr); 
			//pHdr->hwndTab = GetDlgItem(hwndDlg, IDC_TAB);
			//tie.mask = TCIF_TEXT | TCIF_IMAGE; 
			//tie.iImage = -1; 
			//tie.pszText = L"First"; 
			//TabCtrl_InsertItem(pHdr->hwndTab, 0, &tie);
			//pHdr->apRes[0] = DoLockDlgRes(MAKEINTRESOURCE(IDD_PROPTAB_HK)); 

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