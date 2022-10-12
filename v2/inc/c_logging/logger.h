// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef LOGGER_H
#define LOGGER_H

#include "c_logging/log_context.h"
#include "c_logging/log_extensions.h"
#include "c_logging/log_level.h"

#define LOG_MAX_MESSAGE_LENGTH              4096 /*in bytes - a message is not expected to exceed this size in bytes, if it does, only LOG_MAX_MESSAGE_LENGTH characters are retained*/
#define LOG_MAX_PROPERTY_VALUE_PAIR_COUNT   64

#ifdef __cplusplus
extern "C" {
#endif

    void logger_log(LOG_LEVEL log_level, LOG_CONTEXT_HANDLE log_context, const char* file, const char* func, int line_no, const char* format, ...);

#define LOGGER_LOG(log_level, log_context, format, ...) \
    logger_log(log_level, log_context, __FILE__, __FUNCTION__, __LINE__, format MU_IFCOMMALOGIC(MU_COUNT_ARG(__VA_ARGS__)) __VA_ARGS__)

#define LOGGER_LOG_EX(log_level, ...) \
    { \
        LOG_CONTEXT_LOCAL_DEFINE(temp_log_context_9E9FAE19_0339_4637_A8E2_B1E776E5BCB8, NULL, LOG_CONTEXT_NAME(), __VA_ARGS__); \
        LOGGER_LOG(log_level, &temp_log_context_9E9FAE19_0339_4637_A8E2_B1E776E5BCB8, ""); \
    }

#ifdef __cplusplus
}
#endif

#endif /* LOGGER_H */
