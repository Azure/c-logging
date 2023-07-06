// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <wchar.h>

#include "macro_utils/macro_utils.h"

#include "c_logging/log_context_property_type.h"
#include "c_logging/log_context_property_type_if.h"

#include "c_logging/log_context_property_type_wchar_t_ptr.h"

#define LOG_MAX_WCHAR_STRING_LENGTH 512

static int wchar_t_ptr_log_context_property_type_to_string(const void* property_value, char* buffer, size_t buffer_length)
{
    int result;
    if (
        /* Codes_SRS_LOG_CONTEXT_PROPERTY_TYPE_WCHAR_T_PTR_07_001: [ If property_value is NULL, LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(wchar_t_ptr).to_string shall fail and return a negative value. ]*/
        (property_value == NULL) ||
        /* Codes_SRS_LOG_CONTEXT_PROPERTY_TYPE_WCHAR_T_PTR_07_002: [ If buffer is NULL and buffer_length is greater than 0, LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(wchar_t_ptr).to_string shall fail and return a negative value. ]*/
        (buffer == NULL) && (buffer_length > 0)
        )
    {
        (void)printf("Invalid arguments: const void* property_value=%p, char* buffer=%p, size_t buffer_length=%zu\r\n",
            property_value, buffer, buffer_length);
        result = -1;
    }
    else
    {
        /* Codes_SRS_LOG_CONTEXT_PROPERTY_TYPE_WCHAR_T_PTR_07_003: [ If buffer is NULL and buffer_length is 0, LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(wchar_t_ptr).to_string shall return the length of the wchar_t string pointed to by property_value. ]*/
        /* Codes_SRS_LOG_CONTEXT_PROPERTY_TYPE_WCHAR_T_PTR_07_004: [ Otherwise, LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(wchar_t_ptr).to_string shall copy the wchar_t string pointed to by property_value to buffer by using snprintf with buffer, buffer_length and format string %ls and pass in the values list the const wchar_t* value pointed to be property_value. ]*/
        /* Codes_SRS_LOG_CONTEXT_PROPERTY_TYPE_WCHAR_T_PTR_07_005: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(wchar_t_ptr).to_string shall succeed and return the result of snprintf. ]*/
        /* Codes_SRS_LOG_CONTEXT_PROPERTY_TYPE_WCHAR_T_PTR_07_006: [ If any error is encountered (truncation is not an error), LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(wchar_t_ptr).to_string shall fail and return a negative value. ]*/
        result = snprintf(buffer, buffer_length, "%ls", (const wchar_t*)property_value);
    }
    return result;
}

static int wchar_t_ptr_log_context_property_type_copy(void* dst_value, const void* src_value)
{
    int result;

    if (
        /* Codes_SRS_LOG_CONTEXT_PROPERTY_TYPE_WCHAR_T_PTR_07_007: [ If src_value is NULL, LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(wchar_t_ptr).copy shall fail and return a non-zero value. ]*/
        (dst_value == NULL) ||
        /* Codes_SRS_LOG_CONTEXT_PROPERTY_TYPE_WCHAR_T_PTR_07_008: [ If dst_value is NULL, LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(wchar_t_ptr).copy shall fail and return a non-zero value. ]*/
        (src_value == NULL)
        )
    {
        (void)printf("Invalid arguments: void* dst_value=%p, void* src_value=%p\r\n",
            dst_value, src_value);
        result = MU_FAILURE;
    }
    else
    {
        /* Codes_SRS_LOG_CONTEXT_PROPERTY_TYPE_WCHAR_T_PTR_07_009: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(wchar_t_ptr).copy shall copy the entire string (including the null terminator) from src_value to dst_value. ]*/
        (void)wcscpy(dst_value, src_value);
        /* Codes_SRS_LOG_CONTEXT_PROPERTY_TYPE_WCHAR_T_PTR_07_010: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(wchar_t_ptr).copy shall succeed and return 0. ]*/
        result = 0;
    }

    return result;
}

static void wchar_t_ptr_log_context_property_type_free(void* value)
{
    /* Codes_SRS_LOG_CONTEXT_PROPERTY_TYPE_WCHAR_T_PTR_07_011: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(wchar_t_ptr).free shall return. ]*/
    (void)value;
}

