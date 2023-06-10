// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "windows.h"

DWORD mock_GetLastError(void);
DWORD mock_FormatMessageA(DWORD dwFlags, LPCVOID lpSource, DWORD dwMessageId, DWORD dwLanguageId, LPSTR lpBuffer, DWORD nSize, va_list* Arguments);

#define GetLastError mock_GetLastError
#define FormatMessageA mock_FormatMessageA

#include "log_lasterror.c"
