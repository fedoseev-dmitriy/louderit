#pragma once

#define WM_NOTIFYICONTRAY (WM_USER + 1)

// Notify icon for system tray 
class TrayIcon
{
public:
	TrayIcon(HWND hWnd);
	virtual			~TrayIcon();

	bool			Set(HICON hIcon, LPCTSTR szTip);
	bool			Update(HICON hIcon, LPCTSTR szTip);
	bool			Remove();
	bool			Restore();
	bool			ShowBaloon(LPCTSTR szInfo, LPCTSTR szInfoTitle);

protected:
	const HWND m_hWnd;
	bool m_Setted;

private:
	NOTIFYICONDATA nid;
};