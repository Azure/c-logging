// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "macro_utils/macro_utils.h"

#include "c_logging/log_context_property_type.h"
#include "c_logging/log_context_property_type_if.h"

#include "c_logging/log_context_property_boolean_type.h"

static int bool_log_context_property_type_to_string(const void* property_value, char* buffer, size_t buffer_length)
{
    int result;
    /* Codes_SRS_LOG_CONTEXT_PROPERTY_BOOLEAN_TYPE_07_001: [ If property_value is NULL, LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).to_string shall fail and return a negative value. ]*/
    if (property_value == NULL)
    {
        (void)printf("Invalid arguments: const void* property_value=%p, char* buffer=%p, size_t buffer_length=%zu\r\n", property_value, buffer, buffer_length);
        result = -1;
    }
    else
    {
    /* Codes_SRS_LOG_CONTEXT_PROPERTY_BOOLEAN_TYPE_07_002: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).to_string shall call snprintf with buffer, buffer_length and format string PRI_BOOL and pass in the values list the bool value pointed to be property_value. ]*/
    /* Codes_SRS_LOG_CONTEXT_PROPERTY_BOOLEAN_TYPE_07_003: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).to_string shall succeed and return the result of snprintf. ]*/
    /* Codes_SRS_LOG_CONTEXT_PROPERTY_BOOLEAN_TYPE_07_004: [ If any error is encountered, LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).to_string shall fail and return a negative value. ]*/
        result = snprintf(buffer, buffer_length, "%" PRI_BOOL, MU_BOOL_VALUE (*(bool*)property_value));
    }
    return result;
}

static int bool_log_context_property_type_copy(void* dst_value, const void* src_value)
{
    int result;
        if (
            /* Codes_SRS_LOG_CONTEXT_PROPERTY_BOOLEAN_TYPE_07_006: [ If dst_value is NULL, LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).copy shall fail and return a non-zero value. ]*/
            (dst_value == NULL) ||
            /* Codes_SRS_LOG_CONTEXT_PROPERTY_BOOLEAN_TYPE_07_005: [ If src_value is NULL, LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).copy shall fail and return a non-zero value. ]*/
            (src_value == NULL)
            )
        {
            (void)printf("Invalid arguments: void* dst_value = %p, void* src_value = %p\r\n", dst_value, src_value);
            result = MU_FAILURE;
        }
        else
        {
            /* Codes_SRS_LOG_CONTEXT_PROPERTY_BOOLEAN_TYPE_07_007: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).copy shall copy the bytes of the bool value from the address pointed by src_value to dst_value. ]*/
            (void)memcpy(dst_value, src_value, sizeof(bool));
            /* Codes_SRS_LOG_CONTEXT_PROPERTY_BOOLEAN_TYPE_07_008: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).copy shall succeed and return 0. ]*/
            result = 0;
        }
        return result;
}

static void bool_log_context_property_type_free(void* value)
{
    /* Codes_SRS_LOG_CONTEXT_PROPERTY_BOOLEAN_TYPE_07_009: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).free shall return. ]*/
    (void)value;
}

static LOG_CONTEXT_PROPERTY_TYPE bool_log_context_property_type_get_type(void)
{
    /* Codes_SRS_LOG_CONTEXT_PROPERTY_BOOLEAN_TYPE_07_010: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).get_type shall return the property type LOG_CONTEXT_PROPERTY_TYPE_bool. ]*/
    return LOG_CONTEXT_PROPERTY_TYPE_bool;
}

int LOG_CONTEXT_PROPERTY_TYPE_INIT(bool)(void* dst_value, bool src_value)
{
    int result;
    /* Codes_SRS_LOG_CONTEXT_PROPERTY_BOOLEAN_TYPE_07_011: [ If dst_value is NULL, LOG_CONTEXT_PROPERTY_TYPE_INIT(bool) shall fail and return a non-zero value. ]*/
    if (dst_value == NULL)
    {
        (void)printf("Invalid arguments: void* dst_value = %p, void* src_value = %" PRI_BOOL "\r\n", dst_value, MU_BOOL_VALUE(src_value));
        result = MU_FAILURE;
    }
    else
    {
        /* Codes_SRS_LOG_CONTEXT_PROPERTY_BOOLEAN_TYPE_07_012: [ LOG_CONTEXT_PROPERTY_TYPE_INIT(bool) shall copy the src_value bytes of the bool to dst_value. ]*/
        (void)memcpy(dst_value, &src_value, sizeof(bool));
        /* Codes_SRS_LOG_CONTEXT_PROPERTY_BOOLEAN_TYPE_07_013: [ LOG_CONTEXT_PROPERTY_TYPE_INIT(bool) shall succeed and return 0. ]*/
        result = 0;
    }
    return result;
}

int LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(bool)(void)
{
    /* Codes_SRS_LOG_CONTEXT_PROPERTY_BOOLEAN_TYPE_07_014: [ LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(bool) shall return sizeof(bool). ]*/
    return (int)sizeof(bool);
}

const LOG_CONTEXT_PROPERTY_TYPE_IF LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool) =
{
    .to_string = bool_log_context_property_type_to_string,
    .copy = bool_log_context_property_type_copy,
    .free = bool_log_context_property_type_free,
    .get_type = bool_log_context_property_type_get_type
};
