// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef LOG_CONTEXT_PROPERTY_TYPE_IF_H
#define LOG_CONTEXT_PROPERTY_TYPE_IF_H

#ifdef __cplusplus
#include <cstdlib>
#else
#include <stdlib.h>
#endif

#include "c_logging/log_context_property_type.h"

typedef int (*LOG_CONTEXT_PROPERTY_TYPE_TO_STRING)(void* property_value, char* buffer, size_t buffer_length);
typedef int (*LOG_CONTEXT_PROPERTY_TYPE_COPY)(void* dst_value, void* src_value);
typedef void (*LOG_CONTEXT_PROPERTY_TYPE_FREE)(void* value);
typedef LOG_CONTEXT_PROPERTY_TYPE (*LOG_CONTEXT_PROPERTY_TYPE_GET_TYPE)(void);

typedef struct LOG_CONTEXT_PROPERTY_TYPE_IF_TAG
{
    LOG_CONTEXT_PROPERTY_TYPE_TO_STRING to_string;
    LOG_CONTEXT_PROPERTY_TYPE_COPY copy;
    LOG_CONTEXT_PROPERTY_TYPE_FREE free;
    LOG_CONTEXT_PROPERTY_TYPE_GET_TYPE get_type;
} LOG_CONTEXT_PROPERTY_TYPE_IF;

// a convenient macro for obtaining a certain type concrete implementation
// in a consistent way
/* Codes_SRS_LOG_CONTEXT_PROPERTY_TYPE_IF_01_001: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(type_name) shall produce the token {type_name}_log_context_property_type. ]*/
#define LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(type_name) \
    MU_C2(type_name, _log_context_property_type)

/* Codes_SRS_LOG_CONTEXT_PROPERTY_TYPE_IF_01_002: [ LOG_CONTEXT_PROPERTY_TYPE_INIT(type_name) shall produce the token {type_name}_log_context_property_type_init. ]*/
#define LOG_CONTEXT_PROPERTY_TYPE_INIT(type_name) \
    MU_C2(type_name, _log_context_property_type_init)

/* Codes_SRS_LOG_CONTEXT_PROPERTY_TYPE_IF_01_003: [ LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(type_name) shall produce the token {type_name}_log_context_property_type_get_init_data_size. ]*/
#define LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(type_name) \
    MU_C2(type_name, _log_context_property_type_get_init_data_size)

#endif /* LOG_CONTEXT_PROPERTY_TYPE_IF_H */
