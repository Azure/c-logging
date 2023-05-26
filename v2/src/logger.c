// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdint.h>

#include "macro_utils/macro_utils.h"

#include "c_logging/log_level.h"
#include "c_logging/log_context.h"
#include "c_logging/log_sink_if.h"

#include "c_logging/logger.h"

MU_DEFINE_ENUM_STRINGS_WITHOUT_INVALID(LOG_LEVEL, LOG_LEVEL_VALUES);

int logger_init(void)
{
    int result;
    uint32_t i;

    for (i = 0; i < log_sink_count; i++)
    {
        if (log_sinks[i]->init() != 0)
        {
            break;
        }
    }

    if (i < log_sink_count)
    {
        for (uint32_t j = 0; j < i; j++)
        {
            log_sinks[i]->deinit();
        }

        result = MU_FAILURE;
    }
    else
    {
        result = 0;
    }

    return result;
}

void logger_deinit(void)
{
    for (uint32_t i = 0; i < log_sink_count; i++)
    {
        log_sinks[i]->deinit();
    }
}

void logger_log(LOG_LEVEL log_level, LOG_CONTEXT_HANDLE log_context, const char* file, const char* func, int line_no, const char* format, ...)
{
    (void)log_level;
    (void)log_context;
    (void)file;
    (void)func;
    (void)line_no;
    (void)format;
}
