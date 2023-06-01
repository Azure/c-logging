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
    logger_log(log_level, log_context, __FILE__, __FUNCTION__, __LINE__, format MU_IFCOMMALOGIC(MU_COUNT_ARG(__VA_ARGS__)) __VA_ARGS__)

#define LOGGER_LOG_EX(log_level, ...) \
    MU_IF(MU_COUNT_ARG(__VA_ARGS__), \
    do { \
        LOG_CONTEXT_LOCAL_DEFINE(local_context_3DFCB6F0_39A4_4C45_881B_A3BDA8B18CC1, NULL, __VA_ARGS__); \
        /* Codes_SRS_LOGGER_01_008: [ LOGGER_LOG_EX shall call the log function of every sink that is configured to be used. ]*/ \
        logger_log(log_level, &local_context_3DFCB6F0_39A4_4C45_881B_A3BDA8B18CC1, __FILE__, __FUNCTION__, __LINE__, ""); \
    } while (0); \
    , \
    /* Codes_SRS_LOGGER_01_009: [ If no properties are specified in ..., LOGGER_LOG_EX shall call log with log_context being NULL. ] */ \
    logger_log(log_level, NULL, __FILE__, __FUNCTION__, __LINE__, ""); \
    )

#ifdef __cplusplus
}
#endif

#endif /* LOGGER_H */
