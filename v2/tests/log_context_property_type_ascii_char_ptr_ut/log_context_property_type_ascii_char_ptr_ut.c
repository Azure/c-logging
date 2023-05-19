// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

#include "macro_utils/macro_utils.h"

#include "c_logging/log_context_property_type.h"
#include "c_logging/log_context_property_type_if.h"

#include "c_logging/log_context_property_type_ascii_char_ptr.h"

// defines how many mock calls we can have
#define MAX_MOCK_CALL_COUNT (128)

#define MOCK_CALL_TYPE_VALUES \
    MOCK_CALL_TYPE_snprintf, \
    MOCK_CALL_TYPE_vsprintf, \
    MOCK_CALL_TYPE_vsnprintf \

MU_DEFINE_ENUM(MOCK_CALL_TYPE, MOCK_CALL_TYPE_VALUES)

// very poor mans mocks :-(
typedef struct snprintf_CALL_TAG
{
    bool override_result;
    int call_result;
    const char* format_arg;
} snprintf_CALL;

typedef struct vsprintf_CALL_TAG
{
    bool override_result;
    int call_result;
    const char* format_arg;
} vsprintf_CALL;

typedef struct vsnprintf_CALL_TAG
{
    bool override_result;
    int call_result;
    const char* format_arg;
} vsnprintf_CALL;

typedef struct MOCK_CALL_TAG
{
    MOCK_CALL_TYPE mock_call_type;
    union
    {
        snprintf_CALL snprintf_call;
        vsprintf_CALL vsprintf_call;
        vsnprintf_CALL vsnprintf_call;
    } u;
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
        if (expected_calls[actual_call_count].u.snprintf_call.override_result)
        {
            result = expected_calls[actual_call_count].u.snprintf_call.call_result;
        }
        else
        {
            va_list args;
            va_start(args, format);

            expected_calls[actual_call_count].u.snprintf_call.format_arg = format;

            result = vsnprintf(s, n, format, args);

            va_end(args);
        }

        actual_call_count++;
    }

    return result;
}

int mock_vsprintf(char* s, const char* format, va_list args)
{
    int result;

    if ((actual_call_count == expected_call_count) ||
        (expected_calls[actual_call_count].mock_call_type != MOCK_CALL_TYPE_vsprintf))
    {
        actual_and_expected_match = false;
        return -1;
    }
    else
    {
        if (expected_calls[actual_call_count].u.vsprintf_call.override_result)
        {
            result = expected_calls[actual_call_count].u.vsprintf_call.call_result;
        }
        else
        {
            expected_calls[actual_call_count].u.vsprintf_call.format_arg = format;

            result = vsprintf(s, format, args);
        }

        actual_call_count++;
    }

    return result;
}

