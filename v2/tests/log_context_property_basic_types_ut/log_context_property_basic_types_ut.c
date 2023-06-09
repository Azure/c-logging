// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>

#include "macro_utils/macro_utils.h"

#include "c_logging/log_context_property_type.h"
#include "c_logging/log_context_property_type_if.h"
#include "c_logging/log_context_property_basic_types.h"

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
    const char* captured_format_arg;
} snprintf_CALL;

typedef struct MOCK_CALL_TAG
{
    MOCK_CALL_TYPE mock_call_type;
    union
    {
        snprintf_CALL snprintf_call;
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

            expected_calls[actual_call_count].snprintf_call.captured_format_arg = format;

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
    expected_calls[expected_call_count].mock_call_type = MOCK_CALL_TYPE_snprintf;
    expected_calls[expected_call_count].snprintf_call.override_result = false;
    expected_call_count++;
}

#define TEST_BUFFER_SIZE 1024

#define POOR_MANS_ASSERT(cond) \
    if (!(cond)) \
    { \
        (void)printf("%s:%d test failed\r\n", __FUNCTION__, __LINE__); \
        abort(); \
    } \

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int64_t).to_string */

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_001: [ If property_value is NULL, LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int64_t).to_string shall fail and return a negative value. ]*/
static void int64_t_to_string_with_NULL_value_fails(void)
{
    // arrange
    char buffer[TEST_BUFFER_SIZE];
    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int64_t).to_string(NULL, buffer, TEST_BUFFER_SIZE);

    // assert
    POOR_MANS_ASSERT(result < 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_002: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int64_t).to_string shall call snprintf with buffer, buffer_length and format string PRId64 and pass in the values list the int64_t value pointed to be property_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_003: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int64_t).to_string shall succeed and return the result of snprintf. ]*/
static void int64_t_to_string_succeeds(void)
{
    // arrange
    char buffer[TEST_BUFFER_SIZE];
    int64_t int64_t_value = 0;

    setup_mocks();
    setup_expected_snprintf_call();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int64_t).to_string(&int64_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 1);
    POOR_MANS_ASSERT(strcmp(expected_calls[0].snprintf_call.captured_format_arg, "%" PRId64) == 0);
    POOR_MANS_ASSERT(strcmp(buffer, "0") == 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_002: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int64_t).to_string shall call snprintf with buffer, buffer_length and format string PRId64 and pass in the values list the int64_t value pointed to be property_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_003: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int64_t).to_string shall succeed and return the result of snprintf. ]*/
static void int64_t_to_string_INT64_MAX_succeeds(void)
{
    // arrange
    char buffer[TEST_BUFFER_SIZE];
    int64_t int64_t_value = INT64_MAX;

    setup_mocks();
    setup_expected_snprintf_call();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int64_t).to_string(&int64_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 19);
    POOR_MANS_ASSERT(strcmp(expected_calls[0].snprintf_call.captured_format_arg, "%" PRId64) == 0);
    POOR_MANS_ASSERT(strcmp(buffer, "9223372036854775807") == 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_002: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int64_t).to_string shall call snprintf with buffer, buffer_length and format string PRId64 and pass in the values list the int64_t value pointed to be property_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_003: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int64_t).to_string shall succeed and return the result of snprintf. ]*/
static void int64_t_to_string_with_truncation_succeeds(void)
{
    // arrange
    char buffer[2];
    int64_t int64_t_value = 42;

    setup_mocks();
    setup_expected_snprintf_call();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int64_t).to_string(&int64_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 2);
    POOR_MANS_ASSERT(strcmp(expected_calls[0].snprintf_call.captured_format_arg, "%" PRId64) == 0);
    POOR_MANS_ASSERT(strcmp(buffer, "4") == 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_002: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int64_t).to_string shall call snprintf with buffer, buffer_length and format string PRId64 and pass in the values list the int64_t value pointed to be property_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_003: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int64_t).to_string shall succeed and return the result of snprintf. ]*/
static void int64_t_to_string_with_truncation_minus_1_succeeds(void)
{
    // arrange
    char buffer[2];
    int64_t int64_t_value = -1;

    setup_mocks();
    setup_expected_snprintf_call();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int64_t).to_string(&int64_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 2);
    POOR_MANS_ASSERT(strcmp(expected_calls[0].snprintf_call.captured_format_arg, "%" PRId64) == 0);
    POOR_MANS_ASSERT(strcmp(buffer, "-") == 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_002: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int64_t).to_string shall call snprintf with buffer, buffer_length and format string PRId64 and pass in the values list the int64_t value pointed to be property_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_003: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int64_t).to_string shall succeed and return the result of snprintf. ]*/
static void int64_t_to_string_with_negative_value_succeeds(void)
{
    // arrange
    char buffer[TEST_BUFFER_SIZE];
    int64_t int64_t_value = -1;

    setup_mocks();
    setup_expected_snprintf_call();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int64_t).to_string(&int64_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 2);
    POOR_MANS_ASSERT(strcmp(expected_calls[0].snprintf_call.captured_format_arg, "%" PRId64) == 0);
    POOR_MANS_ASSERT(strcmp(buffer, "-1") == 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_002: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int64_t).to_string shall call snprintf with buffer, buffer_length and format string PRId64 and pass in the values list the int64_t value pointed to be property_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_003: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int64_t).to_string shall succeed and return the result of snprintf. ]*/
static void int64_t_to_string_with_just_enough_big_buffer_succeeds(void)
{
    // arrange
    char buffer[2];
    int64_t int64_t_value = 1;

    setup_mocks();
    setup_expected_snprintf_call();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int64_t).to_string(&int64_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 1);
    POOR_MANS_ASSERT(strcmp(expected_calls[0].snprintf_call.captured_format_arg, "%" PRId64) == 0);
    POOR_MANS_ASSERT(strcmp(buffer, "1") == 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_004: [ If any error is encountered, LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int64_t).to_string shall fail and return a negative value. ]*/
static void when_snprintf_fails_int64_t_to_string_also_fails(void)
{
    // arrange
    char buffer[TEST_BUFFER_SIZE];
    int64_t int64_t_value = -1;

    setup_mocks();
    expected_calls[0].mock_call_type = MOCK_CALL_TYPE_snprintf;
    expected_calls[0].snprintf_call.call_result = -1;
    expected_calls[0].snprintf_call.override_result = true;
    expected_call_count = 1;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int64_t).to_string(&int64_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result < 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int64_t).copy */

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_005: [ If src_value is NULL, LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int64_t).copy shall fail and return a non-zero value. ]*/
static void int64_t_copy_called_with_NULL_dst_value_fails(void)
{
    // arrange
    int64_t src = 42;

    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int64_t).copy(NULL, &src);

    // assert
    POOR_MANS_ASSERT(result != 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_006: [ If dst_value is NULL, LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int64_t).copy shall fail and return a non-zero value. ]*/
static void int64_t_copy_called_with_NULL_src_value_fails(void)
{
    // arrange
    int64_t dst;

    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int64_t).copy(&dst, NULL);

    // assert
    POOR_MANS_ASSERT(result != 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_007: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int64_t).copy shall copy the bytes of the int64_t value from the address pointed by src_value to dst_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_008: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int64_t).copy shall succeed and return 0. ]*/
static void int64_t_copy_succeeds(void)
{
    // arrange
    int64_t src = 42;
    int64_t dst = 43;

    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int64_t).copy(&dst, &src);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(dst == 42);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_007: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int64_t).copy shall copy the bytes of the int64_t value from the address pointed by src_value to dst_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_008: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int64_t).copy shall succeed and return 0. ]*/
static void int64_t_copy_succeeds_2(void)
{
    // arrange
    int64_t src = INT64_MAX;
    int64_t dst = 43;

    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int64_t).copy(&dst, &src);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(dst == INT64_MAX);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int64_t).free */

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_009: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int64_t).free shall return. ]*/
static void int64_t_free_returns(void)
{
    // arrange
    int64_t value = INT64_MAX;

    setup_mocks();

    // act
    LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int64_t).free(&value);

    // assert
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int64_t).get_type */

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_010: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int64_t).get_type shall return the property type LOG_CONTEXT_PROPERTY_TYPE_int64_t. ]*/
static void int64_t_get_type_returns_LOG_CONTEXT_PROPERTY_TYPE_int64_t(void)
{
    // arrange
    setup_mocks();

    // act
    LOG_CONTEXT_PROPERTY_TYPE result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int64_t).get_type();

    // assert
    POOR_MANS_ASSERT(result == LOG_CONTEXT_PROPERTY_TYPE_int64_t);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* LOG_CONTEXT_PROPERTY_TYPE_INIT(int64_t) */

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_081: [ If dst_value is NULL, LOG_CONTEXT_PROPERTY_TYPE_INIT(int64_t) shall fail and return a non-zero value. ]*/
static void MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(int64_t), _with_NULL_dst_value_fails)(void)
{
    // arrange
    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_INIT(int64_t)(NULL, 42);

    // assert
    POOR_MANS_ASSERT(result != 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_082: [ LOG_CONTEXT_PROPERTY_TYPE_INIT(int64_t) shall copy the src_value bytes of the int64_t to dst_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_083: [ LOG_CONTEXT_PROPERTY_TYPE_INIT(int64_t) shall succeed and return 0. ]*/
static void MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(int64_t),_with_int64_t_min_value_succeeds)(void)
{
    // arrange
    int64_t dst = 43;
    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_INIT(int64_t)(&dst, INT64_MIN);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(dst == INT64_MIN);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_082: [ LOG_CONTEXT_PROPERTY_TYPE_INIT(int64_t) shall copy the src_value bytes of the int64_t to dst_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_083: [ LOG_CONTEXT_PROPERTY_TYPE_INIT(int64_t) shall succeed and return 0. ]*/
static void MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(int64_t),_with_int64_t_max_value_succeeds)(void)
{
    // arrange
    int64_t dst = 43;
    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_INIT(int64_t)(&dst, INT64_MAX);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(dst == INT64_MAX);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(int64_t) */

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_084: [ LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(int64_t) shall return sizeof(int64_t). ]*/
static void MU_C2(LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(int64_t), _succeeds)(void)
{
    // arrange
    setup_mocks();

    // act
    size_t result = LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(int64_t)();

    // assert
    POOR_MANS_ASSERT(result == sizeof(int64_t));
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint64_t).to_string */

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_011: [ If property_value is NULL, LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint64_t).to_string shall fail and return a negative value. ]*/
static void uint64_t_to_string_with_NULL_value_fails(void)
{
    // arrange
    char buffer[TEST_BUFFER_SIZE];
    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint64_t).to_string(NULL, buffer, TEST_BUFFER_SIZE);

    // assert
    POOR_MANS_ASSERT(result < 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_012: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint64_t).to_string shall call snprintf with buffer, buffer_length and format string PRIu64 and pass in the values list the uint64_t value pointed to be property_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_013: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint64_t).to_string shall succeed and return the result of snprintf. ]*/
static void uint64_t_to_string_succeeds(void)
{
    // arrange
    char buffer[TEST_BUFFER_SIZE];
    uint64_t uint64_t_value = 0;

    setup_mocks();
    setup_expected_snprintf_call();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint64_t).to_string(&uint64_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 1);
    POOR_MANS_ASSERT(strcmp(expected_calls[0].snprintf_call.captured_format_arg, "%" PRIu64) == 0);
    POOR_MANS_ASSERT(strcmp(buffer, "0") == 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_012: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint64_t).to_string shall call snprintf with buffer, buffer_length and format string PRIu64 and pass in the values list the uint64_t value pointed to be property_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_013: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint64_t).to_string shall succeed and return the result of snprintf. ]*/
