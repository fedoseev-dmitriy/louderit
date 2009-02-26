#include "precompiled.h"
#include "tray.h"

//-----------------------------------------------------------------------------
TrayIcon::TrayIcon(HWND hWnd):
	m_hWnd(hWnd),
	m_Setted(false)
{

}

//-----------------------------------------------------------------------------
TrayIcon::~TrayIcon()
{

}

//-----------------------------------------------------------------------------
bool TrayIcon::Set(HICON hIcon, LPCTSTR szTip)
{
	if (m_Setted)
		return false;

	ZeroMemory(&nid, sizeof(NOTIFYICONDATA));
	
	nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = m_hWnd;
    nid.uID = 0;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_NOTIFYICONTRAY;
    nid.hIcon = hIcon;
    lstrcpyn(nid.szTip, szTip, sizeof(nid.szTip));
		
	return (m_Setted = Shell_NotifyIcon(NIM_ADD, &nid));
}

//-----------------------------------------------------------------------------
bool TrayIcon::Update(HICON hIcon, LPCTSTR szTip)
{
	
	if (!m_Setted)
		return false;
	
	nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	nid.hIcon = hIcon;
	lstrcpyn(nid.szTip, szTip, sizeof(nid.szTip));

	return (m_Setted = Shell_NotifyIcon(NIM_MODIFY, &nid));
}

//-----------------------------------------------------------------------------
bool TrayIcon::Remove()
{
	if (!m_Setted)
		return false;	

	return !(m_Setted = !Shell_NotifyIcon(NIM_DELETE, &nid));
}

//-----------------------------------------------------------------------------
bool TrayIcon::Restore()
{
	if (!m_Setted)
		return false;

	m_Setted = false;

	return Set(nid.hIcon, nid.szTip);
}

//-----------------------------------------------------------------------------
bool TrayIcon::ShowBaloon(LPCTSTR szInfo, LPCTSTR szInfoTitle)
{
	if (!m_Setted)
		return false;

	nid.uFlags = NIF_INFO;
	lstrcpyn(nid.szInfo, szInfo, sizeof(nid.szInfo));
	nid.uTimeout = 0;
	lstrcpyn(nid.szInfoTitle, szInfoTitle, sizeof(nid.szInfoTitle));

	return (m_Setted = Shell_NotifyIcon(NIM_MODIFY, &nid));
}