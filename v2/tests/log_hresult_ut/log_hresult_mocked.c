// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdio.h>

#include "windows.h"
#include "psapi.h"

DWORD mock_GetLastError(void);
DWORD mock_FormatMessageA(DWORD dwFlags, LPCVOID lpSource, DWORD dwMessageId, DWORD dwLanguageId, LPSTR lpBuffer, DWORD nSize, va_list* Arguments);
HANDLE mock_GetCurrentProcess(void);
BOOL mock_EnumProcessModules(HANDLE hProcess, HMODULE* lphModule, DWORD cb, LPDWORD lpcbNeeded);
int mock_snprintf(char* s, size_t n, const char* format, ...);

#define GetLastError mock_GetLastError
#define FormatMessageA mock_FormatMessageA
#define GetCurrentProcess mock_GetCurrentProcess
#undef EnumProcessModules
#define EnumProcessModules mock_EnumProcessModules
#define snprintf mock_snprintf

#include "log_hresult.c"
