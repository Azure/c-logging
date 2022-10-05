// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef LOG_CONTEXT_PROPERTY_TYPE_ASCII_CHAR_PTR_H
#define LOG_CONTEXT_PROPERTY_TYPE_ASCII_CHAR_PTR_H

#include "c_logging/log_context_property_type_if.h"

#ifdef __cplusplus
extern "C" {
#endif

int LOG_CONTEXT_PROPERTY_TYPE_INIT(ascii_char_ptr)(void* dst_value, const char* format, ...);
int LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(ascii_char_ptr)(const char* format, ...);

extern const LOG_CONTEXT_PROPERTY_TYPE_IF LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(ascii_char_ptr);

#ifdef __cplusplus
}
#endif

#endif /* LOG_CONTEXT_PROPERTY_TYPE_ASCII_CHAR_PTR_H */
