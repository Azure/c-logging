// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef LOG_CONTEXT_H
#define LOG_CONTEXT_H

#ifdef __cplusplus
#include <cinttypes>
#include <cstdio>
#include <cstdlib>
#else
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#endif

#include "macro_utils/macro_utils.h"

#include "c_logging/log_context_property_value_pair.h"
#include "c_logging/log_context_property_basic_types.h"
#include "c_logging/log_context_property_type_ascii_char_ptr.h"
#include "c_logging/log_context_property_type_struct.h"
#include "c_logging/log_internal_error.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LOG_MAX_STACK_DATA_SIZE                 1024
#define LOG_MAX_STACK_PROPERTY_VALUE_PAIR_COUNT 64

typedef struct LOG_CONTEXT_TAG
{
    uint8_t* values_data;
    uint32_t values_data_length;
    LOG_CONTEXT_PROPERTY_VALUE_PAIR* property_value_pairs_ptr;
    uint32_t property_value_pair_count;
    LOG_CONTEXT_PROPERTY_VALUE_PAIR property_value_pairs[];
} LOG_CONTEXT;

typedef struct LOG_CONTEXT_TAG* LOG_CONTEXT_HANDLE;

uint32_t log_context_get_property_value_pair_count(LOG_CONTEXT_HANDLE log_context);
const LOG_CONTEXT_PROPERTY_VALUE_PAIR* log_context_get_property_value_pairs(LOG_CONTEXT_HANDLE log_context);

// These is an internal API and it is not meant to be called by the users of this module
int internal_log_context_init_from_parent(LOG_CONTEXT_HANDLE dest_log_context, LOG_CONTEXT_HANDLE parent_log_context);
uint32_t internal_log_context_get_values_data_length_or_zero(LOG_CONTEXT_HANDLE log_context);

static uint32_t internal_log_context_get_property_value_pair_count_or_zero(LOG_CONTEXT_HANDLE log_context)
{
    uint32_t result = log_context_get_property_value_pair_count(log_context);
    if (result == UINT32_MAX)
    {
        result = 0;
    }
    return result;
}

// macro set used to define a parameter in a function signature in order
// to make sure that no properties with the same name are added in one context

// DEFINE_PROPERTY_AS_PARAMETER

#define EXPAND_DEFINE_PROPERTY_AS_PARAMETER_LOG_CONTEXT_STRING_PROPERTY(property_name, ...) \
    , int property_name

#define EXPAND_DEFINE_PROPERTY_AS_PARAMETER_LOG_CONTEXT_NAME(log_context_name) \

#define EXPAND_DEFINE_PROPERTY_AS_PARAMETER_LOG_CONTEXT_PROPERTY(property_type, property_name, field_value) \
    , int property_name

#define DEFINE_PROPERTY_AS_PARAMETER(field_desc) \
    MU_C2(EXPAND_DEFINE_PROPERTY_AS_PARAMETER_, field_desc)

// macro set used to define a parameter in a function signature  every time LOG_CONTEXT_NAME is used.
// if used twice a field with the same name is defined => compiler error

// DEFINE_LOG_CONTEXT_NAME_AS_FIELD

#define EXPAND_DEFINE_CONTEXT_NAME_AS_PARAMETER_LOG_CONTEXT_STRING_PROPERTY(property_name, ...) \

#define EXPAND_DEFINE_CONTEXT_NAME_AS_PARAMETER_LOG_CONTEXT_NAME(log_context_name) \
    , int log_context_is_used_multiple_times

#define EXPAND_DEFINE_CONTEXT_NAME_AS_PARAMETER_LOG_CONTEXT_PROPERTY(property_type, property_name, field_value) \

#define DEFINE_CONTEXT_NAME_AS_PARAMETER(field_desc) \
    MU_C2(EXPAND_DEFINE_CONTEXT_NAME_AS_PARAMETER_, field_desc)

#define LOG_CONTEXT_CHECK_VARIABLE_ARGS(...) \
    /* Codes_SRS_LOG_CONTEXT_01_019: [ If 2 properties have the same name for a context a compiler error shall be emitted. ]*/ \
    (void)(void (*)(int MU_FOR_EACH_1(DEFINE_PROPERTY_AS_PARAMETER, __VA_ARGS__)))0x4242; \
    /* Codes_SRS_LOG_CONTEXT_01_011: [ If LOG_CONTEXT_NAME is specified multiple times a compiler error shall be emitted. ]*/ \
    (void)(void (*)(int MU_FOR_EACH_1(DEFINE_CONTEXT_NAME_AS_PARAMETER, __VA_ARGS__)))0x4242; \

