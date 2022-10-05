// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "macro_utils/macro_utils.h"

#include "c_logging/log_context_property_type.h"
#include "c_logging/log_context_property_type_if.h"

#include "c_logging/log_context_property_type_ascii_char_ptr.h"

static int ascii_char_ptr_log_context_property_type_to_string(const void* property_value, char* buffer, size_t buffer_length)
{
    int result;
    if (
        /* Codes_SRS_LOG_CONTEXT_PROPERTY_TYPE_ASCII_CHAR_PTR_01_001: [ If `property_value` is `NULL`, `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(ascii_char_ptr).to_string` shall fail and return a negative value. ]*/
        (property_value == NULL) ||
        /* Codes_SRS_LOG_CONTEXT_PROPERTY_TYPE_ASCII_CHAR_PTR_01_017: [ If `buffer` is `NULL` and `buffer_length` is greater than 0, `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(ascii_char_ptr).to_string` shall fail and return a negative value. ]*/
        (buffer == NULL) && (buffer_length > 0)
        )
    {
        (void)printf("Invalid arguments: const void* property_value=%p, char* buffer=%p, size_t buffer_length=%zu\r\n",
            property_value, buffer, buffer_length);
        result = -1;
    }
    else
    {
        /* Codes_SRS_LOG_CONTEXT_PROPERTY_TYPE_ASCII_CHAR_PTR_01_002: [ If `buffer` is `NULL` and `buffer_length` is 0, `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(ascii_char_ptr).to_string` shall return the length of the string pointed to by `property_value`. ]*/
        /* Codes_SRS_LOG_CONTEXT_PROPERTY_TYPE_ASCII_CHAR_PTR_01_003: [ Otherwise, `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(ascii_char_ptr).to_string` shall copy the string pointed to by `property_value` to `buffer` by using `snprintf` with `buffer`, `buffer_length` and format string `%s` and pass in the values list the `const char*` value pointed to be `property_value`. ]*/
        /* Codes_SRS_LOG_CONTEXT_PROPERTY_TYPE_ASCII_CHAR_PTR_01_004: [ `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(ascii_char_ptr).to_string` shall succeed and return the result of `snprintf`. ]*/
        /* Codes_SRS_LOG_CONTEXT_PROPERTY_TYPE_ASCII_CHAR_PTR_01_005: [ If any error is encountered (truncation is not an error), `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(ascii_char_ptr).to_string` shall fail and return a negative value. ]*/
        result = snprintf(buffer, buffer_length, "%s", (const char*)property_value);
    }
    return result;
}

static int ascii_char_ptr_log_context_property_type_copy(void* dst_value, const void* src_value)
{
    int result;

    if (
        /* Codes_SRS_LOG_CONTEXT_PROPERTY_TYPE_ASCII_CHAR_PTR_01_007: [ If `dst_value` is `NULL`, `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(ascii_char_ptr).copy` shall fail and return a non-zero value. ]*/
        (dst_value == NULL) ||
        /* Codes_SRS_LOG_CONTEXT_PROPERTY_TYPE_ASCII_CHAR_PTR_01_006: [ If `src_value` is `NULL`, `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(ascii_char_ptr).copy` shall fail and return a non-zero value. ]*/
        (src_value == NULL)
        )
    {
        (void)printf("Invalid arguments: void* dst_value=%p, void* src_value=%p\r\n",
            dst_value, src_value);
        result = MU_FAILURE;
    }
    else
    {
        /* Codes_SRS_LOG_CONTEXT_PROPERTY_TYPE_ASCII_CHAR_PTR_01_008: [ `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(ascii_char_ptr).copy` shall copy the entire string (including the null terminator) from `src_value` to `dst_value`. ]*/
        (void)strcpy(dst_value, src_value);
        /* Codes_SRS_LOG_CONTEXT_PROPERTY_TYPE_ASCII_CHAR_PTR_01_009: [ `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(ascii_char_ptr).copy` shall succeed and return 0. ]*/
        result = 0;
    }

    return result;
}

