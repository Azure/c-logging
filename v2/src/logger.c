// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "c_logging/logger.h"
#include "c_logging/log_context.h"

void logger_log(LOG_LEVEL log_level, LOG_CONTEXT_HANDLE log_context, const char* file, const char* func, int line_no, const char* format, ...)
{
    (void)log_level;
    (void)log_context;
    (void)file;
    (void)func;
    (void)line_no;
    (void)format;
}
