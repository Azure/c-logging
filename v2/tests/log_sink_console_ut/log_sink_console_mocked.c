// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdio.h>
#include <time.h>

#define printf mock_printf
#define time mock_time
#define ctime mock_ctime
#define vsnprintf mock_vsnprintf
#define snprintf mock_snprintf

int mock_printf(const char* format, ...);
time_t mock_time(time_t* const _time);
char* mock_ctime(const time_t* timer);
int mock_vsnprintf(char* s, size_t n, const char* format, va_list arg);
int mock_snprintf(char* s, size_t n, const char* format, ...);

#include "log_sink_console.c"
