#pragma once

#define WM_NOTIFYICONTRAY (WM_USER + 1)

// Notify icon for system tray 
class TrayIcon
{
public:
	TrayIcon(HWND hWnd, LPCTSTR szTip);
	virtual			~TrayIcon();

	bool			Set(HICON hIcon);
	bool			Update(HICON hIcon);
	bool			Remove();
	bool			Restore();

protected:
	const HWND m_hWnd;
	const LPCTSTR m_szTip;
	bool m_Setted;

private:
	NOTIFYICONDATA nid;
};