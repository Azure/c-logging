// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <inttypes.h>
#include <stdio.h>

#include "macro_utils/macro_utils.h"

#include "c_logging/log_context_property_type.h"
#include "c_logging/log_context_property_type_if.h"

#include "c_logging/log_context_property_basic_types.h"

#define DEFINE_BASIC_TYPE_TO_STRING(type_name, print_format_string) \
    static int MU_C2(type_name,_log_context_property_to_string)(void* property_value, char* buffer, size_t buffer_size) \
    { \
        int result; \
        /* Codes_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_001: [ If property_value is NULL, LOG_CONTEXT_PROPERTY_TYPE_IMPL(int64_t).to_string shall fail and return -1. ]*/ \
        if (property_value == NULL) \
        { \
            (void)printf("Invalid arguments: void* property_value=%p, char* buffer=%p, size_t buffer_size=%zu\r\n", \
                property_value, buffer, buffer_size); \
            result = -1; \
        } \
        else \
        { \
            /* Codes_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_002: [ LOG_CONTEXT_PROPERTY_TYPE_IMPL(int64_t).to_string shall call snprintf with buffer, buffer_length and format string PRId64 and pass in the values list the int64_t value pointed to be property_value. ]*/ \
            /* Codes_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_003: [ LOG_CONTEXT_PROPERTY_TYPE_IMPL(int64_t).to_string shall succeed and return the result of snprintf. ]*/ \
            /* Codes_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_004: [ If any error is encountered, LOG_CONTEXT_PROPERTY_TYPE_IMPL(int64_t).to_string shall fail and return -1. ]*/ \
            result = snprintf(buffer, buffer_size, "%" print_format_string "", *(type_name*)property_value); \
        } \
        return result; \
    } \

#define DEFINE_BASIC_TYPE_COPY(type_name) \
    static int MU_C2(type_name,_log_context_property_copy)(void* dst_value, void* src_value) \
    { \
        int result; \
        if ( \
            /* Codes_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_005: [ If src_value is NULL, LOG_CONTEXT_PROPERTY_TYPE_IMPL(int64_t).copy shall fail and return a non-zero value. ]*/ \
            (dst_value == NULL) || \
            /* Codes_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_006: [ If dst_value is NULL, LOG_CONTEXT_PROPERTY_TYPE_IMPL(int64_t).copy shall fail and return a non-zero value. ]*/ \
            (src_value == NULL) \
            ) \
        { \
            (void)printf("Invalid arguments: void* dst_value = %p, void* src_value = %p", \
                dst_value, src_value); \
            result = MU_FAILURE; \
        } \
        else \
        { \
            /* Codes_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_007: [ LOG_CONTEXT_PROPERTY_TYPE_IMPL(int64_t).copy shall copy the bytes of the int64_t value from the address pointed by src_value to dst_value. ]*/ \
            *(type_name*)dst_value = *(type_name*)src_value; \
            /* Codes_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_008: [ LOG_CONTEXT_PROPERTY_TYPE_IMPL(int64_t).copy shall succeed and return 0. ]*/ \
            result = 0; \
        } \
        return result; \
    } \

#define DEFINE_BASIC_TYPE_FREE(type_name) \
    static void MU_C2(type_name,_log_context_property_free)(void* value) \
    { \
        /* Codes_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_009: [ LOG_CONTEXT_PROPERTY_TYPE_IMPL(int64_t).free shall return. ]*/ \
        (void)value; \
    } \

#define DEFINE_BASIC_TYPE_GET_TYPE(type_name) \
    static LOG_CONTEXT_PROPERTY_TYPE MU_C2(type_name,_log_context_property_get_type)(void) \
    { \
        /* Codes_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_010: [ LOG_CONTEXT_PROPERTY_TYPE_IMPL(int64_t).get_type shall returns the property type LOG_CONTEXT_PROPERTY_TYPE_int64_t. ]*/ \
        return MU_C2(LOG_CONTEXT_PROPERTY_TYPE_, type_name); \
    } \

#define DEFINE_BASIC_TYPE_IF(type_name) \
    const LOG_CONTEXT_PROPERTY_TYPE_IF MU_C2(type_name, _log_context_property_type) = \
    { \
        .to_string = MU_C2(type_name, _log_context_property_to_string), \
        .copy = MU_C2(type_name, _log_context_property_copy), \
        .free = MU_C2(type_name, _log_context_property_free), \
        .get_type = MU_C2(type_name, _log_context_property_get_type) \
    }; \

#define DEFINE_BASIC_TYPE(type_name, print_format_string) \
    DEFINE_BASIC_TYPE_TO_STRING(type_name, print_format_string) \
    DEFINE_BASIC_TYPE_COPY(type_name) \
    DEFINE_BASIC_TYPE_FREE(type_name) \
    DEFINE_BASIC_TYPE_GET_TYPE(type_name) \

DEFINE_BASIC_TYPE(int64_t, PRId64)
DEFINE_BASIC_TYPE(uint64_t, PRIu64)
DEFINE_BASIC_TYPE(int32_t, PRId32)
DEFINE_BASIC_TYPE(uint32_t, PRIu32)
DEFINE_BASIC_TYPE(int16_t, PRId16)
DEFINE_BASIC_TYPE(uint16_t, PRIu16)
DEFINE_BASIC_TYPE(int8_t, PRId8)
DEFINE_BASIC_TYPE(uint8_t, PRIu8)

MU_FOR_EACH_1(DEFINE_BASIC_TYPE_IF, SUPPORTED_BASIC_TYPES)
