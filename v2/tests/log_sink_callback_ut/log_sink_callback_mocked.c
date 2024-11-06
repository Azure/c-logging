// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdarg.h>
#include <stdint.h>
#include <time.h>

#include "c_logging/log_context.h"
#include "c_logging/log_context_property_value_pair.h"

#define printf mock_printf
#define time mock_time
#define ctime mock_ctime
#define vsnprintf mock_vsnprintf
#define snprintf mock_snprintf
#define log_context_get_property_value_pair_count mock_log_context_get_property_value_pair_count
#define log_context_get_property_value_pairs mock_log_context_get_property_value_pairs
#define log_context_property_to_string mock_log_context_property_to_string

int mock_printf(const char* format, ...);
time_t mock_time(time_t* const _time);
char* mock_ctime(const time_t* timer);
int mock_vsnprintf(char* s, size_t n, const char* format, va_list arg);
int mock_snprintf(char* s, size_t n, const char* format, ...);
uint32_t mock_log_context_get_property_value_pair_count(LOG_CONTEXT_HANDLE log_context);
const LOG_CONTEXT_PROPERTY_VALUE_PAIR* mock_log_context_get_property_value_pairs(LOG_CONTEXT_HANDLE log_context);
int mock_log_context_property_to_string(char* buffer, size_t buffer_size, const LOG_CONTEXT_PROPERTY_VALUE_PAIR* property_value_pairs, size_t property_value_pair_count);

#include "log_sink_callback.c"

#undef printf
#undef time
#undef ctime
#undef vsnprintf
#undef snprintf
#undef log_context_get_property_value_pair_count
#undef log_context_get_property_value_pairs
#undef log_context_property_to_string
