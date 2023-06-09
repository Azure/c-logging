// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdio.h>
#include <stdarg.h>

#define snprintf mock_snprintf
#define vsprintf mock_vsprintf
#define vsnprintf mock_vsnprintf

extern int mock_snprintf(char* s, size_t n, const char* format, ...);
extern int mock_vsprintf(char* s, const char* format, va_list arg_list);
extern int mock_vsnprintf(char* s, size_t n, const char* format, va_list arg_list);

#include "log_context_property_boolean_type.c"
