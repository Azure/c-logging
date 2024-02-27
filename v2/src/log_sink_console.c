// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>

#include "macro_utils/macro_utils.h"

#include "c_logging/log_level.h"
#include "c_logging/log_context.h"
#include "c_logging/log_context_property_to_string.h"
#include "c_logging/log_context_property_value_pair.h"
#include "c_logging/log_sink_if.h"
#include "c_logging/logger.h"

#include "c_logging/log_sink_console.h" // IWYU pragma: keep

#define MIN(a, b) ((a) < (b)) ? (a) : (b)

/* Codes_SRS_LOG_SINK_CONSOLE_01_007: [ LOG_LEVEL_CRITICAL shall be displayed with bright red \x1b[31;1m. ]*/
#define LOG_SINK_CONSOLE_ANSI_COLOR_BRIGHT_RED      "\x1b[31;1m"
/* Codes_SRS_LOG_SINK_CONSOLE_01_008: [ LOG_LEVEL_ERROR shall be displayed with red \x1b[31m. ]*/
#define LOG_SINK_CONSOLE_ANSI_COLOR_RED             "\x1b[31m"
/* Codes_SRS_LOG_SINK_CONSOLE_01_009: [ LOG_LEVEL_WARNING shall be displayed with yellow \x1b[33m. ]*/
#define LOG_SINK_CONSOLE_ANSI_COLOR_YELLOW          "\x1b[33m"
/* Codes_SRS_LOG_SINK_CONSOLE_01_010: [ LOG_LEVEL_INFO shall be displayed with white \x1b[37m. ]*/
#define LOG_SINK_CONSOLE_ANSI_COLOR_WHITE           "\x1b[37m"
/* Codes_SRS_LOG_SINK_CONSOLE_01_011: [ LOG_LEVEL_VERBOSE shall be displayed with gray \x1b[90m. ]*/
#define LOG_SINK_CONSOLE_ANSI_COLOR_GRAY            "\x1b[90m"
#define LOG_SINK_CONSOLE_ANSI_COLOR_RESET           "\x1b[0m"

static const char* level_colors[] =
{
    LOG_SINK_CONSOLE_ANSI_COLOR_BRIGHT_RED,     // LOG_LEVEL_CRITICAL
    LOG_SINK_CONSOLE_ANSI_COLOR_RED,            // LOG_LEVEL_ERROR
    LOG_SINK_CONSOLE_ANSI_COLOR_YELLOW,         // LOG_LEVEL_WARNING
    LOG_SINK_CONSOLE_ANSI_COLOR_WHITE,          // LOG_LEVEL_INFO
    LOG_SINK_CONSOLE_ANSI_COLOR_GRAY,           // LOG_LEVEL_VERBOSE
};

const char error_string[] = "Error formatting log line\r\n";

static int log_sink_console_init(void)
{
    /* Codes_SRS_LOG_SINK_CONSOLE_01_027: [ log_sink_console.init shall return 0. ] */
    return 0;
}

static void log_sink_console_deinit(void)
{
    /* Codes_SRS_LOG_SINK_CONSOLE_01_028: [ log_sink_console.deinit shall return. ] */
}