static void uint64_t_to_string_UINT64_MAX_succeeds(void)
{
    // arrange
    char buffer[TEST_BUFFER_SIZE];
    uint64_t uint64_t_value = UINT64_MAX;

    setup_mocks();
    setup_expected_snprintf_call();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint64_t).to_string(&uint64_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 20);
    POOR_MANS_ASSERT(strcmp(expected_calls[0].snprintf_call.captured_format_arg, "%" PRIu64) == 0);
    POOR_MANS_ASSERT(strcmp(buffer, "18446744073709551615") == 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_012: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint64_t).to_string shall call snprintf with buffer, buffer_length and format string PRIu64 and pass in the values list the uint64_t value pointed to be property_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_013: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint64_t).to_string shall succeed and return the result of snprintf. ]*/
static void uint64_t_to_string_with_truncation_succeeds(void)
{
    // arrange
    char buffer[2];
    uint64_t uint64_t_value = 42;

    setup_mocks();
    setup_expected_snprintf_call();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint64_t).to_string(&uint64_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 2);
    POOR_MANS_ASSERT(strcmp(expected_calls[0].snprintf_call.captured_format_arg, "%" PRIu64) == 0);
    POOR_MANS_ASSERT(strcmp(buffer, "4") == 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_012: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint64_t).to_string shall call snprintf with buffer, buffer_length and format string PRIu64 and pass in the values list the uint64_t value pointed to be property_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_013: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint64_t).to_string shall succeed and return the result of snprintf. ]*/
static void uint64_t_to_string_with_just_enough_big_buffer_succeeds(void)
{
    // arrange
    char buffer[2];
    uint64_t uint64_t_value = 1;

    setup_mocks();
    setup_expected_snprintf_call();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint64_t).to_string(&uint64_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 1);
    POOR_MANS_ASSERT(strcmp(expected_calls[0].snprintf_call.captured_format_arg, "%" PRIu64) == 0);
    POOR_MANS_ASSERT(strcmp(buffer, "1") == 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_014: [ If any error is encountered, LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint64_t).to_string shall fail and return a negative value. ]*/
static void when_snprintf_fails_uint64_t_to_string_also_fails(void)
{
    // arrange
    char buffer[TEST_BUFFER_SIZE];
    uint64_t uint64_t_value = 42;

    setup_mocks();
    expected_calls[0].mock_call_type = MOCK_CALL_TYPE_snprintf;
    expected_calls[0].snprintf_call.call_result = -1;
    expected_calls[0].snprintf_call.override_result = true;
    expected_call_count = 1;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint64_t).to_string(&uint64_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result < 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint64_t).copy */

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_015: [ If src_value is NULL, LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint64_t).copy shall fail and return a non-zero value. ]*/
static void uint64_t_copy_called_with_NULL_dst_value_fails(void)
{
    // arrange
    uint64_t src = 42;

    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint64_t).copy(NULL, &src);

    // assert
    POOR_MANS_ASSERT(result != 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_016: [ If dst_value is NULL, LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint64_t).copy shall fail and return a non-zero value. ]*/
static void uint64_t_copy_called_with_NULL_src_value_fails(void)
{
    // arrange
    uint64_t dst;

    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint64_t).copy(&dst, NULL);

    // assert
    POOR_MANS_ASSERT(result != 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_017: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint64_t).copy shall copy the bytes of the uint64_t value from the address pointed by src_value to dst_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_018: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint64_t).copy shall succeed and return 0. ]*/
static void uint64_t_copy_succeeds(void)
{
    // arrange
    uint64_t src = 42;
    uint64_t dst = 43;

    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint64_t).copy(&dst, &src);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(dst == 42);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_017: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint64_t).copy shall copy the bytes of the uint64_t value from the address pointed by src_value to dst_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_018: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint64_t).copy shall succeed and return 0. ]*/
static void uint64_t_copy_succeeds_2(void)
{
    // arrange
    uint64_t src = UINT64_MAX;
    uint64_t dst = 43;

    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint64_t).copy(&dst, &src);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(dst == UINT64_MAX);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint64_t).free */

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_019: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint64_t).free shall return. ]*/
static void uint64_t_free_returns(void)
{
    // arrange
    uint64_t value = UINT64_MAX;

    setup_mocks();

    // act
    LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint64_t).free(&value);

    // assert
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint64_t).get_type */

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_020: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint64_t).get_type shall return the property type LOG_CONTEXT_PROPERTY_TYPE_uint64_t. ]*/
static void uint64_t_get_type_returns_LOG_CONTEXT_PROPERTY_TYPE_uint64_t(void)
{
    // arrange
    setup_mocks();

    // act
    LOG_CONTEXT_PROPERTY_TYPE result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint64_t).get_type();

    // assert
    POOR_MANS_ASSERT(result == LOG_CONTEXT_PROPERTY_TYPE_uint64_t);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* LOG_CONTEXT_PROPERTY_TYPE_INIT(uint64_t) */

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_085: [ If dst_value is NULL, LOG_CONTEXT_PROPERTY_TYPE_INIT(uint64_t) shall fail and return a non-zero value. ]*/
static void MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(uint64_t), _with_NULL_dst_value_fails)(void)
{
    // arrange
    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_INIT(uint64_t)(NULL, 42);

    // assert
    POOR_MANS_ASSERT(result != 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_086: [ LOG_CONTEXT_PROPERTY_TYPE_INIT(uint64_t) shall copy the src_value bytes of the uint64_t to dst_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_087: [ LOG_CONTEXT_PROPERTY_TYPE_INIT(uint64_t) shall succeed and return 0. ]*/
static void MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(uint64_t), _with_uint64_t_min_value_succeeds)(void)
{
    // arrange
    uint64_t dst = 43;
    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_INIT(uint64_t)(&dst, 0);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(dst == 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_086: [ LOG_CONTEXT_PROPERTY_TYPE_INIT(uint64_t) shall copy the src_value bytes of the uint64_t to dst_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_087: [ LOG_CONTEXT_PROPERTY_TYPE_INIT(uint64_t) shall succeed and return 0. ]*/
static void MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(uint64_t), _with_uint64_t_max_value_succeeds)(void)
{
    // arrange
    uint64_t dst = 43;
    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_INIT(uint64_t)(&dst, UINT64_MAX);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(dst == UINT64_MAX);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(uint64_t) */

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_088: [ LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(uint64_t) shall return sizeof(uint64_t). ]*/
static void MU_C2(LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(uint64_t), _succeeds)(void)
{
    // arrange
    setup_mocks();

    // act
    size_t result = LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(uint64_t)();

    // assert
    POOR_MANS_ASSERT(result == sizeof(uint64_t));
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int32_t).to_string */

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_021: [ If property_value is NULL, LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int32_t).to_string shall fail and return a negative value. ]*/
static void int32_t_to_string_with_NULL_value_fails(void)
{
    // arrange
    char buffer[TEST_BUFFER_SIZE];
    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int32_t).to_string(NULL, buffer, TEST_BUFFER_SIZE);

    // assert
    POOR_MANS_ASSERT(result < 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_022: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int32_t).to_string shall call snprintf with buffer, buffer_length and format string PRId32 and pass in the values list the int32_t value pointed to be property_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_023: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int32_t).to_string shall succeed and return the result of snprintf. ]*/
static void int32_t_to_string_succeeds(void)
{
    // arrange
    char buffer[TEST_BUFFER_SIZE];
    int32_t int32_t_value = 0;

    setup_mocks();
    setup_expected_snprintf_call();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int32_t).to_string(&int32_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 1);
    POOR_MANS_ASSERT(strcmp(expected_calls[0].snprintf_call.captured_format_arg, "%" PRId32) == 0);
    POOR_MANS_ASSERT(strcmp(buffer, "0") == 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_022: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int32_t).to_string shall call snprintf with buffer, buffer_length and format string PRId32 and pass in the values list the int32_t value pointed to be property_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_023: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int32_t).to_string shall succeed and return the result of snprintf. ]*/
