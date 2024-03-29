// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef LOG_LASTERROR_H
#define LOG_LASTERROR_H

#ifndef __cplusplus
#include <stdint.h>
#else
#include <cstdint>
#endif

#include "windows.h"

#include "c_logging/log_context.h"

#ifdef __cplusplus
extern "C" {
#endif

int log_lasterror_fill_property(void* buffer, int dummy);

/* Codes_SRS_LOG_LASTERROR_01_001: [ LOG_LASTERROR shall expand to a LOG_CONTEXT_PROPERTY_CUSTOM_FUNCTION with name LastError, type ascii_char_ptr and value function call being log_lasterror_fill_property. ] */
#define LOG_LASTERROR() \
    LOG_CONTEXT_PROPERTY_CUSTOM_FUNCTION(ascii_char_ptr, LastError, log_lasterror_fill_property, 42)

#ifdef __cplusplus
}
#endif

#endif /* LOG_LASTERROR_H */
