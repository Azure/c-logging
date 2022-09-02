// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdio.h>

#define snprintf mock_snprintf
#define vsprintf mock_vsprintf
#define vsnprintf mock_vsnprintf

extern int mock_snprintf(char* s, size_t n, const char* format, ...);
extern int mock_vsprintf(char* s, const char* format, ...);
extern int mock_vsnprintf(char* s, size_t n, const char* format, ...);

#include "log_context_property_type_ascii_char_ptr.c"
