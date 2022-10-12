// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>

#include "c_logging/logger.h"
#include "c_logging/log_sink.h"
#include "c_logging/log_sink_console.h"
#include "c_logging/log_sink_etw.h"
#include "c_logging/log_context.h"

MU_DEFINE_ENUM_STRINGS_WITHOUT_INVALID(LOG_LEVEL, LOG_LEVEL_VALUES);

const LOG_SINK* log_sinks[] = { &console_log_sink, &etw_log_sink, NULL };
const char logging_error_message[] = "Error generating log message";

void logger_log(LOG_LEVEL log_level, LOG_CONTEXT_HANDLE log_context, const char* file, const char* func, int line_no, const char* format, ...)
{
    uint32_t i;
    va_list args;
    va_start(args, format);

    char message[LOG_MAX_MESSAGE_LENGTH];

    int result = vsnprintf(message, LOG_MAX_MESSAGE_LENGTH, format, args);
    if (result < 0)
    {
        (void)memcpy(message, logging_error_message, sizeof(logging_error_message));
    }
    else if (result == LOG_MAX_MESSAGE_LENGTH)
    {
        // truncate, this is the best we can do
        message[LOG_MAX_MESSAGE_LENGTH - 1] = 0;
    }
    else
    {
        // all ok
    }

    i = 0;
    while (log_sinks[i] != NULL)
    {
        log_sinks[i++]->log_sink_log(log_level, log_context, message, file, func, line_no);
    }

    va_end(args);
}