static void int32_t_to_string_INT32_MAX_succeeds(void)
{
    // arrange
    char buffer[TEST_BUFFER_SIZE];
    int32_t int32_t_value = INT32_MAX;

    setup_mocks();
    setup_expected_snprintf_call();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int32_t).to_string(&int32_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 10);
    POOR_MANS_ASSERT(strcmp(expected_calls[0].snprintf_call.captured_format_arg, "%" PRId32) == 0);
    POOR_MANS_ASSERT(strcmp(buffer, "2147483647") == 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_022: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int32_t).to_string shall call snprintf with buffer, buffer_length and format string PRId32 and pass in the values list the int32_t value pointed to be property_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_023: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int32_t).to_string shall succeed and return the result of snprintf. ]*/
static void int32_t_to_string_with_truncation_succeeds(void)
{
    // arrange
    char buffer[2];
    int32_t int32_t_value = 42;

    setup_mocks();
    setup_expected_snprintf_call();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int32_t).to_string(&int32_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 2);
    POOR_MANS_ASSERT(strcmp(buffer, "4") == 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_022: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int32_t).to_string shall call snprintf with buffer, buffer_length and format string PRId32 and pass in the values list the int32_t value pointed to be property_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_023: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int32_t).to_string shall succeed and return the result of snprintf. ]*/
static void int32_t_to_string_with_truncation_minus_1_succeeds(void)
{
    // arrange
    char buffer[2];
    int32_t int32_t_value = -1;

    setup_mocks();
    setup_expected_snprintf_call();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int32_t).to_string(&int32_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 2);
    POOR_MANS_ASSERT(strcmp(buffer, "-") == 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_022: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int32_t).to_string shall call snprintf with buffer, buffer_length and format string PRId32 and pass in the values list the int32_t value pointed to be property_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_023: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int32_t).to_string shall succeed and return the result of snprintf. ]*/
static void int32_t_to_string_with_negative_value_succeeds(void)
{
    // arrange
    char buffer[TEST_BUFFER_SIZE];
    int32_t int32_t_value = -1;

    setup_mocks();
    setup_expected_snprintf_call();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int32_t).to_string(&int32_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result > 0);
    POOR_MANS_ASSERT(strcmp(expected_calls[0].snprintf_call.captured_format_arg, "%" PRId32) == 0);
    POOR_MANS_ASSERT(strcmp(buffer, "-1") == 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_022: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int32_t).to_string shall call snprintf with buffer, buffer_length and format string PRId32 and pass in the values list the int32_t value pointed to be property_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_023: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int32_t).to_string shall succeed and return the result of snprintf. ]*/
static void int32_t_to_string_with_just_enough_big_buffer_succeeds(void)
{
    // arrange
    char buffer[2];
    int32_t int32_t_value = 1;

    setup_mocks();
    setup_expected_snprintf_call();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int32_t).to_string(&int32_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 1);
    POOR_MANS_ASSERT(strcmp(expected_calls[0].snprintf_call.captured_format_arg, "%" PRId32) == 0);
    POOR_MANS_ASSERT(strcmp(buffer, "1") == 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_024: [ If any error is encountered, LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int32_t).to_string shall fail and return a negative value. ]*/
static void when_snprintf_fails_int32_t_to_string_also_fails(void)
{
    // arrange
    char buffer[TEST_BUFFER_SIZE];
    int32_t int32_t_value = -1;

    setup_mocks();
    expected_calls[0].mock_call_type = MOCK_CALL_TYPE_snprintf;
    expected_calls[0].snprintf_call.call_result = -1;
    expected_calls[0].snprintf_call.override_result = true;
    expected_call_count = 1;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int32_t).to_string(&int32_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result < 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int32_t).copy */

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_025: [ If src_value is NULL, LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int32_t).copy shall fail and return a non-zero value. ]*/
static void int32_t_copy_called_with_NULL_dst_value_fails(void)
{
    // arrange
    int32_t src = 42;

    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int32_t).copy(NULL, &src);

    // assert
    POOR_MANS_ASSERT(result != 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_026: [ If dst_value is NULL, LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int32_t).copy shall fail and return a non-zero value. ]*/
static void int32_t_copy_called_with_NULL_src_value_fails(void)
{
    // arrange
    int32_t dst;

    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int32_t).copy(&dst, NULL);

    // assert
    POOR_MANS_ASSERT(result != 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_027: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int32_t).copy shall copy the bytes of the int32_t value from the address pointed by src_value to dst_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_028: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int32_t).copy shall succeed and return 0. ]*/
static void int32_t_copy_succeeds(void)
{
    // arrange
    int32_t src = 42;
    int32_t dst = 43;

    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int32_t).copy(&dst, &src);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(dst == 42);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_027: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int32_t).copy shall copy the bytes of the int32_t value from the address pointed by src_value to dst_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_028: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int32_t).copy shall succeed and return 0. ]*/
static void int32_t_copy_succeeds_2(void)
{
    // arrange
    int32_t src = INT32_MAX;
    int32_t dst = 43;

    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int32_t).copy(&dst, &src);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(dst == INT32_MAX);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int32_t).free */

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_029: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int32_t).free shall return. ]*/
static void int32_t_free_returns(void)
{
    // arrange
    int32_t value = INT32_MAX;

    setup_mocks();

    // act
    LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int32_t).free(&value);

    // assert
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int32_t).get_type */

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_030: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int32_t).get_type shall return the property type LOG_CONTEXT_PROPERTY_TYPE_int32_t. ]*/
static void int32_t_get_type_returns_LOG_CONTEXT_PROPERTY_TYPE_int32_t(void)
{
    // arrange
    setup_mocks();

    // act
    LOG_CONTEXT_PROPERTY_TYPE result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int32_t).get_type();

    // assert
    POOR_MANS_ASSERT(result == LOG_CONTEXT_PROPERTY_TYPE_int32_t);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* LOG_CONTEXT_PROPERTY_TYPE_INIT(int32_t) */

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_089: [ If dst_value is NULL, LOG_CONTEXT_PROPERTY_TYPE_INIT(int32_t) shall fail and return a non-zero value. ]*/
static void MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(int32_t), _with_NULL_dst_value_fails)(void)
{
    // arrange
    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_INIT(int32_t)(NULL, 42);

    // assert
    POOR_MANS_ASSERT(result != 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_090: [ LOG_CONTEXT_PROPERTY_TYPE_INIT(int32_t) shall copy the src_value bytes of the int32_t to dst_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_091: [ LOG_CONTEXT_PROPERTY_TYPE_INIT(int32_t) shall succeed and return 0. ]*/
static void MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(int32_t), _with_int32_t_min_value_succeeds)(void)
{
    // arrange
    int32_t dst = 43;
    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_INIT(int32_t)(&dst, INT32_MIN);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(dst == INT32_MIN);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_090: [ LOG_CONTEXT_PROPERTY_TYPE_INIT(int32_t) shall copy the src_value bytes of the int32_t to dst_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_091: [ LOG_CONTEXT_PROPERTY_TYPE_INIT(int32_t) shall succeed and return 0. ]*/
static void MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(int32_t), _with_int32_t_max_value_succeeds)(void)
{
    // arrange
    int32_t dst = 43;
    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_INIT(int32_t)(&dst, INT32_MAX);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(dst == INT32_MAX);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(int32_t) */

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_092: [ LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(int32_t) shall return sizeof(int32_t). ]*/
static void MU_C2(LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(int32_t), _succeeds)(void)
{
    // arrange
    setup_mocks();

    // act
    size_t result = LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(int32_t)();

    // assert
    POOR_MANS_ASSERT(result == sizeof(int32_t));
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint32_t).to_string */

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_031: [ If property_value is NULL, LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint32_t).to_string shall fail and return a negative value. ]*/
static void uint32_t_to_string_with_NULL_value_fails(void)
{
    // arrange
    char buffer[TEST_BUFFER_SIZE];
    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint32_t).to_string(NULL, buffer, TEST_BUFFER_SIZE);

    // assert
    POOR_MANS_ASSERT(result < 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_032: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint32_t).to_string shall call snprintf with buffer, buffer_length and format string PRIu32 and pass in the values list the uint32_t_t value pointed to be property_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_033: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint32_t).to_string shall succeed and return the result of snprintf. ]*/
static void uint32_t_to_string_succeeds(void)
{
    // arrange
    char buffer[TEST_BUFFER_SIZE];
    uint32_t uint32_t_value = 0;

    setup_mocks();
    setup_expected_snprintf_call();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint32_t).to_string(&uint32_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 1);
    POOR_MANS_ASSERT(strcmp(expected_calls[0].snprintf_call.captured_format_arg, "%" PRIu32) == 0);
    POOR_MANS_ASSERT(strcmp(buffer, "0") == 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_032: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint32_t).to_string shall call snprintf with buffer, buffer_length and format string PRIu32 and pass in the values list the uint32_t_t value pointed to be property_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_033: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint32_t).to_string shall succeed and return the result of snprintf. ]*/
static void uint32_t_to_string_UINT32_MAX_succeeds(void)
{
    // arrange
    char buffer[TEST_BUFFER_SIZE];
    uint32_t uint32_t_value = UINT32_MAX;

    setup_mocks();
    setup_expected_snprintf_call();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint32_t).to_string(&uint32_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 10);
    POOR_MANS_ASSERT(strcmp(expected_calls[0].snprintf_call.captured_format_arg, "%" PRIu32) == 0);
    POOR_MANS_ASSERT(strcmp(buffer, "4294967295") == 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_032: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint32_t).to_string shall call snprintf with buffer, buffer_length and format string PRIu32 and pass in the values list the uint32_t_t value pointed to be property_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_033: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint32_t).to_string shall succeed and return the result of snprintf. ]*/
