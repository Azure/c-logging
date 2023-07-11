// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdarg.h>
#include <stdlib.h>// IWYU pragma: keep
#include <wchar.h>

#define wcstombs mock_wcstombs
#define vswprintf mock_vswprintf

extern size_t mock_wcstombs(char* dest, wchar_t const* src, size_t max);
extern int mock_vswprintf(wchar_t* s, size_t n, const wchar_t* format, va_list arg_list);

#include "log_context_property_type_wchar_t_ptr.c"
