// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef LOG_CONTEXT_PROPERTY_TYPE_WCHAR_T_PTR_H
#define LOG_CONTEXT_PROPERTY_TYPE_WCHAR_T_PTR_H

#include <wchar.h>
#include "c_logging/log_context_property_type_if.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LOG_MAX_WCHAR_STRING_LENGTH 4096

    int LOG_CONTEXT_PROPERTY_TYPE_INIT(wchar_t_ptr)(void* dst_value, size_t count, const wchar_t* format, ...);
    int LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(wchar_t_ptr)(const wchar_t* format, ...);

    extern const LOG_CONTEXT_PROPERTY_TYPE_IF LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(wchar_t_ptr);

#ifdef __cplusplus
}
#endif

#endif /* LOG_CONTEXT_PROPERTY_TYPE_WCHAR_T_PTR_H */
