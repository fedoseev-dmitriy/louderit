#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#define NOMINMAX
// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
//#include <malloc.h>
//#include <memory.h>
#include <tchar.h>

#include <iostream>
#include <list>
#include <algorithm>
#include <vector>
#include <set>
#include <ostream>
#include <string>
#include <sstream>
// TODO: reference additional headers your program requires here
#include <commctrl.h>
#include <windowsx.h>
#include <MMSystem.h>
#include <ShellAPI.h>
#include <shlobj.h>
#include <strsafe.h>
#include <functiondiscoverykeys.h>
#include <MMSystem.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>

#pragma comment(lib, "Winmm.lib")
//#pragma comment(lib, "lhook.lib")

#include "Trace.h"

using namespace std;

