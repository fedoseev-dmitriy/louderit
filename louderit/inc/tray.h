#pragma once

#define WM_NOTIFYICONTRAY (WM_USER + 1)

// Notify icon for system tray 
class TrayIcon
{
public:
					TrayIcon(HWND hwnd);
	virtual			~TrayIcon();

	bool			Add(HICON icon, const wchar_t* tip_text);
	bool			Update(HICON icon, const wchar_t* tip_text);
	bool			Remove();
	bool			Restore();
	bool			ShowBaloon(const wchar_t* szInfo, const wchar_t* szInfoTitle);

protected:
	const HWND		hwnd_;
	bool			is_setted_;

private:
	NOTIFYICONDATA	nid;
};