#include "precompiled.h"
#include "tray.h"

//-----------------------------------------------------------------------------
TrayIcon::TrayIcon(HWND hWnd, LPCTSTR szTip):
	m_hWnd(hWnd),
	m_szTip(szTip),
	m_Setted(false)
{

}

//-----------------------------------------------------------------------------
TrayIcon::~TrayIcon()
{

}

//-----------------------------------------------------------------------------
bool TrayIcon::Set(HICON hIcon)
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
    lstrcpyn(nid.szTip, m_szTip, sizeof(nid.szTip));
	
	return (m_Setted = Shell_NotifyIcon(NIM_ADD, &nid));
}

//-----------------------------------------------------------------------------
bool TrayIcon::Update(HICON hIcon)
{
	
	if (!m_Setted)
		return false;
	
	nid.hIcon = hIcon;

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

	return Set(nid.hIcon);
}