// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "macro_utils/macro_utils.h"

#include "c_logging/log_context.h"

#include "c_logging/log_errno.h"

// defines how many mock calls we can have
#define MAX_MOCK_CALL_COUNT (128)

#define MOCK_CALL_TYPE_VALUES \
    MOCK_CALL_TYPE_errno, \
    MOCK_CALL_TYPE_strerror_s \

MU_DEFINE_ENUM(MOCK_CALL_TYPE, MOCK_CALL_TYPE_VALUES)

// very poor mans mocks :-(

typedef struct errno_CALL_TAG
{
    bool override_result;
    errno_t call_result;
} errno_CALL;

typedef struct strerror_s_CALL_TAG
{
    bool override_result;
    errno_t call_result;
    const char* buffer_payload;
    char* captured_s;
    rsize_t captured_maxsize;
    errno_t captured_errnum;
} strerror_s_CALL;

typedef struct MOCK_CALL_TAG
{
    MOCK_CALL_TYPE mock_call_type;
    union
    {
        errno_CALL errno_call;
        strerror_s_CALL strerror_s_call;
    };
} MOCK_CALL;

static MOCK_CALL expected_calls[MAX_MOCK_CALL_COUNT];
static size_t expected_call_count;
static size_t actual_call_count;
static bool actual_and_expected_match;

const char TEST_FORMATTED_errno_0[] = "Nothing to see here";
const char TEST_FORMATTED_errno_42[] = "The answer to live";

#define POOR_MANS_ASSERT(cond) \
    if (!(cond)) \
    { \
        (void)printf("%s:%d test failed\r\n", __FUNCTION__, __LINE__); \
        abort(); \
    } \

static void setup_mocks(void)
{
    expected_call_count = 0;
    actual_call_count = 0;
    actual_and_expected_match = true;
}

errno_t mock_errno(void)
{
    errno_t result;
    
    if ((actual_call_count == expected_call_count) ||
        (expected_calls[actual_call_count].mock_call_type != MOCK_CALL_TYPE_errno))
    {
        actual_and_expected_match = false;
        result = MU_FAILURE;
    }
    else
    {
        if (expected_calls[actual_call_count].errno_call.override_result)
        {
            result = expected_calls[actual_call_count].errno_call.call_result;
        }
        else
        {
            result = 0;
        }
    
        actual_call_count++;
    }
    
    return result;
}

errno_t mock_strerror_s(char* s, rsize_t maxsize, errno_t errnum)
{
    errno_t result;

    if ((actual_call_count == expected_call_count) ||
        (expected_calls[actual_call_count].mock_call_type != MOCK_CALL_TYPE_strerror_s))
    {
        actual_and_expected_match = false;
        return -1;
    }
    else
    {
        if (expected_calls[actual_call_count].strerror_s_call.override_result)
        {
            expected_calls[actual_call_count].strerror_s_call.captured_maxsize = maxsize;
            expected_calls[actual_call_count].strerror_s_call.captured_s = s;
            expected_calls[actual_call_count].strerror_s_call.captured_errnum = errnum;

            (void)strcpy(s, expected_calls[actual_call_count].strerror_s_call.buffer_payload);
            result = expected_calls[actual_call_count].strerror_s_call.call_result;
        }
        else
        {
            result = strerror_s(s, maxsize, errnum);
        }

        actual_call_count++;
    }

    return result;
}

static void setup_errno_call(void)
{
    expected_calls[expected_call_count].mock_call_type = MOCK_CALL_TYPE_errno;
    expected_calls[expected_call_count].errno_call.override_result = false;
    expected_call_count++;
}

static void setup_strerror_s_call(void)
{
    expected_calls[expected_call_count].mock_call_type = MOCK_CALL_TYPE_strerror_s;
    expected_calls[expected_call_count].strerror_s_call.override_result = false;
    expected_call_count++;
}

/* log_errno_fill_property */

