// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "macro_utils/macro_utils.h"

#include "c_logging/log_context_property_basic_types.h"
#include "c_logging/log_context_property_to_string.h"
#include "c_logging/log_context_property_type_ascii_char_ptr.h"
#include "c_logging/log_context_property_value_pair.h"
#include "c_logging/log_level.h"
#include "c_logging/log_sink_if.h"
#include "c_logging/log_context.h"
#include "c_logging/logger.h"

#include "c_logging/log_sink_callback.h"

// defines how many mock calls we can have
#define MAX_MOCK_CALL_COUNT (128)

#define MOCK_CALL_TYPE_VALUES \
    MOCK_CALL_TYPE_printf, \
    MOCK_CALL_TYPE_time, \
    MOCK_CALL_TYPE_ctime, \
    MOCK_CALL_TYPE_vsnprintf, \
    MOCK_CALL_TYPE_snprintf, \
    MOCK_CALL_TYPE_log_context_get_property_value_pair_count, \
    MOCK_CALL_TYPE_log_context_get_property_value_pairs, \
    MOCK_CALL_TYPE_log_context_property_to_string, \
    MOCK_CALL_TYPE_log_callback \

MU_DEFINE_ENUM(MOCK_CALL_TYPE, MOCK_CALL_TYPE_VALUES)

// very poor mans mocks :-(

#define MAX_PRINTF_CAPTURED_OUPUT_SIZE (LOG_MAX_MESSAGE_LENGTH * 2)

typedef struct printf_CALL_TAG
{
    bool override_result;
    int call_result;
    char captured_output[MAX_PRINTF_CAPTURED_OUPUT_SIZE];
} printf_CALL;

typedef struct time_CALL_TAG
{
    bool override_result;
    time_t call_result;
    time_t* captured__time;
} time_CALL;

typedef struct ctime_CALL_TAG
{
    bool override_result;
    char* call_result;
    const time_t* captured_timer;
} ctime_CALL;

typedef struct vsnprintf_CALL_TAG
{
    bool override_result;
    int call_result;
    const char* captured_format_arg;
} vsnprintf_CALL;

typedef struct snprintf_CALL_TAG
{
    bool override_result;
    int call_result;
    const char* captured_format_arg;
} snprintf_CALL;

typedef struct log_context_get_property_value_pair_count_CALL_TAG
{
    bool override_result;
    uint32_t call_result;
    LOG_CONTEXT_HANDLE captured_log_context;
} log_context_get_property_value_pair_count_CALL;

typedef struct log_context_get_property_value_pairs_CALL_TAG
{
    bool override_result;
    const LOG_CONTEXT_PROPERTY_VALUE_PAIR* call_result;
    LOG_CONTEXT_HANDLE captured_log_context;
} log_context_get_property_value_pairs_CALL;

typedef struct log_context_property_to_string_CALL_TAG
{
    bool override_result;
    int call_result;
    char* captured_buffer;
    size_t captured_buffer_size;
    const LOG_CONTEXT_PROPERTY_VALUE_PAIR* captured_property_value_pairs;
    size_t captured_property_value_pair_count;
} log_context_property_to_string_CALL;

typedef struct log_callback_CALL_TAG
{
    void* captured_context;
    LOG_LEVEL captured_log_level;
    char captured_output[MAX_PRINTF_CAPTURED_OUPUT_SIZE];
} log_callback_CALL;

typedef struct MOCK_CALL_TAG
{
    MOCK_CALL_TYPE mock_call_type;
    union
    {
        printf_CALL printf_call;
        time_CALL time_call;
        ctime_CALL ctime_call;
        vsnprintf_CALL vsnprintf_call;
        snprintf_CALL snprintf_call;
        log_context_get_property_value_pair_count_CALL log_context_get_property_value_pair_count_call;
        log_context_get_property_value_pairs_CALL log_context_get_property_value_pairs_call;
        log_context_property_to_string_CALL log_context_property_to_string_call;
        log_callback_CALL log_callback_call;
    };
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
        if (expected_calls[actual_call_count].printf_call.override_result)
        {
            result = expected_calls[actual_call_count].printf_call.call_result;
        }
        else
        {
            va_list args;
            va_start(args, format);
            // also capture the result in a variable for comparisons in tests
            (void)vsnprintf(expected_calls[actual_call_count].printf_call.captured_output, sizeof(expected_calls[actual_call_count].printf_call.captured_output),
                format, args);
            va_end(args);

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
        if (expected_calls[actual_call_count].time_call.override_result)
        {
            result = expected_calls[actual_call_count].time_call.call_result;
        }
        else
        {
            expected_calls[actual_call_count].time_call.captured__time = _time;
            result = time(_time);
        }

        actual_call_count++;
    }

    return result;
}

char* mock_ctime(const time_t* timer)
{
    char* result;

    if ((actual_call_count == expected_call_count) ||
        (expected_calls[actual_call_count].mock_call_type != MOCK_CALL_TYPE_ctime))
    {
        actual_and_expected_match = false;
        result = NULL;
    }
    else
    {
        if (expected_calls[actual_call_count].ctime_call.override_result)
        {
            result = expected_calls[actual_call_count].ctime_call.call_result;
        }
        else
        {
            expected_calls[actual_call_count].ctime_call.captured_timer = timer;
            result = ctime(timer);
        }

        actual_call_count++;
    }

    return result;
}

int mock_vsnprintf(char* s, size_t n, const char* format, va_list args)
{
    int result;

    if ((actual_call_count == expected_call_count) ||
        (expected_calls[actual_call_count].mock_call_type != MOCK_CALL_TYPE_vsnprintf))
    {
        actual_and_expected_match = false;
        result = -1;
    }
    else
    {
        if (expected_calls[actual_call_count].vsnprintf_call.override_result)
        {
            result = expected_calls[actual_call_count].vsnprintf_call.call_result;
        }
        else
        {
            expected_calls[actual_call_count].vsnprintf_call.captured_format_arg = format;

            result = vsnprintf(s, n, format, args);

            va_end(args);
        }

        actual_call_count++;
    }

    return result;
}

int mock_snprintf(char* s, size_t n, const char* format, ...)
{
    int result;

    if ((actual_call_count == expected_call_count) ||
        (expected_calls[actual_call_count].mock_call_type != MOCK_CALL_TYPE_snprintf))
    {
        actual_and_expected_match = false;
        result =-1;
    }
    else
    {
        if (expected_calls[actual_call_count].snprintf_call.override_result)
        {
            result = expected_calls[actual_call_count].snprintf_call.call_result;
        }
        else
        {
            expected_calls[actual_call_count].snprintf_call.captured_format_arg = format;

            va_list args;
            va_start(args, format);

            result = vsnprintf(s, n, format, args);

            va_end(args);
        }

        actual_call_count++;
    }

    return result;
}

