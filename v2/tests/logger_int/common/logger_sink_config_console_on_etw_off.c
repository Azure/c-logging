// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdint.h>

#include "macro_utils/macro_utils.h" // IWYU pragma: keep

#include "c_logging/log_sink_if.h"
#include "c_logging/log_sink_console.h"

const LOG_SINK_IF* log_sinks[] = {
    & log_sink_console
};
const uint32_t log_sink_count = MU_COUNT_ARRAY_ITEMS(log_sinks);