static void ascii_char_ptr_log_context_property_type_free(void* value)
{
    /* Codes_SRS_LOG_CONTEXT_PROPERTY_TYPE_ASCII_CHAR_PTR_01_018: [ `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(ascii_char_ptr).free` shall return. ]*/
    (void)value;
}

static LOG_CONTEXT_PROPERTY_TYPE ascii_char_ptr_log_context_property_type_get_type(void)
{
    /* Codes_SRS_LOG_CONTEXT_PROPERTY_TYPE_ASCII_CHAR_PTR_01_010: [ `LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(ascii_char_ptr).get_type` shall return the property type `LOG_CONTEXT_PROPERTY_TYPE_ascii_char_ptr`. ]*/
    return LOG_CONTEXT_PROPERTY_TYPE_ascii_char_ptr;
}

int LOG_CONTEXT_PROPERTY_TYPE_INIT(ascii_char_ptr)(void* dst_value, const char* format, ...)
{
    int result;

    if (
        /* Codes_SRS_LOG_CONTEXT_PROPERTY_TYPE_ASCII_CHAR_PTR_01_011: [ If `dst_value` is `NULL`, `LOG_CONTEXT_PROPERTY_TYPE_INIT(ascii_char_ptr)` shall fail and return a non-zero value. ]*/
        (dst_value == NULL) ||
        /* Codes_SRS_LOG_CONTEXT_PROPERTY_TYPE_ASCII_CHAR_PTR_01_019: [ If `format` is `NULL`, `LOG_CONTEXT_PROPERTY_TYPE_INIT(ascii_char_ptr)` shall fail and return a non-zero value. ]*/
        (format == NULL)
       )
    {
        (void)printf("Invalid arguments: void* dst_value=%p, const char* format=%s\r\n",
            dst_value, MU_P_OR_NULL(format));
        result = MU_FAILURE;
    }
    else
    {
        va_list args;
        va_start(args, format);

        /* Codes_SRS_LOG_CONTEXT_PROPERTY_TYPE_ASCII_CHAR_PTR_01_012: [ `LOG_CONTEXT_PROPERTY_TYPE_INIT(ascii_char_ptr)` shall initialize by calling snprintf the memory at `dst_value` with the `printf` style formatted string given by `format` and the arguments in `...`. ]*/
        int vsprintf_result = vsprintf(dst_value, format, args);

        if (vsprintf_result < 0)
        {
            /* Codes_SRS_LOG_CONTEXT_PROPERTY_TYPE_ASCII_CHAR_PTR_01_014: [ If formatting the string fails, `LOG_CONTEXT_PROPERTY_TYPE_INIT(ascii_char_ptr)` shall fail and return a non-zero value. ]*/
            result = MU_FAILURE;
        }
        else
        {
            /* Codes_SRS_LOG_CONTEXT_PROPERTY_TYPE_ASCII_CHAR_PTR_01_013: [ `LOG_CONTEXT_PROPERTY_TYPE_INIT(ascii_char_ptr)` shall succeed and return 0. ]*/
            result = 0;
        }

        va_end(args);
    }

    return result;
}

int LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(ascii_char_ptr)(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    /* Codes_SRS_LOG_CONTEXT_PROPERTY_TYPE_ASCII_CHAR_PTR_01_015: [ If snprintf fails, `LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(ascii_char_ptr)` shall return a negative value. ]*/
    /* Codes_SRS_LOG_CONTEXT_PROPERTY_TYPE_ASCII_CHAR_PTR_01_016: [ Otherwise, on success, `LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(ascii_char_ptr)` shall return the amount of memory needed to store the `printf` style formatted string given by `format` and the arguments in `...`. ]*/
    int result = vsnprintf(NULL, 0, format, args);
    if (result >= 0)
    {
        result++;
    }
    va_end(args);

    return result;
}

const LOG_CONTEXT_PROPERTY_TYPE_IF LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(ascii_char_ptr) =
{
    .to_string = ascii_char_ptr_log_context_property_type_to_string,
    .copy = ascii_char_ptr_log_context_property_type_copy,
    .free = ascii_char_ptr_log_context_property_type_free,
    .get_type = ascii_char_ptr_log_context_property_type_get_type
};
