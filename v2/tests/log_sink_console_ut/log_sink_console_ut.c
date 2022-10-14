// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "macro_utils/macro_utils.h"

#include "c_logging/log_sink_console.h"
#include "c_logging/log_context.h"

static size_t asserts_failed = 0;

// defines how many mock calls we can have
#define MAX_MOCK_CALL_COUNT (128)

#define MOCK_CALL_TYPE_VALUES \
    MOCK_CALL_TYPE_printf, \
    MOCK_CALL_TYPE_time \

MU_DEFINE_ENUM(MOCK_CALL_TYPE, MOCK_CALL_TYPE_VALUES)

// very poor mans mocks :-(

typedef struct printf_CALL_TAG
{
    bool override_result;
    int call_result;
} printf_CALL;

typedef struct time_CALL_TAG
{
    bool override_result;
    int call_result;
    time_t* captured__time;
} time_CALL;

typedef struct MOCK_CALL_TAG
{
    MOCK_CALL_TYPE mock_call_type;
    union
    {
        printf_CALL printf_call;
        time_CALL time_call;
    } u;
} MOCK_CALL;

static MOCK_CALL expected_calls[MAX_MOCK_CALL_COUNT];
static size_t expected_call_count;
static size_t actual_call_count;
static bool actual_and_expected_match;

static void setup_mocks(void)
{
    expected_call_count = 0;
    actual_call_count = 0;
    actual_and_expected_match = true;
}

int mock_printf(const char* format, ...)
{
    int result;

    if ((actual_call_count == expected_call_count) ||
        (expected_calls[actual_call_count].mock_call_type != MOCK_CALL_TYPE_printf))
    {
        actual_and_expected_match = false;
        result = -1;
    }
    else
    {
        if (expected_calls[actual_call_count].u.printf_call.override_result)
        {
            result = expected_calls[actual_call_count].u.printf_call.call_result;
        }
        else
        {
            va_list args;
            va_start(args, format);
            result = vprintf(format, args);
            va_end(args);
        }

        actual_call_count++;
    }

    return result;
}

time_t mock_time(time_t* const _time)
{
    time_t result;

    if ((actual_call_count == expected_call_count) ||
        (expected_calls[actual_call_count].mock_call_type != MOCK_CALL_TYPE_time))
    {
        actual_and_expected_match = false;
        result = (time_t)-1;
    }
    else
    {
        if (expected_calls[actual_call_count].u.time_call.override_result)
        {
            result = expected_calls[actual_call_count].u.time_call.call_result;
        }
        else
        {
            expected_calls[actual_call_count].u.time_call.captured__time = _time;
            result = time(_time);
        }

        actual_call_count++;
    }

    return result;
}

#define POOR_MANS_ASSERT(cond) \
    if (!(cond)) \
    { \
        (void)printf("%s:%d test failed\r\n", __FUNCTION__, __LINE__); \
        asserts_failed++; \
    } \

static void setup_printf_call(void)
{
    expected_calls[expected_call_count].mock_call_type = MOCK_CALL_TYPE_printf;
    expected_calls[expected_call_count].u.printf_call.override_result = false;
    expected_call_count++;
}

static void setup_time_call(void)
{
    expected_calls[expected_call_count].mock_call_type = MOCK_CALL_TYPE_time;
    expected_calls[expected_call_count].u.printf_call.override_result = false;
    expected_call_count++;
}

/* log_sink_console.log_sink_log */

/* Tests_SRS_LOG_SINK_CONSOLE_01_001: [ If `message_format` is `NULL`, `log_sink_console.log_sink_log` shall print an error and return. ]*/
static void log_sink_console_log_with_NULL_message_format_returns(void)
{
    // arrange
    setup_mocks();
    setup_printf_call();

    // act
    log_sink_console.log_sink_log(LOG_LEVEL_CRITICAL, NULL, __FILE__, __FUNCTION__, __LINE__, NULL);

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Codes_SRS_LOG_SINK_CONSOLE_01_002: [ `log_sink_console.log_sink_log` shall obtain the time by calling `time`. ]*/
static void log_sink_console_log_prints_one_line(void)
{
    // arrange
    setup_mocks();
    setup_time_call();

    // act
    log_sink_console.log_sink_log(LOG_LEVEL_CRITICAL, NULL, __FILE__, __FUNCTION__, __LINE__, "test");

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(expected_calls[0].u.time_call.captured__time == NULL);
    POOR_MANS_ASSERT(actual_and_expected_match);
}


/* very "poor man's" way of testing, as no test harness and mocking framework are available */
int main(void)
{
    log_sink_console_log_with_NULL_message_format_returns();

    return asserts_failed;
}
