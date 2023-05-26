// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdint.h>

#include "c_logging/log_sink_if.h"
#include "c_logging/log_sink_console.h"
#include "c_logging/log_sink_etw.h"

const LOG_SINK_IF* log_sinks[] = 
{
#ifdef USE_TRACELOGGING
    &log_sink_console
#endif // USE_TRACELOGGING
#ifdef CALL_CONSOLE_LOGGER
    , &log_sink_etw
}; // CALL_CONSOLE_LOGGER

const uint32_t log_sink_count = MU_COUNT_ARRAY_ITEMS(log_sinks);
