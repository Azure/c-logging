// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "macro_utils/macro_utils.h"

#include "c_logging/log_context_property_type.h"
#include "c_logging/log_context_property_type_if.h"
#include "c_logging/log_context_property_bool_type.h"

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

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).to_string */

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BOOL_TYPE_07_001: [ If property_value is NULL, LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).to_string shall fail and return a negative value. ]*/
static void bool_to_string_with_NULL_value_fails(void)
{
    // arrange
    char buffer[TEST_BUFFER_SIZE];
    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).to_string(NULL, buffer, TEST_BUFFER_SIZE);

    // assert
    POOR_MANS_ASSERT(result < 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BOOL_TYPE_07_002: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).to_string shall call snprintf with buffer, buffer_length and format string PRI_BOOL and pass in the values list the bool value pointed to be property_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BOOL_TYPE_07_003: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).to_string shall succeed and return the result of snprintf. ]*/
static void bool_false_to_string_succeeds(void)
{
    // arrange
    char buffer[TEST_BUFFER_SIZE];
    bool bool_value = false;

    setup_mocks();
    setup_expected_snprintf_call();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).to_string(&bool_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 9);
    POOR_MANS_ASSERT(strcmp(expected_calls[0].snprintf_call.captured_format_arg, "%" PRI_BOOL) == 0);
    char bool_value_buffer[TEST_BUFFER_SIZE];
    (void)snprintf(bool_value_buffer, sizeof(bool_value_buffer), "%" PRI_BOOL, MU_BOOL_VALUE(bool_value));
    POOR_MANS_ASSERT(strcmp(buffer, bool_value_buffer) == 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BOOL_TYPE_07_002: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).to_string shall call snprintf with buffer, buffer_length and format string PRI_BOOL and pass in the values list the bool value pointed to be property_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BOOL_TYPE_07_003: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).to_string shall succeed and return the result of snprintf. ]*/
static void bool_true_to_string_succeeds(void)
{
    // arrange
    char buffer[TEST_BUFFER_SIZE];
    bool bool_value = true;

    setup_mocks();
    setup_expected_snprintf_call();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).to_string(&bool_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 8);
    POOR_MANS_ASSERT(strcmp(expected_calls[0].snprintf_call.captured_format_arg, "%" PRI_BOOL) == 0);
    char bool_value_buffer[TEST_BUFFER_SIZE];
    (void)snprintf(bool_value_buffer, sizeof(bool_value_buffer), "%" PRI_BOOL, MU_BOOL_VALUE(bool_value));
    POOR_MANS_ASSERT(strcmp(buffer, bool_value_buffer) == 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BOOL_TYPE_07_002: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).to_string shall call snprintf with buffer, buffer_length and format string PRI_BOOL and pass in the values list the bool value pointed to be property_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BOOL_TYPE_07_003: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).to_string shall succeed and return the result of snprintf. ]*/
static void bool_false_to_string_with_just_enough_big_buffer_succeeds(void)
{
    // arrange
    char buffer[9];
    bool bool_value = true;

    setup_mocks();
    setup_expected_snprintf_call();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).to_string(&bool_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 8);
    POOR_MANS_ASSERT(strcmp(expected_calls[0].snprintf_call.captured_format_arg, "%" PRI_BOOL) == 0);
    char bool_value_buffer[9];
    (void)snprintf(bool_value_buffer, sizeof(bool_value_buffer), "%" PRI_BOOL, MU_BOOL_VALUE(bool_value));
    POOR_MANS_ASSERT(strcmp(buffer, bool_value_buffer) == 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BOOL_TYPE_07_002: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).to_string shall call snprintf with buffer, buffer_length and format string PRI_BOOL and pass in the values list the bool value pointed to be property_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BOOL_TYPE_07_003: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).to_string shall succeed and return the result of snprintf. ]*/
static void bool_true_to_string_with_just_enough_big_buffer_succeeds(void)
{
    // arrange
    char buffer[10];
    bool bool_value = false;

    setup_mocks();
    setup_expected_snprintf_call();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).to_string(&bool_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 9);
    POOR_MANS_ASSERT(strcmp(expected_calls[0].snprintf_call.captured_format_arg, "%" PRI_BOOL) == 0);
    char bool_value_buffer[10];
    (void)snprintf(bool_value_buffer, sizeof(bool_value_buffer), "%" PRI_BOOL, MU_BOOL_VALUE(bool_value));
    POOR_MANS_ASSERT(strcmp(buffer, bool_value_buffer) == 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BOOL_TYPE_07_004: [ If any error is encountered, LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).to_string shall fail and return a negative value. ]*/
static void when_snprintf_fails_bool_to_string_also_fails(void)
{
    // arrange
    char buffer[TEST_BUFFER_SIZE];
    bool bool_value = -1;

    setup_mocks();
    expected_calls[0].mock_call_type = MOCK_CALL_TYPE_snprintf;
    expected_calls[0].snprintf_call.call_result = -1;
    expected_calls[0].snprintf_call.override_result = true;
    expected_call_count = 1;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).to_string(&bool_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result < 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).copy */

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BOOL_TYPE_07_005: [ If src_value is NULL, LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).copy shall fail and return a non-zero value. ]*/
static void bool_copy_called_with_NULL_dst_value_fails(void)
{
    // arrange
    bool src = true;

    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).copy(NULL, &src);

    // assert
    POOR_MANS_ASSERT(result != 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BOOL_TYPE_07_006: [ If dst_value is NULL, LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).copy shall fail and return a non-zero value. ]*/
