// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>

#include "macro_utils/macro_utils.h"

#include "c_logging/log_level.h"
#include "c_logging/logger.h"
#include "c_logging/log_context.h"
#include "c_logging/log_sink_if.h"

#include "c_logging/log_sink_console.h"

/* Codes_SRS_LOG_SINK_CONSOLE_01_007: [ `LOG_LEVEL_CRITICAL` shall be displayed with bright red `\x1b[31;1m`. ]*/
#define LOG_SINK_CONSOLE_ANSI_COLOR_BRIGHT_RED      "\x1b[31;1m"
/* Codes_SRS_LOG_SINK_CONSOLE_01_008: [ `LOG_LEVEL_ERROR` shall be displayed with red `\x1b[31m`. ]*/
#define LOG_SINK_CONSOLE_ANSI_COLOR_RED             "\x1b[31m"
/* Codes_SRS_LOG_SINK_CONSOLE_01_009: [ `LOG_LEVEL_WARNING` shall be displayed with bright yellow `\x1b[33;1m`. ]*/
#define LOG_SINK_CONSOLE_ANSI_COLOR_BRIGHT_YELLOW   "\x1b[33;1m"
#define LOG_SINK_CONSOLE_ANSI_COLOR_YELLOW          "\x1b[33m"
#define LOG_SINK_CONSOLE_ANSI_COLOR_WHITE           "\x1b[37m"
#define LOG_SINK_CONSOLE_ANSI_COLOR_RESET           "\x1b[0m"

static const char* level_colors[] =
{
    LOG_SINK_CONSOLE_ANSI_COLOR_BRIGHT_RED,     // LOG_LEVEL_CRITICAL
    LOG_SINK_CONSOLE_ANSI_COLOR_RED,            // LOG_LEVEL_ERROR
    LOG_SINK_CONSOLE_ANSI_COLOR_BRIGHT_YELLOW,  // LOG_LEVEL_WARNING
    LOG_SINK_CONSOLE_ANSI_COLOR_YELLOW,         // LOG_LEVEL_INFO
    LOG_SINK_CONSOLE_ANSI_COLOR_WHITE,          // LOG_LEVEL_VERBOSE
};

static int log_n_properties(char* buffer, size_t buffer_size, const LOG_CONTEXT_PROPERTY_VALUE_PAIR* property_value_pairs, size_t property_value_pair_count)
{
    int result = 0;
    for (size_t i = 0; i < property_value_pair_count; i++)
    {
        if (property_value_pairs[i].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_struct)
        {
            (void)printf("%s%s{ ", property_value_pairs[i].name, property_value_pairs[i].name[0] == 0 ? "" : "=");
            uint8_t struct_properties_count = *(uint8_t*)(property_value_pairs[i].value);
            log_n_properties(buffer, buffer_size, &property_value_pairs[i + 1], struct_properties_count);
            i += struct_properties_count;
            (void)printf("} ");
        }
        else
        {
            int snprintf_result = snprintf(buffer, buffer_size, " %s=", property_value_pairs[i].name);
            if (snprintf_result < 0)
            {
                // error
            }
            else
            {
                buffer += snprintf_result;
                buffer_size -= snprintf_result;

                int to_string_result = property_value_pairs[i].type->to_string(property_value_pairs[i].value, buffer, buffer_size);
                if (to_string_result < 0)
                {
                    // error
                }
                else
                {
                    buffer += to_string_result;
                    buffer_size -= to_string_result;
                }
            }
        }
    }

    return result;
}

static void copy_error_string(char* buffer, size_t buffer_size)
{
    (void)buffer;
    (void)buffer_size;
}

static void log_sink_console_log(LOG_LEVEL log_level, LOG_CONTEXT_HANDLE log_context, const char* file, const char* func, int line, const char* message_format, ...)
{
    if (message_format == NULL)
    {
        /* Codes_SRS_LOG_SINK_CONSOLE_01_001: [ If `message_format` is `NULL`, `log_sink_console.log_sink_log` shall print an error and return. ]*/
        (void)printf("Invalid arguments: LOG_LEVEL log_level=%" PRI_MU_ENUM ", LOG_CONTEXT_HANDLE log_context=%p, const char* file=%s, const char* func=%s, int line=%d, const char* message_format=%s\r\n",
            MU_ENUM_VALUE(LOG_LEVEL, log_level), log_context, MU_P_OR_NULL(file), MU_P_OR_NULL(func), line, MU_P_OR_NULL(message_format));
    }
    else
    {
        char temp[LOG_MAX_MESSAGE_LENGTH];
        char* buffer = temp;
        size_t buffer_size = sizeof(temp);

        /* Codes_SRS_LOG_SINK_CONSOLE_01_002: [ `log_sink_console.log_sink_log` shall obtain the time by calling `time`. ]*/
        time_t t = time(NULL);

        /* Codes_SRS_LOG_SINK_CONSOLE_01_003: [ `log_sink_console.log_sink_log` shall convert the time to string by calling `ctime`. ]*/
        /* Codes_SRS_LOG_SINK_CONSOLE_01_004: [ `log_sink_console.log_sink_log` shall print a line in the format: `{log_level} Time: {formatted time} File:{file}:{line} Func:{func} {optional context information} {formatted message}` ]*/
        int snprintf_result = snprintf(buffer, buffer_size, "%s%s Time:%.24s File:%s:%d Func:%s ",
            /* Codes_SRS_LOG_SINK_CONSOLE_01_006: [ `log_sink_console.log_sink_log` shall color the lines using ANSI color codes (https://en.wikipedia.org/wiki/ANSI_escape_code#Colors), as follows: ]*/
            level_colors[log_level],
            MU_ENUM_TO_STRING(LOG_LEVEL, log_level),
            ctime(&t),
            MU_P_OR_NULL(file),
            line,
            MU_P_OR_NULL(func));
        if (snprintf_result < 0)
        {
            // error
        }
        else
        {
            if (snprintf_result > buffer_size)
            {
                snprintf_result = (int)buffer_size;
            }
            buffer += snprintf_result;
            buffer_size -= snprintf_result;

            if (log_context != NULL)
            {
                size_t property_value_pair_count = log_context_get_property_value_pair_count(log_context);
                const LOG_CONTEXT_PROPERTY_VALUE_PAIR* property_value_pairs = log_context_get_property_value_pairs(log_context);
                log_n_properties(buffer, buffer_size, property_value_pairs, property_value_pair_count);
            }

            if (buffer_size > 0)
            {
                va_list args;
                va_start(args, message_format);
                int vsnprintf_result = vsnprintf(buffer, buffer_size, message_format, args);
                if (vsnprintf_result < 0)
                {
                    copy_error_string(buffer, buffer_size);
                }
                va_end(args);
            }
        }

        /* Codes_SRS_LOG_SINK_CONSOLE_01_005: [ In order to not break the line in multiple parts when displayed on the console, `log_sink_console.log_sink_log` shall print the line in such a way that only one `printf` call is made. ]*/
        /* Codes_SRS_LOG_SINK_CONSOLE_01_012: [ At the end of each line that is printed, the color shall be reset by using the `\x1b[0m` code. ]*/
        (void)printf("%s%s\r\n", temp, LOG_SINK_CONSOLE_ANSI_COLOR_RESET);
    }
}

const LOG_SINK_IF log_sink_console = { .log_sink_log = log_sink_console_log };