// SETUP_PROPERTY_PAIR

#define EXPAND_SETUP_PROPERTY_PAIR_LOG_CONTEXT_STRING_PROPERTY(property_name, ...) \
    /* Codes_SRS_LOG_CONTEXT_01_007: [ LOG_CONTEXT_STRING_PROPERTY shall expand to code allocating a property/value pair of type ascii_char_ptr and the name property_name. ]*/ \
    property_value_pair->value = data_pos; \
    property_value_pair->name = MU_TOSTRING(property_name); \
    property_value_pair->type = &ascii_char_ptr##_log_context_property_type; \
    /* Codes_SRS_LOG_CONTEXT_01_008: [ LOG_CONTEXT_STRING_PROPERTY shall expand to code that stores as value a string that is constructed using printf-like formatting based on format and all the arguments in .... ]*/ \
    data_pos += sprintf(property_value_pair->value, __VA_ARGS__) + 1; \
    property_value_pair++; \

#define EXPAND_SETUP_PROPERTY_PAIR_LOG_CONTEXT_NAME(log_context_name) \
    /* Codes_SRS_LOG_CONTEXT_01_012: [ The name of the struct property shall be the context name specified by using LOG_CONTEXT_NAME (if specified). ]*/ \
    /* Codes_SRS_LOG_CONTEXT_01_017: [ The name of the struct property shall be the context name specified by using LOG_CONTEXT_NAME (if specified). ]*/ \
    first_property_value_pair->name = MU_TOSTRING(log_context_name); \

#define EXPAND_SETUP_PROPERTY_PAIR_LOG_CONTEXT_PROPERTY(property_type, property_name, field_value) \
    /* Codes_SRS_LOG_CONTEXT_01_004: [ LOG_CONTEXT_PROPERTY shall expand to code allocating a property/value pair entry with the type property_type and the name property_name. ]*/ \
    property_value_pair->value = data_pos; \
    property_value_pair->name = MU_TOSTRING(property_name); \
    property_value_pair->type = &property_type##_log_context_property_type; \
    /* Codes_SRS_LOG_CONTEXT_01_005: [ LOG_CONTEXT_PROPERTY shall expand to code copying the value property_value to be the value of the property/value pair. ]*/ \
    (void)property_type##_log_context_property_type_init((void*)data_pos, field_value); \
    data_pos += sizeof(property_type); \
    property_value_pair++; \

#define SETUP_PROPERTY_PAIR(field_desc) \
    MU_C2(EXPAND_SETUP_PROPERTY_PAIR_, field_desc)

// COUNT_PROPERTY

#define EXPAND_COUNT_PROPERTY_LOG_CONTEXT_STRING_PROPERTY(property_name, ...) \
    + 1

#define EXPAND_COUNT_PROPERTY_LOG_CONTEXT_NAME(log_context_name) \

#define EXPAND_COUNT_PROPERTY_LOG_CONTEXT_PROPERTY(property_type, property_name, field_value) \
    + 1

#define COUNT_PROPERTY(field_desc) \
    MU_C2(EXPAND_COUNT_PROPERTY_, field_desc)

// COUNT_DATA_BYTES

#define EXPAND_COUNT_DATA_BYTES_LOG_CONTEXT_STRING_PROPERTY(property_name, ...) \
    + snprintf(NULL, 0, __VA_ARGS__) + 1

#define EXPAND_COUNT_DATA_BYTES_LOG_CONTEXT_NAME(log_context_name) \

#define EXPAND_COUNT_DATA_BYTES_LOG_CONTEXT_PROPERTY(property_type, property_name, field_value) \
    + sizeof(property_type)

#define COUNT_DATA_BYTES(field_desc) \
    MU_C2(EXPAND_COUNT_DATA_BYTES_, field_desc)