static void bool_copy_called_with_NULL_src_value_fails(void)
{
    // arrange
    bool dst;

    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).copy(&dst, NULL);

    // assert
    POOR_MANS_ASSERT(result != 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BOOL_TYPE_07_007: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).copy shall copy the bytes of the bool value from the address pointed by src_value to dst_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BOOL_TYPE_07_008: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).copy shall succeed and return 0. ]*/
static void bool_copy_true_succeeds(void)
{
    // arrange
    bool src = true;
    bool dst = false;

    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).copy(&dst, &src);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(dst == true);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BOOL_TYPE_07_007: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).copy shall copy the bytes of the bool value from the address pointed by src_value to dst_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BOOL_TYPE_07_008: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).copy shall succeed and return 0. ]*/
static void bool_copy_false_succeeds(void)
{
    // arrange
    bool src = false;
    bool dst = true;

    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).copy(&dst, &src);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(dst == false);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).free */

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BOOL_TYPE_07_009: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).free shall return. ]*/
static void bool_true_free_returns(void)
{
    // arrange
    bool value = true;

    setup_mocks();

    // act
    LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).free(&value);

    // assert
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BOOL_TYPE_07_009: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).free shall return. ]*/
static void bool_false_free_returns(void)
{
    // arrange
    bool value = true;

    setup_mocks();

    // act
    LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).free(&value);

    // assert
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).get_type */

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BOOL_TYPE_07_010: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).get_type shall return the property type LOG_CONTEXT_PROPERTY_TYPE_bool. ]*/
static void bool_get_type_returns_LOG_CONTEXT_PROPERTY_TYPE_bool(void)
{
    // arrange
    setup_mocks();

    // act
    LOG_CONTEXT_PROPERTY_TYPE result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).get_type();

    // assert
    POOR_MANS_ASSERT(result == LOG_CONTEXT_PROPERTY_TYPE_bool);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* LOG_CONTEXT_PROPERTY_TYPE_INIT(bool) */

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BOOL_TYPE_07_011: [ If dst_value is NULL, LOG_CONTEXT_PROPERTY_TYPE_INIT(bool) shall fail and return a non-zero value. ]*/
static void MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(bool), _with_NULL_dst_value_fails)(void)
{
    // arrange
    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_INIT(bool)(NULL, true);

    // assert
    POOR_MANS_ASSERT(result != 0);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BOOL_TYPE_07_012: [ LOG_CONTEXT_PROPERTY_TYPE_INIT(bool) shall copy the src_value bytes of the bool to dst_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BOOL_TYPE_07_013: [ LOG_CONTEXT_PROPERTY_TYPE_INIT(bool) shall succeed and return 0. ]*/
static void MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(bool), _with_bool_true_succeeds)(void)
{
    // arrange
    bool dst = false;
    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_INIT(bool)(&dst, true);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(dst == true);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BOOL_TYPE_07_012: [ LOG_CONTEXT_PROPERTY_TYPE_INIT(bool) shall copy the src_value bytes of the bool to dst_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_BOOL_TYPE_07_013: [ LOG_CONTEXT_PROPERTY_TYPE_INIT(bool) shall succeed and return 0. ]*/
static void MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(bool), _with_bool_false_succeeds)(void)
{
    // arrange
    bool dst = true;
    setup_mocks();

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_INIT(bool)(&dst, false);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(dst == false);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(bool) */

/* Tests_SRS_LOG_CONTEXT_PROPERTY_BOOL_TYPE_07_014: [ LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(bool) shall return sizeof(bool). ]*/
static void MU_C2(LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(bool), _succeeds)(void)
{
    // arrange
    setup_mocks();

    // act
    size_t result = LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(bool)();

    // assert
    POOR_MANS_ASSERT(result == sizeof(bool));
    POOR_MANS_ASSERT(actual_and_expected_match);
}


/* very "poor man's" way of testing, as no test harness and mocking framework are available */
int main(void)
{
    bool_to_string_with_NULL_value_fails();
    bool_false_to_string_succeeds();
    bool_true_to_string_succeeds();
    bool_false_to_string_with_just_enough_big_buffer_succeeds();
    bool_true_to_string_with_just_enough_big_buffer_succeeds();
    when_snprintf_fails_bool_to_string_also_fails();

    bool_copy_called_with_NULL_dst_value_fails();
    bool_copy_called_with_NULL_src_value_fails();
    bool_copy_true_succeeds();
    bool_copy_false_succeeds();

    bool_true_free_returns();
    bool_false_free_returns();

    bool_get_type_returns_LOG_CONTEXT_PROPERTY_TYPE_bool();

    MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(bool), _with_NULL_dst_value_fails)();
    MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(bool), _with_bool_true_succeeds)();
    MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(bool), _with_bool_false_succeeds)();

    MU_C2(LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(bool), _succeeds)();

    return 0;
}
