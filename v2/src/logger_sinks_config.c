// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdint.h>

#include "c_logging/log_sink_if.h"
#include "c_logging/log_sink_console.h"
#include "c_logging/log_sink_etw.h"

#if USE_LOG_SINK_CONSOLE || USE_LOG_SINK_ETW
const LOG_SINK_IF* log_sinks[] = {
#ifdef USE_LOG_SINK_CONSOLE
    & log_sink_console,
#endif // USE_LOG_SINK_CONSOLE
#ifdef USE_LOG_SINK_ETW
    & log_sink_etw
#endif // USE_LOG_SINK_ETW
};
const uint32_t log_sink_count = MU_COUNT_ARRAY_ITEMS(log_sinks);

#else // USE_LOG_SINK_CONSOLE || USE_LOG_SINK_ETW

const LOG_SINK_IF* log_sinks[];
const uint32_t log_sink_count = 0;

#endif // USE_LOG_SINK_CONSOLE || USE_LOG_SINK_ETW

