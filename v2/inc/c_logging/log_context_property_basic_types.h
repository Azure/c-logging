// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef LOG_CONTEXT_PROPERTY_BASIC_TYPES_H
#define LOG_CONTEXT_PROPERTY_BASIC_TYPES_H

#ifdef __cplusplus
#include <cstdint>
#include <cstddef>
#else
#include <stdint.h>
#include <stddef.h>
#endif

#include "macro_utils/macro_utils.h"

#include "c_logging/log_context_property_type_if.h"

#define SUPPORTED_BASIC_TYPES \
    int64_t, \
    uint64_t, \
    int32_t, \
    uint32_t, \
    int16_t, \
    uint16_t, \
    int8_t, \
    uint8_t

#define DECLARE_BASIC_TYPE(type_name) \
    int LOG_CONTEXT_PROPERTY_TYPE_INIT(type_name)(void* dst_value, type_name src_value); \
    int LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(type_name)(type_name src_value); \
    extern const LOG_CONTEXT_PROPERTY_TYPE_IF LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(type_name); \

MU_FOR_EACH_1(DECLARE_BASIC_TYPE, SUPPORTED_BASIC_TYPES)

#endif /* LOG_CONTEXT_PROPERTY_BASIC_TYPES_H */