static void uint32_t_to_string_with_truncation_succeeds(void)
{
    // arrange
    char buffer[2];
    uint32_t uint32_t_value = 42;

    setup_mocks();
    setup_expected_snprintf_call();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint32_t).to_string(&uint32_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 2);
    POOR_MANS_ASSERT(strcmp(expected_calls[0].snprintf_call.captured_format_arg, "%" PRIu32) == 0);
    POOR_MANS_ASSERT(strcmp(buffer, "4") == 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_032: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint32_t).to_string shall call snprintf with buffer, buffer_length and format string PRIu32 and pass in the values list the uint32_t_t value pointed to be property_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_033: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint32_t).to_string shall succeed and return the result of snprintf. ]*/
static void uint32_t_to_string_with_just_enough_big_buffer_succeeds(void)
{
    // arrange
    char buffer[2];
    uint32_t uint32_t_value = 1;

    setup_mocks();
    setup_expected_snprintf_call();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint32_t).to_string(&uint32_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 1);
    POOR_MANS_ASSERT(strcmp(expected_calls[0].snprintf_call.captured_format_arg, "%" PRIu32) == 0);
    POOR_MANS_ASSERT(strcmp(buffer, "1") == 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_034: [ If any error is encountered, LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint32_t).to_string shall fail and return a negative value. ]*/
static void when_snprintf_fails_uint32_t_to_string_also_fails(void)
{
    // arrange
    char buffer[TEST_BUFFER_SIZE];
    uint32_t uint32_t_value = 42;

    setup_mocks();
    expected_calls[0].mock_call_type = MOCK_CALL_TYPE_snprintf;
    expected_calls[0].snprintf_call.call_result = -1;
    expected_calls[0].snprintf_call.override_result = true;
    expected_call_count = 1;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint32_t).to_string(&uint32_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result < 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint32_t).copy */

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_035: [ If src_value is NULL, LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint32_t).copy shall fail and return a non-zero value. ]*/
static void uint32_t_copy_called_with_NULL_dst_value_fails(void)
{
    // arrange
    uint32_t src = 42;

    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint32_t).copy(NULL, &src);

    // assert
    POOR_MANS_ASSERT(result != 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_036: [ If dst_value is NULL, LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint32_t).copy shall fail and return a non-zero value. ]*/
static void uint32_t_copy_called_with_NULL_src_value_fails(void)
{
    // arrange
    uint32_t dst;

    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint32_t).copy(&dst, NULL);

    // assert
    POOR_MANS_ASSERT(result != 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_037: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint32_t).copy shall copy the bytes of the uint32_t_t value from the address pointed by src_value to dst_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_038: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint32_t).copy shall succeed and return 0. ]*/
static void uint32_t_copy_succeeds(void)
{
    // arrange
    uint32_t src = 42;
    uint32_t dst = 43;

    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint32_t).copy(&dst, &src);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(dst == 42);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_037: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint32_t).copy shall copy the bytes of the uint32_t_t value from the address pointed by src_value to dst_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_038: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint32_t).copy shall succeed and return 0. ]*/
static void uint32_t_copy_succeeds_2(void)
{
    // arrange
    uint32_t src = UINT32_MAX;
    uint32_t dst = 43;

    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint32_t).copy(&dst, &src);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(dst == UINT32_MAX);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint32_t).free */

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_039: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint32_t).free shall return. ]*/
static void uint32_t_free_returns(void)
{
    // arrange
    uint32_t value = UINT32_MAX;

    setup_mocks();

    // act
    LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint32_t).free(&value);

    // assert
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint32_t).get_type */

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_040: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint32_t).get_type shall return the property type LOG_CONTEXT_PROPERTY_TYPE_uint32_t_t. ]*/
static void uint32_t_get_type_returns_LOG_CONTEXT_PROPERTY_TYPE_uint32_t(void)
{
    // arrange
    setup_mocks();

    // act
    LOG_CONTEXT_PROPERTY_TYPE result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint32_t).get_type();

    // assert
    POOR_MANS_ASSERT(result == LOG_CONTEXT_PROPERTY_TYPE_uint32_t);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* LOG_CONTEXT_PROPERTY_TYPE_INIT(uint32_t) */

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_093: [ If dst_value is NULL, LOG_CONTEXT_PROPERTY_TYPE_INIT(uint32_t) shall fail and return a non-zero value. ]*/
static void MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(uint32_t), _with_NULL_dst_value_fails)(void)
{
    // arrange
    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_INIT(uint32_t)(NULL, 42);

    // assert
    POOR_MANS_ASSERT(result != 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_094: [ LOG_CONTEXT_PROPERTY_TYPE_INIT(uint32_t) shall copy the src_value bytes of the uint32_t to dst_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_095: [ LOG_CONTEXT_PROPERTY_TYPE_INIT(uint32_t) shall succeed and return 0. ]*/
static void MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(uint32_t), _with_uint32_t_min_value_succeeds)(void)
{
    // arrange
    uint32_t dst = 43;
    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_INIT(uint32_t)(&dst, 0);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(dst == 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_094: [ LOG_CONTEXT_PROPERTY_TYPE_INIT(uint32_t) shall copy the src_value bytes of the uint32_t to dst_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_095: [ LOG_CONTEXT_PROPERTY_TYPE_INIT(uint32_t) shall succeed and return 0. ]*/
static void MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(uint32_t), _with_uint32_t_max_value_succeeds)(void)
{
    // arrange
    uint32_t dst = 43;
    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_INIT(uint32_t)(&dst, UINT32_MAX);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(dst == UINT32_MAX);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(uint32_t) */

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_096: [ LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(uint32_t) shall return sizeof(uint32_t). ]*/
static void MU_C2(LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(uint32_t), _succeeds)(void)
{
    // arrange
    setup_mocks();

    // act
    size_t result = LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(uint32_t)();

    // assert
    POOR_MANS_ASSERT(result == sizeof(uint32_t));
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int16_t).to_string */

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_041: [ If property_value is NULL, LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int16_t).to_string shall fail and return a negative value. ]*/
static void int16_t_to_string_with_NULL_value_fails(void)
{
    // arrange
    char buffer[TEST_BUFFER_SIZE];
    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int16_t).to_string(NULL, buffer, TEST_BUFFER_SIZE);

    // assert
    POOR_MANS_ASSERT(result < 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_042: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int16_t).to_string shall call snprintf with buffer, buffer_length and format string PRId16 and pass in the values list the int16_t value pointed to be property_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_043: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int16_t).to_string shall succeed and return the result of snprintf. ]*/
static void int16_t_to_string_succeeds(void)
{
    // arrange
    char buffer[TEST_BUFFER_SIZE];
    int16_t int16_t_value = 0;

    setup_mocks();
    setup_expected_snprintf_call();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int16_t).to_string(&int16_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 1);
    POOR_MANS_ASSERT(strcmp(expected_calls[0].snprintf_call.captured_format_arg, "%" PRId16) == 0);
    POOR_MANS_ASSERT(strcmp(buffer, "0") == 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_042: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int16_t).to_string shall call snprintf with buffer, buffer_length and format string PRId16 and pass in the values list the int16_t value pointed to be property_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_043: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int16_t).to_string shall succeed and return the result of snprintf. ]*/
static void int16_t_to_string_INT16_MAX_succeeds(void)
{
    // arrange
    char buffer[TEST_BUFFER_SIZE];
    int16_t int16_t_value = INT16_MAX;

    setup_mocks();
    setup_expected_snprintf_call();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int16_t).to_string(&int16_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 5);
    POOR_MANS_ASSERT(strcmp(expected_calls[0].snprintf_call.captured_format_arg, "%" PRId16) == 0);
    POOR_MANS_ASSERT(strcmp(buffer, "32767") == 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_042: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int16_t).to_string shall call snprintf with buffer, buffer_length and format string PRId16 and pass in the values list the int16_t value pointed to be property_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_043: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int16_t).to_string shall succeed and return the result of snprintf. ]*/
static void int16_t_to_string_with_truncation_succeeds(void)
{
    // arrange
    char buffer[2];
    int16_t int16_t_value = 42;

    setup_mocks();
    setup_expected_snprintf_call();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int16_t).to_string(&int16_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 2);
    POOR_MANS_ASSERT(strcmp(expected_calls[0].snprintf_call.captured_format_arg, "%" PRId16) == 0);
    POOR_MANS_ASSERT(strcmp(buffer, "4") == 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_042: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int16_t).to_string shall call snprintf with buffer, buffer_length and format string PRId16 and pass in the values list the int16_t value pointed to be property_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_043: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int16_t).to_string shall succeed and return the result of snprintf. ]*/
static void int16_t_to_string_with_truncation_minus_1_succeeds(void)
{
    // arrange
    char buffer[2];
    int16_t int16_t_value = -1;

    setup_mocks();
    setup_expected_snprintf_call();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int16_t).to_string(&int16_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 2);
    POOR_MANS_ASSERT(strcmp(expected_calls[0].snprintf_call.captured_format_arg, "%" PRId16) == 0);
    POOR_MANS_ASSERT(strcmp(buffer, "-") == 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_042: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int16_t).to_string shall call snprintf with buffer, buffer_length and format string PRId16 and pass in the values list the int16_t value pointed to be property_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_043: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int16_t).to_string shall succeed and return the result of snprintf. ]*/
