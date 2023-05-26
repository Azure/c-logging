// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef LOGGER_H
#define LOGGER_H

#include "c_logging/log_context.h"
#include "c_logging/log_level.h"
#include "c_logging/log_sink_if.h"

#define LOG_MAX_MESSAGE_LENGTH              4096 /*in bytes - a message is not expected to exceed this size in bytes, if it does, only LOG_MAX_MESSAGE_LENGTH characters are retained*/

#ifdef __cplusplus
extern "C" {
#endif

    extern const uint32_t log_sink_count;
    extern const LOG_SINK_IF* log_sinks[];

    int logger_init(void);
    void logger_deinit(void);
    void logger_log(LOG_LEVEL log_level, LOG_CONTEXT_HANDLE log_context, const char* file, const char* func, int line_no, const char* format, ...);

#define LOGGER_LOG(log_level, log_context, format, ...) \
    /* To be implemented */

#define LOGGER_LOG_EX(log_level, ...) \
    /* To be implemented */

#ifdef __cplusplus
}
#endif

#endif /* LOGGER_H */
