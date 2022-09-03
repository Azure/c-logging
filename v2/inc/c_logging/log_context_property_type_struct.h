// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef LOG_CONTEXT_PROPERTY_TYPE_STRUCT_H
#define LOG_CONTEXT_PROPERTY_TYPE_STRUCT_H

#ifdef __cplusplus
#include <cstdint>
#else
#include <stdint.h>
#endif

#include "c_logging/log_context_property_type_if.h"

#ifdef __cplusplus
extern "C" {
#endif

int LOG_CONTEXT_PROPERTY_TYPE_INIT(struct)(void* dst_value, uint8_t src_value);
int LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(struct)(uint8_t src_value);

extern const LOG_CONTEXT_PROPERTY_TYPE_IF LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(struct);

#ifdef __cplusplus
}
#endif

#endif /* LOG_CONTEXT_PROPERTY_TYPE_STRUCT_H */