static void log_sink_console_log(LOG_LEVEL log_level, LOG_CONTEXT_HANDLE log_context, const char* file, const char* func, int line, const char* message_format, va_list args)
{

    if (message_format == NULL)
    {
        /* Codes_SRS_LOG_SINK_CONSOLE_01_001: [ If message_format is NULL, log_sink_console.log shall print an error and return. ]*/
        (void)printf("Invalid arguments: LOG_LEVEL log_level=%" PRI_MU_ENUM ", LOG_CONTEXT_HANDLE log_context=%p, const char* file=%s, const char* func=%s, int line=%d, const char* message_format=%s\r\n",
            MU_ENUM_VALUE(LOG_LEVEL, log_level), log_context, MU_P_OR_NULL(file), MU_P_OR_NULL(func), line, MU_P_OR_NULL(message_format));
    }
    else
    {
        /* Codes_SRS_LOG_SINK_CONSOLE_01_021: [ log_sink_console.log shall print at most LOG_MAX_MESSAGE_LENGTH characters including the null terminator (the rest of the context shall be truncated). ]*/
        char temp[LOG_MAX_MESSAGE_LENGTH];
        char* buffer = temp;
        size_t buffer_size = sizeof(temp);

        /* Codes_SRS_LOG_SINK_CONSOLE_01_002: [ log_sink_console.log shall obtain the time by calling time. ]*/
        time_t t = time(NULL);
        /* Codes_SRS_LOG_SINK_CONSOLE_01_024: [ If the call to ctime fails then log_sink_console.log shall print the time as NULL. ]*/
        char* ctime_result = (t == (time_t)-1) ? NULL : ctime(&t);

        /* Codes_SRS_LOG_SINK_CONSOLE_01_003: [ log_sink_console.log shall convert the time to string by calling ctime. ]*/
        /* Codes_SRS_LOG_SINK_CONSOLE_01_004: [ log_sink_console.log shall print a line in the format: {log_level} Time: {formatted time} File:{file}:{line} Func:{func} {optional context information} {formatted message} ]*/
        int snprintf_result = snprintf(buffer, buffer_size, "%s%s Time:%.24s File:%s:%d Func:%s",
            /* Codes_SRS_LOG_SINK_CONSOLE_01_006: [ log_sink_console.log shall color the lines using ANSI color codes (https://en.wikipedia.org/wiki/ANSI_escape_code#Colors), as follows: ]*/
            level_colors[log_level],
            MU_ENUM_TO_STRING(LOG_LEVEL, log_level),
            /* Codes_SRS_LOG_SINK_CONSOLE_01_023: [ If the call to time fails then log_sink_console.log shall print the time as NULL. ]*/
            MU_P_OR_NULL(ctime_result),
            MU_P_OR_NULL(file),
            line,
            MU_P_OR_NULL(func));
        if (snprintf_result < 0)
        {
            /* Codes_SRS_LOG_SINK_CONSOLE_01_022: [ If any encoding error occurs during formatting of the line (i.e. if any printf class functions fails), log_sink_console.log shall print Error formatting log line and return. ]*/
            (void)printf(error_string);
        }
        else
        {
            bool error = false;

            snprintf_result = MIN(snprintf_result, (int)buffer_size);
            buffer += snprintf_result;
            buffer_size -= snprintf_result;

            /* Codes_SRS_LOG_SINK_CONSOLE_01_013: [ If log_context is non-NULL: ]*/
            if (log_context != NULL)
            {
                /* Codes_SRS_LOG_SINK_CONSOLE_01_014: [ log_sink_console.log shall call log_context_get_property_value_pair_count to obtain the count of properties to print. ]*/
                size_t property_value_pair_count = log_context_get_property_value_pair_count(log_context);
                /* Codes_SRS_LOG_SINK_CONSOLE_01_015: [ log_sink_console.log shall call log_context_get_property_value_pairs to obtain the properties to print. ]*/
                const LOG_CONTEXT_PROPERTY_VALUE_PAIR* property_value_pairs = log_context_get_property_value_pairs(log_context);

                /* Codes_SRS_LOG_SINK_CONSOLE_42_001: [ log_sink_console.log shall call log_context_property_to_string to print the properties to the string buffer. ]*/
                int log_n_properties_result = log_context_property_to_string(buffer, buffer_size, property_value_pairs, property_value_pair_count); // lgtm[cpp/unguardednullreturndereference] Tests and code review ensure that NULL access cannot happen
                if (log_n_properties_result < 0)
                {
                    /* Codes_SRS_LOG_SINK_CONSOLE_01_022: [ If any encoding error occurs during formatting of the line (i.e. if any printf class functions fails), log_sink_console.log shall print Error formatting log line and return. ]*/
                    error = true;
                }
                else
                {
                    log_n_properties_result = MIN(log_n_properties_result, (int)buffer_size);
                    buffer += log_n_properties_result;
                    buffer_size -= log_n_properties_result;
                }
            }

            if (!error)
            {
                if (buffer_size > 1)
                {
                    *buffer = ' ';
                    buffer++;
                    buffer_size--;

                    int vsnprintf_result = vsnprintf(buffer, buffer_size, message_format, args);
                    if (vsnprintf_result < 0)
                    {
                        /* Codes_SRS_LOG_SINK_CONSOLE_01_022: [ If any encoding error occurs during formatting of the line (i.e. if any printf class functions fails), log_sink_console.log shall print Error formatting log line and return. ]*/
                        error = true;
                    }
                    else
                    {
                        // all ok
                    }
                }
            }

            if (error)
            {
                (void)printf(error_string);
            }
            else
            {
                /* Codes_SRS_LOG_SINK_CONSOLE_01_005: [ In order to not break the line in multiple parts when displayed on the console, log_sink_console.log shall print the line in such a way that only one printf call is made. ]*/
                /* Codes_SRS_LOG_SINK_CONSOLE_01_012: [ At the end of each line that is printed, the color shall be reset by using the \x1b[0m code. ]*/
                (void)printf("%s%s\r\n", temp, LOG_SINK_CONSOLE_ANSI_COLOR_RESET);
            }
        }
    }
}

const LOG_SINK_IF log_sink_console =
{
    .init = log_sink_console_init,
    .deinit = log_sink_console_deinit,
    .log = log_sink_console_log
};
