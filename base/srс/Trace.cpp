//
//	MODULE:		Trace.cpp
//
//	PURPOSE:	Provides TRACE macros (ala MFC) which wrap the 
//				OutputDebugString API call - removing these
//				calls in release build.
//
//	USAGE:		TRACE(...)  for TCHAR-strings
//				TRACEA(...) for char-strings
//				TRACEW(...) for WCHAR-strings
//
//	HISTORY:	v1.0 16/04/2002 J Brown		- Initial version
//				v1.1 25/01/2009 Dmitriy Fedoseev - replace _s (security)
//

#include "precompiled.h"
#include <windows.h>
#include <stdarg.h>
#include "Trace.h"

//
//	Wide-character (UNICODE) version
//
void TraceW(LPCWSTR szFmt, ...)
{
	wchar_t szBuf[0x200];

	va_list arg;

	if(szFmt == 0) return;

	va_start(arg, szFmt);

	_vsnwprintf_s( szBuf,0x200, _TRUNCATE, szFmt, arg);
	OutputDebugStringW(szBuf);
	
	va_end(arg);
}

//
//	Single-character (ANSI) version
//
void TraceA(LPCSTR szFmt, ...)
{
	char szBuf[0x200];

	va_list arg;

	if(szFmt == 0) return;

	va_start(arg, szFmt);

	_vsnprintf_s( szBuf,0x200, _TRUNCATE, szFmt, arg);
	OutputDebugStringA(szBuf);
	
	va_end(arg);
}