static void int16_t_to_string_with_negative_value_succeeds(void)
{
    // arrange
    char buffer[TEST_BUFFER_SIZE];
    int16_t int16_t_value = -1;

    setup_mocks();
    setup_expected_snprintf_call();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int16_t).to_string(&int16_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result > 0);
    POOR_MANS_ASSERT(strcmp(expected_calls[0].snprintf_call.captured_format_arg, "%" PRId16) == 0);
    POOR_MANS_ASSERT(strcmp(buffer, "-1") == 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_042: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int16_t).to_string shall call snprintf with buffer, buffer_length and format string PRId16 and pass in the values list the int16_t value pointed to be property_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_043: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int16_t).to_string shall succeed and return the result of snprintf. ]*/
static void int16_t_to_string_with_just_enough_big_buffer_succeeds(void)
{
    // arrange
    char buffer[2];
    int16_t int16_t_value = 1;

    setup_mocks();
    setup_expected_snprintf_call();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int16_t).to_string(&int16_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 1);
    POOR_MANS_ASSERT(strcmp(expected_calls[0].snprintf_call.captured_format_arg, "%" PRId16) == 0);
    POOR_MANS_ASSERT(strcmp(buffer, "1") == 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_044: [ If any error is encountered, LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int16_t).to_string shall fail and return a negative value. ]*/
static void when_snprintf_fails_int16_t_to_string_also_fails(void)
{
    // arrange
    char buffer[TEST_BUFFER_SIZE];
    int16_t int16_t_value = -1;

    setup_mocks();
    expected_calls[0].mock_call_type = MOCK_CALL_TYPE_snprintf;
    expected_calls[0].snprintf_call.call_result = -1;
    expected_calls[0].snprintf_call.override_result = true;
    expected_call_count = 1;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int16_t).to_string(&int16_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result < 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int16_t).copy */

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_045: [ If src_value is NULL, LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int16_t).copy shall fail and return a non-zero value. ]*/
static void int16_t_copy_called_with_NULL_dst_value_fails(void)
{
    // arrange
    int16_t src = 42;

    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int16_t).copy(NULL, &src);

    // assert
    POOR_MANS_ASSERT(result != 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_046: [ If dst_value is NULL, LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int16_t).copy shall fail and return a non-zero value. ]*/
static void int16_t_copy_called_with_NULL_src_value_fails(void)
{
    // arrange
    int16_t dst;

    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int16_t).copy(&dst, NULL);

    // assert
    POOR_MANS_ASSERT(result != 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_047: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int16_t).copy shall copy the bytes of the int16_t value from the address pointed by src_value to dst_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_048: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int16_t).copy shall succeed and return 0. ]*/
static void int16_t_copy_succeeds(void)
{
    // arrange
    int16_t src = 42;
    int16_t dst = 43;

    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int16_t).copy(&dst, &src);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(dst == 42);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_047: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int16_t).copy shall copy the bytes of the int16_t value from the address pointed by src_value to dst_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_048: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int16_t).copy shall succeed and return 0. ]*/
static void int16_t_copy_succeeds_2(void)
{
    // arrange
    int16_t src = INT16_MAX;
    int16_t dst = 43;

    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int16_t).copy(&dst, &src);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(dst == INT16_MAX);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int16_t).free */

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_049: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int16_t).free shall return. ]*/
static void int16_t_free_returns(void)
{
    // arrange
    int16_t value = INT16_MAX;

    setup_mocks();

    // act
    LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int16_t).free(&value);

    // assert
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int16_t).get_type */

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_050: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int16_t).get_type shall return the property type LOG_CONTEXT_PROPERTY_TYPE_int16_t. ]*/
static void int16_t_get_type_returns_LOG_CONTEXT_PROPERTY_TYPE_int16_t(void)
{
    // arrange
    setup_mocks();

    // act
    LOG_CONTEXT_PROPERTY_TYPE result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int16_t).get_type();

    // assert
    POOR_MANS_ASSERT(result == LOG_CONTEXT_PROPERTY_TYPE_int16_t);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* LOG_CONTEXT_PROPERTY_TYPE_INIT(int16_t) */

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_097: [ If dst_value is NULL, LOG_CONTEXT_PROPERTY_TYPE_INIT(int16_t) shall fail and return a non-zero value. ]*/
static void MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(int16_t), _with_NULL_dst_value_fails)(void)
{
    // arrange
    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_INIT(int16_t)(NULL, 42);

    // assert
    POOR_MANS_ASSERT(result != 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_098: [ LOG_CONTEXT_PROPERTY_TYPE_INIT(int16_t) shall copy the src_value bytes of the int16_t to dst_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_099: [ LOG_CONTEXT_PROPERTY_TYPE_INIT(int16_t) shall succeed and return 0. ]*/
static void MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(int16_t), _with_int16_t_min_value_succeeds)(void)
{
    // arrange
    int16_t dst = 43;
    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_INIT(int16_t)(&dst, INT16_MIN);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(dst == INT16_MIN);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_098: [ LOG_CONTEXT_PROPERTY_TYPE_INIT(int16_t) shall copy the src_value bytes of the int16_t to dst_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_099: [ LOG_CONTEXT_PROPERTY_TYPE_INIT(int16_t) shall succeed and return 0. ]*/
static void MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(int16_t), _with_int16_t_max_value_succeeds)(void)
{
    // arrange
    int16_t dst = 43;
    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_INIT(int16_t)(&dst, INT16_MAX);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(dst == INT16_MAX);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(int16_t) */

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_100: [ LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(int16_t) shall return sizeof(int16_t). ]*/
static void MU_C2(LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(int16_t), _succeeds)(void)
{
    // arrange
    setup_mocks();

    // act
    size_t result = LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(int16_t)();

    // assert
    POOR_MANS_ASSERT(result == sizeof(int16_t));
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint16_t).to_string */

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_051: [ If property_value is NULL, LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint16_t).to_string shall fail and return a negative value. ]*/
static void uint16_t_to_string_with_NULL_value_fails(void)
{
    // arrange
    char buffer[TEST_BUFFER_SIZE];
    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint16_t).to_string(NULL, buffer, TEST_BUFFER_SIZE);

    // assert
    POOR_MANS_ASSERT(result < 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_052: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint16_t).to_string shall call snprintf with buffer, buffer_length and format string PRIu16 and pass in the values list the uint16_t_t value pointed to be property_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_053: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint16_t).to_string shall succeed and return the result of snprintf. ]*/
static void uint16_t_to_string_succeeds(void)
{
    // arrange
    char buffer[TEST_BUFFER_SIZE];
    uint16_t uint16_t_value = 0;

    setup_mocks();
    setup_expected_snprintf_call();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint16_t).to_string(&uint16_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 1);
    POOR_MANS_ASSERT(strcmp(expected_calls[0].snprintf_call.captured_format_arg, "%" PRIu16) == 0);
    POOR_MANS_ASSERT(strcmp(buffer, "0") == 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_052: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint16_t).to_string shall call snprintf with buffer, buffer_length and format string PRIu16 and pass in the values list the uint16_t_t value pointed to be property_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_053: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint16_t).to_string shall succeed and return the result of snprintf. ]*/
static void uint16_t_to_string_UINT16_MAX_succeeds(void)
{
    // arrange
    char buffer[TEST_BUFFER_SIZE];
    uint16_t uint16_t_value = UINT16_MAX;

    setup_mocks();
    setup_expected_snprintf_call();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint16_t).to_string(&uint16_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 5);
    POOR_MANS_ASSERT(strcmp(expected_calls[0].snprintf_call.captured_format_arg, "%" PRIu16) == 0);
    POOR_MANS_ASSERT(strcmp(buffer, "65535") == 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_052: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint16_t).to_string shall call snprintf with buffer, buffer_length and format string PRIu16 and pass in the values list the uint16_t_t value pointed to be property_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_053: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint16_t).to_string shall succeed and return the result of snprintf. ]*/
static void uint16_t_to_string_with_truncation_succeeds(void)
{
    // arrange
    char buffer[2];
    uint16_t uint16_t_value = 42;

    setup_mocks();
    setup_expected_snprintf_call();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint16_t).to_string(&uint16_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 2);
    POOR_MANS_ASSERT(strcmp(expected_calls[0].snprintf_call.captured_format_arg, "%" PRIu16) == 0);
    POOR_MANS_ASSERT(strcmp(buffer, "4") == 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_052: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint16_t).to_string shall call snprintf with buffer, buffer_length and format string PRIu16 and pass in the values list the uint16_t_t value pointed to be property_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_053: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint16_t).to_string shall succeed and return the result of snprintf. ]*/
static void uint16_t_to_string_with_just_enough_big_buffer_succeeds(void)
{
    // arrange
    char buffer[2];
    uint16_t uint16_t_value = 1;

    setup_mocks();
    setup_expected_snprintf_call();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint16_t).to_string(&uint16_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 1);
    POOR_MANS_ASSERT(strcmp(expected_calls[0].snprintf_call.captured_format_arg, "%" PRIu16) == 0);
    POOR_MANS_ASSERT(strcmp(buffer, "1") == 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_054: [ If any error is encountered, LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint16_t).to_string shall fail and return a negative value. ]*/
static void when_snprintf_fails_uint16_t_to_string_also_fails(void)
{
    // arrange
    char buffer[TEST_BUFFER_SIZE];
    uint16_t uint16_t_value = 42;

    setup_mocks();
    expected_calls[0].mock_call_type = MOCK_CALL_TYPE_snprintf;
    expected_calls[0].snprintf_call.call_result = -1;
    expected_calls[0].snprintf_call.override_result = true;
    expected_call_count = 1;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint16_t).to_string(&uint16_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result < 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint16_t).copy */

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_055: [ If src_value is NULL, LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint16_t).copy shall fail and return a non-zero value. ]*/
static void uint16_t_copy_called_with_NULL_dst_value_fails(void)
{
    // arrange
    uint16_t src = 42;

    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint16_t).copy(NULL, &src);

    // assert
    POOR_MANS_ASSERT(result != 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_056: [ If dst_value is NULL, LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint16_t).copy shall fail and return a non-zero value. ]*/
