// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdint.h>

#include "macro_utils/macro_utils.h" // IWYU pragma: keep

#include "c_logging/log_sink_if.h"

LOG_SINK_IF** log_sinks;
uint32_t log_sink_count = 0;
