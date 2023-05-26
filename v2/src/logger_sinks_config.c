// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdint.h>

#include "c_logging/log_sink_if.h"
#include "c_logging/log_sink_console.h"
#include "c_logging/log_sink_etw.h"

const LOG_SINK_IF* log_sinks[] = 
{
    &log_sink_console,
    &log_sink_etw
};

const uint32_t log_sink_count = MU_COUNT_ARRAY_ITEMS(log_sinks);
