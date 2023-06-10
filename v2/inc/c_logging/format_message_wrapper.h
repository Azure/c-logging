// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef LOG_HRESULT_H
#define LOG_HRESULT_H

#ifndef __cplusplus
#include <stdint.h>
#else
#include <cstdint>
#endif

#include "windows.h"

#include "c_logging/log_context.h"

int log_hresult_fill_property(void* buffer, HRESULT hresult);

#define LOG_HRESULT(hresult) \
    LOG_CONTEXT_PROPERTY_CUSTOM_FUNCTION(ascii_char_ptr, hresult, log_hresult_fill_property)

#endif /* LOG_HRESULT_H */
