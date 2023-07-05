// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <wchar.h>

#include "macro_utils/macro_utils.h"

#include "c_logging/log_context_property_type.h"
#include "c_logging/log_context_property_type_if.h"

#include "c_logging/log_context_property_type_wchar_t_ptr.h"

// defines how many mock calls we can have
#define MAX_MOCK_CALL_COUNT (128)

#define MOCK_CALL_TYPE_VALUES \
    MOCK_CALL_TYPE_snprintf, \
    MOCK_CALL_TYPE_vswprintf, \
    MOCK_CALL_TYPE_vsnwprintf \

MU_DEFINE_ENUM(MOCK_CALL_TYPE, MOCK_CALL_TYPE_VALUES)

// very poor mans mocks :-(
typedef struct snprintf_CALL_TAG
{
    bool override_result;
    int call_result;
    const char* format_arg;
} snprintf_CALL;

typedef struct vswprintf_CALL_TAG
{
    bool override_result;
    int call_result;
    const wchar_t* format_arg;
} vswprintf_CALL;

typedef struct vsnwprintf_CALL_TAG
{
    bool override_result;
    int call_result;
    const wchar_t* format_arg;
} vsnwprintf_CALL;

typedef struct MOCK_CALL_TAG
{
    MOCK_CALL_TYPE mock_call_type;
    union
    {
        snprintf_CALL snprintf_call;
        vswprintf_CALL vswprintf_call;
        vsnwprintf_CALL vsnwprintf_call;
    };
} MOCK_CALL;

static MOCK_CALL expected_calls[MAX_MOCK_CALL_COUNT];
static size_t expected_call_count;
static size_t actual_call_count;
static bool actual_and_expected_match;

int mock_snprintf(char* s, size_t n, const char* format, ...)
{
    int result;

    if ((actual_call_count == expected_call_count) ||
        (expected_calls[actual_call_count].mock_call_type != MOCK_CALL_TYPE_snprintf))
    {
        actual_and_expected_match = false;
        return -1;
    }
    else
    {
        if (expected_calls[actual_call_count].snprintf_call.override_result)
        {
            result = expected_calls[actual_call_count].snprintf_call.call_result;
        }
        else
        {
            va_list args;
            va_start(args, format);

            expected_calls[actual_call_count].snprintf_call.format_arg = format;

            result = vsnprintf(s, n, format, args);

            va_end(args);
        }

        actual_call_count++;
    }

    return result;
}

int mock_vswprintf(wchar_t* s, size_t n, const wchar_t* format, va_list arg_list)
{
    int result;

    if ((actual_call_count == expected_call_count) ||
        (expected_calls[actual_call_count].mock_call_type != MOCK_CALL_TYPE_vswprintf))
    {
        actual_and_expected_match = false;
        return -1;
    }
    else
    {
        if (expected_calls[actual_call_count].vswprintf_call.override_result)
        {
            result = expected_calls[actual_call_count].vswprintf_call.call_result;
        }
        else
        {
            expected_calls[actual_call_count].vswprintf_call.format_arg = format;

            result = vswprintf(s, n, format, arg_list);
        }

        actual_call_count++;
    }

    return result;
}

int mock_vsnwprintf(wchar_t* s, size_t n, const wchar_t* format, va_list arg_list)
{
    int result;

    if ((actual_call_count == expected_call_count) ||
        (expected_calls[actual_call_count].mock_call_type != MOCK_CALL_TYPE_vsnwprintf))
    {
        actual_and_expected_match = false;
        return -1;
    }
    else
    {
        if (expected_calls[actual_call_count].vsnwprintf_call.override_result)
        {
            result = expected_calls[actual_call_count].vsnwprintf_call.call_result;
        }
        else
        {
            expected_calls[actual_call_count].vsnwprintf_call.format_arg = format;

            result = _vsnwprintf(s, n, format, arg_list);
        }

        actual_call_count++;
    }

    return result;
}

static void setup_mocks(void)
{
    expected_call_count = 0;
    actual_call_count = 0;
    actual_and_expected_match = true;
}

