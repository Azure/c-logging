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

#ifdef __cplusplus
extern "C" {
#endif

int log_hresult_fill_property(void* buffer, HRESULT hresult);

/* Codes_SRS_LOG_HRESULT_01_012: [ LOG_HRESULT shall expand to a LOG_CONTEXT_PROPERTY_CUSTOM_FUNCTION with name hresult, type ascii_char_ptr and value function call being log_hresult_fill_property(hresult). ] */
#define LOG_HRESULT(the_hresult) \
    LOG_CONTEXT_PROPERTY_CUSTOM_FUNCTION(ascii_char_ptr, hresult, log_hresult_fill_property, the_hresult)

#ifdef __cplusplus
}
#endif

#endif /* LOG_HRESULT_H */
