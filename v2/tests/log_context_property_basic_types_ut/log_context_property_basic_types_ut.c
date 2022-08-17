// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>

#include "macro_utils/macro_utils.h"

#include "c_logging/log_context_property_type.h"
#include "c_logging/log_context_property_type_if.h"
#include "c_logging/log_context_property_basic_types.h"

static size_t asserts_failed = 0;

// defines how many mock calls we can have
#define MAX_MOCK_CALL_COUNT (128)

#define MOCK_CALL_TYPE_VALUES \
    MOCK_CALL_TYPE_snprintf \

MU_DEFINE_ENUM(MOCK_CALL_TYPE, MOCK_CALL_TYPE_VALUES)

// very poor mans mocks :-(
typedef struct snprintf_CALL_TAG
{
    bool override_result;
    int call_result;
} snprintf_CALL;

typedef struct MOCK_CALL_TAG
{
    MOCK_CALL_TYPE mock_call_type;
    union
    {
        snprintf_CALL snprintf_call;
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

            result = vsnprintf(s, n, format, args);

            va_end(args);
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
    expected_calls[0].mock_call_type = MOCK_CALL_TYPE_snprintf;
    expected_calls[0].u.snprintf_call.override_result = false;
    expected_call_count = 1;
}

#define TEST_BUFFER_SIZE 1024

#define POOR_MANS_ASSERT(cond) \
    if (!(cond)) \
    { \
        (void)printf("%s:%d test failed\r\n", __FUNCTION__, __LINE__); \
        asserts_failed++; \
    } \

/* LOG_CONTEXT_PROPERTY_TYPE_IMPL(int64_t).to_string */

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_001: [ If property_value is NULL, LOG_CONTEXT_PROPERTY_TYPE_IMPL(int64_t).to_string shall fail and return -1. ]*/
static void int32_t_to_string_with_NULL_value_fails(void)
{
    // arrange
    char buffer[TEST_BUFFER_SIZE];
    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IMPL(int32_t).to_string(NULL, buffer, TEST_BUFFER_SIZE);

    // assert
    POOR_MANS_ASSERT(result < 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_002: [ LOG_CONTEXT_PROPERTY_TYPE_IMPL(int64_t).to_string shall call snprintf with buffer, buffer_length and format string PRId64 and pass in the values list the int64_t value pointed to be property_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_003: [ LOG_CONTEXT_PROPERTY_TYPE_IMPL(int64_t).to_string shall succeed and return the result of snprintf. ]*/
static void int32_t_to_string_succeeds(void)
{
    // arrange
    char buffer[TEST_BUFFER_SIZE];
    int32_t int32_t_value = 0;

    setup_mocks();
    setup_expected_snprintf_call();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IMPL(int32_t).to_string(&int32_t_value, buffer, TEST_BUFFER_SIZE);

    // assert
    POOR_MANS_ASSERT(result == 1);
    POOR_MANS_ASSERT(strcmp(buffer, "0") == 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_002: [ LOG_CONTEXT_PROPERTY_TYPE_IMPL(int64_t).to_string shall call snprintf with buffer, buffer_length and format string PRId64 and pass in the values list the int64_t value pointed to be property_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_003: [ LOG_CONTEXT_PROPERTY_TYPE_IMPL(int64_t).to_string shall succeed and return the result of snprintf. ]*/
static void int32_t_to_string_INT32_MAX_succeeds(void)
{
    // arrange
    char buffer[TEST_BUFFER_SIZE];
    int32_t int32_t_value = INT32_MAX;

    setup_mocks();
    setup_expected_snprintf_call();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IMPL(int32_t).to_string(&int32_t_value, buffer, TEST_BUFFER_SIZE);

    // assert
    POOR_MANS_ASSERT(result == 10);
    POOR_MANS_ASSERT(strcmp(buffer, "2147483647") == 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_002: [ LOG_CONTEXT_PROPERTY_TYPE_IMPL(int64_t).to_string shall call snprintf with buffer, buffer_length and format string PRId64 and pass in the values list the int64_t value pointed to be property_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_003: [ LOG_CONTEXT_PROPERTY_TYPE_IMPL(int64_t).to_string shall succeed and return the result of snprintf. ]*/
static void int32_t_to_string_with_truncation_succeeds(void)
{
    // arrange
    char buffer[2];
    int32_t int32_t_value = 42;

    setup_mocks();
    setup_expected_snprintf_call();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IMPL(int32_t).to_string(&int32_t_value, buffer, 2);

    // assert
    POOR_MANS_ASSERT(result == 2);
    POOR_MANS_ASSERT(strcmp(buffer, "4") == 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_002: [ LOG_CONTEXT_PROPERTY_TYPE_IMPL(int64_t).to_string shall call snprintf with buffer, buffer_length and format string PRId64 and pass in the values list the int64_t value pointed to be property_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_003: [ LOG_CONTEXT_PROPERTY_TYPE_IMPL(int64_t).to_string shall succeed and return the result of snprintf. ]*/
static void int32_t_to_string_with_negative_value_succeeds(void)
{
    // arrange
    char buffer[TEST_BUFFER_SIZE];
    int32_t int32_t_value = -1;

    setup_mocks();
    setup_expected_snprintf_call();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IMPL(int32_t).to_string(&int32_t_value, buffer, TEST_BUFFER_SIZE);

    // assert
    POOR_MANS_ASSERT(result > 0);
    POOR_MANS_ASSERT(strcmp(buffer, "-1") == 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_004: [ If any error is encountered, LOG_CONTEXT_PROPERTY_TYPE_IMPL(int64_t).to_string shall fail and return -1. ]*/
static void when_snprintf_fails_int32_t_to_string_also_fails(void)
{
    // arrange
    char buffer[TEST_BUFFER_SIZE];
    int32_t int32_t_value = -1;

    setup_mocks();
    expected_calls[0].mock_call_type = MOCK_CALL_TYPE_snprintf;
    expected_calls[0].u.snprintf_call.call_result = -1;
    expected_calls[0].u.snprintf_call.override_result = true;
    expected_call_count = 1;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IMPL(int32_t).to_string(&int32_t_value, buffer, TEST_BUFFER_SIZE);

    // assert
    POOR_MANS_ASSERT(result < 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* LOG_CONTEXT_PROPERTY_TYPE_IMPL(int64_t).copy */

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_005: [ If src_value is NULL, LOG_CONTEXT_PROPERTY_TYPE_IMPL(int64_t).copy shall fail and return a non-zero value. ]*/
static void int32_t_copy_called_with_NULL_dst_value_fails(void)
{
    // arrange
    int32_t src = 42;

    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IMPL(int32_t).copy(NULL, &src);

    // assert
    POOR_MANS_ASSERT(result != 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_006: [ If dst_value is NULL, LOG_CONTEXT_PROPERTY_TYPE_IMPL(int64_t).copy shall fail and return a non-zero value. ]*/
static void int32_t_copy_called_with_NULL_src_value_fails(void)
{
    // arrange
    int32_t dst;

    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IMPL(int32_t).copy(&dst, NULL);

    // assert
    POOR_MANS_ASSERT(result != 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_007: [ LOG_CONTEXT_PROPERTY_TYPE_IMPL(int64_t).copy shall copy the bytes of the int64_t value from the address pointed by src_value to dst_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_008: [ LOG_CONTEXT_PROPERTY_TYPE_IMPL(int64_t).copy shall succeed and return 0. ]*/
static void int32_t_copy_succeeds(void)
{
    // arrange
    int32_t src = 42;
    int32_t dst = 43;

    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IMPL(int32_t).copy(&dst, &src);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(dst == 42);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_007: [ LOG_CONTEXT_PROPERTY_TYPE_IMPL(int64_t).copy shall copy the bytes of the int64_t value from the address pointed by src_value to dst_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_008: [ LOG_CONTEXT_PROPERTY_TYPE_IMPL(int64_t).copy shall succeed and return 0. ]*/
static void int32_t_copy_succeeds_2(void)
{
    // arrange
    int32_t src = INT32_MAX;
    int32_t dst = 43;

    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IMPL(int32_t).copy(&dst, &src);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(dst == INT32_MAX);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* LOG_CONTEXT_PROPERTY_TYPE_IMPL(int64_t).free */

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_009: [ LOG_CONTEXT_PROPERTY_TYPE_IMPL(int64_t).free shall return. ]*/
static void int32_t_free_returns(void)
{
    // arrange
    int32_t value = INT32_MAX;

    setup_mocks();

    // act
    LOG_CONTEXT_PROPERTY_TYPE_IMPL(int32_t).free(&value);

    // assert
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* LOG_CONTEXT_PROPERTY_TYPE_IMPL(int64_t).get_type */

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_010: [ LOG_CONTEXT_PROPERTY_TYPE_IMPL(int64_t).get_type shall returns the property type LOG_CONTEXT_PROPERTY_TYPE_int64_t. ]*/
static void int32_t_get_type_returns_LOG_CONTEXT_PROPERTY_TYPE_int64_t(void)
{
    // arrange
    setup_mocks();

    // act
    LOG_CONTEXT_PROPERTY_TYPE result = LOG_CONTEXT_PROPERTY_TYPE_IMPL(int32_t).get_type();

    // assert
    POOR_MANS_ASSERT(result == LOG_CONTEXT_PROPERTY_TYPE_int32_t);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* very "poor man's" way of testing, as no test harness and mocking framework are available */
int main(void)
{
    int32_t_to_string_with_NULL_value_fails();
    int32_t_to_string_succeeds();
    int32_t_to_string_INT32_MAX_succeeds();
    int32_t_to_string_with_truncation_succeeds();
    int32_t_to_string_with_negative_value_succeeds();
    when_snprintf_fails_int32_t_to_string_also_fails();

    int32_t_copy_called_with_NULL_dst_value_fails();
    int32_t_copy_called_with_NULL_src_value_fails();
    int32_t_copy_succeeds();
    int32_t_copy_succeeds_2();

    int32_t_free_returns();

    int32_t_get_type_returns_LOG_CONTEXT_PROPERTY_TYPE_int64_t();

    return asserts_failed;
}
