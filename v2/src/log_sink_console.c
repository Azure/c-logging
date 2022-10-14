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

#define LOG_SINK_CONSOLE_ANSI_COLOR_BRIGHT_RED      "\x1b[31;1m"
#define LOG_SINK_CONSOLE_ANSI_COLOR_RED             "\x1b[31m"
#define LOG_SINK_CONSOLE_ANSI_COLOR_BRIGHT_YELLOW   "\x1b[33;1m"
#define LOG_SINK_CONSOLE_ANSI_COLOR_YELLOW          "\x1b[33m"
#define LOG_SINK_CONSOLE_ANSI_COLOR_WHITE           "\x1b[37m"
#define LOG_SINK_CONSOLE_ANSI_COLOR_RESET           "\x1b[0m"

static const char* level_colors[] =
{
    LOG_SINK_CONSOLE_ANSI_COLOR_BRIGHT_RED, // LOG_LEVEL_CRITICAL
    LOG_SINK_CONSOLE_ANSI_COLOR_RED, // LOG_LEVEL_ERROR
    LOG_SINK_CONSOLE_ANSI_COLOR_BRIGHT_YELLOW, // LOG_LEVEL_WARNING
    LOG_SINK_CONSOLE_ANSI_COLOR_YELLOW, // LOG_LEVEL_INFO
    LOG_SINK_CONSOLE_ANSI_COLOR_WHITE, // LOG_LEVEL_VERBOSE
};

static void log_n_properties(const LOG_CONTEXT_PROPERTY_VALUE_PAIR* property_value_pairs, size_t property_value_pair_count)
{
    char temp[LOG_MAX_MESSAGE_LENGTH];
    for (size_t i = 0; i < property_value_pair_count; i++)
    {
        if (property_value_pairs[i].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_struct)
        {
            (void)printf("%s%s{ ", property_value_pairs[i].name, property_value_pairs[i].name[0] == 0 ? "" : "=");
            uint8_t struct_properties_count = *(uint8_t*)(property_value_pairs[i].value);
            log_n_properties(&property_value_pairs[i + 1], struct_properties_count);
            i += struct_properties_count;
            (void)printf("} ");
        }
        else
        {
            property_value_pairs[i].type->to_string(property_value_pairs[i].value, temp, LOG_MAX_MESSAGE_LENGTH);
            (void)printf("%s=%s ", property_value_pairs[i].name, temp);
        }
    }
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
        /* Codes_SRS_LOG_SINK_CONSOLE_01_002: [ `log_sink_console.log_sink_log` shall obtain the time by calling `time`. ]*/
        time_t t = time(NULL);

        (void)printf("%s%s Time:%.24s File:%s:%d Func:%s ", level_colors[log_level], MU_ENUM_TO_STRING(LOG_LEVEL, log_level), ctime(&t), MU_P_OR_NULL(file), line, MU_P_OR_NULL(func));

        if (log_context != NULL)
        {
            size_t property_value_pair_count = log_context_get_property_value_pair_count(log_context);
            const LOG_CONTEXT_PROPERTY_VALUE_PAIR* property_value_pairs = log_context_get_property_value_pairs(log_context);
            log_n_properties(property_value_pairs, property_value_pair_count);
        }

        va_list args;
        va_start(args, message_format);
        (void)vprintf(message_format, args);
        (void)printf("%s\r\n", LOG_SINK_CONSOLE_ANSI_COLOR_RESET);
        va_end(args);
    }
}

const LOG_SINK_IF log_sink_console = { .log_sink_log = log_sink_console_log };