static LOG_CONTEXT_PROPERTY_TYPE wchar_t_ptr_log_context_property_type_get_type(void)
{
    /* Codes_SRS_LOG_CONTEXT_PROPERTY_TYPE_WCHAR_T_PTR_07_012: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(wchar_t_ptr).get_type shall return the property type LOG_CONTEXT_PROPERTY_TYPE_wchar_t_ptr. ]*/
    return LOG_CONTEXT_PROPERTY_TYPE_wchar_t_ptr;
}

int LOG_CONTEXT_PROPERTY_TYPE_INIT(wchar_t_ptr)(void* dst_value, size_t count, const wchar_t* format, ...)
{
    int result;

    if (
        /* Codes_SRS_LOG_CONTEXT_PROPERTY_TYPE_WCHAR_T_PTR_07_013: [ If dst_value is NULL, LOG_CONTEXT_PROPERTY_TYPE_INIT(wchar_t_ptr) shall fail and return a non-zero value. ]*/
        (dst_value == NULL) ||
        /* Codes_SRS_LOG_CONTEXT_PROPERTY_TYPE_WCHAR_T_PTR_07_014: [ If format is NULL, LOG_CONTEXT_PROPERTY_TYPE_INIT(wchar_t_ptr) shall fail and return a non-zero value. ]*/
        (format == NULL)
        )
    {
        (void)printf("Invalid arguments: void* dst_value=%p, const wchar_t* format=%p\r\n",
            dst_value, format);
        result = MU_FAILURE;
    }
    else
    {
        va_list args;
        va_start(args, format);

        /* Codes_SRS_LOG_CONTEXT_PROPERTY_TYPE_WCHAR_T_PTR_07_015: [ LOG_CONTEXT_PROPERTY_TYPE_INIT(wchar_t_ptr) shall initialize by calling vswprintf the memory at dst_value with the wprintf style formatted wchar_t string given by format and the arguments in .... ]*/
        int vswprintf_result = vswprintf(dst_value, count, format, args);

        if (vswprintf_result < 0)
        {
        /* Codes_SRS_LOG_CONTEXT_PROPERTY_TYPE_WCHAR_T_PTR_07_017: [ If formatting the string fails, LOG_CONTEXT_PROPERTY_TYPE_INIT(wchar_t_ptr) shall fail and return a non-zero value. ]*/
            result = MU_FAILURE;
        }
        else
        {
            /* Codes_SRS_LOG_CONTEXT_PROPERTY_TYPE_WCHAR_T_PTR_07_016: [ LOG_CONTEXT_PROPERTY_TYPE_INIT(wchar_t_ptr) shall succeed and return 0. ]*/
            result = 0;
        }

        va_end(args);
    }

    return result;
}

int LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(wchar_t_ptr)(const wchar_t* format, ...)
{
    va_list args;
    va_start(args, format);

    wchar_t* buffer = malloc(sizeof(wchar_t)* LOG_MAX_WCHAR_STRING_LENGTH);
    /* Codes_SRS_LOG_CONTEXT_PROPERTY_TYPE_WCHAR_T_PTR_07_018: [ If _vsnwprintf fails, LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(wchar_t_ptr) shall return a negative value. ]*/
    /* Codes_SRS_LOG_CONTEXT_PROPERTY_TYPE_WCHAR_T_PTR_07_019: [ Otherwise, on success, LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(wchar_t_ptr) shall return the amount of memory needed in number of wide-chracters to store the wprintf style formatted wchar_t string given by format and the arguments in .... ]*/
    int result = vswprintf(buffer, LOG_MAX_WCHAR_STRING_LENGTH, format, args)*2;
    if (result >= 0)
    {
        result+=2;
    }
    else
    {
        (void)printf("wchar_t string length exceeds LOG_MAX_WCHAR_STRING_LENGTH, wchar string has been truncated");
    }
    free(buffer);
    va_end(args);

    return result;
}

const LOG_CONTEXT_PROPERTY_TYPE_IF LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(wchar_t_ptr) =
{
    .to_string = wchar_t_ptr_log_context_property_type_to_string,
    .copy = wchar_t_ptr_log_context_property_type_copy,
    .free = wchar_t_ptr_log_context_property_type_free,
    .get_type = wchar_t_ptr_log_context_property_type_get_type
};