static void uint16_t_copy_called_with_NULL_src_value_fails(void)
{
    // arrange
    uint16_t dst;

    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint16_t).copy(&dst, NULL);

    // assert
    POOR_MANS_ASSERT(result != 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_057: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint16_t).copy shall copy the bytes of the uint16_t_t value from the address pointed by src_value to dst_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_058: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint16_t).copy shall succeed and return 0. ]*/
static void uint16_t_copy_succeeds(void)
{
    // arrange
    uint16_t src = 42;
    uint16_t dst = 43;

    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint16_t).copy(&dst, &src);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(dst == 42);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_057: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint16_t).copy shall copy the bytes of the uint16_t_t value from the address pointed by src_value to dst_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_058: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint16_t).copy shall succeed and return 0. ]*/
static void uint16_t_copy_succeeds_2(void)
{
    // arrange
    uint16_t src = UINT16_MAX;
    uint16_t dst = 43;

    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint16_t).copy(&dst, &src);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(dst == UINT16_MAX);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint16_t).free */

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_059: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint16_t).free shall return. ]*/
static void uint16_t_free_returns(void)
{
    // arrange
    uint16_t value = UINT16_MAX;

    setup_mocks();

    // act
    LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint16_t).free(&value);

    // assert
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint16_t).get_type */

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_060: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint16_t).get_type shall return the property type LOG_CONTEXT_PROPERTY_TYPE_uint16_t_t. ]*/
static void uint16_t_get_type_returns_LOG_CONTEXT_PROPERTY_TYPE_uint16_t(void)
{
    // arrange
    setup_mocks();

    // act
    LOG_CONTEXT_PROPERTY_TYPE result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint16_t).get_type();

    // assert
    POOR_MANS_ASSERT(result == LOG_CONTEXT_PROPERTY_TYPE_uint16_t);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* LOG_CONTEXT_PROPERTY_TYPE_INIT(uint16_t) */

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_101: [ If dst_value is NULL, LOG_CONTEXT_PROPERTY_TYPE_INIT(uint16_t) shall fail and return a non-zero value. ]*/
static void MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(uint16_t), _with_NULL_dst_value_fails)(void)
{
    // arrange
    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_INIT(uint16_t)(NULL, 42);

    // assert
    POOR_MANS_ASSERT(result != 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_102: [ LOG_CONTEXT_PROPERTY_TYPE_INIT(uint16_t) shall copy the src_value bytes of the uint16_t to dst_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_103: [ LOG_CONTEXT_PROPERTY_TYPE_INIT(uint16_t) shall succeed and return 0. ]*/
static void MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(uint16_t), _with_uint16_t_min_value_succeeds)(void)
{
    // arrange
    uint16_t dst = 43;
    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_INIT(uint16_t)(&dst, 0);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(dst == 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_102: [ LOG_CONTEXT_PROPERTY_TYPE_INIT(uint16_t) shall copy the src_value bytes of the uint16_t to dst_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_103: [ LOG_CONTEXT_PROPERTY_TYPE_INIT(uint16_t) shall succeed and return 0. ]*/
static void MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(uint16_t), _with_uint16_t_max_value_succeeds)(void)
{
    // arrange
    uint16_t dst = 43;
    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_INIT(uint16_t)(&dst, UINT16_MAX);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(dst == UINT16_MAX);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(uint16_t) */

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_104: [ LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(uint16_t) shall return sizeof(uint16_t). ]*/
static void MU_C2(LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(uint16_t), _succeeds)(void)
{
    // arrange
    setup_mocks();

    // act
    size_t result = LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(uint16_t)();

    // assert
    POOR_MANS_ASSERT(result == sizeof(uint16_t));
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int8_t).to_string */

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_061: [ If property_value is NULL, LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int8_t).to_string shall fail and return a negative value. ]*/
static void int8_t_to_string_with_NULL_value_fails(void)
{
    // arrange
    char buffer[TEST_BUFFER_SIZE];
    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int8_t).to_string(NULL, buffer, TEST_BUFFER_SIZE);

    // assert
    POOR_MANS_ASSERT(result < 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_062: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int8_t).to_string shall call snprintf with buffer, buffer_length and format string PRId8 and pass in the values list the int8_t value pointed to be property_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_063: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int8_t).to_string shall succeed and return the result of snprintf. ]*/
static void int8_t_to_string_succeeds(void)
{
    // arrange
    char buffer[TEST_BUFFER_SIZE];
    int8_t int8_t_value = 0;

    setup_mocks();
    setup_expected_snprintf_call();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int8_t).to_string(&int8_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 1);
    POOR_MANS_ASSERT(strcmp(expected_calls[0].snprintf_call.captured_format_arg, "%" PRId8) == 0);
    POOR_MANS_ASSERT(strcmp(buffer, "0") == 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_062: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int8_t).to_string shall call snprintf with buffer, buffer_length and format string PRId8 and pass in the values list the int8_t value pointed to be property_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_063: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int8_t).to_string shall succeed and return the result of snprintf. ]*/
static void int8_t_to_string_INT8_MAX_succeeds(void)
{
    // arrange
    char buffer[TEST_BUFFER_SIZE];
    int8_t int8_t_value = INT8_MAX;

    setup_mocks();
    setup_expected_snprintf_call();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int8_t).to_string(&int8_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 3);
    POOR_MANS_ASSERT(strcmp(expected_calls[0].snprintf_call.captured_format_arg, "%" PRId8) == 0);
    POOR_MANS_ASSERT(strcmp(buffer, "127") == 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_062: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int8_t).to_string shall call snprintf with buffer, buffer_length and format string PRId8 and pass in the values list the int8_t value pointed to be property_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_063: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int8_t).to_string shall succeed and return the result of snprintf. ]*/
static void int8_t_to_string_with_truncation_succeeds(void)
{
    // arrange
    char buffer[2];
    int8_t int8_t_value = 42;

    setup_mocks();
    setup_expected_snprintf_call();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int8_t).to_string(&int8_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 2);
    POOR_MANS_ASSERT(strcmp(expected_calls[0].snprintf_call.captured_format_arg, "%" PRId8) == 0);
    POOR_MANS_ASSERT(strcmp(buffer, "4") == 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_062: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int8_t).to_string shall call snprintf with buffer, buffer_length and format string PRId8 and pass in the values list the int8_t value pointed to be property_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_063: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int8_t).to_string shall succeed and return the result of snprintf. ]*/
static void int8_t_to_string_with_truncation_minus_1_succeeds(void)
{
    // arrange
    char buffer[2];
    int8_t int8_t_value = -1;

    setup_mocks();
    setup_expected_snprintf_call();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int8_t).to_string(&int8_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 2);
    POOR_MANS_ASSERT(strcmp(expected_calls[0].snprintf_call.captured_format_arg, "%" PRId8) == 0);
    POOR_MANS_ASSERT(strcmp(buffer, "-") == 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_062: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int8_t).to_string shall call snprintf with buffer, buffer_length and format string PRId8 and pass in the values list the int8_t value pointed to be property_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_063: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int8_t).to_string shall succeed and return the result of snprintf. ]*/
static void int8_t_to_string_with_negative_value_succeeds(void)
{
    // arrange
    char buffer[TEST_BUFFER_SIZE];
    int8_t int8_t_value = -1;

    setup_mocks();
    setup_expected_snprintf_call();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int8_t).to_string(&int8_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result > 0);
    POOR_MANS_ASSERT(strcmp(expected_calls[0].snprintf_call.captured_format_arg, "%" PRId8) == 0);
    POOR_MANS_ASSERT(strcmp(buffer, "-1") == 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_062: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int8_t).to_string shall call snprintf with buffer, buffer_length and format string PRId8 and pass in the values list the int8_t value pointed to be property_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_063: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int8_t).to_string shall succeed and return the result of snprintf. ]*/
static void int8_t_to_string_with_just_enough_big_buffer_succeeds(void)
{
    // arrange
    char buffer[2];
    int8_t int8_t_value = 1;

    setup_mocks();
    setup_expected_snprintf_call();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int8_t).to_string(&int8_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 1);
    POOR_MANS_ASSERT(strcmp(expected_calls[0].snprintf_call.captured_format_arg, "%" PRId8) == 0);
    POOR_MANS_ASSERT(strcmp(buffer, "1") == 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_064: [ If any error is encountered, LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int8_t).to_string shall fail and return a negative value. ]*/
static void when_snprintf_fails_int8_t_to_string_also_fails(void)
{
    // arrange
    char buffer[TEST_BUFFER_SIZE];
    int8_t int8_t_value = -1;

    setup_mocks();
    expected_calls[0].mock_call_type = MOCK_CALL_TYPE_snprintf;
    expected_calls[0].snprintf_call.call_result = -1;
    expected_calls[0].snprintf_call.override_result = true;
    expected_call_count = 1;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int8_t).to_string(&int8_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result < 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int8_t).copy */

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_065: [ If src_value is NULL, LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int8_t).copy shall fail and return a non-zero value. ]*/
static void int8_t_copy_called_with_NULL_dst_value_fails(void)
{
    // arrange
    int8_t src = 42;

    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int8_t).copy(NULL, &src);

    // assert
    POOR_MANS_ASSERT(result != 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_066: [ If dst_value is NULL, LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int8_t).copy shall fail and return a non-zero value. ]*/
static void int8_t_copy_called_with_NULL_src_value_fails(void)
{
    // arrange
    int8_t dst;

    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int8_t).copy(&dst, NULL);

    // assert
    POOR_MANS_ASSERT(result != 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_067: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int8_t).copy shall copy the byte of the int8_t value from the address pointed by src_value to dst_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_068: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int8_t).copy shall succeed and return 0. ]*/
