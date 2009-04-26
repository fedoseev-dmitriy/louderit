#include "precompiled.h"
#include "tray.h"

//-----------------------------------------------------------------------------
TrayIcon::TrayIcon(HWND hWnd):
	hwnd_(hWnd),
	is_setted_(false)
{

}

//-----------------------------------------------------------------------------
TrayIcon::~TrayIcon()
{

}

//-----------------------------------------------------------------------------
bool TrayIcon::Add( HICON icon, const wchar_t* tip_text )
{
	if (is_setted_)
		return false;

	ZeroMemory(&nid, sizeof(NOTIFYICONDATA));
	
	nid.cbSize = NOTIFYICONDATA_V2_SIZE;
    nid.hWnd = hwnd_;
    nid.uID = 0;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_NOTIFYICONTRAY;
    nid.hIcon = icon;
    _tcscpy_s(nid.szTip, tip_text);
		
	return is_setted_ = Shell_NotifyIcon(NIM_ADD, &nid);
}

//-----------------------------------------------------------------------------
bool TrayIcon::Update(HICON hIcon, const wchar_t* tip_text)
{
	
	if (!is_setted_)
		return false;
	
	nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	nid.hIcon = hIcon;
	_tcscpy_s(nid.szTip, tip_text);

	return is_setted_ = Shell_NotifyIcon(NIM_MODIFY, &nid);
}

//-----------------------------------------------------------------------------
bool TrayIcon::Remove()
{
	if (!is_setted_)
		return false;	

	return is_setted_ = Shell_NotifyIcon(NIM_DELETE, &nid);
}

//-----------------------------------------------------------------------------
bool TrayIcon::Restore()
{
	if (!is_setted_)
		return false;

	is_setted_ = false;

	return Add(nid.hIcon, nid.szTip);
}

//-----------------------------------------------------------------------------
bool TrayIcon::ShowBaloon(const wchar_t* szInfo, const wchar_t* szInfoTitle)
{
	if (!is_setted_)
		return false;

	nid.uFlags = NIF_INFO;
	_tcscpy_s(nid.szInfo, szInfo);
	nid.uTimeout = 0;
	_tcscpy_s(nid.szInfoTitle, szInfoTitle);

	return is_setted_ = Shell_NotifyIcon(NIM_MODIFY, &nid);
}