uint32_t mock_log_context_get_property_value_pair_count(LOG_CONTEXT_HANDLE log_context)
{
    uint32_t result;

    if ((actual_call_count == expected_call_count) ||
        (expected_calls[actual_call_count].mock_call_type != MOCK_CALL_TYPE_log_context_get_property_value_pair_count))
    {
        actual_and_expected_match = false;
        result = 0;
    }
    else
    {
        if (expected_calls[actual_call_count].log_context_get_property_value_pair_count_call.override_result)
        {
            result = expected_calls[actual_call_count].log_context_get_property_value_pair_count_call.call_result;
        }
        else
        {
            expected_calls[actual_call_count].log_context_get_property_value_pair_count_call.captured_log_context = log_context;

            result = log_context_get_property_value_pair_count(log_context);
        }

        actual_call_count++;
    }

    return result;
}

const LOG_CONTEXT_PROPERTY_VALUE_PAIR* mock_log_context_get_property_value_pairs(LOG_CONTEXT_HANDLE log_context)
{
    const LOG_CONTEXT_PROPERTY_VALUE_PAIR* result;

    if ((actual_call_count == expected_call_count) ||
        (expected_calls[actual_call_count].mock_call_type != MOCK_CALL_TYPE_log_context_get_property_value_pairs))
    {
        actual_and_expected_match = false;
        result = NULL;
    }
    else
    {
        if (expected_calls[actual_call_count].log_context_get_property_value_pairs_call.override_result)
        {
            result = expected_calls[actual_call_count].log_context_get_property_value_pairs_call.call_result;
        }
        else
        {
            expected_calls[actual_call_count].log_context_get_property_value_pairs_call.captured_log_context = log_context;

            result = log_context_get_property_value_pairs(log_context);
        }

        actual_call_count++;
    }

    return result;
}

int mock_log_context_property_to_string(char* buffer, size_t buffer_size, const LOG_CONTEXT_PROPERTY_VALUE_PAIR* property_value_pairs, size_t property_value_pair_count)
{
    int result;

    if ((actual_call_count == expected_call_count) ||
        (expected_calls[actual_call_count].mock_call_type != MOCK_CALL_TYPE_log_context_property_to_string))
    {
        actual_and_expected_match = false;
        result = -1;
    }
    else
    {
        if (expected_calls[actual_call_count].log_context_property_to_string_call.override_result)
        {
            result = expected_calls[actual_call_count].log_context_property_to_string_call.call_result;
        }
        else
        {
            expected_calls[actual_call_count].log_context_property_to_string_call.captured_buffer = buffer;
            expected_calls[actual_call_count].log_context_property_to_string_call.captured_buffer_size = buffer_size;
            expected_calls[actual_call_count].log_context_property_to_string_call.captured_property_value_pairs = property_value_pairs;
            expected_calls[actual_call_count].log_context_property_to_string_call.captured_property_value_pair_count = property_value_pair_count;

            result = log_context_property_to_string(buffer, buffer_size, property_value_pairs, property_value_pair_count);
        }

        actual_call_count++;
    }

    return result;
}

static void mock_log_callback(void* context, LOG_LEVEL log_level, const char* message)
{
    if ((actual_call_count == expected_call_count) ||
        (expected_calls[actual_call_count].mock_call_type != MOCK_CALL_TYPE_log_callback))
    {
        actual_and_expected_match = false;
    }
    else
    {
        expected_calls[actual_call_count].log_callback_call.captured_context = context;
        expected_calls[actual_call_count].log_callback_call.captured_log_level = log_level;
        (void)snprintf(expected_calls[actual_call_count].log_callback_call.captured_output, sizeof(expected_calls[actual_call_count].log_callback_call.captured_output), "%s", message);

        actual_call_count++;
    }
}

#define POOR_MANS_ASSERT(cond) \
    if (!(cond)) \
    { \
        (void)printf("%s:%d test failed\r\n", __FUNCTION__, __LINE__); \
        abort(); \
    } \

static void test_init(void)
{
    log_sink_callback.init();
    int result = log_sink_callback_set_callback(mock_log_callback, (void*)0x42);
    POOR_MANS_ASSERT(result == 0);
}

static void setup_printf_call(void)
{
    expected_calls[expected_call_count].mock_call_type = MOCK_CALL_TYPE_printf;
    expected_calls[expected_call_count].printf_call.override_result = false;
    expected_call_count++;
}

static void setup_time_call(void)
{
    expected_calls[expected_call_count].mock_call_type = MOCK_CALL_TYPE_time;
    expected_calls[expected_call_count].time_call.override_result = false;
    expected_call_count++;
}

static void setup_ctime_call(void)
{
    expected_calls[expected_call_count].mock_call_type = MOCK_CALL_TYPE_ctime;
    expected_calls[expected_call_count].ctime_call.override_result = false;
    expected_call_count++;
}

static void setup_vsnprintf_call(void)
{
    expected_calls[expected_call_count].mock_call_type = MOCK_CALL_TYPE_vsnprintf;
    expected_calls[expected_call_count].vsnprintf_call.override_result = false;
    expected_call_count++;
}

static void setup_snprintf_call(void)
{
    expected_calls[expected_call_count].mock_call_type = MOCK_CALL_TYPE_snprintf;
    expected_calls[expected_call_count].snprintf_call.override_result = false;
    expected_call_count++;
}

static void setup_log_context_get_property_value_pair_count_call(void)
{
    expected_calls[expected_call_count].mock_call_type = MOCK_CALL_TYPE_log_context_get_property_value_pair_count;
    expected_calls[expected_call_count].log_context_get_property_value_pair_count_call.override_result = false;
    expected_call_count++;
}

static void setup_log_context_get_property_value_pairs_call(void)
{
    expected_calls[expected_call_count].mock_call_type = MOCK_CALL_TYPE_log_context_get_property_value_pairs;
    expected_calls[expected_call_count].log_context_get_property_value_pairs_call.override_result = false;
    expected_call_count++;
}

static void setup_log_context_property_to_string_call(void)
{
    expected_calls[expected_call_count].mock_call_type = MOCK_CALL_TYPE_log_context_property_to_string;
    expected_calls[expected_call_count].log_context_property_to_string_call.override_result = false;
    expected_call_count++;
}

static void setup_log_callback_call(void)
{
    expected_calls[expected_call_count].mock_call_type = MOCK_CALL_TYPE_log_callback;
    expected_call_count++;
}

static void validate_log_line(const char* actual_string, const char* expected_format, const char* file, int line, const char* func, const char* expected_message)
{
    char expected_string[LOG_MAX_MESSAGE_LENGTH * 2];
    int snprintf_result = snprintf(expected_string, sizeof(expected_string), expected_format, file, line, func);
    POOR_MANS_ASSERT(snprintf_result >= 0);
    char day_of_week[4];
    char month[4];
    int day;
    int hour;
    int minute;
    int second;
    int year;
    char message[LOG_MAX_MESSAGE_LENGTH];
    int scanned_values = sscanf(actual_string, expected_string, day_of_week, month, &day, &hour, &minute, &second, &year, message);
    POOR_MANS_ASSERT(scanned_values == 8);
    POOR_MANS_ASSERT(strcmp(message, expected_message) == 0);
}

