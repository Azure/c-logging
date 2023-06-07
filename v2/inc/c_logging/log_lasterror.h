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

int log_lasterror_fill_property(void* buffer);

/* Codes_SRS_LOG_LASTERROR_01_001: [ LOG_LASTERROR shall expand to a LOG_CONTEXT_PROPERTY with name LastError, type int32_t and value the result of calling GetLastError. ] */
#define LOG_LASTERROR() \
    LOG_CONTEXT_PROPERTY_CUSTOM_FUNCTION(int32_t, LastError, log_lasterror_fill_property)

#endif /* LOG_LASTERROR_H */