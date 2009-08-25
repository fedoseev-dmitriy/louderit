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
bool TrayIcon::Add(HICON hIcon, LPCTSTR szTip)
{
	if (m_Setted)
		return false;

	ZeroMemory(&nid, sizeof(NOTIFYICONDATA));
	
	nid.cbSize = NOTIFYICONDATA_V2_SIZE;
    nid.hWnd = m_hWnd;
    nid.uID = 0;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_NOTIFYICONTRAY;
    nid.hIcon = hIcon;
    _tcscpy_s(nid.szTip, szTip);
		
	return m_Setted = Shell_NotifyIcon(NIM_ADD, &nid);
}

//-----------------------------------------------------------------------------
bool TrayIcon::Update(HICON hIcon, const wchar_t* szTip)
{
	
	if (!m_Setted)
		return false;
	
	nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	nid.hIcon = hIcon;
	_tcscpy_s(nid.szTip, szTip);

	return m_Setted = Shell_NotifyIcon(NIM_MODIFY, &nid);
}

//-----------------------------------------------------------------------------
bool TrayIcon::Remove()
{
	if (!m_Setted)
		return false;	

	return m_Setted = Shell_NotifyIcon(NIM_DELETE, &nid);
}

//-----------------------------------------------------------------------------
bool TrayIcon::Restore()
{
	if (!m_Setted)
		return false;

	m_Setted = false;

	return Add(nid.hIcon, nid.szTip);
}

//-----------------------------------------------------------------------------
bool TrayIcon::ShowBaloon(LPCTSTR szInfo, const wchar_t* szInfoTitle)
{
	if (!m_Setted)
		return false;

	nid.uFlags = NIF_INFO;
	_tcscpy_s(nid.szInfo, szInfo);
	nid.uTimeout = 0;
	_tcscpy_s(nid.szInfoTitle, szInfoTitle);

	return m_Setted = Shell_NotifyIcon(NIM_MODIFY, &nid);
}