static void validate_log_line_with_NULL_time(const char* actual_string, const char* expected_format,const char* file, int line, const char* func, const char* expected_message)
{
    char expected_string[LOG_MAX_MESSAGE_LENGTH * 2];
    int snprintf_result = snprintf(expected_string, sizeof(expected_string), expected_format, file, line, func, expected_message);
    POOR_MANS_ASSERT(snprintf_result >= 0);
    char message[LOG_MAX_MESSAGE_LENGTH];
    int scanned_values = sscanf(actual_string, expected_string, message);
    POOR_MANS_ASSERT(scanned_values == 1);
    POOR_MANS_ASSERT(strcmp(message, expected_message) == 0);
}

static void test_log_sink_callback_log(LOG_LEVEL log_level, LOG_CONTEXT_HANDLE log_context, const char* file, const char* func, int line, const char* message_format, ...)
{
    va_list args;
    va_start(args, message_format);
    log_sink_callback.log(log_level, log_context, file, func, line, message_format, args);
    va_end(args);
}

/* log_sink_callback.init */

/* Tests_SRS_LOG_SINK_CALLBACK_42_001: [ log_sink_callback.init shall return 0. ] */
static void log_sink_callback_init_returns(void)
{
    // arrange
    setup_mocks();

    // act
    log_sink_callback.init();

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* log_sink_callback.deinit */

/* Tests_SRS_LOG_SINK_CALLBACK_42_002: [ log_sink_callback.deinit shall return. ] */
static void log_sink_callback_deinit_returns(void)
{
    // arrange
    setup_mocks();

    // act
    log_sink_callback.deinit();

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* log_sink_callback_set_callback */

/* Tests_SRS_LOG_SINK_CALLBACK_42_003: [ If log_callback is NULL then log_sink_callback_set_callback shall fail and return a non-zero value. ]*/
static void log_sink_callback_set_callback_with_NULL_log_callback_fails(void)
{
    // arrange
    setup_mocks();

    // act
    int result = log_sink_callback_set_callback(NULL, (void*)0x42);

    // assert
    POOR_MANS_ASSERT(result != 0);
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_SINK_CALLBACK_42_004: [ log_sink_callback_set_callback shall store log_callback and context so that it is used by all future calls to log_sink_callback.log. ]*/
/* Tests_SRS_LOG_SINK_CALLBACK_42_005: [ log_sink_callback_set_callback shall return 0. ]*/
static void log_sink_callback_set_callback_succeeds(void)
{
    // arrange
    setup_mocks();

    // act
    int result = log_sink_callback_set_callback(mock_log_callback, (void*)0x42);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* log_sink_callback.log */

/* Tests_SRS_LOG_SINK_CALLBACK_42_006: [ If message_format is NULL, log_sink_callback.log shall call the log_callback with an error message and return. ]*/
static void log_sink_callback_log_with_NULL_message_format_calls_callback_with_error(void)
{
    // arrange
    test_init();
    setup_mocks();

    setup_log_callback_call();

    // act
    test_log_sink_callback_log(LOG_LEVEL_CRITICAL, NULL, __FILE__, __FUNCTION__, __LINE__, NULL);

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(strcmp(expected_calls[0].log_callback_call.captured_output, "Error logging: invalid arguments") == 0);
}

/* Tests_SRS_LOG_SINK_CALLBACK_42_007: [ log_sink_callback.log shall obtain the time by calling time. ]*/
/* Tests_SRS_LOG_SINK_CALLBACK_42_008: [ log_sink_callback.log shall write the time to string by calling ctime. ]*/
/* Tests_SRS_LOG_SINK_CALLBACK_42_009: [ log_sink_callback.log shall create a line in the format: Time: {formatted time} File:{file}:{line} Func:{func} {optional context information} {formatted message}. ]*/
/* Tests_SRS_LOG_SINK_CALLBACK_42_018: [ log_sink_callback.log shall call log_callback with its context, log_level, and the formatted message. ]*/
static void log_sink_callback_log_calls_callback_with_one_CRITICAL_log_line(void)
{
    // arrange
    test_init();
    setup_mocks();

    setup_time_call();
    setup_ctime_call();
    setup_snprintf_call();
    setup_vsnprintf_call();
    setup_log_callback_call();

    // act
    int line_no = __LINE__;
    test_log_sink_callback_log(LOG_LEVEL_CRITICAL, NULL, __FILE__, __FUNCTION__, line_no, "test");

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(expected_calls[0].time_call.captured__time == NULL);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(expected_calls[actual_call_count - 1].log_callback_call.captured_context == (void*)0x42);
    POOR_MANS_ASSERT(expected_calls[actual_call_count - 1].log_callback_call.captured_log_level == LOG_LEVEL_CRITICAL);
    validate_log_line(expected_calls[actual_call_count - 1].log_callback_call.captured_output, "Time:%%s %%s %%d %%d:%%d:%%d %%d File:%s:%d Func:%s %%[^\r\n]", __FILE__, line_no, __FUNCTION__, "test");
}

/* Tests_SRS_LOG_SINK_CALLBACK_42_007: [ log_sink_callback.log shall obtain the time by calling time. ]*/
/* Tests_SRS_LOG_SINK_CALLBACK_42_008: [ log_sink_callback.log shall write the time to string by calling ctime. ]*/
/* Tests_SRS_LOG_SINK_CALLBACK_42_009: [ log_sink_callback.log shall create a line in the format: Time: {formatted time} File:{file}:{line} Func:{func} {optional context information} {formatted message}. ]*/
/* Tests_SRS_LOG_SINK_CALLBACK_42_018: [ log_sink_callback.log shall call log_callback with its context, log_level, and the formatted message. ]*/
static void log_sink_callback_log_calls_callback_with_one_ERROR_log_line(void)
{
    // arrange
    test_init();
    setup_mocks();

    setup_time_call();
    setup_ctime_call();
    setup_snprintf_call();
    setup_vsnprintf_call();
    setup_log_callback_call();

    // act
    int line_no = __LINE__;
    test_log_sink_callback_log(LOG_LEVEL_ERROR, NULL, __FILE__, __FUNCTION__, line_no, "other message");

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(expected_calls[0].time_call.captured__time == NULL);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(expected_calls[actual_call_count - 1].log_callback_call.captured_context == (void*)0x42);
    POOR_MANS_ASSERT(expected_calls[actual_call_count - 1].log_callback_call.captured_log_level == LOG_LEVEL_ERROR);
    validate_log_line(expected_calls[actual_call_count - 1].log_callback_call.captured_output, "Time:%%s %%s %%d %%d:%%d:%%d %%d File:%s:%d Func:%s %%[^\r\n]", __FILE__, line_no, __FUNCTION__, "other message");
}

/* Tests_SRS_LOG_SINK_CALLBACK_42_007: [ log_sink_callback.log shall obtain the time by calling time. ]*/
/* Tests_SRS_LOG_SINK_CALLBACK_42_008: [ log_sink_callback.log shall write the time to string by calling ctime. ]*/
/* Tests_SRS_LOG_SINK_CALLBACK_42_009: [ log_sink_callback.log shall create a line in the format: Time: {formatted time} File:{file}:{line} Func:{func} {optional context information} {formatted message}. ]*/
/* Tests_SRS_LOG_SINK_CALLBACK_42_018: [ log_sink_callback.log shall call log_callback with its context, log_level, and the formatted message. ]*/
static void log_sink_callback_log_calls_callback_with_one_WARNING_log_line(void)
{
    // arrange
    test_init();
    setup_mocks();

    setup_time_call();
    setup_ctime_call();
    setup_snprintf_call();
    setup_vsnprintf_call();
    setup_log_callback_call();

    // act
    int line_no = __LINE__;
    test_log_sink_callback_log(LOG_LEVEL_WARNING, NULL, __FILE__, __FUNCTION__, line_no, "test");

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(expected_calls[0].time_call.captured__time == NULL);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(expected_calls[actual_call_count - 1].log_callback_call.captured_context == (void*)0x42);
    POOR_MANS_ASSERT(expected_calls[actual_call_count - 1].log_callback_call.captured_log_level == LOG_LEVEL_WARNING);
    validate_log_line(expected_calls[actual_call_count - 1].log_callback_call.captured_output, "Time:%%s %%s %%d %%d:%%d:%%d %%d File:%s:%d Func:%s %%[^\r\n]", __FILE__, line_no, __FUNCTION__, "test");
}

/* Tests_SRS_LOG_SINK_CALLBACK_42_007: [ log_sink_callback.log shall obtain the time by calling time. ]*/
/* Tests_SRS_LOG_SINK_CALLBACK_42_008: [ log_sink_callback.log shall write the time to string by calling ctime. ]*/
/* Tests_SRS_LOG_SINK_CALLBACK_42_009: [ log_sink_callback.log shall create a line in the format: Time: {formatted time} File:{file}:{line} Func:{func} {optional context information} {formatted message}. ]*/
/* Tests_SRS_LOG_SINK_CALLBACK_42_018: [ log_sink_callback.log shall call log_callback with its context, log_level, and the formatted message. ]*/
static void log_sink_callback_log_calls_callback_with_one_INFO_log_line(void)
{
    // arrange
    test_init();
    setup_mocks();

    setup_time_call();
    setup_ctime_call();
    setup_snprintf_call();
    setup_vsnprintf_call();
    setup_log_callback_call();

    // act
    int line_no = __LINE__;
    test_log_sink_callback_log(LOG_LEVEL_INFO, NULL, __FILE__, __FUNCTION__, line_no, "test");

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(expected_calls[0].time_call.captured__time == NULL);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(expected_calls[actual_call_count - 1].log_callback_call.captured_context == (void*)0x42);
    POOR_MANS_ASSERT(expected_calls[actual_call_count - 1].log_callback_call.captured_log_level == LOG_LEVEL_INFO);
    validate_log_line(expected_calls[actual_call_count - 1].log_callback_call.captured_output, "Time:%%s %%s %%d %%d:%%d:%%d %%d File:%s:%d Func:%s %%[^\r\n]", __FILE__, line_no, __FUNCTION__, "test");
}

/* Tests_SRS_LOG_SINK_CALLBACK_42_007: [ log_sink_callback.log shall obtain the time by calling time. ]*/
/* Tests_SRS_LOG_SINK_CALLBACK_42_008: [ log_sink_callback.log shall write the time to string by calling ctime. ]*/
/* Tests_SRS_LOG_SINK_CALLBACK_42_009: [ log_sink_callback.log shall create a line in the format: Time: {formatted time} File:{file}:{line} Func:{func} {optional context information} {formatted message}. ]*/
/* Tests_SRS_LOG_SINK_CALLBACK_42_018: [ log_sink_callback.log shall call log_callback with its context, log_level, and the formatted message. ]*/
static void log_sink_callback_log_calls_callback_with_one_VERBOSE_log_line(void)
{
    // arrange
    test_init();
    setup_mocks();

    setup_time_call();
    setup_ctime_call();
    setup_snprintf_call();
    setup_vsnprintf_call();
    setup_log_callback_call();

    // act
    int line_no = __LINE__;
    test_log_sink_callback_log(LOG_LEVEL_VERBOSE, NULL, __FILE__, __FUNCTION__, line_no, "a very long and verbose message here 42");

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(expected_calls[0].time_call.captured__time == NULL);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(expected_calls[actual_call_count - 1].log_callback_call.captured_context == (void*)0x42);
    POOR_MANS_ASSERT(expected_calls[actual_call_count - 1].log_callback_call.captured_log_level == LOG_LEVEL_VERBOSE);
    validate_log_line(expected_calls[actual_call_count - 1].log_callback_call.captured_output, "Time:%%s %%s %%d %%d:%%d:%%d %%d File:%s:%d Func:%s %%[^\r\n]", __FILE__, line_no, __FUNCTION__, "a very long and verbose message here 42");
}

/* Tests_SRS_LOG_SINK_CALLBACK_42_017: [ If any encoding error occurs during formatting of the line (i.e. if any printf class functions fails), log_sink_callback.log shall call the log_callback with Error formatting log line and return. ]*/
static void when_snprintf_fails_log_sink_callback_log_prints_error_formatting(void)
{
    // arrange
    test_init();
    setup_mocks();

    setup_time_call();
    setup_ctime_call();
    setup_snprintf_call();
    setup_log_callback_call();
    expected_calls[2].snprintf_call.override_result = true;
    expected_calls[2].snprintf_call.call_result = -1;

    // act
    int line_no = __LINE__;
    test_log_sink_callback_log(LOG_LEVEL_VERBOSE, NULL, __FILE__, __FUNCTION__, line_no, "test");

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(expected_calls[0].time_call.captured__time == NULL);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(expected_calls[actual_call_count - 1].log_callback_call.captured_context == (void*)0x42);
    POOR_MANS_ASSERT(strcmp(expected_calls[actual_call_count - 1].log_callback_call.captured_output, "Error formatting log line") == 0);
}

/* Tests_SRS_LOG_SINK_CALLBACK_42_017: [ If any encoding error occurs during formatting of the line (i.e. if any printf class functions fails), log_sink_callback.log shall call the log_callback with Error formatting log line and return. ]*/
static void when_vsnprintf_fails_log_sink_callback_log_prints_error_formatting(void)
{
    // arrange
    test_init();
    setup_mocks();

    setup_time_call();
    setup_ctime_call();
    setup_snprintf_call();
    setup_vsnprintf_call();
    setup_log_callback_call();
    expected_calls[3].vsnprintf_call.override_result = true;
    expected_calls[3].vsnprintf_call.call_result = -1;

    // act
    int line_no = __LINE__;
    test_log_sink_callback_log(LOG_LEVEL_VERBOSE, NULL, __FILE__, __FUNCTION__, line_no, "test");

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(expected_calls[0].time_call.captured__time == NULL);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(expected_calls[actual_call_count - 1].log_callback_call.captured_context == (void*)0x42);
    POOR_MANS_ASSERT(strcmp(expected_calls[actual_call_count - 1].log_callback_call.captured_output, "Error formatting log line") == 0);
}

/* Tests_SRS_LOG_SINK_CALLBACK_42_010: [ If the call to time fails then log_sink_callback.log shall format the time as NULL. ]*/
static void when_time_fails_log_sink_callback_log_prints_time_as_NULL(void)
{
    // arrange
    test_init();
    setup_mocks();

    setup_time_call();
    setup_snprintf_call();
    setup_vsnprintf_call();
    setup_log_callback_call();
    expected_calls[0].time_call.override_result = true;
    expected_calls[0].time_call.call_result = (time_t)-1;

    // act
    int line_no = __LINE__;
    test_log_sink_callback_log(LOG_LEVEL_VERBOSE, NULL, __FILE__, __FUNCTION__, line_no, "test");

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(expected_calls[0].time_call.captured__time == NULL);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(expected_calls[actual_call_count - 1].log_callback_call.captured_context == (void*)0x42);
    POOR_MANS_ASSERT(expected_calls[actual_call_count - 1].log_callback_call.captured_log_level == LOG_LEVEL_VERBOSE);
    validate_log_line_with_NULL_time(expected_calls[actual_call_count - 1].log_callback_call.captured_output, "Time:NULL File:%s:%d Func:%s %%[^\r\n]", __FILE__, line_no, __FUNCTION__, "test");
}

/* Tests_SRS_LOG_SINK_CALLBACK_42_011: [ If the call to ctime fails then log_sink_callback.log shall format the time as NULL. ]*/
static void when_ctime_returns_NULL_log_sink_callback_log_prints_time_as_NULL(void)
{
    // arrange
    test_init();
    setup_mocks();

    setup_time_call();
    setup_ctime_call();
    setup_snprintf_call();
    setup_vsnprintf_call();
    setup_log_callback_call();
    expected_calls[1].ctime_call.override_result = true;
    expected_calls[1].ctime_call.call_result = NULL;

    // act
    int line_no = __LINE__;
    test_log_sink_callback_log(LOG_LEVEL_VERBOSE, NULL, __FILE__, __FUNCTION__, line_no, "test");

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(expected_calls[0].time_call.captured__time == NULL);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(expected_calls[actual_call_count - 1].log_callback_call.captured_context == (void*)0x42);
    POOR_MANS_ASSERT(expected_calls[actual_call_count - 1].log_callback_call.captured_log_level == LOG_LEVEL_VERBOSE);
    validate_log_line_with_NULL_time(expected_calls[actual_call_count - 1].log_callback_call.captured_output, "Time:NULL File:%s:%d Func:%s %%[^\r\n]", __FILE__, line_no, __FUNCTION__, "test");
}

/* Tests_SRS_LOG_SINK_CALLBACK_42_012: [ If log_context is non-NULL: ]*/
/* Tests_SRS_LOG_SINK_CALLBACK_42_013: [ log_sink_callback.log shall call log_context_get_property_value_pair_count to obtain the count of properties. ]*/
/* Tests_SRS_LOG_SINK_CALLBACK_42_014: [ log_sink_callback.log shall call log_context_get_property_value_pairs to obtain the properties. ]*/
/* Tests_SRS_LOG_SINK_CALLBACK_42_015: [ log_sink_callback.log shall call log_context_property_to_string to write the properties to the string buffer. ]*/
static void log_sink_callback_log_with_non_NULL_context_prints_one_property(void)
{
    // arrange
    test_init();
    setup_mocks();

    setup_time_call();
    setup_ctime_call();
    setup_snprintf_call();
    setup_log_context_get_property_value_pair_count_call();
    setup_log_context_get_property_value_pairs_call();

    setup_log_context_property_to_string_call();

    setup_vsnprintf_call();
    setup_log_callback_call();

    LOG_CONTEXT_LOCAL_DEFINE(context_1, NULL, LOG_CONTEXT_PROPERTY(int32_t, x, 42));

    // act
    int line_no = __LINE__;
    test_log_sink_callback_log(LOG_LEVEL_VERBOSE, &context_1, __FILE__, __FUNCTION__, line_no, "test");

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(expected_calls[0].time_call.captured__time == NULL);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(expected_calls[actual_call_count - 1].log_callback_call.captured_context == (void*)0x42);
    POOR_MANS_ASSERT(expected_calls[actual_call_count - 1].log_callback_call.captured_log_level == LOG_LEVEL_VERBOSE);
    validate_log_line(expected_calls[actual_call_count - 1].log_callback_call.captured_output, "Time:%%s %%s %%d %%d:%%d:%%d %%d File:%s:%d Func:%s %%[^\r\n]", __FILE__, line_no, __FUNCTION__, "{ x=42 } test");
}

/* Tests_SRS_LOG_SINK_CALLBACK_42_012: [ If log_context is non-NULL: ]*/
/* Tests_SRS_LOG_SINK_CALLBACK_42_013: [ log_sink_callback.log shall call log_context_get_property_value_pair_count to obtain the count of properties. ]*/
/* Tests_SRS_LOG_SINK_CALLBACK_42_014: [ log_sink_callback.log shall call log_context_get_property_value_pairs to obtain the properties. ]*/
/* Tests_SRS_LOG_SINK_CALLBACK_42_015: [ log_sink_callback.log shall call log_context_property_to_string to write the properties to the string buffer. ]*/
static void log_sink_callback_log_with_non_NULL_context_prints_2_properties(void)
{
    // arrange
    test_init();
    setup_mocks();

    setup_time_call();
    setup_ctime_call();
    setup_snprintf_call();
    setup_log_context_get_property_value_pair_count_call();
    setup_log_context_get_property_value_pairs_call();

    setup_log_context_property_to_string_call();

    setup_vsnprintf_call();
    setup_log_callback_call();

    LOG_CONTEXT_LOCAL_DEFINE(context_1, NULL, LOG_CONTEXT_PROPERTY(int32_t, x, 42), LOG_CONTEXT_PROPERTY(uint32_t, y, 1));

    // act
    int line_no = __LINE__;
    test_log_sink_callback_log(LOG_LEVEL_VERBOSE, &context_1, __FILE__, __FUNCTION__, line_no, "test");

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(expected_calls[0].time_call.captured__time == NULL);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(expected_calls[actual_call_count - 1].log_callback_call.captured_context == (void*)0x42);
    POOR_MANS_ASSERT(expected_calls[actual_call_count - 1].log_callback_call.captured_log_level == LOG_LEVEL_VERBOSE);
    validate_log_line(expected_calls[actual_call_count - 1].log_callback_call.captured_output, "Time:%%s %%s %%d %%d:%%d:%%d %%d File:%s:%d Func:%s %%[^\r\n]", __FILE__, line_no, __FUNCTION__, "{ x=42 y=1 } test");
}

/* Tests_SRS_LOG_SINK_CALLBACK_42_012: [ If log_context is non-NULL: ]*/
/* Tests_SRS_LOG_SINK_CALLBACK_42_013: [ log_sink_callback.log shall call log_context_get_property_value_pair_count to obtain the count of properties. ]*/
/* Tests_SRS_LOG_SINK_CALLBACK_42_014: [ log_sink_callback.log shall call log_context_get_property_value_pairs to obtain the properties. ]*/
/* Tests_SRS_LOG_SINK_CALLBACK_42_015: [ log_sink_callback.log shall call log_context_property_to_string to write the properties to the string buffer. ]*/
static void log_sink_callback_log_with_non_NULL_context_with_2_levels_works(void)
{
    // arrange
    test_init();
    setup_mocks();

    setup_time_call();
    setup_ctime_call();
    setup_snprintf_call();
    setup_log_context_get_property_value_pair_count_call();
    setup_log_context_get_property_value_pairs_call();

    setup_log_context_property_to_string_call();

    setup_vsnprintf_call();
    setup_log_callback_call();

    LOG_CONTEXT_LOCAL_DEFINE(context_1, NULL, LOG_CONTEXT_PROPERTY(int32_t, x, 42));
    LOG_CONTEXT_LOCAL_DEFINE(context_2, &context_1, LOG_CONTEXT_PROPERTY(uint32_t, y, 1));

    // act
    int line_no = __LINE__;
    test_log_sink_callback_log(LOG_LEVEL_VERBOSE, &context_2, __FILE__, __FUNCTION__, line_no, "test");

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(expected_calls[0].time_call.captured__time == NULL);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(expected_calls[actual_call_count - 1].log_callback_call.captured_context == (void*)0x42);
    POOR_MANS_ASSERT(expected_calls[actual_call_count - 1].log_callback_call.captured_log_level == LOG_LEVEL_VERBOSE);
    validate_log_line(expected_calls[actual_call_count - 1].log_callback_call.captured_output, "Time:%%s %%s %%d %%d:%%d:%%d %%d File:%s:%d Func:%s %%[^\r\n]", __FILE__, line_no, __FUNCTION__, "{ { x=42 } y=1 } test");
}

/* Tests_SRS_LOG_SINK_CALLBACK_42_012: [ If log_context is non-NULL: ]*/
/* Tests_SRS_LOG_SINK_CALLBACK_42_013: [ log_sink_callback.log shall call log_context_get_property_value_pair_count to obtain the count of properties. ]*/
/* Tests_SRS_LOG_SINK_CALLBACK_42_014: [ log_sink_callback.log shall call log_context_get_property_value_pairs to obtain the properties. ]*/
/* Tests_SRS_LOG_SINK_CALLBACK_42_015: [ log_sink_callback.log shall call log_context_property_to_string to write the properties to the string buffer. ]*/
static void log_sink_callback_log_with_non_NULL_named_contexts_with_2_levels_works(void)
{
    // arrange
    test_init();
    setup_mocks();

    setup_time_call();
    setup_ctime_call();
    setup_snprintf_call();
    setup_log_context_get_property_value_pair_count_call();
    setup_log_context_get_property_value_pairs_call();

    setup_log_context_property_to_string_call();

    setup_vsnprintf_call();
    setup_log_callback_call();

    LOG_CONTEXT_LOCAL_DEFINE(context_1, NULL, LOG_CONTEXT_NAME(haga), LOG_CONTEXT_PROPERTY(int32_t, x, 42));
    LOG_CONTEXT_LOCAL_DEFINE(context_2, &context_1, LOG_CONTEXT_NAME(uaga), LOG_CONTEXT_PROPERTY(uint32_t, y, 1));

    // act
    int line_no = __LINE__;
    test_log_sink_callback_log(LOG_LEVEL_VERBOSE, &context_2, __FILE__, __FUNCTION__, line_no, "test");

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(expected_calls[0].time_call.captured__time == NULL);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(expected_calls[actual_call_count - 1].log_callback_call.captured_context == (void*)0x42);
    POOR_MANS_ASSERT(expected_calls[actual_call_count - 1].log_callback_call.captured_log_level == LOG_LEVEL_VERBOSE);
    validate_log_line(expected_calls[actual_call_count - 1].log_callback_call.captured_output, "Time:%%s %%s %%d %%d:%%d:%%d %%d File:%s:%d Func:%s %%[^\r\n]", __FILE__, line_no, __FUNCTION__, "uaga={ haga={ x=42 } y=1 } test");
}

/* Tests_SRS_LOG_SINK_CALLBACK_42_012: [ If log_context is non-NULL: ]*/
/* Tests_SRS_LOG_SINK_CALLBACK_42_013: [ log_sink_callback.log shall call log_context_get_property_value_pair_count to obtain the count of properties. ]*/
/* Tests_SRS_LOG_SINK_CALLBACK_42_014: [ log_sink_callback.log shall call log_context_get_property_value_pairs to obtain the properties. ]*/
/* Tests_SRS_LOG_SINK_CALLBACK_42_015: [ log_sink_callback.log shall call log_context_property_to_string to write the properties to the string buffer. ]*/
static void log_sink_callback_log_with_non_NULL_empty_context_works(void)
{
    // arrange
    test_init();
    setup_mocks();

    setup_time_call();
    setup_ctime_call();
    setup_snprintf_call();
    setup_log_context_get_property_value_pair_count_call();
    setup_log_context_get_property_value_pairs_call();

    setup_log_context_property_to_string_call();

    setup_vsnprintf_call();
    setup_log_callback_call();

    LOG_CONTEXT_LOCAL_DEFINE(context_1, NULL);

    // act
    int line_no = __LINE__;
    test_log_sink_callback_log(LOG_LEVEL_VERBOSE, &context_1, __FILE__, __FUNCTION__, line_no, "test");

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(expected_calls[0].time_call.captured__time == NULL);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(expected_calls[actual_call_count - 1].log_callback_call.captured_context == (void*)0x42);
    POOR_MANS_ASSERT(expected_calls[actual_call_count - 1].log_callback_call.captured_log_level == LOG_LEVEL_VERBOSE);
    validate_log_line(expected_calls[actual_call_count - 1].log_callback_call.captured_output, "Time:%%s %%s %%d %%d:%%d:%%d %%d File:%s:%d Func:%s %%[^\r\n]", __FILE__, line_no, __FUNCTION__, "{ } test");
}

/* Tests_SRS_LOG_SINK_CALLBACK_42_012: [ If log_context is non-NULL: ]*/
/* Tests_SRS_LOG_SINK_CALLBACK_42_013: [ log_sink_callback.log shall call log_context_get_property_value_pair_count to obtain the count of properties. ]*/
/* Tests_SRS_LOG_SINK_CALLBACK_42_014: [ log_sink_callback.log shall call log_context_get_property_value_pairs to obtain the properties. ]*/
/* Tests_SRS_LOG_SINK_CALLBACK_42_015: [ log_sink_callback.log shall call log_context_property_to_string to write the properties to the string buffer. ]*/
static void log_sink_callback_log_with_non_NULL_dynamically_allocated_context(void)
{
    // arrange
    test_init();
    setup_mocks();

    setup_time_call();
    setup_ctime_call();
    setup_snprintf_call();
    setup_log_context_get_property_value_pair_count_call();
    setup_log_context_get_property_value_pairs_call();

    setup_log_context_property_to_string_call();

    setup_vsnprintf_call();
    setup_log_callback_call();

    LOG_CONTEXT_HANDLE context_1;
    LOG_CONTEXT_CREATE(context_1, NULL, LOG_CONTEXT_PROPERTY(int32_t, x, 42));
    LOG_CONTEXT_HANDLE context_2;
    LOG_CONTEXT_CREATE(context_2, context_1, LOG_CONTEXT_PROPERTY(uint32_t, y, 1));

    // act
    int line_no = __LINE__;
    test_log_sink_callback_log(LOG_LEVEL_VERBOSE, context_2, __FILE__, __FUNCTION__, line_no, "test");

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(expected_calls[0].time_call.captured__time == NULL);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(expected_calls[actual_call_count - 1].log_callback_call.captured_context == (void*)0x42);
    POOR_MANS_ASSERT(expected_calls[actual_call_count - 1].log_callback_call.captured_log_level == LOG_LEVEL_VERBOSE);
    validate_log_line(expected_calls[actual_call_count - 1].log_callback_call.captured_output, "Time:%%s %%s %%d %%d:%%d:%%d %%d File:%s:%d Func:%s %%[^\r\n]", __FILE__, line_no, __FUNCTION__, "{ { x=42 } y=1 } test");

    // cleanup
    LOG_CONTEXT_DESTROY(context_1);
    LOG_CONTEXT_DESTROY(context_2);
}

/* Tests_SRS_LOG_SINK_CALLBACK_42_017: [ If any encoding error occurs during formatting of the line (i.e. if any printf class functions fails), log_sink_callback.log shall call the log_callback with Error formatting log line and return. ]*/
static void when_snprintf_fails_for_context_open_brace_log_sink_callback_log_prints_error_formatting(void)
{
    // arrange
    test_init();
    setup_mocks();

    setup_time_call();
    setup_ctime_call();
    setup_snprintf_call();
    setup_log_context_get_property_value_pair_count_call();
    setup_log_context_get_property_value_pairs_call();

    setup_log_context_property_to_string_call();
    setup_log_callback_call();

    expected_calls[5].log_context_property_to_string_call.override_result = true;
    expected_calls[5].log_context_property_to_string_call.call_result = -1;

    LOG_CONTEXT_LOCAL_DEFINE(context_1, NULL, LOG_CONTEXT_PROPERTY(int32_t, x, 42));
    LOG_CONTEXT_LOCAL_DEFINE(context_2, &context_1, LOG_CONTEXT_PROPERTY(uint32_t, y, 1));

    // act
    int line_no = __LINE__;
    test_log_sink_callback_log(LOG_LEVEL_VERBOSE, &context_2, __FILE__, __FUNCTION__, line_no, "test");

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(expected_calls[0].time_call.captured__time == NULL);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(expected_calls[actual_call_count - 1].log_callback_call.captured_context == (void*)0x42);
    POOR_MANS_ASSERT(strcmp(expected_calls[actual_call_count - 1].log_callback_call.captured_output, "Error formatting log line") == 0);
}

/* Tests_SRS_LOG_SINK_CALLBACK_42_016: [ log_sink_callback.log shall include at most LOG_MAX_MESSAGE_LENGTH characters including the null terminator in the callback argument (the rest of the context shall be truncated). ]*/
static void when_printing_the_message_exceeds_log_line_size_it_is_truncated(void)
{
    // arrange
    test_init();
    setup_mocks();

    setup_time_call();
    setup_snprintf_call();
    setup_vsnprintf_call();
    setup_log_callback_call();

    expected_calls[0].time_call.override_result = true;
    expected_calls[0].time_call.call_result = (time_t)-1;

    char* message_string_too_big = malloc(LOG_MAX_MESSAGE_LENGTH);
    POOR_MANS_ASSERT(message_string_too_big != NULL);

    (void)memset(message_string_too_big, 'x', LOG_MAX_MESSAGE_LENGTH - 1);
    message_string_too_big[LOG_MAX_MESSAGE_LENGTH - 1] = '\0';

    // act
    int line_no = __LINE__;
    test_log_sink_callback_log(LOG_LEVEL_VERBOSE, NULL, __FILE__, __FUNCTION__, line_no, message_string_too_big);

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(expected_calls[0].time_call.captured__time == NULL);
    POOR_MANS_ASSERT(actual_and_expected_match);

    char expected_string[LOG_MAX_MESSAGE_LENGTH];
    (void)snprintf(expected_string, sizeof(expected_string), "Time:NULL File:%s:%d Func:%s %s",
        __FILE__, line_no, __FUNCTION__, message_string_too_big);
    POOR_MANS_ASSERT(expected_calls[actual_call_count - 1].log_callback_call.captured_context == (void*)0x42);
    POOR_MANS_ASSERT(expected_calls[actual_call_count - 1].log_callback_call.captured_log_level == LOG_LEVEL_VERBOSE);
    POOR_MANS_ASSERT(strcmp(expected_calls[actual_call_count - 1].log_callback_call.captured_output, expected_string) == 0);

    // cleanup
    free(message_string_too_big);
}

/* Tests_SRS_LOG_SINK_CALLBACK_42_016: [ log_sink_callback.log shall include at most LOG_MAX_MESSAGE_LENGTH characters including the null terminator in the callback argument (the rest of the context shall be truncated). ]*/
static void when_printing_the_file_exceeds_log_line_size_it_is_truncated(void)
{
    // arrange
    test_init();
    setup_mocks();

    setup_time_call();
    setup_snprintf_call();
    setup_log_callback_call();

    expected_calls[0].time_call.override_result = true;
    expected_calls[0].time_call.call_result = (time_t)-1;

    char* file_too_big = malloc(LOG_MAX_MESSAGE_LENGTH);
    POOR_MANS_ASSERT(file_too_big != NULL);

    (void)memset(file_too_big, 'x', LOG_MAX_MESSAGE_LENGTH - 1);
    file_too_big[LOG_MAX_MESSAGE_LENGTH - 1] = '\0';

    // act
    int line_no = __LINE__;
    test_log_sink_callback_log(LOG_LEVEL_VERBOSE, NULL, file_too_big, __FUNCTION__, line_no, "a");

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(expected_calls[0].time_call.captured__time == NULL);
    POOR_MANS_ASSERT(actual_and_expected_match);

    char expected_string[LOG_MAX_MESSAGE_LENGTH];
    (void)snprintf(expected_string, sizeof(expected_string), "Time:NULL File:%s:%d Func:%s a",
        file_too_big, line_no, __FUNCTION__);
    POOR_MANS_ASSERT(expected_calls[actual_call_count - 1].log_callback_call.captured_context == (void*)0x42);
    POOR_MANS_ASSERT(expected_calls[actual_call_count - 1].log_callback_call.captured_log_level == LOG_LEVEL_VERBOSE);
    POOR_MANS_ASSERT(strcmp(expected_calls[actual_call_count - 1].log_callback_call.captured_output, expected_string) == 0);

    // cleanup
    free(file_too_big);
}

/* Tests_SRS_LOG_SINK_CALLBACK_42_016: [ log_sink_callback.log shall include at most LOG_MAX_MESSAGE_LENGTH characters including the null terminator in the callback argument (the rest of the context shall be truncated). ]*/
static void when_printing_a_property_value_exceeds_log_line_size_it_is_truncated(void)
{
    // arrange
    test_init();
    setup_mocks();

    setup_time_call();
    setup_snprintf_call();
    setup_log_context_get_property_value_pair_count_call();
    setup_log_context_get_property_value_pairs_call();
    setup_log_context_property_to_string_call();
    setup_log_callback_call();

    expected_calls[0].time_call.override_result = true;
    expected_calls[0].time_call.call_result = (time_t)-1;

    char* string_property_value_too_big = malloc(LOG_MAX_MESSAGE_LENGTH);
    POOR_MANS_ASSERT(string_property_value_too_big != NULL);

    (void)memset(string_property_value_too_big, 'x', LOG_MAX_MESSAGE_LENGTH - 1);
    string_property_value_too_big[LOG_MAX_MESSAGE_LENGTH - 1] = '\0';

    LOG_CONTEXT_HANDLE context_1;
    LOG_CONTEXT_CREATE(context_1, NULL, LOG_CONTEXT_STRING_PROPERTY(hagauaga, "%s", string_property_value_too_big));

    // act
    int line_no = __LINE__;
    test_log_sink_callback_log(LOG_LEVEL_VERBOSE, context_1, __FILE__, __FUNCTION__, line_no, "a");

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(expected_calls[0].time_call.captured__time == NULL);
    POOR_MANS_ASSERT(actual_and_expected_match);

    char expected_string[LOG_MAX_MESSAGE_LENGTH];
    (void)snprintf(expected_string, sizeof(expected_string), "Time:NULL File:%s:%d Func:%s { hagauaga=%s } a",
        __FILE__, line_no, __FUNCTION__, string_property_value_too_big);
    POOR_MANS_ASSERT(expected_calls[actual_call_count - 1].log_callback_call.captured_context == (void*)0x42);
    POOR_MANS_ASSERT(expected_calls[actual_call_count - 1].log_callback_call.captured_log_level == LOG_LEVEL_VERBOSE);
    POOR_MANS_ASSERT(strcmp(expected_calls[actual_call_count - 1].log_callback_call.captured_output, expected_string) == 0);

    // cleanup
    free(string_property_value_too_big);
    LOG_CONTEXT_DESTROY(context_1);
}

/* Tests_SRS_LOG_SINK_CALLBACK_42_016: [ log_sink_callback.log shall include at most LOG_MAX_MESSAGE_LENGTH characters including the null terminator in the callback argument (the rest of the context shall be truncated). ]*/
static void when_printing_a_property_name_exceeds_log_line_size_it_is_truncated(void)
{
    // arrange
    test_init();
    setup_mocks();

    setup_time_call();
    setup_snprintf_call();
    setup_log_context_get_property_value_pair_count_call();
    setup_log_context_get_property_value_pairs_call();
    setup_log_context_property_to_string_call();
    setup_log_callback_call();

    expected_calls[0].time_call.override_result = true;
    expected_calls[0].time_call.call_result = (time_t)-1;

    char* dummy_file_name = malloc(LOG_MAX_MESSAGE_LENGTH);
    POOR_MANS_ASSERT(dummy_file_name != NULL);

    char string_without_property_name[LOG_MAX_MESSAGE_LENGTH];
    int line_no = __LINE__;
    // obtain length without the property name (this is how many chars we have to subtract from the max log message length)
    int string_without_property_name_length = snprintf(string_without_property_name, sizeof(string_without_property_name), "Time:NULL File::%d Func:%s { ",
        line_no, __FUNCTION__);

    // build a filename of LOG_MAX_MESSAGE_LENGTH - string_without_property_name_length - 1
    // this will leave only one char space for the property name, but we will use 2 chars when time comes for it
    (void)memset(dummy_file_name, 'x', LOG_MAX_MESSAGE_LENGTH - string_without_property_name_length - 1 - 1);
    dummy_file_name[LOG_MAX_MESSAGE_LENGTH - string_without_property_name_length - 1 - 1] = '\0';

    LOG_CONTEXT_HANDLE context_1;
    // 2 chars property name
    LOG_CONTEXT_CREATE(context_1, NULL, LOG_CONTEXT_STRING_PROPERTY(ab, "x"));

    // act
    test_log_sink_callback_log(LOG_LEVEL_VERBOSE, context_1, dummy_file_name, __FUNCTION__, line_no, "a");

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(expected_calls[0].time_call.captured__time == NULL);
    POOR_MANS_ASSERT(actual_and_expected_match);

    char expected_string[LOG_MAX_MESSAGE_LENGTH];
    (void)snprintf(expected_string, sizeof(expected_string), "Time:NULL File:%s:%d Func:%s { ab=x } a",
        dummy_file_name, line_no, __FUNCTION__);
    POOR_MANS_ASSERT(expected_calls[actual_call_count - 1].log_callback_call.captured_context == (void*)0x42);
    POOR_MANS_ASSERT(expected_calls[actual_call_count - 1].log_callback_call.captured_log_level == LOG_LEVEL_VERBOSE);
    POOR_MANS_ASSERT(strcmp(expected_calls[actual_call_count - 1].log_callback_call.captured_output, expected_string) == 0);

    // cleanup
    free(dummy_file_name);
    LOG_CONTEXT_DESTROY(context_1);
}

/* very "poor man's" way of testing, as no test harness and mocking framework are available */
int main(void)
{
    log_sink_callback_log_with_NULL_message_format_calls_callback_with_error();
    log_sink_callback_log_calls_callback_with_one_CRITICAL_log_line();
    log_sink_callback_log_calls_callback_with_one_ERROR_log_line();
    log_sink_callback_log_calls_callback_with_one_WARNING_log_line();
    log_sink_callback_log_calls_callback_with_one_INFO_log_line();
    log_sink_callback_log_calls_callback_with_one_VERBOSE_log_line();

    when_snprintf_fails_log_sink_callback_log_prints_error_formatting();
    when_vsnprintf_fails_log_sink_callback_log_prints_error_formatting();
    when_time_fails_log_sink_callback_log_prints_time_as_NULL();
    when_ctime_returns_NULL_log_sink_callback_log_prints_time_as_NULL();

    log_sink_callback_log_with_non_NULL_context_prints_one_property();
    log_sink_callback_log_with_non_NULL_context_prints_2_properties();
    log_sink_callback_log_with_non_NULL_context_with_2_levels_works();

    log_sink_callback_log_with_non_NULL_named_contexts_with_2_levels_works();

    log_sink_callback_log_with_non_NULL_empty_context_works();

    log_sink_callback_log_with_non_NULL_dynamically_allocated_context();

    when_snprintf_fails_for_context_open_brace_log_sink_callback_log_prints_error_formatting();

    when_printing_the_message_exceeds_log_line_size_it_is_truncated();
    when_printing_the_file_exceeds_log_line_size_it_is_truncated();
    when_printing_a_property_value_exceeds_log_line_size_it_is_truncated();
    when_printing_a_property_name_exceeds_log_line_size_it_is_truncated();

    return 0;
}
