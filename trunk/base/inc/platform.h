#pragma once

#define SAFE_RELEASE(punk)  \
	if ((punk) != NULL)  \
{ (punk)->Release(); (punk) = NULL; }

#define EXIT_ON_ERROR(hres)  \
	if (FAILED(hres)) { MessageBox(0, L"ERROR", L"ERROR", 0); }