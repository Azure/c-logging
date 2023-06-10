// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef FORMAT_MESSAGE_NO_NEWLINE_H
#define FORMAT_MESSAGE_NO_NEWLINE_H

#ifndef __cplusplus
#include <stdarg.h>
#else
#include <cstdarg>
#endif

#include "windows.h"

DWORD FormatMessageA_no_newline(DWORD dwFlags, LPCVOID lpSource, DWORD dwMessageId, DWORD dwLanguageId, LPSTR lpBuffer, DWORD nSize, va_list* Arguments);

#endif /* FORMAT_MESSAGE_NO_NEWLINE_H */
