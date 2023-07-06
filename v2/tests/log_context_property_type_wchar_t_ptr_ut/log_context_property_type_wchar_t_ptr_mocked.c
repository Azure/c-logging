// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdio.h>
#include <stdarg.h>
#include <wchar.h>

#define snprintf mock_snprintf
#define vswprintf mock_vswprintf

extern int mock_snprintf(char* s, size_t n, const char* format, ...);
extern int mock_vswprintf(wchar_t* s, size_t n, const wchar_t* format, va_list arg_list);

#include "log_context_property_type_wchar_t_ptr.c"
