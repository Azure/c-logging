// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdint.h>

#include "macro_utils/macro_utils.h" // IWYU pragma: keep

#include "c_logging/log_sink_if.h"

#ifdef USE_LOG_SINK_CONSOLE
#include "c_logging/log_sink_console.h"
#endif // USE_LOG_SINK_CONSOLE

#ifdef USE_LOG_SINK_CALLBACK
#include "c_logging/log_sink_callback.h"
#endif // USE_LOG_SINK_CALLBACK

#ifdef USE_LOG_SINK_ETW
#include "c_logging/log_sink_etw.h"
#endif // USE_LOG_SINK_ETW

#if defined(USE_LOG_SINK_CONSOLE) || defined(USE_LOG_SINK_CALLBACK) || defined(USE_LOG_SINK_ETW)
const LOG_SINK_IF* all_sinks[] = {
#ifdef USE_LOG_SINK_CONSOLE
    &log_sink_console,
#endif // USE_LOG_SINK_CONSOLE
#ifdef USE_LOG_SINK_CALLBACK
    &log_sink_callback,
#endif // USE_LOG_SINK_CALLBACK
#ifdef USE_LOG_SINK_ETW
    &log_sink_etw
#endif // USE_LOG_SINK_ETW
};

const LOG_SINK_IF** log_sinks = all_sinks;
uint32_t log_sink_count = MU_COUNT_ARRAY_ITEMS(all_sinks);

#else // USE_LOG_SINK_CONSOLE || USE_LOG_SINK_ETW

const LOG_SINK_IF** log_sinks = NULL;
uint32_t log_sink_count = 0;

#endif // USE_LOG_SINK_CONSOLE || USE_LOG_SINK_ETW

