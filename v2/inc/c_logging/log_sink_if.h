// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef LOG_SINK_IF_H
#define LOG_SINK_IF_H

#include "c_logging/log_context.h"
#include "c_logging/log_level.h"

typedef int (*LOG_SINK_INIT_FUNC)(void);
typedef void (*LOG_SINK_LOG_FUNC)(LOG_LEVEL log_level, LOG_CONTEXT_HANDLE log_context, const char* file, const char* func, int line, const char* message_format, va_list args);
typedef void (*LOG_SINK_DEINIT_FUNC)(void);

typedef struct LOG_SINK_IF_TAG
{
    LOG_SINK_INIT_FUNC init;
    LOG_SINK_LOG_FUNC log;
    LOG_SINK_DEINIT_FUNC deinit;
} LOG_SINK_IF;

#endif /* LOG_SINK_IF_H */
