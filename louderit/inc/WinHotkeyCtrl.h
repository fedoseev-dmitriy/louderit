// WinHotkeyCtrl.h
#pragma once

//namespace NWinHotkeyCtrl {
BOOL InitWinHotkeyCtrls();
BOOL SubClassWinHotkeyCtrl(HWND);
DWORD GetWinHotkey(HWND);
void SetWinHotkey(HWND, DWORD);
//} // namespace NWinHotkeyCtrl