// Macro that can be used to create a context on the stack
// We allocate on the stack enough space for a max payload for the context.
#define LOG_CONTEXT_LOCAL_DEFINE(destination_context, parent_context, ...) \
    uint8_t MU_C2(values_log_data_, destination_context)[LOG_MAX_STACK_DATA_SIZE]; \
    LOG_CONTEXT destination_context; \
    MU_IF(MU_COUNT_ARG(__VA_ARGS__), LOG_CONTEXT_CHECK_VARIABLE_ARGS(__VA_ARGS__),) \
    LOG_CONTEXT_PROPERTY_VALUE_PAIR MU_C2(property_values_pair_, destination_context)[LOG_MAX_STACK_PROPERTY_VALUE_PAIR_COUNT]; \
    { \
        destination_context.values_data = MU_C2(values_log_data_, destination_context); \
        destination_context.values_data_length = internal_log_context_get_values_data_length_or_zero(parent_context) + 1 MU_IF(MU_COUNT_ARG(__VA_ARGS__), MU_FOR_EACH_1(COUNT_DATA_BYTES, __VA_ARGS__),); \
        destination_context.property_value_pairs_ptr = MU_C2(property_values_pair_, destination_context); \
        destination_context.property_value_pair_count = internal_log_context_get_property_value_pair_count_or_zero(parent_context) + 1 MU_IF(MU_COUNT_ARG(__VA_ARGS__), MU_FOR_EACH_1(COUNT_PROPERTY, __VA_ARGS__),); \
        /* Codes_SRS_LOG_CONTEXT_01_024: [ If the number of properties to be stored in the log context exceeds LOG_MAX_STACK_PROPERTY_VALUE_PAIR_COUNT, an error shall be reported by calling log_internal_error_report and no properties shall be stored in the context. ]*/ \
        if (destination_context.property_value_pair_count > LOG_MAX_STACK_PROPERTY_VALUE_PAIR_COUNT) \
        { \
            (void)printf("Too many properties: property_value_pair_count = %" PRIu32 "\r\n", destination_context.property_value_pair_count); \
            destination_context.property_value_pair_count = 0; \
            destination_context.values_data_length = 0; \
            log_internal_error_report(); \
        } \
        /* Codes_SRS_LOG_CONTEXT_01_025: [ If the memory size needed for all properties to be stored in the context exceeds LOG_MAX_STACK_DATA_SIZE, an error shall be reported by calling log_internal_error_report and no properties shall be stored in the context. ]*/ \
        else if (destination_context.values_data_length > LOG_MAX_STACK_DATA_SIZE) \
        { \
            (void)printf("Data length too big: values_data_length = %" PRIu32 "\r\n", destination_context.values_data_length); \
            destination_context.property_value_pair_count = 0; \
            destination_context.values_data_length = 0; \
            log_internal_error_report(); \
        } \
        else \
        { \
            /* Codes_SRS_LOG_CONTEXT_01_018: [ If parent_context is non-NULL, the created context shall include all the property/value pairs of parent_context. ]*/ \
            internal_log_context_init_from_parent(&destination_context, parent_context); \
            LOG_CONTEXT_PROPERTY_VALUE_PAIR* property_value_pair = destination_context.property_value_pairs_ptr + internal_log_context_get_property_value_pair_count_or_zero(parent_context) + 1; \
            LOG_CONTEXT_PROPERTY_VALUE_PAIR* first_property_value_pair = destination_context.property_value_pairs_ptr; \
            (void)property_value_pair; \
            uint8_t* data_pos = destination_context.values_data; \
            *data_pos = (uint8_t)(internal_log_context_get_property_value_pair_count_or_zero(parent_context) MU_IF(MU_COUNT_ARG(__VA_ARGS__), MU_FOR_EACH_1(COUNT_PROPERTY, __VA_ARGS__),)); \
            /* Codes_SRS_LOG_CONTEXT_01_015: [ LOG_CONTEXT_LOCAL_DEFINE shall store one property/value pair that with a property type of struct with as many fields as the total number of properties passed to LOG_CONTEXT_LOCAL_DEFINE in the ... arguments. ]*/ \
            first_property_value_pair->value = data_pos; \
            first_property_value_pair->name = ""; \
            first_property_value_pair->type = &struct_log_context_property_type; \
            data_pos += 1 + internal_log_context_get_values_data_length_or_zero(parent_context); \
            /* Codes_SRS_LOG_CONTEXT_01_016: [ LOG_CONTEXT_LOCAL_DEFINE shall store the property types and values specified by using LOG_CONTEXT_PROPERTY in the context. ]*/ \
            MU_IF(MU_COUNT_ARG(__VA_ARGS__), MU_FOR_EACH_1(SETUP_PROPERTY_PAIR, __VA_ARGS__),) \
        } \
    } \