/* Tests_SRS_LOG_ERRNO_WIN32_01_002: [ If buffer is NULL, log_errno_fill_property shall return 512 to indicate how many bytes shall be reserved for the string formatted errno. ] */
static void log_errno_fill_property_with_NULL_returns_needed_buffer_size(void)
{
    // arrange
    setup_mocks();

    // act
    int result = log_errno_fill_property(NULL, 42);

    // assert
    POOR_MANS_ASSERT(result == 512);
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_ERRNO_WIN32_01_003: [ Otherwise, log_errno_fill_property shall obtain the errno value. ] */
/* Tests_SRS_LOG_ERRNO_WIN32_01_004: [ log_errno_fill_property shall call strerror_s with buffer, 512 and the errno value. ] */
/* Tests_SRS_LOG_ERRNO_WIN32_01_007: [ log_errno_fill_property shall return 512. ] */
static void log_errno_fill_property_with_non_NULL_formats_0(void)
{
    // arrange
    char buffer[512];

    setup_mocks();
    setup_errno_call();
    setup_strerror_s_call();
    expected_calls[1].strerror_s_call.override_result = true;
    expected_calls[1].strerror_s_call.call_result = 0;
    expected_calls[1].strerror_s_call.buffer_payload = TEST_FORMATTED_errno_0;

    // act
    int result = log_errno_fill_property(buffer, 42);

    // assert
    POOR_MANS_ASSERT(result == 512);
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(expected_calls[1].strerror_s_call.captured_errnum == 0);
    POOR_MANS_ASSERT(expected_calls[1].strerror_s_call.captured_s == buffer);
    POOR_MANS_ASSERT(expected_calls[1].strerror_s_call.captured_maxsize == 512);
    POOR_MANS_ASSERT(strcmp(buffer, TEST_FORMATTED_errno_0) == 0);
}

/* Tests_SRS_LOG_ERRNO_WIN32_01_003: [ Otherwise, log_errno_fill_property shall obtain the errno value. ] */
/* Tests_SRS_LOG_ERRNO_WIN32_01_004: [ log_errno_fill_property shall call strerror_s with buffer, 512 and the errno value. ] */
/* Tests_SRS_LOG_ERRNO_WIN32_01_007: [ log_errno_fill_property shall return 512. ] */
static void log_errno_fill_property_with_non_NULL_formats_42(void)
{
    // arrange
    char buffer[512];

    setup_mocks();
    setup_errno_call();
    setup_strerror_s_call();
    expected_calls[0].errno_call.override_result = true;
    expected_calls[0].errno_call.call_result = 42;
    expected_calls[1].strerror_s_call.override_result = true;
    expected_calls[1].strerror_s_call.call_result = 0;
    expected_calls[1].strerror_s_call.buffer_payload = TEST_FORMATTED_errno_42;

    // act
    int result = log_errno_fill_property(buffer, 42);

    // assert
    POOR_MANS_ASSERT(result == 512);
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(expected_calls[1].strerror_s_call.captured_errnum == 42);
    POOR_MANS_ASSERT(expected_calls[1].strerror_s_call.captured_s == buffer);
    POOR_MANS_ASSERT(expected_calls[1].strerror_s_call.captured_maxsize == 512);
    POOR_MANS_ASSERT(strcmp(buffer, TEST_FORMATTED_errno_42) == 0);
}

/* Tests_SRS_LOG_ERRNO_WIN32_01_005: [ If strerror_s fails, log_errno_fill_property shall copy in buffer the string failure in strerror_s and return 512. ] */
static void when_strerror_s_fails_log_errno_fill_property_places_failure_string_in_buffer(void)
{
    // arrange
    char buffer[512];

    setup_mocks();
    setup_errno_call();
    setup_strerror_s_call();
    
    expected_calls[0].errno_call.override_result = true;
    expected_calls[0].errno_call.call_result = 42;
    expected_calls[1].strerror_s_call.override_result = true;
    expected_calls[1].strerror_s_call.call_result = 1;

    // act
    int result = log_errno_fill_property(buffer, 1);

    // assert
    POOR_MANS_ASSERT(result == 512);
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(expected_calls[1].strerror_s_call.captured_errnum == 42);
    POOR_MANS_ASSERT(expected_calls[1].strerror_s_call.captured_s == buffer);
    POOR_MANS_ASSERT(expected_calls[1].strerror_s_call.captured_maxsize == 512);
    POOR_MANS_ASSERT(strcmp(buffer, "failure in strerror_s") == 0);
}

/* LOG_ERRNO */

/* Tests_SRS_LOG_ERRNO_WIN32_01_001: [ LOG_ERRNO shall expand to a LOG_CONTEXT_PROPERTY_CUSTOM_FUNCTION with name errno_value, type ascii_char_ptr and value function call being log_errno_fill_property. ] */
static void LOG_ERRNO_emits_the_underlying_property(void)
{
    // arrange
    setup_mocks();
    setup_errno_call();
    setup_strerror_s_call();

    expected_calls[0].errno_call.override_result = true;
    expected_calls[0].errno_call.call_result = 42;
    expected_calls[1].strerror_s_call.override_result = true;
    expected_calls[1].strerror_s_call.call_result = 0;
    expected_calls[1].strerror_s_call.buffer_payload = TEST_FORMATTED_errno_42;

    // act
    LOG_CONTEXT_LOCAL_DEFINE(log_context, NULL, LOG_ERRNO());

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);

    uint32_t property_count = log_context_get_property_value_pair_count(&log_context);
    const LOG_CONTEXT_PROPERTY_VALUE_PAIR* properties = log_context_get_property_value_pairs(&log_context);
    POOR_MANS_ASSERT(property_count == 2);
    POOR_MANS_ASSERT(strcmp(properties[0].name, "") == 0);
    POOR_MANS_ASSERT(properties[0].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_struct);
    POOR_MANS_ASSERT(*(uint8_t*)properties[0].value == 1);
    POOR_MANS_ASSERT(strcmp(properties[1].name, "errno_value") == 0);
    POOR_MANS_ASSERT(properties[1].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_ascii_char_ptr);
    POOR_MANS_ASSERT(strcmp(properties[1].value, TEST_FORMATTED_errno_42) == 0);
}

/* very "poor man's" way of testing, as no test harness and mocking framework are available */
int main(void)
{
#ifdef _MSC_VER
    // make abort not popup
    _set_abort_behavior(_CALL_REPORTFAULT, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
#endif

    log_errno_fill_property_with_NULL_returns_needed_buffer_size();
    log_errno_fill_property_with_non_NULL_formats_0();
    log_errno_fill_property_with_non_NULL_formats_42();
    
    when_strerror_s_fails_log_errno_fill_property_places_failure_string_in_buffer();

    LOG_ERRNO_emits_the_underlying_property();

    return 0;
}
