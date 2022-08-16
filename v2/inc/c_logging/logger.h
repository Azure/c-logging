// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef LOGGER_H
#define LOGGER_H

#include "macro_utils/macro_utils.h"

#include "c_logging/log_context.h"

#define LOG_LEVEL_VALUES \
    LOG_LEVEL_CRITICAL, \
    LOG_LEVEL_ERROR, \
    LOG_LEVEL_WARNING, \
    LOG_LEVEL_INFO, \
    LOG_LEVEL_VERBOSE

MU_DEFINE_ENUM_WITHOUT_INVALID(LOG_LEVEL, LOG_LEVEL_VALUES);

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