static void int8_t_copy_succeeds(void)
{
    // arrange
    int8_t src = 42;
    int8_t dst = 43;

    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int8_t).copy(&dst, &src);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(dst == 42);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_067: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int8_t).copy shall copy the byte of the int8_t value from the address pointed by src_value to dst_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_068: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int8_t).copy shall succeed and return 0. ]*/
static void int8_t_copy_succeeds_2(void)
{
    // arrange
    int8_t src = INT8_MAX;
    int8_t dst = 43;

    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int8_t).copy(&dst, &src);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(dst == INT8_MAX);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int8_t).free */

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_069: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int8_t).free shall return. ]*/
static void int8_t_free_returns(void)
{
    // arrange
    int8_t value = INT8_MAX;

    setup_mocks();

    // act
    LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int8_t).free(&value);

    // assert
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int8_t).get_type */

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_070: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int8_t).get_type shall return the property type LOG_CONTEXT_PROPERTY_TYPE_int8_t. ]*/
static void int8_t_get_type_returns_LOG_CONTEXT_PROPERTY_TYPE_int8_t(void)
{
    // arrange
    setup_mocks();

    // act
    LOG_CONTEXT_PROPERTY_TYPE result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int8_t).get_type();

    // assert
    POOR_MANS_ASSERT(result == LOG_CONTEXT_PROPERTY_TYPE_int8_t);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* LOG_CONTEXT_PROPERTY_TYPE_INIT(int8_t) */

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_105: [ If dst_value is NULL, LOG_CONTEXT_PROPERTY_TYPE_INIT(int8_t) shall fail and return a non-zero value. ]*/
static void MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(int8_t), _with_NULL_dst_value_fails)(void)
{
    // arrange
    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_INIT(int8_t)(NULL, 42);

    // assert
    POOR_MANS_ASSERT(result != 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_106: [ LOG_CONTEXT_PROPERTY_TYPE_INIT(int8_t) shall copy the src_value byte of the int8_t to dst_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_107: [ LOG_CONTEXT_PROPERTY_TYPE_INIT(int8_t) shall succeed and return 0. ]*/
static void MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(int8_t), _with_int8_t_min_value_succeeds)(void)
{
    // arrange
    int8_t dst = 43;
    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_INIT(int8_t)(&dst, INT8_MIN);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(dst == INT8_MIN);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_106: [ LOG_CONTEXT_PROPERTY_TYPE_INIT(int8_t) shall copy the src_value byte of the int8_t to dst_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_107: [ LOG_CONTEXT_PROPERTY_TYPE_INIT(int8_t) shall succeed and return 0. ]*/
static void MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(int8_t), _with_int8_t_max_value_succeeds)(void)
{
    // arrange
    int8_t dst = 43;
    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_INIT(int8_t)(&dst, INT8_MAX);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(dst == INT8_MAX);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(int8_t) */

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_108: [ LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(int8_t) shall return sizeof(int8_t). ]*/
static void MU_C2(LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(int8_t), _succeeds)(void)
{
    // arrange
    setup_mocks();

    // act
    size_t result = LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(int8_t)();

    // assert
    POOR_MANS_ASSERT(result == sizeof(int8_t));
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint8_t).to_string */

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_071: [ If property_value is NULL, LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint8_t).to_string shall fail and return a negative value. ]*/
static void uint8_t_to_string_with_NULL_value_fails(void)
{
    // arrange
    char buffer[TEST_BUFFER_SIZE];
    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint8_t).to_string(NULL, buffer, TEST_BUFFER_SIZE);

    // assert
    POOR_MANS_ASSERT(result < 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_072: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint8_t).to_string shall call snprintf with buffer, buffer_length and format string PRIu8 and pass in the values list the uint8_t_t value pointed to be property_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_073: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint8_t).to_string shall succeed and return the result of snprintf. ]*/
static void uint8_t_to_string_succeeds(void)
{
    // arrange
    char buffer[TEST_BUFFER_SIZE];
    uint8_t uint8_t_value = 0;

    setup_mocks();
    setup_expected_snprintf_call();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint8_t).to_string(&uint8_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 1);
    POOR_MANS_ASSERT(strcmp(expected_calls[0].snprintf_call.captured_format_arg, "%" PRIu8) == 0);
    POOR_MANS_ASSERT(strcmp(buffer, "0") == 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_072: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint8_t).to_string shall call snprintf with buffer, buffer_length and format string PRIu8 and pass in the values list the uint8_t_t value pointed to be property_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_073: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint8_t).to_string shall succeed and return the result of snprintf. ]*/
static void uint8_t_to_string_UINT8_MAX_succeeds(void)
{
    // arrange
    char buffer[TEST_BUFFER_SIZE];
    uint8_t uint8_t_value = UINT8_MAX;

    setup_mocks();
    setup_expected_snprintf_call();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint8_t).to_string(&uint8_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 3);
    POOR_MANS_ASSERT(strcmp(expected_calls[0].snprintf_call.captured_format_arg, "%" PRIu8) == 0);
    POOR_MANS_ASSERT(strcmp(buffer, "255") == 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_072: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint8_t).to_string shall call snprintf with buffer, buffer_length and format string PRIu8 and pass in the values list the uint8_t_t value pointed to be property_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_073: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint8_t).to_string shall succeed and return the result of snprintf. ]*/
static void uint8_t_to_string_with_truncation_succeeds(void)
{
    // arrange
    char buffer[2];
    uint8_t uint8_t_value = 42;

    setup_mocks();
    setup_expected_snprintf_call();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint8_t).to_string(&uint8_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 2);
    POOR_MANS_ASSERT(strcmp(expected_calls[0].snprintf_call.captured_format_arg, "%" PRIu8) == 0);
    POOR_MANS_ASSERT(strcmp(buffer, "4") == 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_072: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint8_t).to_string shall call snprintf with buffer, buffer_length and format string PRIu8 and pass in the values list the uint8_t_t value pointed to be property_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_073: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint8_t).to_string shall succeed and return the result of snprintf. ]*/
static void uint8_t_to_string_with_just_enough_big_buffer_succeeds(void)
{
    // arrange
    char buffer[2];
    uint8_t uint8_t_value = 1;

    setup_mocks();
    setup_expected_snprintf_call();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint8_t).to_string(&uint8_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 1);
    POOR_MANS_ASSERT(strcmp(expected_calls[0].snprintf_call.captured_format_arg, "%" PRIu8) == 0);
    POOR_MANS_ASSERT(strcmp(buffer, "1") == 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_074: [ If any error is encountered, LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint8_t).to_string shall fail and return a negative value. ]*/
static void when_snprintf_fails_uint8_t_to_string_also_fails(void)
{
    // arrange
    char buffer[TEST_BUFFER_SIZE];
    uint8_t uint8_t_value = 42;

    setup_mocks();
    expected_calls[0].mock_call_type = MOCK_CALL_TYPE_snprintf;
    expected_calls[0].snprintf_call.call_result = -1;
    expected_calls[0].snprintf_call.override_result = true;
    expected_call_count = 1;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint8_t).to_string(&uint8_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result < 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint8_t).copy */

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_075: [ If src_value is NULL, LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint8_t).copy shall fail and return a non-zero value. ]*/
static void uint8_t_copy_called_with_NULL_dst_value_fails(void)
{
    // arrange
    uint8_t src = 42;

    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint8_t).copy(NULL, &src);

    // assert
    POOR_MANS_ASSERT(result != 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_076: [ If dst_value is NULL, LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint8_t).copy shall fail and return a non-zero value. ]*/
static void uint8_t_copy_called_with_NULL_src_value_fails(void)
{
    // arrange
    uint8_t dst;

    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint8_t).copy(&dst, NULL);

    // assert
    POOR_MANS_ASSERT(result != 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_077: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint8_t).copy shall copy the byte of the uint8_t_t value from the address pointed by src_value to dst_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_078: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint8_t).copy shall succeed and return 0. ]*/
static void uint8_t_copy_succeeds(void)
{
    // arrange
    uint8_t src = 42;
    uint8_t dst = 43;

    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint8_t).copy(&dst, &src);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(dst == 42);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_077: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint8_t).copy shall copy the byte of the uint8_t_t value from the address pointed by src_value to dst_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_078: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint8_t).copy shall succeed and return 0. ]*/
