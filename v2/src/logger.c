// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <inttypes.h>                 // for PRIu32
#include <stdio.h>
#include <stdarg.h>

#include "macro_utils/macro_utils.h"

#include "c_logging/log_level.h"
#include "c_logging/log_context.h"
#include "c_logging/log_sink_if.h"

#include "c_logging/logger.h"

MU_DEFINE_ENUM_STRINGS_WITHOUT_INVALID(LOG_LEVEL, LOG_LEVEL_VALUES);

#define LOGGER_STATE_VALUES \
    LOGGER_STATE_NOT_INITIALIZED, \
    LOGGER_STATE_INITIALIZED

MU_DEFINE_ENUM(LOGGER_STATE, LOGGER_STATE_VALUES)
MU_DEFINE_ENUM_STRINGS(LOGGER_STATE, LOGGER_STATE_VALUES)

static LOGGER_STATE logger_state = LOGGER_STATE_NOT_INITIALIZED;

int logger_init(void)
{
    int result;
    uint32_t i;

    if (logger_state != LOGGER_STATE_NOT_INITIALIZED)
    {
        /* Codes_SRS_LOGGER_01_002: [ If logger is already initialized, logger_init shall fail and return a non-zero value. ] */
        (void)printf("logger_init called in state %" PRI_MU_ENUM "\r\n", MU_ENUM_VALUE(LOGGER_STATE, logger_state));
        result = MU_FAILURE;
    }
    else
    {
        /* Codes_SRS_LOGGER_01_003: [ logger_init shall call the init function of every sink that is configured to be used. ] */
        for (i = 0; i < log_sink_count; i++)
        {
            if (log_sinks[i]->init() != 0)
            {
                (void)printf("init of sink at index %" PRIu32 " failed\r\n", i);
                break;
            }
        }

        if (i < log_sink_count)
        {
            /* Codes_SRS_LOGGER_01_004: [ If init fails, all sinks already initialized shall have their deinit function called and logger_init shall fail and return a non-zero value. ] */
            for (uint32_t j = 0; j < i; j++)
            {
                log_sinks[j]->deinit();
            }

            result = MU_FAILURE;
        }
        else
        {
            logger_state = LOGGER_STATE_INITIALIZED;

            /* Codes_SRS_LOGGER_01_005: [ Otherwise, logger_init shall succeed and return 0. ] */
            result = 0;
        }
    }

    return result;
}

void logger_deinit(void)
{
    if (logger_state != LOGGER_STATE_INITIALIZED)
    {
        /* Codes_SRS_LOGGER_01_006: [ If logger is not initialized, logger_deinit shall return. ] */
        (void)printf("logger_deinit called in state %" PRI_MU_ENUM "\r\n", MU_ENUM_VALUE(LOGGER_STATE, logger_state));
    }
    else
    {
        /* Codes_SRS_LOGGER_01_007: [ logger_deinit shall call the deinit function of every sink that is configured to be used. ] */
        for (uint32_t i = 0; i < log_sink_count; i++)
        {
            log_sinks[i]->deinit();
        }

        logger_state = LOGGER_STATE_NOT_INITIALIZED;
    }
}

LOGGER_CONFIG logger_get_config(void)
{
    LOGGER_CONFIG result = { .log_sinks = log_sinks, .log_sink_count = log_sink_count };

    return result;
}

void logger_set_config(LOGGER_CONFIG new_config)
{
    log_sinks = new_config.log_sinks;
    log_sink_count = new_config.log_sink_count;
}

void logger_log(LOG_LEVEL log_level, LOG_CONTEXT_HANDLE log_context, const char* file, const char* func, int line_no, const char* format, ...)
{
    va_list args;

    va_start(args, format);

    /* Codes_SRS_LOGGER_01_001: [ LOGGER_LOG shall call the log function of every sink that is configured to be used. ] */
    for (uint32_t i = 0; i < log_sink_count; i++)
    {
        va_list args_copy;

        va_copy(args_copy, args);
        log_sinks[i]->log(log_level, log_context, file, func, line_no, format, args_copy);
        va_end(args_copy);
    }

    va_end(args);
}
