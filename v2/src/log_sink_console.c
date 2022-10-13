// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "c_logging/log_level.h"
#include "c_logging/log_context.h"
#include "c_logging/log_sink_if.h"

static void log_sink_console_log(LOG_LEVEL log_level, LOG_CONTEXT_HANDLE log_context, const char* file, const char* func, int line, const char* message_format, ...)
{
    (void)log_level;
    (void)log_context;
    (void)file;
    (void)func;
    (void)line;
    (void)message_format;
}

const LOG_SINK_IF log_sink_console = { .log_sink_log = log_sink_console_log };
