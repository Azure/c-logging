// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdio.h>
#include <stddef.h>
#include <inttypes.h>

#include "macro_utils/macro_utils.h"

#include "c_logging/log_context_property_type.h"
#include "c_logging/log_context_property_type_if.h"

#include "c_logging/log_context_property_type_struct.h"

static int struct_log_context_property_type_to_string(const void* property_value, char* buffer, size_t buffer_length)
{
    int result;

    /* Codes_SRS_LOG_CONTEXT_PROPERTY_TYPE_STRUCT_01_001: [ If property_value is NULL, LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(struct).to_string shall fail and return a negative value. ]*/
    if (property_value == NULL)
    {
        (void)printf("Invalid arguments: const void* property_value=%p, char* buffer=%p, size_t buffer_length=%zu\r\n",
            property_value, buffer, buffer_length);
        result = -1;
    }
    else
    {
        if (
            /* Codes_SRS_LOG_CONTEXT_PROPERTY_TYPE_STRUCT_01_003: [ If buffer is NULL, LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(struct).to_string shall return 0 (the length of an empty string). ]*/
            (buffer == NULL) ||
            /* Codes_SRS_LOG_CONTEXT_PROPERTY_TYPE_STRUCT_01_015: [ If buffer_length is 0, LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(struct).to_string shall return 0 (the length of an empty string). ]*/
            (buffer_length == 0)
            )
        {
            // return 0
        }
        else
        {
            /* Codes_SRS_LOG_CONTEXT_PROPERTY_TYPE_STRUCT_01_004: [ Otherwise, LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(struct).to_string shall write 0 as the first element in buffer. ]*/
            buffer[0] = '\0';
        }

        result = 0;
    }

    return result;
}

static int struct_log_context_property_type_copy(void* dst_value, const void* src_value)
{
    int result;

    if (
        /* Codes_SRS_LOG_CONTEXT_PROPERTY_TYPE_STRUCT_01_005: [ If src_value is NULL, LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(struct).copy shall fail and return a non-zero value. ]*/
        (src_value == NULL) ||
        /* Codes_SRS_LOG_CONTEXT_PROPERTY_TYPE_STRUCT_01_006: [ If dst_value is NULL, LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(struct).copy shall fail and return a non-zero value. ]*/
        (dst_value == NULL)
        )
    {
        (void)printf("Invalid arguments: void* dst_value=%p, void* src_value=%p\r\n",
            dst_value, src_value);
        result = MU_FAILURE;
    }
    else
    {
        /* Codes_SRS_LOG_CONTEXT_PROPERTY_TYPE_STRUCT_01_007: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(struct).copy shall copy the number of fields associated with the structure from src_value to dst_value. ]*/
        *(uint8_t*)dst_value = *(uint8_t*)src_value;

        /* Codes_SRS_LOG_CONTEXT_PROPERTY_TYPE_STRUCT_01_008: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(struct).copy shall succeed and return 0. ]*/
        result = 0;
    }

    return result;
}

static void struct_log_context_property_type_free(void* value)
{
    /* Codes_SRS_LOG_CONTEXT_PROPERTY_TYPE_STRUCT_01_009: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(struct).free shall return. ]*/
    (void)value;
}

static LOG_CONTEXT_PROPERTY_TYPE struct_log_context_property_type_get_type(void)
{
    /* Codes_SRS_LOG_CONTEXT_PROPERTY_TYPE_STRUCT_01_010: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(struct).get_type shall return the property type LOG_CONTEXT_PROPERTY_TYPE_struct. ]*/
    return LOG_CONTEXT_PROPERTY_TYPE_struct;
}

int LOG_CONTEXT_PROPERTY_TYPE_INIT(struct)(void* dst_value, uint8_t src_value)
{
    int result;

    /* Codes_SRS_LOG_CONTEXT_PROPERTY_TYPE_STRUCT_01_011: [ If dst_value is NULL, LOG_CONTEXT_PROPERTY_TYPE_INIT(struct) shall fail and return a non-zero value. ]*/
    if (dst_value == NULL)
    {
        (void)printf("Invalid arguments: void* dst_value=%p, uint8_t src_value=%" PRIu8 "\r\n",
            dst_value, src_value);
        result = MU_FAILURE;
    }
    else
    {
        /* Codes_SRS_LOG_CONTEXT_PROPERTY_TYPE_STRUCT_01_012: [ LOG_CONTEXT_PROPERTY_TYPE_INIT(struct) shall copy the field_count byte to dst_value. ]*/
        *(uint8_t*)dst_value = src_value;

        /* Codes_SRS_LOG_CONTEXT_PROPERTY_TYPE_STRUCT_01_013: [ LOG_CONTEXT_PROPERTY_TYPE_INIT(struct) shall succeed and return 0. ]*/
        result = 0;
    }

    return result;
}

int LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(struct)(uint8_t src_value)
{
    (void)src_value;

    /* Codes_SRS_LOG_CONTEXT_PROPERTY_TYPE_STRUCT_01_014: [ LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(struct) shall return sizeof(uint8_t). ]*/
    return sizeof(uint8_t);
}

const LOG_CONTEXT_PROPERTY_TYPE_IF LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(struct) =
{
    .to_string = struct_log_context_property_type_to_string,
    .copy = struct_log_context_property_type_copy,
    .free = struct_log_context_property_type_free,
    .get_type = struct_log_context_property_type_get_type,
};