LOG_CONTEXT_HANDLE log_context_create(LOG_CONTEXT_HANDLE parent_context, uint32_t properties_count, uint32_t data_size);
void log_context_destroy(LOG_CONTEXT_HANDLE log_context);

// macro that can be used to create a dynamically allocated context
#define LOG_CONTEXT_CREATE(destination_context, parent_context, ...) \
    { \
        destination_context = log_context_create(parent_context, internal_log_context_get_property_value_pair_count_or_zero(parent_context) + 1 MU_IF(MU_COUNT_ARG(__VA_ARGS__), MU_FOR_EACH_1(COUNT_PROPERTY, __VA_ARGS__),), internal_log_context_get_values_data_length_or_zero(parent_context) + 1 MU_IF(MU_COUNT_ARG(__VA_ARGS__), MU_FOR_EACH_1(COUNT_DATA_BYTES, __VA_ARGS__),)); \
        MU_IF(MU_COUNT_ARG(__VA_ARGS__), LOG_CONTEXT_CHECK_VARIABLE_ARGS(__VA_ARGS__),) \
        if (destination_context != NULL) \
        { \
            /* Codes_SRS_LOG_CONTEXT_01_003: [ LOG_CONTEXT_CREATE shall store the property types and values specified by using LOG_CONTEXT_PROPERTY in the context. ]*/ \
            LOG_CONTEXT_PROPERTY_VALUE_PAIR* property_value_pair = destination_context->property_value_pairs_ptr + internal_log_context_get_property_value_pair_count_or_zero(parent_context) + 1; \
            LOG_CONTEXT_PROPERTY_VALUE_PAIR* first_property_value_pair = destination_context->property_value_pairs_ptr; \
            (void)property_value_pair; \
            uint8_t* data_pos = destination_context->values_data; \
            *data_pos = (uint8_t)(internal_log_context_get_property_value_pair_count_or_zero(parent_context) MU_IF(MU_COUNT_ARG(__VA_ARGS__), MU_FOR_EACH_1(COUNT_PROPERTY, __VA_ARGS__),)); \
            /* Codes_SRS_LOG_CONTEXT_01_013: [ LOG_CONTEXT_CREATE shall store one property/value pair that with a property type of struct with as many fields as the total number of properties passed to LOG_CONTEXT_CREATE. ]*/ \
            first_property_value_pair->value = data_pos; \
            /* Codes_SRS_LOG_CONTEXT_01_009: [ LOG_CONTEXT_NAME shall be optional. ]*/ \
            /* Codes_SRS_LOG_CONTEXT_01_010: [** If LOG_CONTEXT_NAME is not used the name for the context shall be empty string. ]*/ \
            first_property_value_pair->name = ""; \
            first_property_value_pair->type = &struct_log_context_property_type; \
            data_pos += 1 + internal_log_context_get_values_data_length_or_zero(parent_context); \
            MU_IF(MU_COUNT_ARG(__VA_ARGS__), MU_FOR_EACH_1(SETUP_PROPERTY_PAIR, __VA_ARGS__),) \
        } \
    } \

// destroy a dynamically allocated context
#define LOG_CONTEXT_DESTROY(log_context_handle) \
    log_context_destroy(log_context_handle)

// when a user wants to specify the log context name to be printed or published, the user can use this macro
// otherwise, the default is to use empty string ("") as context name
// LOG_CONTEXT_NAME(log_context_name)

// when a user wants to specify properties for a context, they can use LOG_CONTEXT_PROPERTY
// LOG_CONTEXT_PROPERTY(property_type, property_name, property_value)

// when a user wants to specify string fields in a context using printf-style formatting, they can use LOG_CONTEXT_STRING_PROPERTY
// LOG_CONTEXT_STRING_PROPERTY(property_name, property_conversion_specifier, property_value)

#ifdef __cplusplus
}
#endif

#endif /* LOG_CONTEXT_H */