static void setup_expected_snprintf_call(void)
{
    expected_calls[expected_call_count].mock_call_type = MOCK_CALL_TYPE_snprintf;
    expected_calls[expected_call_count].snprintf_call.override_result = false;
    expected_call_count++;
}

static void setup_expected_vswprintf_call(void)
{
    expected_calls[expected_call_count].mock_call_type = MOCK_CALL_TYPE_vswprintf;
    expected_calls[expected_call_count].vswprintf_call.override_result = false;
    expected_call_count++;
}

static void setup_expected_vsnwprintf_call(void)
{
    expected_calls[expected_call_count].mock_call_type = MOCK_CALL_TYPE_vsnwprintf;
    expected_calls[expected_call_count].vsnwprintf_call.override_result = false;
    expected_call_count++;
}

#define POOR_MANS_ASSERT(cond) \
    if (!(cond)) \
    { \
        (void)printf("%s:%d test failed\r\n", __FUNCTION__, __LINE__); \
        abort(); \
    } \

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(wchar_t_ptr).to_string */

/* Tests_SRS_LOG_CONTEXT_PROPERTY_TYPE_WCHAR_T_PTR_07_001: [ If property_value is NULL, LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(wchar_t_ptr).to_string shall fail and return a negative value. ]*/
static void wchar_t_ptr_to_string_with_NULL_property_value_fails(void)
{
    // arrange
    char buffer[128];
    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(wchar_t_ptr).to_string(NULL, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result < 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(actual_call_count == expected_call_count);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_TYPE_WCHAR_T_PTR_07_002: [ If buffer is NULL and buffer_length is greater than 0, LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(wchar_t_ptr).to_string shall fail and return a negative value. ]*/
static void wchar_t_ptr_to_string_with_NULL_buffer_and_non_zero_buffer_length_fails(void)
{
    // arrange
    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(wchar_t_ptr).to_string(L"cucu", NULL, 1);

    // assert
    POOR_MANS_ASSERT(result < 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(actual_call_count == expected_call_count);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_TYPE_WCHAR_T_PTR_07_003: [ If buffer is NULL and buffer_length is 0, LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(wchar_t_ptr).to_string shall return the length of the wchar_t string pointed to by property_value. ]*/
static void wchar_t_ptr_to_string_with_NULL_buffer_and_zero_buffer_length_returns_string_length(void)
{
    // arrange
    setup_mocks();
    setup_expected_snprintf_call();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(wchar_t_ptr).to_string(L"cucu", NULL, 0);

    // assert
    POOR_MANS_ASSERT(result == (int)strlen("cucu"));
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(actual_call_count == expected_call_count);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_TYPE_WCHAR_T_PTR_07_004: [ Otherwise, LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(wchar_t_ptr).to_string shall copy the wchar_t string pointed to by property_value to buffer by using snprintf with buffer, buffer_length and format string %ls and pass in the values list the const wchar_t* value pointed to be property_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_TYPE_WCHAR_T_PTR_07_005: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(wchar_t_ptr).to_string shall succeed and return the result of snprintf. ]*/
static void wchar_t_ptr_to_string_copies_the_string(void)
{
    // arrange
    char buffer[4];
    setup_mocks();
    setup_expected_snprintf_call();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(wchar_t_ptr).to_string(L"bau", buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 3);
    POOR_MANS_ASSERT(strcmp(buffer, "bau") == 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(actual_call_count == expected_call_count);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_TYPE_WCHAR_T_PTR_07_006: [ If any error is encountered (truncation is not an error), LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(wchar_t_ptr).to_string shall fail and return a negative value. ]*/
static void when_snprintf_fails_wchar_t_ptr_to_string_also_fails(void)
{
    // arrange
    char buffer[4];
    setup_mocks();
    expected_calls[0].mock_call_type = MOCK_CALL_TYPE_snprintf;
    expected_calls[0].snprintf_call.override_result = true;
    expected_calls[0].snprintf_call.call_result = -1;
    expected_call_count = 1;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(wchar_t_ptr).to_string(L"bau", buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result < 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(actual_call_count == expected_call_count);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_TYPE_WCHAR_T_PTR_07_006: [ If any error is encountered (truncation is not an error), LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(wchar_t_ptr).to_string shall fail and return a negative value. ]*/
static void to_string_with_truncation_succeeds(void)
{
    // arrange
    char buffer[2];
    setup_mocks();
    setup_expected_snprintf_call();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(wchar_t_ptr).to_string(L"bau", buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 3);
    POOR_MANS_ASSERT(strcmp(buffer, "b") == 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(actual_call_count == expected_call_count);
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(wchar_t_ptr).copy */

/* Tests_SRS_LOG_CONTEXT_PROPERTY_TYPE_WCHAR_T_PTR_07_007: [ If src_value is NULL, LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(wchar_t_ptr).copy shall fail and return a non-zero value. ]*/
static void wchar_t_ptr_copy_with_NULL_src_value_fails(void)
{
    // arrange
    wchar_t buffer[128];
    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(wchar_t_ptr).copy(buffer, NULL);

    // assert
    POOR_MANS_ASSERT(result != 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(actual_call_count == expected_call_count);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_TYPE_WCHAR_T_PTR_07_008: [ If dst_value is NULL, LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(wchar_t_ptr).copy shall fail and return a non-zero value. ]*/
static void wchar_t_ptr_copy_with_NULL_dst_value_fails(void)
{
    // arrange
    wchar_t buffer[128];
    setup_mocks();
    setup_expected_vswprintf_call();
    LOG_CONTEXT_PROPERTY_TYPE_INIT(wchar_t_ptr)(buffer, wcslen(L"cucu") + 1, L"cucu");
    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(wchar_t_ptr).copy(NULL, buffer);

    // assert
    POOR_MANS_ASSERT(result != 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(actual_call_count == expected_call_count);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_TYPE_WCHAR_T_PTR_07_009: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(wchar_t_ptr).copy shall copy the entire string(including the null terminator) from src_value to dst_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_TYPE_WCHAR_T_PTR_07_010: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(wchar_t_ptr).copy shall succeed and return 0. ]*/
static void wchar_t_ptr_copy_succeeds(void)
{
    // arrange
    wchar_t src_buffer[5];
    wchar_t dst_buffer[5];
    setup_mocks();
    setup_expected_vswprintf_call();
    LOG_CONTEXT_PROPERTY_TYPE_INIT(wchar_t_ptr)(src_buffer, wcslen(L"cucu") + 1, L"cucu");
    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(wchar_t_ptr).copy(dst_buffer, src_buffer);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(wcscmp(dst_buffer, L"cucu") == 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(actual_call_count == expected_call_count);
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(wchar_t_ptr).free */

/* Tests_SRS_LOG_CONTEXT_PROPERTY_TYPE_WCHAR_T_PTR_07_011: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(wchar_t_ptr).free shall return. ]*/
static void wchar_t_ptr_free_returns(void)
{
    // arrange
    char buffer[128];
    setup_mocks();
    setup_expected_vswprintf_call();
    LOG_CONTEXT_PROPERTY_TYPE_INIT(wchar_t_ptr)(buffer, wcslen(L"cucu") + 1, L"cucu");
    setup_mocks();

    // act
    LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(wchar_t_ptr).free(buffer);

    // assert
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(actual_call_count == expected_call_count);
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(wchar_t_ptr).get_type */

/* Tests_SRS_LOG_CONTEXT_PROPERTY_TYPE_WCHAR_T_PTR_07_012: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(wchar_t_ptr).get_type shall return the property type LOG_CONTEXT_PROPERTY_TYPE_wchar_t_ptr. ]*/
static void wchar_t_ptr_get_type_returns_LOG_CONTEXT_PROPERTY_TYPE_wchar_t_ptr(void)
{
    // arrange
    setup_mocks();

    // act
    LOG_CONTEXT_PROPERTY_TYPE result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(wchar_t_ptr).get_type();

    // assert
    POOR_MANS_ASSERT(result == LOG_CONTEXT_PROPERTY_TYPE_wchar_t_ptr);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(actual_call_count == expected_call_count);
}

/* LOG_CONTEXT_PROPERTY_TYPE_INIT(wchar_t_ptr) */

/* Tests_SRS_LOG_CONTEXT_PROPERTY_TYPE_WCHAR_T_PTR_07_013: [ If dst_value is NULL, LOG_CONTEXT_PROPERTY_TYPE_INIT(wchar_t_ptr) shall fail and return a non-zero value. ]*/
static void wchar_t_ptr_LOG_CONTEXT_PROPERTY_TYPE_INIT_with_NULL_dst_value_fails(void)
{
    // arrange
    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_INIT(wchar_t_ptr)(NULL, wcslen(L"cucu") + 1, L"cucu");

    // assert
    POOR_MANS_ASSERT(result != 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(actual_call_count == expected_call_count);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_TYPE_WCHAR_T_PTR_07_014: [ If format is NULL, LOG_CONTEXT_PROPERTY_TYPE_INIT(wchar_t_ptr) shall fail and return a non-zero value. ]*/
static void wchar_t_ptr_LOG_CONTEXT_PROPERTY_TYPE_INIT_with_NULL_format_fails(void)
{
    // arrange
    char buffer[5];
    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_INIT(wchar_t_ptr)(buffer, 1, NULL);

    // assert
    POOR_MANS_ASSERT(result != 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(actual_call_count == expected_call_count);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_TYPE_WCHAR_T_PTR_07_015: [ LOG_CONTEXT_PROPERTY_TYPE_INIT(wchar_t_ptr) shall initialize by calling vswprintf the memory at dst_value with the wprintf style formatted wchar_t string given by format and the arguments in .... ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_TYPE_WCHAR_T_PTR_07_016: [ LOG_CONTEXT_PROPERTY_TYPE_INIT(wchar_t_ptr) shall succeed and return 0. ]*/
static void wchar_t_ptr_LOG_CONTEXT_PROPERTY_TYPE_INIT_succeeds(void)
{
    // arrange
    wchar_t buffer[5];
    setup_mocks();
    setup_expected_vswprintf_call();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_INIT(wchar_t_ptr)(buffer, wcslen(L"cucu") + 1, L"cucu");

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(wcscmp(buffer, L"cucu") == 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(actual_call_count == expected_call_count);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_TYPE_WCHAR_T_PTR_07_015: [ LOG_CONTEXT_PROPERTY_TYPE_INIT(wchar_t_ptr) shall initialize by calling vswprintf the memory at dst_value with the wprintf style formatted wchar_t string given by format and the arguments in .... ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_TYPE_WCHAR_T_PTR_07_016: [ LOG_CONTEXT_PROPERTY_TYPE_INIT(wchar_t_ptr) shall succeed and return 0. ]*/
static void wchar_t_ptr_LOG_CONTEXT_PROPERTY_TYPE_INIT_with_multiple_args_succeeds(void)
{
    // arrange
    wchar_t buffer[43];
    setup_mocks();
    setup_expected_vswprintf_call();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_INIT(wchar_t_ptr)(buffer, 50, L"The answer is %d and let's say hello %ls", 42, L"world");

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(wcscmp(buffer, L"The answer is 42 and let's say hello world") == 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(actual_call_count == expected_call_count);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_TYPE_WCHAR_T_PTR_07_017: [ If formatting the string fails, LOG_CONTEXT_PROPERTY_TYPE_INIT(wchar_t_ptr) shall fail and return a non-zero value. ]*/
static void when_underlying_call_fails_wchar_t_ptr_LOG_CONTEXT_PROPERTY_TYPE_INIT_also_fails(void)
{
    // arrange
    char buffer[5];
    setup_mocks();
    expected_calls[0].mock_call_type = MOCK_CALL_TYPE_vswprintf;
    expected_calls[0].vswprintf_call.override_result = true;
    expected_calls[0].vswprintf_call.call_result = -1;
    expected_call_count = 1;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_INIT(wchar_t_ptr)(buffer, wcslen(L"cucu") + 1, L"cucu");

    // assert
    POOR_MANS_ASSERT(result != 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(actual_call_count == expected_call_count);
}

/* LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(wchar_t_ptr)*/

/* Tests_SRS_LOG_CONTEXT_PROPERTY_TYPE_WCHAR_T_PTR_07_018: [ If _vsnwprintf fails, LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(wchar_t_ptr) shall return a negative value. ]*/
static void when_underlying_call_fails_wchar_t_ptr_LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE_also_fails(void)
{
    // arrange
    setup_mocks();
    expected_calls[0].mock_call_type = MOCK_CALL_TYPE_vsnwprintf;
    expected_calls[0].vsnwprintf_call.override_result = true;
    expected_calls[0].vsnwprintf_call.call_result = -1;
    expected_call_count = 1;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(wchar_t_ptr)(L"cucu");

    // assert
    POOR_MANS_ASSERT(result < 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(actual_call_count == expected_call_count);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_TYPE_WCHAR_T_PTR_07_019: [ Otherwise, on success, LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(wchar_t_ptr) shall return the amount of memory needed in number of wide-chracters to store the wprintf style formatted wchar_t string given by format and the arguments in .... ]*/
static void wchar_t_ptr_LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE_succeeds(void)
{
    // arrange
    setup_mocks();
    setup_expected_vsnwprintf_call();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(wchar_t_ptr)(L"cucu");

    // assert
    POOR_MANS_ASSERT(result == 5);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(actual_call_count == expected_call_count);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_TYPE_WCHAR_T_PTR_07_019: [ Otherwise, on success, LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(wchar_t_ptr) shall return the amount of memory needed in number of wide-chracters to store the wprintf style formatted wchar_t string given by format and the arguments in .... ]*/
static void wchar_t_ptr_LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE_with_multiple_args_succeeds(void)
{
    // arrange
    setup_mocks();
    setup_expected_vsnwprintf_call();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(wchar_t_ptr)(L"The answer is %d and let's say hello %ls", 43, L"world");

    // assert
    POOR_MANS_ASSERT(result == 43);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(actual_call_count == expected_call_count);
}

/* very "poor man's" way of testing, as no test harness and mocking framework are available */
int main(void)
{
    wchar_t_ptr_to_string_with_NULL_property_value_fails();
    wchar_t_ptr_to_string_with_NULL_buffer_and_non_zero_buffer_length_fails();
    wchar_t_ptr_to_string_with_NULL_buffer_and_zero_buffer_length_returns_string_length();
    wchar_t_ptr_to_string_copies_the_string();
    when_snprintf_fails_wchar_t_ptr_to_string_also_fails();
    to_string_with_truncation_succeeds();

    wchar_t_ptr_copy_with_NULL_src_value_fails();
    wchar_t_ptr_copy_with_NULL_dst_value_fails();
    wchar_t_ptr_copy_succeeds();

    wchar_t_ptr_free_returns();

    wchar_t_ptr_get_type_returns_LOG_CONTEXT_PROPERTY_TYPE_wchar_t_ptr();

    wchar_t_ptr_LOG_CONTEXT_PROPERTY_TYPE_INIT_with_NULL_dst_value_fails();
    wchar_t_ptr_LOG_CONTEXT_PROPERTY_TYPE_INIT_with_NULL_format_fails();
    wchar_t_ptr_LOG_CONTEXT_PROPERTY_TYPE_INIT_succeeds();
    wchar_t_ptr_LOG_CONTEXT_PROPERTY_TYPE_INIT_with_multiple_args_succeeds();
    when_underlying_call_fails_wchar_t_ptr_LOG_CONTEXT_PROPERTY_TYPE_INIT_also_fails();

    when_underlying_call_fails_wchar_t_ptr_LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE_also_fails();
    wchar_t_ptr_LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE_succeeds();
    wchar_t_ptr_LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE_with_multiple_args_succeeds();

    return 0;
}