static void uint8_t_copy_succeeds_2(void)
{
    // arrange
    uint8_t src = UINT8_MAX;
    uint8_t dst = 43;

    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint8_t).copy(&dst, &src);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(dst == UINT8_MAX);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint8_t).free */

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_079: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint8_t).free shall return. ]*/
static void uint8_t_free_returns(void)
{
    // arrange
    uint8_t value = UINT8_MAX;

    setup_mocks();

    // act
    LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint8_t).free(&value);

    // assert
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint8_t).get_type */

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_080: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint8_t).get_type shall return the property type LOG_CONTEXT_PROPERTY_TYPE_uint8_t_t. ]*/
static void uint8_t_get_type_returns_LOG_CONTEXT_PROPERTY_TYPE_uint8_t(void)
{
    // arrange
    setup_mocks();

    // act
    LOG_CONTEXT_PROPERTY_TYPE result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint8_t).get_type();

    // assert
    POOR_MANS_ASSERT(result == LOG_CONTEXT_PROPERTY_TYPE_uint8_t);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* LOG_CONTEXT_PROPERTY_TYPE_INIT(uint8_t) */

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_109: [ If dst_value is NULL, LOG_CONTEXT_PROPERTY_TYPE_INIT(uint8_t) shall fail and return a non-zero value. ]*/
static void MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(uint8_t), _with_NULL_dst_value_fails)(void)
{
    // arrange
    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_INIT(uint8_t)(NULL, 42);

    // assert
    POOR_MANS_ASSERT(result != 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_110: [ LOG_CONTEXT_PROPERTY_TYPE_INIT(uint8_t) shall copy the src_value byte of the uint8_t to dst_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_111: [ LOG_CONTEXT_PROPERTY_TYPE_INIT(uint8_t) shall succeed and return 0. ]*/
static void MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(uint8_t), _with_uint8_t_min_value_succeeds)(void)
{
    // arrange
    uint8_t dst = 43;
    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_INIT(uint8_t)(&dst, 0);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(dst == 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_110: [ LOG_CONTEXT_PROPERTY_TYPE_INIT(uint8_t) shall copy the src_value byte of the uint8_t to dst_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_111: [ LOG_CONTEXT_PROPERTY_TYPE_INIT(uint8_t) shall succeed and return 0. ]*/
static void MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(uint8_t), _with_uint8_t_max_value_succeeds)(void)
{
    // arrange
    uint8_t dst = 43;
    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_INIT(uint8_t)(&dst, UINT8_MAX);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(dst == UINT8_MAX);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(uint8_t) */

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BASIC_TYPES_01_112: [ LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(uint8_t) shall return sizeof(uint8_t). ]*/
static void MU_C2(LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(uint8_t), _succeeds)(void)
{
    // arrange
    setup_mocks();

    // act
    size_t result = LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(uint8_t)();

    // assert
    POOR_MANS_ASSERT(result == sizeof(uint8_t));
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* very "poor man's" way of testing, as no test harness and mocking framework are available */
int main(void)
{
    int64_t_to_string_with_NULL_value_fails();
    int64_t_to_string_succeeds();
    int64_t_to_string_INT64_MAX_succeeds();
    int64_t_to_string_with_truncation_succeeds();
    int64_t_to_string_with_truncation_minus_1_succeeds();
    int64_t_to_string_with_negative_value_succeeds();
    int64_t_to_string_with_just_enough_big_buffer_succeeds();
    when_snprintf_fails_int64_t_to_string_also_fails();

    int64_t_copy_called_with_NULL_dst_value_fails();
    int64_t_copy_called_with_NULL_src_value_fails();
    int64_t_copy_succeeds();
    int64_t_copy_succeeds_2();

    int64_t_free_returns();

    int64_t_get_type_returns_LOG_CONTEXT_PROPERTY_TYPE_int64_t();

    MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(int64_t), _with_NULL_dst_value_fails)();
    MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(int64_t), _with_int64_t_min_value_succeeds)();
    MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(int64_t), _with_int64_t_max_value_succeeds)();

    MU_C2(LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(int64_t), _succeeds)();

    uint64_t_to_string_with_NULL_value_fails();
    uint64_t_to_string_succeeds();
    uint64_t_to_string_UINT64_MAX_succeeds();
    uint64_t_to_string_with_truncation_succeeds();
    uint64_t_to_string_with_just_enough_big_buffer_succeeds();
    when_snprintf_fails_uint64_t_to_string_also_fails();

    uint64_t_copy_called_with_NULL_dst_value_fails();
    uint64_t_copy_called_with_NULL_src_value_fails();
    uint64_t_copy_succeeds();
    uint64_t_copy_succeeds_2();

    uint64_t_free_returns();

    uint64_t_get_type_returns_LOG_CONTEXT_PROPERTY_TYPE_uint64_t();

    MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(uint64_t), _with_NULL_dst_value_fails)();
    MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(uint64_t), _with_uint64_t_min_value_succeeds)();
    MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(uint64_t), _with_uint64_t_max_value_succeeds)();

    MU_C2(LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(uint64_t), _succeeds)();

    int32_t_to_string_with_NULL_value_fails();
    int32_t_to_string_succeeds();
    int32_t_to_string_INT32_MAX_succeeds();
    int32_t_to_string_with_truncation_succeeds();
    int32_t_to_string_with_truncation_minus_1_succeeds();
    int32_t_to_string_with_negative_value_succeeds();
    int32_t_to_string_with_just_enough_big_buffer_succeeds();
    when_snprintf_fails_int32_t_to_string_also_fails();

    int32_t_copy_called_with_NULL_dst_value_fails();
    int32_t_copy_called_with_NULL_src_value_fails();
    int32_t_copy_succeeds();
    int32_t_copy_succeeds_2();

    int32_t_free_returns();

    int32_t_get_type_returns_LOG_CONTEXT_PROPERTY_TYPE_int32_t();

    MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(int32_t), _with_NULL_dst_value_fails)();
    MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(int32_t), _with_int32_t_min_value_succeeds)();
    MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(int32_t), _with_int32_t_max_value_succeeds)();

    MU_C2(LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(int32_t), _succeeds)();

    uint32_t_to_string_with_NULL_value_fails();
    uint32_t_to_string_succeeds();
    uint32_t_to_string_UINT32_MAX_succeeds();
    uint32_t_to_string_with_truncation_succeeds();
    uint32_t_to_string_with_just_enough_big_buffer_succeeds();
    when_snprintf_fails_uint32_t_to_string_also_fails();

    uint32_t_copy_called_with_NULL_dst_value_fails();
    uint32_t_copy_called_with_NULL_src_value_fails();
    uint32_t_copy_succeeds();
    uint32_t_copy_succeeds_2();

    uint32_t_free_returns();

    uint32_t_get_type_returns_LOG_CONTEXT_PROPERTY_TYPE_uint32_t();

    MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(uint32_t), _with_NULL_dst_value_fails)();
    MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(uint32_t), _with_uint32_t_min_value_succeeds)();
    MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(uint32_t), _with_uint32_t_max_value_succeeds)();

    MU_C2(LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(uint32_t), _succeeds)();

    int16_t_to_string_with_NULL_value_fails();
    int16_t_to_string_succeeds();
    int16_t_to_string_INT16_MAX_succeeds();
    int16_t_to_string_with_truncation_succeeds();
    int16_t_to_string_with_truncation_minus_1_succeeds();
    int16_t_to_string_with_negative_value_succeeds();
    int16_t_to_string_with_just_enough_big_buffer_succeeds();
    when_snprintf_fails_int16_t_to_string_also_fails();

    int16_t_copy_called_with_NULL_dst_value_fails();
    int16_t_copy_called_with_NULL_src_value_fails();
    int16_t_copy_succeeds();
    int16_t_copy_succeeds_2();

    int16_t_free_returns();

    int16_t_get_type_returns_LOG_CONTEXT_PROPERTY_TYPE_int16_t();

    MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(int16_t), _with_NULL_dst_value_fails)();
    MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(int16_t), _with_int16_t_min_value_succeeds)();
    MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(int16_t), _with_int16_t_max_value_succeeds)();

    MU_C2(LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(int16_t), _succeeds)();

    uint16_t_to_string_with_NULL_value_fails();
    uint16_t_to_string_succeeds();
    uint16_t_to_string_UINT16_MAX_succeeds();
    uint16_t_to_string_with_truncation_succeeds();
    uint16_t_to_string_with_just_enough_big_buffer_succeeds();
    when_snprintf_fails_uint16_t_to_string_also_fails();

    uint16_t_copy_called_with_NULL_dst_value_fails();
    uint16_t_copy_called_with_NULL_src_value_fails();
    uint16_t_copy_succeeds();
    uint16_t_copy_succeeds_2();

    uint16_t_free_returns();

    uint16_t_get_type_returns_LOG_CONTEXT_PROPERTY_TYPE_uint16_t();

    MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(uint16_t), _with_NULL_dst_value_fails)();
    MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(uint16_t), _with_uint16_t_min_value_succeeds)();
    MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(uint16_t), _with_uint16_t_max_value_succeeds)();

    MU_C2(LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(uint16_t), _succeeds)();

    int8_t_to_string_with_NULL_value_fails();
    int8_t_to_string_succeeds();
    int8_t_to_string_INT8_MAX_succeeds();
    int8_t_to_string_with_truncation_succeeds();
    int8_t_to_string_with_truncation_minus_1_succeeds();
    int8_t_to_string_with_negative_value_succeeds();
    int8_t_to_string_with_just_enough_big_buffer_succeeds();
    when_snprintf_fails_int8_t_to_string_also_fails();

    int8_t_copy_called_with_NULL_dst_value_fails();
    int8_t_copy_called_with_NULL_src_value_fails();
    int8_t_copy_succeeds();
    int8_t_copy_succeeds_2();

    int8_t_free_returns();

    int8_t_get_type_returns_LOG_CONTEXT_PROPERTY_TYPE_int8_t();

    MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(int8_t), _with_NULL_dst_value_fails)();
    MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(int8_t), _with_int8_t_min_value_succeeds)();
    MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(int8_t), _with_int8_t_max_value_succeeds)();

    MU_C2(LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(int8_t), _succeeds)();

    uint8_t_to_string_with_NULL_value_fails();
    uint8_t_to_string_succeeds();
    uint8_t_to_string_UINT8_MAX_succeeds();
    uint8_t_to_string_with_truncation_succeeds();
    uint8_t_to_string_with_just_enough_big_buffer_succeeds();
    when_snprintf_fails_uint8_t_to_string_also_fails();

    uint8_t_copy_called_with_NULL_dst_value_fails();
    uint8_t_copy_called_with_NULL_src_value_fails();
    uint8_t_copy_succeeds();
    uint8_t_copy_succeeds_2();

    uint8_t_free_returns();

    uint8_t_get_type_returns_LOG_CONTEXT_PROPERTY_TYPE_uint8_t();

    MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(uint8_t), _with_NULL_dst_value_fails)();
    MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(uint8_t), _with_uint8_t_min_value_succeeds)();
    MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(uint8_t), _with_uint8_t_max_value_succeeds)();

    MU_C2(LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(uint8_t), _succeeds)();

    return 0;
}