int mock_vsnprintf(char* s, size_t n, const char* format, va_list arg_list)
{
    int result;

    if ((actual_call_count == expected_call_count) ||
        (expected_calls[actual_call_count].mock_call_type != MOCK_CALL_TYPE_vsnprintf))
    {
        actual_and_expected_match = false;
        return -1;
    }
    else
    {
        if (expected_calls[actual_call_count].u.vsnprintf_call.override_result)
        {
            result = expected_calls[actual_call_count].u.vsnprintf_call.call_result;
        }
        else
        {
            expected_calls[actual_call_count].u.vsnprintf_call.format_arg = format;

            result = vsnprintf(s, n, format, arg_list);
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
    expected_calls[expected_call_count].u.snprintf_call.override_result = false;
    expected_call_count++;
}

static void setup_expected_vsprintf_call(void)
{
    expected_calls[expected_call_count].mock_call_type = MOCK_CALL_TYPE_vsprintf;
    expected_calls[expected_call_count].u.vsprintf_call.override_result = false;
    expected_call_count++;
}

static void setup_expected_vsnprintf_call(void)
{
    expected_calls[expected_call_count].mock_call_type = MOCK_CALL_TYPE_vsnprintf;
    expected_calls[expected_call_count].u.vsnprintf_call.override_result = false;
    expected_call_count++;
}

#define POOR_MANS_ASSERT(cond) \
    if (!(cond)) \
    { \
        (void)printf("%s:%d test failed\r\n", __FUNCTION__, __LINE__); \
        abort(); \
    } \

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(ascii_char_ptr).to_string */

/* Tests_SRS_LOG_CONTEXT_PROPERTY_TYPE_ASCII_CHAR_PTR_01_001: [ If property_value is NULL, LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(ascii_char_ptr).to_string shall fail and return a negative value. ]*/
static void ascii_char_ptr_to_string_with_NULL_property_value_fails(void)
{
    // arrange
    char buffer[128];
    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(ascii_char_ptr).to_string(NULL, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result < 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(actual_call_count == expected_call_count);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_TYPE_ASCII_CHAR_PTR_01_017: [ If buffer is NULL and buffer_length is greater than 0, LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(ascii_char_ptr).to_string shall fail and return a negative value. ]*/
static void ascii_char_ptr_to_string_with_NULL_buffer_and_non_zero_buffer_length_fails(void)
{
    // arrange
    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(ascii_char_ptr).to_string("cucu", NULL, 1);

    // assert
    POOR_MANS_ASSERT(result < 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(actual_call_count == expected_call_count);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_TYPE_ASCII_CHAR_PTR_01_002: [ If buffer is NULL and buffer_length is 0, LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(ascii_char_ptr).to_string shall return the length of the string pointed to by property_value. ]*/
static void ascii_char_ptr_to_string_with_NULL_buffer_and_zero_buffer_length_returns_string_length(void)
{
    // arrange
    setup_mocks();
    setup_expected_snprintf_call();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(ascii_char_ptr).to_string("cucu", NULL, 0);

    // assert
    POOR_MANS_ASSERT(result == (int)strlen("cucu"));
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(actual_call_count == expected_call_count);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_TYPE_ASCII_CHAR_PTR_01_003: [ Otherwise, LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(ascii_char_ptr).to_string shall copy the string pointed to by property_value to buffer by using snprintf with buffer, buffer_length and format string %s and pass in the values list the const char* value pointed to be property_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_TYPE_ASCII_CHAR_PTR_01_004: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(ascii_char_ptr).to_string shall succeed and return the result of snprintf. ]*/
static void ascii_char_ptr_to_string_copies_the_string(void)
{
    // arrange
    char buffer[4];
    setup_mocks();
    setup_expected_snprintf_call();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(ascii_char_ptr).to_string("bau", buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 3);
    POOR_MANS_ASSERT(strcmp(buffer, "bau") == 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(actual_call_count == expected_call_count);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_TYPE_ASCII_CHAR_PTR_01_005: [ If any error is encountered (truncation is not an error), LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(ascii_char_ptr).to_string shall fail and return a negative value. ]*/
static void when_snprintf_fails_ascii_char_ptr_to_string_also_fails(void)
{
    // arrange
    char buffer[4];
    setup_mocks();
    expected_calls[0].mock_call_type = MOCK_CALL_TYPE_snprintf;
    expected_calls[0].u.snprintf_call.override_result = true;
    expected_calls[0].u.snprintf_call.call_result = -1;
    expected_call_count = 1;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(ascii_char_ptr).to_string("bau", buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result < 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(actual_call_count == expected_call_count);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_TYPE_ASCII_CHAR_PTR_01_005: [ If any error is encountered (truncation is not an error), LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(ascii_char_ptr).to_string shall fail and return a negative value. ]*/
static void to_string_with_truncation_succeeds(void)
{
    // arrange
    char buffer[2];
    setup_mocks();
    setup_expected_snprintf_call();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(ascii_char_ptr).to_string("bau", buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 3);
    POOR_MANS_ASSERT(strcmp(buffer, "b") == 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(actual_call_count == expected_call_count);
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(ascii_char_ptr).copy */

/* Tests_SRS_LOG_CONTEXT_PROPERTY_TYPE_ASCII_CHAR_PTR_01_006: [ If src_value is NULL, LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(ascii_char_ptr).copy shall fail and return a non-zero value. ]*/
static void ascii_char_ptr_copy_with_NULL_src_value_fails(void)
{
    // arrange
    char buffer[128];
    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(ascii_char_ptr).copy(buffer, NULL);

    // assert
    POOR_MANS_ASSERT(result != 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(actual_call_count == expected_call_count);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_TYPE_ASCII_CHAR_PTR_01_007: [ If dst_value is NULL, LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(ascii_char_ptr).copy shall fail and return a non-zero value. ]*/
static void ascii_char_ptr_copy_with_NULL_dst_value_fails(void)
{
    // arrange
    char buffer[128];
    setup_mocks();
    setup_expected_vsprintf_call();
    LOG_CONTEXT_PROPERTY_TYPE_INIT(ascii_char_ptr)(buffer, "cucu");
    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(ascii_char_ptr).copy(NULL, buffer);

    // assert
    POOR_MANS_ASSERT(result != 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(actual_call_count == expected_call_count);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_TYPE_ASCII_CHAR_PTR_01_008: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(ascii_char_ptr).copy shall copy the entire string (including the null terminator) from src_value to dst_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_TYPE_ASCII_CHAR_PTR_01_009: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(ascii_char_ptr).copy shall succeed and return 0. ]*/
static void ascii_char_ptr_copy_succeeds(void)
{
    // arrange
    char src_buffer[5];
    char dst_buffer[5];
    setup_mocks();
    setup_expected_vsprintf_call();
    LOG_CONTEXT_PROPERTY_TYPE_INIT(ascii_char_ptr)(src_buffer, "cucu");
    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(ascii_char_ptr).copy(dst_buffer, src_buffer);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(strcmp(dst_buffer, "cucu") == 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(actual_call_count == expected_call_count);
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(ascii_char_ptr).free */

/* Tests_SRS_LOG_CONTEXT_PROPERTY_TYPE_ASCII_CHAR_PTR_01_018: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(ascii_char_ptr).free shall return. ]*/
static void ascii_char_ptr_free_returns(void)
{
    // arrange
    char buffer[128];
    setup_mocks();
    setup_expected_vsprintf_call();
    LOG_CONTEXT_PROPERTY_TYPE_INIT(ascii_char_ptr)(buffer, "cucu");
    setup_mocks();

    // act
    LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(ascii_char_ptr).free(buffer);

    // assert
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(actual_call_count == expected_call_count);
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(ascii_char_ptr).get_type */

/* Tests_SRS_LOG_CONTEXT_PROPERTY_TYPE_ASCII_CHAR_PTR_01_010: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(ascii_char_ptr).get_type shall return the property type LOG_CONTEXT_PROPERTY_TYPE_ascii_char_ptr. ]*/
static void ascii_char_ptr_get_type_returns_LOG_CONTEXT_PROPERTY_TYPE_ascii_char_ptr(void)
{
    // arrange
    setup_mocks();

    // act
    LOG_CONTEXT_PROPERTY_TYPE result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(ascii_char_ptr).get_type();

    // assert
    POOR_MANS_ASSERT(result == LOG_CONTEXT_PROPERTY_TYPE_ascii_char_ptr);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(actual_call_count == expected_call_count);
}

/* LOG_CONTEXT_PROPERTY_TYPE_INIT(ascii_char_ptr) */

/* Tests_SRS_LOG_CONTEXT_PROPERTY_TYPE_ASCII_CHAR_PTR_01_011: [ If dst_value is NULL, LOG_CONTEXT_PROPERTY_TYPE_INIT(ascii_char_ptr) shall fail and return a non-zero value. ]*/
static void ascii_char_ptr_LOG_CONTEXT_PROPERTY_TYPE_INIT_with_NULL_dst_value_fails(void)
{
    // arrange
    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_INIT(ascii_char_ptr)(NULL, "cucu");

    // assert
    POOR_MANS_ASSERT(result != 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(actual_call_count == expected_call_count);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_TYPE_ASCII_CHAR_PTR_01_019: [ If format is NULL, LOG_CONTEXT_PROPERTY_TYPE_INIT(ascii_char_ptr) shall fail and return a non-zero value. ]*/
static void ascii_char_ptr_LOG_CONTEXT_PROPERTY_TYPE_INIT_with_NULL_format_fails(void)
{
    // arrange
    char buffer[5];
    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_INIT(ascii_char_ptr)(buffer, NULL);

    // assert
    POOR_MANS_ASSERT(result != 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(actual_call_count == expected_call_count);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_TYPE_ASCII_CHAR_PTR_01_012: [ LOG_CONTEXT_PROPERTY_TYPE_INIT(ascii_char_ptr) shall initialize by calling snprintf the memory at dst_value with the printf style formatted string given by format and the arguments in .... ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_TYPE_ASCII_CHAR_PTR_01_013: [ LOG_CONTEXT_PROPERTY_TYPE_INIT(ascii_char_ptr) shall succeed and return 0. ]*/
static void ascii_char_ptr_LOG_CONTEXT_PROPERTY_TYPE_INIT_succeeds(void)
{
    // arrange
    char buffer[5];
    setup_mocks();
    setup_expected_vsprintf_call();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_INIT(ascii_char_ptr)(buffer, "cucu");

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(strcmp(buffer, "cucu") == 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(actual_call_count == expected_call_count);
}

static void ascii_char_ptr_LOG_CONTEXT_PROPERTY_TYPE_INIT_with_multiple_args_succeeds(void)
{
    // arrange
    char buffer[43];
    setup_mocks();
    setup_expected_vsprintf_call();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_INIT(ascii_char_ptr)(buffer, "The answer is %d and let's say hello %s", 42, "world");

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(strcmp(buffer, "The answer is 42 and let's say hello world") == 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(actual_call_count == expected_call_count);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_TYPE_ASCII_CHAR_PTR_01_014: [ If formatting the string fails, LOG_CONTEXT_PROPERTY_TYPE_INIT(ascii_char_ptr) shall fail and return a non-zero value. ]*/
static void when_underlying_call_fails_ascii_char_ptr_LOG_CONTEXT_PROPERTY_TYPE_INIT_also_fails(void)
{
    // arrange
    char buffer[5];
    setup_mocks();
    expected_calls[0].mock_call_type = MOCK_CALL_TYPE_vsprintf;
    expected_calls[0].u.vsprintf_call.override_result = true;
    expected_calls[0].u.vsprintf_call.call_result = -1;
    expected_call_count = 1;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_INIT(ascii_char_ptr)(buffer, "cucu");

    // assert
    POOR_MANS_ASSERT(result != 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(actual_call_count == expected_call_count);
}

/* LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(ascii_char_ptr)*/

/* Tests_SRS_LOG_CONTEXT_PROPERTY_TYPE_ASCII_CHAR_PTR_01_015: [ If snprintf fails, LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(ascii_char_ptr) shall return a negative value. ]*/
static void when_underlying_call_fails_ascii_char_ptr_LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE_also_fails(void)
{
    // arrange
    setup_mocks();
    expected_calls[0].mock_call_type = MOCK_CALL_TYPE_vsnprintf;
    expected_calls[0].u.vsnprintf_call.override_result = true;
    expected_calls[0].u.vsnprintf_call.call_result = -1;
    expected_call_count = 1;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(ascii_char_ptr)("cucu");

    // assert
    POOR_MANS_ASSERT(result < 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(actual_call_count == expected_call_count);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_TYPE_ASCII_CHAR_PTR_01_016: [ Otherwise, on success, LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(ascii_char_ptr) shall return the amount of memory needed to store the printf style formatted string given by format and the arguments in .... ]*/
static void ascii_char_ptr_LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE_succeeds(void)
{
    // arrange
    setup_mocks();
    setup_expected_vsnprintf_call();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(ascii_char_ptr)("cucu");

    // assert
    POOR_MANS_ASSERT(result == 5);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(actual_call_count == expected_call_count);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_TYPE_ASCII_CHAR_PTR_01_016: [ Otherwise, on success, LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(ascii_char_ptr) shall return the amount of memory needed to store the printf style formatted string given by format and the arguments in .... ]*/
static void ascii_char_ptr_LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE_with_multiple_args_succeeds(void)
{
    // arrange
    setup_mocks();
    setup_expected_vsnprintf_call();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(ascii_char_ptr)("The answer is %d and let's say hello %s", 42, "world");

    // assert
    POOR_MANS_ASSERT(result == 43);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(actual_call_count == expected_call_count);
}

/* very "poor man's" way of testing, as no test harness and mocking framework are available */
int main(void)
{
    ascii_char_ptr_to_string_with_NULL_property_value_fails();
    ascii_char_ptr_to_string_with_NULL_buffer_and_non_zero_buffer_length_fails();
    ascii_char_ptr_to_string_with_NULL_buffer_and_zero_buffer_length_returns_string_length();
    ascii_char_ptr_to_string_copies_the_string();
    when_snprintf_fails_ascii_char_ptr_to_string_also_fails();
    to_string_with_truncation_succeeds();

    ascii_char_ptr_copy_with_NULL_src_value_fails();
    ascii_char_ptr_copy_with_NULL_dst_value_fails();
    ascii_char_ptr_copy_succeeds();

    ascii_char_ptr_free_returns();

    ascii_char_ptr_get_type_returns_LOG_CONTEXT_PROPERTY_TYPE_ascii_char_ptr();

    ascii_char_ptr_LOG_CONTEXT_PROPERTY_TYPE_INIT_with_NULL_dst_value_fails();
    ascii_char_ptr_LOG_CONTEXT_PROPERTY_TYPE_INIT_with_NULL_format_fails();
    ascii_char_ptr_LOG_CONTEXT_PROPERTY_TYPE_INIT_succeeds();
    ascii_char_ptr_LOG_CONTEXT_PROPERTY_TYPE_INIT_with_multiple_args_succeeds();
    when_underlying_call_fails_ascii_char_ptr_LOG_CONTEXT_PROPERTY_TYPE_INIT_also_fails();

    when_underlying_call_fails_ascii_char_ptr_LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE_also_fails();
    ascii_char_ptr_LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE_succeeds();
    ascii_char_ptr_LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE_with_multiple_args_succeeds();

    return 0;
}
