// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef LOGGER_H
#define LOGGER_H

#include "c_logging/log_context.h"
#include "c_logging/log_level.h"

#ifdef __cplusplus
extern "C" {
#endif

    void logger_log(LOG_LEVEL log_level, LOG_CONTEXT_HANDLE log_context, const char* file, const char* func, int line_no, const char* format, ...);

#define LOGGER_LOG(log_level, log_context, format, ...) \
    /* To be implemented */

#define LOGGER_LOG_EX(log_level, ...) \
    /* To be implemented */

#ifdef __cplusplus
}
#endif

#endif /* LOGGER_H */
