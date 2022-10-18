// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "macro_utils/macro_utils.h"

#include "c_logging/log_context_property_basic_types.h"
#include "c_logging/log_context_property_type_ascii_char_ptr.h"
#include "c_logging/log_context_property_value_pair.h"
#include "c_logging/log_level.h"
#include "c_logging/log_sink_if.h"
#include "c_logging/log_context.h"
#include "c_logging/logger.h"

#include "c_logging/log_sink_console.h"

static size_t asserts_failed = 0;

// defines how many mock calls we can have
#define MAX_MOCK_CALL_COUNT (128)

#define MOCK_CALL_TYPE_VALUES \
    MOCK_CALL_TYPE_printf, \
    MOCK_CALL_TYPE_time, \
    MOCK_CALL_TYPE_ctime, \
    MOCK_CALL_TYPE_vsnprintf, \
    MOCK_CALL_TYPE_snprintf, \
    MOCK_CALL_TYPE_log_context_get_property_value_pair_count, \
    MOCK_CALL_TYPE_log_context_get_property_value_pairs \

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
            // also capture the result in a variable for comparisons in tests
            (void)vsnprintf(expected_calls[actual_call_count].u.printf_call.captured_output, sizeof(expected_calls[actual_call_count].u.printf_call.captured_output),
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
        if (expected_calls[actual_call_count].u.ctime_call.override_result)
        {
            result = expected_calls[actual_call_count].u.ctime_call.call_result;
        }
        else
        {
            expected_calls[actual_call_count].u.ctime_call.captured_timer = timer;
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
        if (expected_calls[actual_call_count].u.vsnprintf_call.override_result)
        {
            result = expected_calls[actual_call_count].u.vsnprintf_call.call_result;
        }
        else
        {
            expected_calls[actual_call_count].u.vsnprintf_call.captured_format_arg = format;

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
        if (expected_calls[actual_call_count].u.snprintf_call.override_result)
        {
            result = expected_calls[actual_call_count].u.snprintf_call.call_result;
        }
        else
        {
            expected_calls[actual_call_count].u.snprintf_call.captured_format_arg = format;

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
        result = UINT32_MAX;
    }
    else
    {
        if (expected_calls[actual_call_count].u.log_context_get_property_value_pair_count_call.override_result)
        {
            result = expected_calls[actual_call_count].u.log_context_get_property_value_pair_count_call.call_result;
        }
        else
        {
            expected_calls[actual_call_count].u.log_context_get_property_value_pair_count_call.captured_log_context = log_context;

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
        if (expected_calls[actual_call_count].u.log_context_get_property_value_pairs_call.override_result)
        {
            result = expected_calls[actual_call_count].u.log_context_get_property_value_pairs_call.call_result;
        }
        else
        {
            expected_calls[actual_call_count].u.log_context_get_property_value_pairs_call.captured_log_context = log_context;

            result = log_context_get_property_value_pairs(log_context);
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
    expected_calls[expected_call_count].u.time_call.override_result = false;
    expected_call_count++;
}

static void setup_ctime_call(void)
{
    expected_calls[expected_call_count].mock_call_type = MOCK_CALL_TYPE_ctime;
    expected_calls[expected_call_count].u.ctime_call.override_result = false;
    expected_call_count++;
}

static void setup_vsnprintf_call(void)
{
    expected_calls[expected_call_count].mock_call_type = MOCK_CALL_TYPE_vsnprintf;
    expected_calls[expected_call_count].u.vsnprintf_call.override_result = false;
    expected_call_count++;
}

static void setup_snprintf_call(void)
{
    expected_calls[expected_call_count].mock_call_type = MOCK_CALL_TYPE_snprintf;
    expected_calls[expected_call_count].u.snprintf_call.override_result = false;
    expected_call_count++;
}

static void setup_log_context_get_property_value_pair_count_call(void)
{
    expected_calls[expected_call_count].mock_call_type = MOCK_CALL_TYPE_log_context_get_property_value_pair_count;
    expected_calls[expected_call_count].u.log_context_get_property_value_pair_count_call.override_result = false;
    expected_call_count++;
}

static void setup_log_context_get_property_value_pairs_call(void)
{
    expected_calls[expected_call_count].mock_call_type = MOCK_CALL_TYPE_log_context_get_property_value_pairs;
    expected_calls[expected_call_count].u.log_context_get_property_value_pairs_call.override_result = false;
    expected_call_count++;
}

static void validate_log_line(const char* actual_string, const char* expected_format, const char* expected_log_level_string, const char* file, int line, const char* func, const char* expected_message)
{
    char expected_string[LOG_MAX_MESSAGE_LENGTH * 2];
    int snprintf_result = snprintf(expected_string, sizeof(expected_string), expected_format, expected_log_level_string, file, line, func, expected_message);
    POOR_MANS_ASSERT(snprintf_result > 0);
    char day_of_week[4];
    char month[4];
    int day;
    int hour;
    int minute;
    int second;
    int year;
    char reset_color_code[10];
    char anything_else[200];
    int scanned_values = sscanf(actual_string, expected_string, day_of_week, month, &day, &hour, &minute, &second, &year, reset_color_code, anything_else);
    POOR_MANS_ASSERT(scanned_values == 8); // the last one should not get scanned as there should be nothing past \r\n
    size_t actual_string_length = strlen(actual_string);
    POOR_MANS_ASSERT(actual_string[actual_string_length - 2] == '\r');
    POOR_MANS_ASSERT(actual_string[actual_string_length - 1] == '\n');
    POOR_MANS_ASSERT(strcmp(reset_color_code, "\x1b[0m") == 0);
}

static void validate_log_line_with_NULL_time(const char* actual_string, const char* expected_format, const char* expected_log_level_string, const char* file, int line, const char* func, const char* expected_message)
{
    char expected_string[LOG_MAX_MESSAGE_LENGTH * 2];
    int snprintf_result = snprintf(expected_string, sizeof(expected_string), expected_format, expected_log_level_string, file, line, func, expected_message);
    POOR_MANS_ASSERT(snprintf_result > 0);
    char reset_color_code[10];
    char anything_else[200];
    int scanned_values = sscanf(actual_string, expected_string, reset_color_code, anything_else);
    POOR_MANS_ASSERT(scanned_values == 1); // the last one should not get scanned as there should be nothing past \r\n
    size_t actual_string_length = strlen(actual_string);
    POOR_MANS_ASSERT(actual_string[actual_string_length - 2] == '\r');
    POOR_MANS_ASSERT(actual_string[actual_string_length - 1] == '\n');
    POOR_MANS_ASSERT(strcmp(reset_color_code, "\x1b[0m") == 0);
}

/* log_sink_console.log_sink_log */

/* Tests_SRS_LOG_SINK_CONSOLE_01_001: [ If message_format is NULL, log_sink_console.log_sink_log shall print an error and return. ]*/
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

/* Tests_SRS_LOG_SINK_CONSOLE_01_002: [ log_sink_console.log_sink_log shall obtain the time by calling time. ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_003: [ log_sink_console.log_sink_log shall convert the time to string by calling ctime. ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_004: [ log_sink_console.log_sink_log shall print a line in the format: {log_level} Time: {formatted time} File:{file}:{line} Func:{func} {optional context information} {formatted message} ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_005: [ In order to not break the line in multiple parts when displayed on the console, log_sink_console.log_sink_log shall print the line in such a way that only one printf call is made. ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_006: [ log_sink_console.log_sink_log shall color the lines using ANSI color codes (https://en.wikipedia.org/wiki/ANSI_escape_code#Colors), as follows: ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_007: [ LOG_LEVEL_CRITICAL shall be displayed with bright red \x1b[31;1m. ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_012: [ At the end of each line that is printed, the color shall be reset by using the \x1b[0m code. ]*/
static void log_sink_console_log_prints_one_CRITICAL_log_line(void)
{
    // arrange
    setup_mocks();
    setup_time_call();
    setup_ctime_call();
    setup_snprintf_call();
    setup_vsnprintf_call();
    setup_printf_call();

    // act
    int line_no = __LINE__;
    log_sink_console.log_sink_log(LOG_LEVEL_CRITICAL, NULL, __FILE__, __FUNCTION__, line_no, "test");

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(expected_calls[0].u.time_call.captured__time == NULL);
    POOR_MANS_ASSERT(actual_and_expected_match);
    validate_log_line(expected_calls[4].u.printf_call.captured_output, "\x1b[31;1m%s Time:%%s %%s %%d %%d:%%d:%%d %%d File:%s:%d Func:%s %s%%s\r\n%%s", MU_ENUM_TO_STRING(LOG_LEVEL, LOG_LEVEL_CRITICAL), __FILE__, line_no, __FUNCTION__, "test");
}

/* Tests_SRS_LOG_SINK_CONSOLE_01_002: [ log_sink_console.log_sink_log shall obtain the time by calling time. ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_003: [ log_sink_console.log_sink_log shall convert the time to string by calling ctime. ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_004: [ log_sink_console.log_sink_log shall print a line in the format: {log_level} Time: {formatted time} File:{file}:{line} Func:{func} {optional context information} {formatted message} ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_005: [ In order to not break the line in multiple parts when displayed on the console, log_sink_console.log_sink_log shall print the line in such a way that only one printf call is made. ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_006: [ log_sink_console.log_sink_log shall color the lines using ANSI color codes (https://en.wikipedia.org/wiki/ANSI_escape_code#Colors), as follows: ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_008: [ LOG_LEVEL_ERROR shall be displayed with red \x1b[31m. ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_012: [ At the end of each line that is printed, the color shall be reset by using the \x1b[0m code. ]*/
static void log_sink_console_log_prints_one_ERROR_log_line(void)
{
    // arrange
    setup_mocks();
    setup_time_call();
    setup_ctime_call();
    setup_snprintf_call();
    setup_vsnprintf_call();
    setup_printf_call();

    // act
    int line_no = __LINE__;
    log_sink_console.log_sink_log(LOG_LEVEL_ERROR, NULL, __FILE__, __FUNCTION__, line_no, "test");

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(expected_calls[0].u.time_call.captured__time == NULL);
    POOR_MANS_ASSERT(actual_and_expected_match);
    validate_log_line(expected_calls[4].u.printf_call.captured_output, "\x1b[31m%s Time:%%s %%s %%d %%d:%%d:%%d %%d File:%s:%d Func:%s %s%%s\r\n%%s", MU_ENUM_TO_STRING(LOG_LEVEL, LOG_LEVEL_ERROR), __FILE__, line_no, __FUNCTION__, "test");
}

/* Tests_SRS_LOG_SINK_CONSOLE_01_002: [ log_sink_console.log_sink_log shall obtain the time by calling time. ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_003: [ log_sink_console.log_sink_log shall convert the time to string by calling ctime. ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_004: [ log_sink_console.log_sink_log shall print a line in the format: {log_level} Time: {formatted time} File:{file}:{line} Func:{func} {optional context information} {formatted message} ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_005: [ In order to not break the line in multiple parts when displayed on the console, log_sink_console.log_sink_log shall print the line in such a way that only one printf call is made. ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_006: [ log_sink_console.log_sink_log shall color the lines using ANSI color codes (https://en.wikipedia.org/wiki/ANSI_escape_code#Colors), as follows: ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_009: [ LOG_LEVEL_WARNING shall be displayed with bright yellow \x1b[33;1m. ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_012: [ At the end of each line that is printed, the color shall be reset by using the \x1b[0m code. ]*/
static void log_sink_console_log_prints_one_WARNING_log_line(void)
{
    // arrange
    setup_mocks();
    setup_time_call();
    setup_ctime_call();
    setup_snprintf_call();
    setup_vsnprintf_call();
    setup_printf_call();

    // act
    int line_no = __LINE__;
    log_sink_console.log_sink_log(LOG_LEVEL_WARNING, NULL, __FILE__, __FUNCTION__, line_no, "test");

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(expected_calls[0].u.time_call.captured__time == NULL);
    POOR_MANS_ASSERT(actual_and_expected_match);
    validate_log_line(expected_calls[4].u.printf_call.captured_output, "\x1b[33;1m%s Time:%%s %%s %%d %%d:%%d:%%d %%d File:%s:%d Func:%s %s%%s\r\n%%s", MU_ENUM_TO_STRING(LOG_LEVEL, LOG_LEVEL_WARNING), __FILE__, line_no, __FUNCTION__, "test");
}

/* Tests_SRS_LOG_SINK_CONSOLE_01_002: [ log_sink_console.log_sink_log shall obtain the time by calling time. ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_003: [ log_sink_console.log_sink_log shall convert the time to string by calling ctime. ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_004: [ log_sink_console.log_sink_log shall print a line in the format: {log_level} Time: {formatted time} File:{file}:{line} Func:{func} {optional context information} {formatted message} ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_005: [ In order to not break the line in multiple parts when displayed on the console, log_sink_console.log_sink_log shall print the line in such a way that only one printf call is made. ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_006: [ log_sink_console.log_sink_log shall color the lines using ANSI color codes (https://en.wikipedia.org/wiki/ANSI_escape_code#Colors), as follows: ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_010: [ LOG_LEVEL_INFO shall be displayed with yellow \x1b[33m. ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_012: [ At the end of each line that is printed, the color shall be reset by using the \x1b[0m code. ]*/
static void log_sink_console_log_prints_one_INFO_log_line(void)
{
    // arrange
    setup_mocks();
    setup_time_call();
    setup_ctime_call();
    setup_snprintf_call();
    setup_vsnprintf_call();
    setup_printf_call();

    // act
    int line_no = __LINE__;
    log_sink_console.log_sink_log(LOG_LEVEL_INFO, NULL, __FILE__, __FUNCTION__, line_no, "test");

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(expected_calls[0].u.time_call.captured__time == NULL);
    POOR_MANS_ASSERT(actual_and_expected_match);
    validate_log_line(expected_calls[4].u.printf_call.captured_output, "\x1b[33m%s Time:%%s %%s %%d %%d:%%d:%%d %%d File:%s:%d Func:%s %s%%s\r\n%%s", MU_ENUM_TO_STRING(LOG_LEVEL, LOG_LEVEL_INFO), __FILE__, line_no, __FUNCTION__, "test");
}

/* Tests_SRS_LOG_SINK_CONSOLE_01_002: [ log_sink_console.log_sink_log shall obtain the time by calling time. ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_003: [ log_sink_console.log_sink_log shall convert the time to string by calling ctime. ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_004: [ log_sink_console.log_sink_log shall print a line in the format: {log_level} Time: {formatted time} File:{file}:{line} Func:{func} {optional context information} {formatted message} ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_005: [ In order to not break the line in multiple parts when displayed on the console, log_sink_console.log_sink_log shall print the line in such a way that only one printf call is made. ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_006: [ log_sink_console.log_sink_log shall color the lines using ANSI color codes (https://en.wikipedia.org/wiki/ANSI_escape_code#Colors), as follows: ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_011: [ LOG_LEVEL_VERBOSE shall be displayed with white \x1b[37m. ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_012: [ At the end of each line that is printed, the color shall be reset by using the \x1b[0m code. ]*/
static void log_sink_console_log_prints_one_VERBOSE_log_line(void)
{
    // arrange
    setup_mocks();
    setup_time_call();
    setup_ctime_call();
    setup_snprintf_call();
    setup_vsnprintf_call();
    setup_printf_call();

    // act
    int line_no = __LINE__;
    log_sink_console.log_sink_log(LOG_LEVEL_VERBOSE, NULL, __FILE__, __FUNCTION__, line_no, "test");

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(expected_calls[0].u.time_call.captured__time == NULL);
    POOR_MANS_ASSERT(actual_and_expected_match);
    validate_log_line(expected_calls[4].u.printf_call.captured_output, "\x1b[37m%s Time:%%s %%s %%d %%d:%%d:%%d %%d File:%s:%d Func:%s %s%%s\r\n%%s", MU_ENUM_TO_STRING(LOG_LEVEL, LOG_LEVEL_VERBOSE), __FILE__, line_no, __FUNCTION__, "test");
}

/* Tests_SRS_LOG_SINK_CONSOLE_01_022: [ If any encoding error occurs during formatting of the line (i.e. if any printf class functions fails), log_sink_console.log_sink_log shall print Error formatting log line and return. ]*/
static void when_snprintf_fails_log_sink_console_log_prints_error_formatting(void)
{
    // arrange
    setup_mocks();
    setup_time_call();
    setup_ctime_call();
    setup_snprintf_call();
    setup_printf_call();
    expected_calls[2].u.snprintf_call.override_result = true;
    expected_calls[2].u.snprintf_call.call_result = -1;

    // act
    int line_no = __LINE__;
    log_sink_console.log_sink_log(LOG_LEVEL_VERBOSE, NULL, __FILE__, __FUNCTION__, line_no, "test");

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(expected_calls[0].u.time_call.captured__time == NULL);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(strcmp(expected_calls[3].u.printf_call.captured_output, "Error formatting log line\r\n") == 0);
}

/* Tests_SRS_LOG_SINK_CONSOLE_01_022: [ If any encoding error occurs during formatting of the line (i.e. if any printf class functions fails), log_sink_console.log_sink_log shall print Error formatting log line and return. ]*/
static void when_vsnprintf_fails_log_sink_console_log_prints_error_formatting(void)
{
    // arrange
    setup_mocks();
    setup_time_call();
    setup_ctime_call();
    setup_snprintf_call();
    setup_vsnprintf_call();
    setup_printf_call();
    expected_calls[3].u.vsnprintf_call.override_result = true;
    expected_calls[3].u.vsnprintf_call.call_result = -1;

    // act
    int line_no = __LINE__;
    log_sink_console.log_sink_log(LOG_LEVEL_VERBOSE, NULL, __FILE__, __FUNCTION__, line_no, "test");

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(expected_calls[0].u.time_call.captured__time == NULL);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(strcmp(expected_calls[4].u.printf_call.captured_output, "Error formatting log line\r\n") == 0);
}

/* Tests_SRS_LOG_SINK_CONSOLE_01_023: [ If the call to time fails then log_sink_console.log_sink_log shall print the time as NULL. ]*/
static void when_time_fails_log_sink_console_log_prints_time_as_NULL(void)
{
    // arrange
    setup_mocks();
    setup_time_call();
    setup_snprintf_call();
    setup_vsnprintf_call();
    setup_printf_call();
    expected_calls[0].u.time_call.override_result = true;
    expected_calls[0].u.time_call.call_result = (time_t)-1;

    // act
    int line_no = __LINE__;
    log_sink_console.log_sink_log(LOG_LEVEL_VERBOSE, NULL, __FILE__, __FUNCTION__, line_no, "test");

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(expected_calls[0].u.time_call.captured__time == NULL);
    POOR_MANS_ASSERT(actual_and_expected_match);
    validate_log_line_with_NULL_time(expected_calls[3].u.printf_call.captured_output, "\x1b[37m%s Time:NULL File:%s:%d Func:%s %s%%s\r\n%%s", MU_ENUM_TO_STRING(LOG_LEVEL, LOG_LEVEL_VERBOSE), __FILE__, line_no, __FUNCTION__, "test");
}

/* Tests_SRS_LOG_SINK_CONSOLE_01_024: [ If the call to ctime fails then log_sink_console.log_sink_log shall print the time as NULL. ]*/
static void when_ctime_returns_NULL_log_sink_console_log_prints_time_as_NULL(void)
{
    // arrange
    setup_mocks();
    setup_time_call();
    setup_ctime_call();
    setup_snprintf_call();
    setup_vsnprintf_call();
    setup_printf_call();
    expected_calls[1].u.ctime_call.override_result = true;
    expected_calls[1].u.ctime_call.call_result = NULL;

    // act
    int line_no = __LINE__;
    log_sink_console.log_sink_log(LOG_LEVEL_VERBOSE, NULL, __FILE__, __FUNCTION__, line_no, "test");

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(expected_calls[0].u.time_call.captured__time == NULL);
    POOR_MANS_ASSERT(actual_and_expected_match);
    validate_log_line_with_NULL_time(expected_calls[4].u.printf_call.captured_output, "\x1b[37m%s Time:NULL File:%s:%d Func:%s %s%%s\r\n%%s", MU_ENUM_TO_STRING(LOG_LEVEL, LOG_LEVEL_VERBOSE), __FILE__, line_no, __FUNCTION__, "test");
}

/* Tests_SRS_LOG_SINK_CONSOLE_01_013: [ If log_context is non-NULL: ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_014: [ log_sink_console.log_sink_log shall call log_context_get_property_value_pair_count to obtain the count of properties to print. ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_015: [ log_sink_console.log_sink_log shall call log_context_get_property_value_pairs to obtain the properties to print. ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_016: [ For each property: ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_017: [ If the property type is struct (used as a container for context properties): ]*/
  /* Tests_SRS_LOG_SINK_CONSOLE_01_025: [ log_sink_console.log_sink_log shall print the struct property name and an opening brace. ]*/
  /* Tests_SRS_LOG_SINK_CONSOLE_01_018: [ log_sink_console.log_sink_log shall obtain the number of fields in the struct. ]*/
  /* Tests_SRS_LOG_SINK_CONSOLE_01_019: [ log_sink_console.log_sink_log shall print the next n properties as being the fields that are part of the struct. ]*/
  /* Tests_SRS_LOG_SINK_CONSOLE_01_026: [ log_sink_console.log_sink_log shall print a closing brace as end of the struct. ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_020: [ Otherwise log_sink_console.log_sink_log shall call to_string for the property and print its name and value. ]*/
static void log_sink_console_log_with_non_NULL_context_prints_one_property(void)
{
    // arrange
    setup_mocks();
    setup_time_call();
    setup_ctime_call();
    setup_snprintf_call();
    setup_log_context_get_property_value_pair_count_call();
    setup_log_context_get_property_value_pairs_call();
    
    setup_snprintf_call(); // context opening
    setup_snprintf_call(); // property
    setup_snprintf_call(); // context closing

    setup_vsnprintf_call();
    setup_printf_call();

    LOG_CONTEXT_LOCAL_DEFINE(context_1, NULL, LOG_CONTEXT_PROPERTY(int32_t, x, 42));

    // act
    int line_no = __LINE__;
    log_sink_console.log_sink_log(LOG_LEVEL_VERBOSE, &context_1, __FILE__, __FUNCTION__, line_no, "test");

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(expected_calls[0].u.time_call.captured__time == NULL);
    POOR_MANS_ASSERT(actual_and_expected_match);
    validate_log_line(expected_calls[actual_call_count - 1].u.printf_call.captured_output, "\x1b[37m%s Time:%%s %%s %%d %%d:%%d:%%d %%d File:%s:%d Func:%s { x = 42 } %s%%s\r\n%%s", MU_ENUM_TO_STRING(LOG_LEVEL, LOG_LEVEL_VERBOSE), __FILE__, line_no, __FUNCTION__, "test");
}

/* Tests_SRS_LOG_SINK_CONSOLE_01_013: [ If log_context is non-NULL: ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_014: [ log_sink_console.log_sink_log shall call log_context_get_property_value_pair_count to obtain the count of properties to print. ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_015: [ log_sink_console.log_sink_log shall call log_context_get_property_value_pairs to obtain the properties to print. ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_016: [ For each property: ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_017: [ If the property type is struct (used as a container for context properties): ]*/
  /* Tests_SRS_LOG_SINK_CONSOLE_01_025: [ log_sink_console.log_sink_log shall print the struct property name and an opening brace. ]*/
  /* Tests_SRS_LOG_SINK_CONSOLE_01_018: [ log_sink_console.log_sink_log shall obtain the number of fields in the struct. ]*/
  /* Tests_SRS_LOG_SINK_CONSOLE_01_019: [ log_sink_console.log_sink_log shall print the next n properties as being the fields that are part of the struct. ]*/
  /* Tests_SRS_LOG_SINK_CONSOLE_01_026: [ log_sink_console.log_sink_log shall print a closing brace as end of the struct. ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_020: [ Otherwise log_sink_console.log_sink_log shall call to_string for the property and print its name and value. ]*/
static void log_sink_console_log_with_non_NULL_context_prints_2_properties(void)
{
    // arrange
    setup_mocks();
    setup_time_call();
    setup_ctime_call();
    setup_snprintf_call();
    setup_log_context_get_property_value_pair_count_call();
    setup_log_context_get_property_value_pairs_call();

    setup_snprintf_call(); // context opening
    setup_snprintf_call(); // property
    setup_snprintf_call(); // property
    setup_snprintf_call(); // context closing

    setup_vsnprintf_call();
    setup_printf_call();

    LOG_CONTEXT_LOCAL_DEFINE(context_1, NULL, LOG_CONTEXT_PROPERTY(int32_t, x, 42), LOG_CONTEXT_PROPERTY(uint32_t, y, 1));

    // act
    int line_no = __LINE__;
    log_sink_console.log_sink_log(LOG_LEVEL_VERBOSE, &context_1, __FILE__, __FUNCTION__, line_no, "test");

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(expected_calls[0].u.time_call.captured__time == NULL);
    POOR_MANS_ASSERT(actual_and_expected_match);
    validate_log_line(expected_calls[actual_call_count - 1].u.printf_call.captured_output, "\x1b[37m%s Time:%%s %%s %%d %%d:%%d:%%d %%d File:%s:%d Func:%s { x=42 y=1 } %s%%s\r\n%%s", MU_ENUM_TO_STRING(LOG_LEVEL, LOG_LEVEL_VERBOSE), __FILE__, line_no, __FUNCTION__, "test");
}

/* Tests_SRS_LOG_SINK_CONSOLE_01_013: [ If log_context is non-NULL: ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_014: [ log_sink_console.log_sink_log shall call log_context_get_property_value_pair_count to obtain the count of properties to print. ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_015: [ log_sink_console.log_sink_log shall call log_context_get_property_value_pairs to obtain the properties to print. ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_016: [ For each property: ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_017: [ If the property type is struct (used as a container for context properties): ]*/
  /* Tests_SRS_LOG_SINK_CONSOLE_01_025: [ log_sink_console.log_sink_log shall print the struct property name and an opening brace. ]*/
  /* Tests_SRS_LOG_SINK_CONSOLE_01_018: [ log_sink_console.log_sink_log shall obtain the number of fields in the struct. ]*/
  /* Tests_SRS_LOG_SINK_CONSOLE_01_019: [ log_sink_console.log_sink_log shall print the next n properties as being the fields that are part of the struct. ]*/
  /* Tests_SRS_LOG_SINK_CONSOLE_01_026: [ log_sink_console.log_sink_log shall print a closing brace as end of the struct. ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_020: [ Otherwise log_sink_console.log_sink_log shall call to_string for the property and print its name and value. ]*/
static void log_sink_console_log_with_non_NULL_context_with_2_levels_works(void)
{
    // arrange
    setup_mocks();
    setup_time_call();
    setup_ctime_call();
    setup_snprintf_call();
    setup_log_context_get_property_value_pair_count_call();
    setup_log_context_get_property_value_pairs_call();

    setup_snprintf_call(); // context opening
    setup_snprintf_call(); // context opening
    setup_snprintf_call(); // property
    setup_snprintf_call(); // context closing
    setup_snprintf_call(); // property
    setup_snprintf_call(); // context closing

    setup_vsnprintf_call();
    setup_printf_call();

    LOG_CONTEXT_LOCAL_DEFINE(context_1, NULL, LOG_CONTEXT_PROPERTY(int32_t, x, 42));
    LOG_CONTEXT_LOCAL_DEFINE(context_2, &context_1, LOG_CONTEXT_PROPERTY(uint32_t, y, 1));

    // act
    int line_no = __LINE__;
    log_sink_console.log_sink_log(LOG_LEVEL_VERBOSE, &context_2, __FILE__, __FUNCTION__, line_no, "test");

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(expected_calls[0].u.time_call.captured__time == NULL);
    POOR_MANS_ASSERT(actual_and_expected_match);
    validate_log_line(expected_calls[actual_call_count - 1].u.printf_call.captured_output, "\x1b[37m%s Time:%%s %%s %%d %%d:%%d:%%d %%d File:%s:%d Func:%s { { x=42 } y=1 } %s%%s\r\n%%s", MU_ENUM_TO_STRING(LOG_LEVEL, LOG_LEVEL_VERBOSE), __FILE__, line_no, __FUNCTION__, "test");
}

/* Tests_SRS_LOG_SINK_CONSOLE_01_013: [ If log_context is non-NULL: ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_014: [ log_sink_console.log_sink_log shall call log_context_get_property_value_pair_count to obtain the count of properties to print. ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_015: [ log_sink_console.log_sink_log shall call log_context_get_property_value_pairs to obtain the properties to print. ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_016: [ For each property: ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_017: [ If the property type is struct (used as a container for context properties): ]*/
  /* Tests_SRS_LOG_SINK_CONSOLE_01_025: [ log_sink_console.log_sink_log shall print the struct property name and an opening brace. ]*/
  /* Tests_SRS_LOG_SINK_CONSOLE_01_018: [ log_sink_console.log_sink_log shall obtain the number of fields in the struct. ]*/
  /* Tests_SRS_LOG_SINK_CONSOLE_01_019: [ log_sink_console.log_sink_log shall print the next n properties as being the fields that are part of the struct. ]*/
  /* Tests_SRS_LOG_SINK_CONSOLE_01_026: [ log_sink_console.log_sink_log shall print a closing brace as end of the struct. ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_020: [ Otherwise log_sink_console.log_sink_log shall call to_string for the property and print its name and value. ]*/
static void log_sink_console_log_with_non_NULL_named_contexts_with_2_levels_works(void)
{
    // arrange
    setup_mocks();
    setup_time_call();
    setup_ctime_call();
    setup_snprintf_call();
    setup_log_context_get_property_value_pair_count_call();
    setup_log_context_get_property_value_pairs_call();

    setup_snprintf_call(); // context opening
    setup_snprintf_call(); // context opening
    setup_snprintf_call(); // property
    setup_snprintf_call(); // context closing
    setup_snprintf_call(); // property
    setup_snprintf_call(); // context closing

    setup_vsnprintf_call();
    setup_printf_call();

    LOG_CONTEXT_LOCAL_DEFINE(context_1, NULL, LOG_CONTEXT_NAME(haga), LOG_CONTEXT_PROPERTY(int32_t, x, 42));
    LOG_CONTEXT_LOCAL_DEFINE(context_2, &context_1, LOG_CONTEXT_NAME(uaga), LOG_CONTEXT_PROPERTY(uint32_t, y, 1));

    // act
    int line_no = __LINE__;
    log_sink_console.log_sink_log(LOG_LEVEL_VERBOSE, &context_2, __FILE__, __FUNCTION__, line_no, "test");

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(expected_calls[0].u.time_call.captured__time == NULL);
    POOR_MANS_ASSERT(actual_and_expected_match);
    validate_log_line(expected_calls[actual_call_count - 1].u.printf_call.captured_output, "\x1b[37m%s Time:%%s %%s %%d %%d:%%d:%%d %%d File:%s:%d Func:%s uaga = { haga = { x=42 } y=1 } %s%%s\r\n%%s", MU_ENUM_TO_STRING(LOG_LEVEL, LOG_LEVEL_VERBOSE), __FILE__, line_no, __FUNCTION__, "test");
}

/* Tests_SRS_LOG_SINK_CONSOLE_01_013: [ If log_context is non-NULL: ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_014: [ log_sink_console.log_sink_log shall call log_context_get_property_value_pair_count to obtain the count of properties to print. ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_015: [ log_sink_console.log_sink_log shall call log_context_get_property_value_pairs to obtain the properties to print. ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_016: [ For each property: ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_017: [ If the property type is struct (used as a container for context properties): ]*/
  /* Tests_SRS_LOG_SINK_CONSOLE_01_025: [ log_sink_console.log_sink_log shall print the struct property name and an opening brace. ]*/
  /* Tests_SRS_LOG_SINK_CONSOLE_01_018: [ log_sink_console.log_sink_log shall obtain the number of fields in the struct. ]*/
  /* Tests_SRS_LOG_SINK_CONSOLE_01_026: [ log_sink_console.log_sink_log shall print a closing brace as end of the struct. ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_020: [ Otherwise log_sink_console.log_sink_log shall call to_string for the property and print its name and value. ]*/
static void log_sink_console_log_with_non_NULL_empty_context_works(void)
{
    // arrange
    setup_mocks();
    setup_time_call();
    setup_ctime_call();
    setup_snprintf_call();
    setup_log_context_get_property_value_pair_count_call();
    setup_log_context_get_property_value_pairs_call();

    setup_snprintf_call(); // context opening
    setup_snprintf_call(); // context closing

    setup_vsnprintf_call();
    setup_printf_call();

    LOG_CONTEXT_LOCAL_DEFINE(context_1, NULL);

    // act
    int line_no = __LINE__;
    log_sink_console.log_sink_log(LOG_LEVEL_VERBOSE, &context_1, __FILE__, __FUNCTION__, line_no, "test");

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(expected_calls[0].u.time_call.captured__time == NULL);
    POOR_MANS_ASSERT(actual_and_expected_match);
    validate_log_line(expected_calls[actual_call_count - 1].u.printf_call.captured_output, "\x1b[37m%s Time:%%s %%s %%d %%d:%%d:%%d %%d File:%s:%d Func:%s { } %s%%s\r\n%%s", MU_ENUM_TO_STRING(LOG_LEVEL, LOG_LEVEL_VERBOSE), __FILE__, line_no, __FUNCTION__, "test");
}

/* Tests_SRS_LOG_SINK_CONSOLE_01_013: [ If log_context is non-NULL: ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_014: [ log_sink_console.log_sink_log shall call log_context_get_property_value_pair_count to obtain the count of properties to print. ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_015: [ log_sink_console.log_sink_log shall call log_context_get_property_value_pairs to obtain the properties to print. ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_016: [ For each property: ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_017: [ If the property type is struct (used as a container for context properties): ]*/
  /* Tests_SRS_LOG_SINK_CONSOLE_01_025: [ log_sink_console.log_sink_log shall print the struct property name and an opening brace. ]*/
  /* Tests_SRS_LOG_SINK_CONSOLE_01_018: [ log_sink_console.log_sink_log shall obtain the number of fields in the struct. ]*/
  /* Tests_SRS_LOG_SINK_CONSOLE_01_019: [ log_sink_console.log_sink_log shall print the next n properties as being the fields that are part of the struct. ]*/
  /* Tests_SRS_LOG_SINK_CONSOLE_01_026: [ log_sink_console.log_sink_log shall print a closing brace as end of the struct. ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_020: [ Otherwise log_sink_console.log_sink_log shall call to_string for the property and print its name and value. ]*/
static void log_sink_console_log_with_non_NULL_dynamically_allocated_context(void)
{
    // arrange
    setup_mocks();
    setup_time_call();
    setup_ctime_call();
    setup_snprintf_call();
    setup_log_context_get_property_value_pair_count_call();
    setup_log_context_get_property_value_pairs_call();

    setup_snprintf_call(); // context opening
    setup_snprintf_call(); // context opening
    setup_snprintf_call(); // property
    setup_snprintf_call(); // context closing
    setup_snprintf_call(); // property
    setup_snprintf_call(); // context closing

    setup_vsnprintf_call();
    setup_printf_call();

    LOG_CONTEXT_HANDLE context_1;
    LOG_CONTEXT_CREATE(context_1, NULL, LOG_CONTEXT_PROPERTY(int32_t, x, 42));
    LOG_CONTEXT_HANDLE context_2;
    LOG_CONTEXT_CREATE(context_2, context_1, LOG_CONTEXT_PROPERTY(uint32_t, y, 1));

    // act
    int line_no = __LINE__;
    log_sink_console.log_sink_log(LOG_LEVEL_VERBOSE, context_2, __FILE__, __FUNCTION__, line_no, "test");

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(expected_calls[0].u.time_call.captured__time == NULL);
    POOR_MANS_ASSERT(actual_and_expected_match);
    validate_log_line(expected_calls[actual_call_count - 1].u.printf_call.captured_output, "\x1b[37m%s Time:%%s %%s %%d %%d:%%d:%%d %%d File:%s:%d Func:%s { { x=42 } y=1 } %s%%s\r\n%%s", MU_ENUM_TO_STRING(LOG_LEVEL, LOG_LEVEL_VERBOSE), __FILE__, line_no, __FUNCTION__, "test");

    // cleanup
    LOG_CONTEXT_DESTROY(context_1);
    LOG_CONTEXT_DESTROY(context_2);
}

/* Tests_SRS_LOG_SINK_CONSOLE_01_022: [ If any encoding error occurs during formatting of the line (i.e. if any printf class functions fails), log_sink_console.log_sink_log shall print Error formatting log line and return. ]*/
static void when_snprintf_fails_for_context_open_brace_log_sink_console_log_prints_error_formatting(void)
{
    // arrange
    setup_mocks();
    setup_time_call();
    setup_ctime_call();
    setup_snprintf_call();
    setup_log_context_get_property_value_pair_count_call();
    setup_log_context_get_property_value_pairs_call();

    setup_snprintf_call(); // context opening
    setup_printf_call();

    expected_calls[5].u.snprintf_call.override_result = true;
    expected_calls[5].u.snprintf_call.call_result = -1;

    LOG_CONTEXT_LOCAL_DEFINE(context_1, NULL, LOG_CONTEXT_PROPERTY(int32_t, x, 42));
    LOG_CONTEXT_LOCAL_DEFINE(context_2, &context_1, LOG_CONTEXT_PROPERTY(uint32_t, y, 1));

    // act
    int line_no = __LINE__;
    log_sink_console.log_sink_log(LOG_LEVEL_VERBOSE, &context_2, __FILE__, __FUNCTION__, line_no, "test");

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(expected_calls[0].u.time_call.captured__time == NULL);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(strcmp(expected_calls[actual_call_count - 1].u.printf_call.captured_output, "Error formatting log line\r\n") == 0);
}

/* Tests_SRS_LOG_SINK_CONSOLE_01_022: [ If any encoding error occurs during formatting of the line (i.e. if any printf class functions fails), log_sink_console.log_sink_log shall print Error formatting log line and return. ]*/
static void when_snprintf_fails_for_inner_context_open_brace_log_sink_console_log_prints_error_formatting(void)
{
    // arrange
    setup_mocks();
    setup_time_call();
    setup_ctime_call();
    setup_snprintf_call();
    setup_log_context_get_property_value_pair_count_call();
    setup_log_context_get_property_value_pairs_call();

    setup_snprintf_call(); // context opening
    setup_snprintf_call(); // context opening
    setup_printf_call();

    expected_calls[6].u.snprintf_call.override_result = true;
    expected_calls[6].u.snprintf_call.call_result = -1;

    LOG_CONTEXT_LOCAL_DEFINE(context_1, NULL, LOG_CONTEXT_PROPERTY(int32_t, x, 42));
    LOG_CONTEXT_LOCAL_DEFINE(context_2, &context_1, LOG_CONTEXT_PROPERTY(uint32_t, y, 1));

    // act
    int line_no = __LINE__;
    log_sink_console.log_sink_log(LOG_LEVEL_VERBOSE, &context_2, __FILE__, __FUNCTION__, line_no, "test");

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(expected_calls[0].u.time_call.captured__time == NULL);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(strcmp(expected_calls[actual_call_count - 1].u.printf_call.captured_output, "Error formatting log line\r\n") == 0);
}

/* Tests_SRS_LOG_SINK_CONSOLE_01_022: [ If any encoding error occurs during formatting of the line (i.e. if any printf class functions fails), log_sink_console.log_sink_log shall print Error formatting log line and return. ]*/
static void when_snprintf_fails_for_property_in_inner_context_log_sink_console_log_prints_error_formatting(void)
{
    // arrange
    setup_mocks();
    setup_time_call();
    setup_ctime_call();
    setup_snprintf_call();
    setup_log_context_get_property_value_pair_count_call();
    setup_log_context_get_property_value_pairs_call();

    setup_snprintf_call(); // context opening
    setup_snprintf_call(); // context opening
    setup_snprintf_call(); // property
    setup_printf_call();

    expected_calls[7].u.snprintf_call.override_result = true;
    expected_calls[7].u.snprintf_call.call_result = -1;

    LOG_CONTEXT_LOCAL_DEFINE(context_1, NULL, LOG_CONTEXT_PROPERTY(int32_t, x, 42));
    LOG_CONTEXT_LOCAL_DEFINE(context_2, &context_1, LOG_CONTEXT_PROPERTY(uint32_t, y, 1));

    // act
    int line_no = __LINE__;
    log_sink_console.log_sink_log(LOG_LEVEL_VERBOSE, &context_2, __FILE__, __FUNCTION__, line_no, "test");

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(expected_calls[0].u.time_call.captured__time == NULL);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(strcmp(expected_calls[actual_call_count - 1].u.printf_call.captured_output, "Error formatting log line\r\n") == 0);
}

/* Tests_SRS_LOG_SINK_CONSOLE_01_022: [ If any encoding error occurs during formatting of the line (i.e. if any printf class functions fails), log_sink_console.log_sink_log shall print Error formatting log line and return. ]*/
static void when_snprintf_fails_for_closing_of_inner_context_log_sink_console_log_prints_error_formatting(void)
{
    // arrange
    setup_mocks();
    setup_time_call();
    setup_ctime_call();
    setup_snprintf_call();
    setup_log_context_get_property_value_pair_count_call();
    setup_log_context_get_property_value_pairs_call();

    setup_snprintf_call(); // context opening
    setup_snprintf_call(); // context opening
    setup_snprintf_call(); // property
    setup_snprintf_call(); // context closing
    setup_printf_call();

    expected_calls[8].u.snprintf_call.override_result = true;
    expected_calls[8].u.snprintf_call.call_result = -1;

    LOG_CONTEXT_LOCAL_DEFINE(context_1, NULL, LOG_CONTEXT_PROPERTY(int32_t, x, 42));
    LOG_CONTEXT_LOCAL_DEFINE(context_2, &context_1, LOG_CONTEXT_PROPERTY(uint32_t, y, 1));

    // act
    int line_no = __LINE__;
    log_sink_console.log_sink_log(LOG_LEVEL_VERBOSE, &context_2, __FILE__, __FUNCTION__, line_no, "test");

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(expected_calls[0].u.time_call.captured__time == NULL);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(strcmp(expected_calls[actual_call_count - 1].u.printf_call.captured_output, "Error formatting log line\r\n") == 0);
}

/* Tests_SRS_LOG_SINK_CONSOLE_01_022: [ If any encoding error occurs during formatting of the line (i.e. if any printf class functions fails), log_sink_console.log_sink_log shall print Error formatting log line and return. ]*/
static void when_snprintf_fails_for_property_in_outer_context_log_sink_console_log_prints_error_formatting(void)
{
    // arrange
    setup_mocks();
    setup_time_call();
    setup_ctime_call();
    setup_snprintf_call();
    setup_log_context_get_property_value_pair_count_call();
    setup_log_context_get_property_value_pairs_call();

    setup_snprintf_call(); // context opening
    setup_snprintf_call(); // context opening
    setup_snprintf_call(); // property
    setup_snprintf_call(); // context closing
    setup_snprintf_call(); // property
    setup_printf_call();

    expected_calls[9].u.snprintf_call.override_result = true;
    expected_calls[9].u.snprintf_call.call_result = -1;

    LOG_CONTEXT_LOCAL_DEFINE(context_1, NULL, LOG_CONTEXT_PROPERTY(int32_t, x, 42));
    LOG_CONTEXT_LOCAL_DEFINE(context_2, &context_1, LOG_CONTEXT_PROPERTY(uint32_t, y, 1));

    // act
    int line_no = __LINE__;
    log_sink_console.log_sink_log(LOG_LEVEL_VERBOSE, &context_2, __FILE__, __FUNCTION__, line_no, "test");

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(expected_calls[0].u.time_call.captured__time == NULL);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(strcmp(expected_calls[actual_call_count - 1].u.printf_call.captured_output, "Error formatting log line\r\n") == 0);
}

/* Tests_SRS_LOG_SINK_CONSOLE_01_022: [ If any encoding error occurs during formatting of the line (i.e. if any printf class functions fails), log_sink_console.log_sink_log shall print Error formatting log line and return. ]*/
static void when_snprintf_fails_for_closing_of_outer_context_log_sink_console_log_prints_error_formatting(void)
{
    // arrange
    setup_mocks();
    setup_time_call();
    setup_ctime_call();
    setup_snprintf_call();
    setup_log_context_get_property_value_pair_count_call();
    setup_log_context_get_property_value_pairs_call();

    setup_snprintf_call(); // context opening
    setup_snprintf_call(); // context opening
    setup_snprintf_call(); // property
    setup_snprintf_call(); // context closing
    setup_snprintf_call(); // property
    setup_snprintf_call(); // context closing
    setup_printf_call();

    expected_calls[10].u.snprintf_call.override_result = true;
    expected_calls[10].u.snprintf_call.call_result = -1;

    LOG_CONTEXT_LOCAL_DEFINE(context_1, NULL, LOG_CONTEXT_PROPERTY(int32_t, x, 42));
    LOG_CONTEXT_LOCAL_DEFINE(context_2, &context_1, LOG_CONTEXT_PROPERTY(uint32_t, y, 1));

    // act
    int line_no = __LINE__;
    log_sink_console.log_sink_log(LOG_LEVEL_VERBOSE, &context_2, __FILE__, __FUNCTION__, line_no, "test");

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(expected_calls[0].u.time_call.captured__time == NULL);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(strcmp(expected_calls[actual_call_count - 1].u.printf_call.captured_output, "Error formatting log line\r\n") == 0);
}

/* Tests_SRS_LOG_SINK_CONSOLE_01_021: [ log_sink_console.log_sink_log shall print at most LOG_MAX_MESSAGE_LENGTH characters including the null terminator (the rest of the context shall be truncated). ]*/
static void when_printing_the_message_exceeds_log_line_size_it_is_truncated(void)
{
    // arrange
    setup_mocks();
    setup_time_call();
    setup_snprintf_call();
    setup_vsnprintf_call();
    setup_printf_call();

    expected_calls[0].u.time_call.override_result = true;
    expected_calls[0].u.time_call.call_result = (time_t)-1;

    char* message_string_too_big = malloc(LOG_MAX_MESSAGE_LENGTH);
    POOR_MANS_ASSERT(message_string_too_big != NULL);

    (void)memset(message_string_too_big, 'x', LOG_MAX_MESSAGE_LENGTH - 1);
    message_string_too_big[LOG_MAX_MESSAGE_LENGTH - 1] = '\0';

    // act
    int line_no = __LINE__;
    log_sink_console.log_sink_log(LOG_LEVEL_VERBOSE, NULL, __FILE__, __FUNCTION__, line_no, message_string_too_big);

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(expected_calls[0].u.time_call.captured__time == NULL);
    POOR_MANS_ASSERT(actual_and_expected_match);

    char expected_string[LOG_MAX_MESSAGE_LENGTH];
    char expected_logged_printf_string[LOG_MAX_MESSAGE_LENGTH * 2];
    (void)snprintf(expected_string, sizeof(expected_string), "\x1b[37m%s Time:NULL File:%s:%d Func:%s %s", MU_ENUM_TO_STRING(LOG_LEVEL, LOG_LEVEL_VERBOSE),
        __FILE__, line_no, __FUNCTION__, message_string_too_big);
    (void)snprintf(expected_logged_printf_string, sizeof(expected_logged_printf_string), "%s\x1b[0m\r\n", expected_string);
    POOR_MANS_ASSERT(strcmp(expected_calls[actual_call_count - 1].u.printf_call.captured_output, expected_logged_printf_string) == 0);

    // cleanup
    free(message_string_too_big);
}

/* Tests_SRS_LOG_SINK_CONSOLE_01_021: [ log_sink_console.log_sink_log shall print at most LOG_MAX_MESSAGE_LENGTH characters including the null terminator (the rest of the context shall be truncated). ]*/
static void when_printing_the_file_exceeds_log_line_size_it_is_truncated(void)
{
    // arrange
    setup_mocks();
    setup_time_call();
    setup_snprintf_call();
    setup_printf_call();

    expected_calls[0].u.time_call.override_result = true;
    expected_calls[0].u.time_call.call_result = (time_t)-1;

    char* file_too_big = malloc(LOG_MAX_MESSAGE_LENGTH);
    POOR_MANS_ASSERT(file_too_big != NULL);

    (void)memset(file_too_big, 'x', LOG_MAX_MESSAGE_LENGTH - 1);
    file_too_big[LOG_MAX_MESSAGE_LENGTH - 1] = '\0';

    // act
    int line_no = __LINE__;
    log_sink_console.log_sink_log(LOG_LEVEL_VERBOSE, NULL, file_too_big, __FUNCTION__, line_no, "a");

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(expected_calls[0].u.time_call.captured__time == NULL);
    POOR_MANS_ASSERT(actual_and_expected_match);

    char expected_string[LOG_MAX_MESSAGE_LENGTH];
    char expected_logged_printf_string[LOG_MAX_MESSAGE_LENGTH * 2];
    (void)snprintf(expected_string, sizeof(expected_string), "\x1b[37m%s Time:NULL File:%s:%d Func:%s a", MU_ENUM_TO_STRING(LOG_LEVEL, LOG_LEVEL_VERBOSE),
        file_too_big, line_no, __FUNCTION__);
    (void)snprintf(expected_logged_printf_string, sizeof(expected_logged_printf_string), "%s\x1b[0m\r\n", expected_string);
    POOR_MANS_ASSERT(strcmp(expected_calls[actual_call_count - 1].u.printf_call.captured_output, expected_logged_printf_string) == 0);

    // cleanup
    free(file_too_big);
}

/* Tests_SRS_LOG_SINK_CONSOLE_01_021: [ log_sink_console.log_sink_log shall print at most LOG_MAX_MESSAGE_LENGTH characters including the null terminator (the rest of the context shall be truncated). ]*/
static void when_printing_a_property_value_exceeds_log_line_size_it_is_truncated(void)
{
    // arrange
    setup_mocks();
    setup_time_call();
    setup_snprintf_call();
    setup_log_context_get_property_value_pair_count_call();
    setup_log_context_get_property_value_pairs_call();

    setup_snprintf_call(); // context opening
    setup_snprintf_call(); // context opening
    setup_snprintf_call(); // property
    setup_printf_call();

    expected_calls[0].u.time_call.override_result = true;
    expected_calls[0].u.time_call.call_result = (time_t)-1;

    char* string_property_value_too_big = malloc(LOG_MAX_MESSAGE_LENGTH);
    POOR_MANS_ASSERT(string_property_value_too_big != NULL);

    (void)memset(string_property_value_too_big, 'x', LOG_MAX_MESSAGE_LENGTH - 1);
    string_property_value_too_big[LOG_MAX_MESSAGE_LENGTH - 1] = '\0';

    LOG_CONTEXT_HANDLE context_1;
    LOG_CONTEXT_CREATE(context_1, NULL, LOG_CONTEXT_STRING_PROPERTY(hagauaga, "%s", string_property_value_too_big));

    // act
    int line_no = __LINE__;
    log_sink_console.log_sink_log(LOG_LEVEL_VERBOSE, context_1, __FILE__, __FUNCTION__, line_no, "a");

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(expected_calls[0].u.time_call.captured__time == NULL);
    POOR_MANS_ASSERT(actual_and_expected_match);

    char expected_string[LOG_MAX_MESSAGE_LENGTH];
    char expected_logged_printf_string[LOG_MAX_MESSAGE_LENGTH * 2];
    (void)snprintf(expected_string, sizeof(expected_string), "\x1b[37m%s Time:NULL File:%s:%d Func:%s { hagauaga=%s } a", MU_ENUM_TO_STRING(LOG_LEVEL, LOG_LEVEL_VERBOSE),
        __FILE__, line_no, __FUNCTION__, string_property_value_too_big);
    (void)snprintf(expected_logged_printf_string, sizeof(expected_logged_printf_string), "%s\x1b[0m\r\n", expected_string);
    POOR_MANS_ASSERT(strcmp(expected_calls[actual_call_count - 1].u.printf_call.captured_output, expected_logged_printf_string) == 0);

    // cleanup
    free(string_property_value_too_big);
    LOG_CONTEXT_DESTROY(context_1);
}

/* Tests_SRS_LOG_SINK_CONSOLE_01_021: [ log_sink_console.log_sink_log shall print at most LOG_MAX_MESSAGE_LENGTH characters including the null terminator (the rest of the context shall be truncated). ]*/
static void when_printing_a_property_name_exceeds_log_line_size_it_is_truncated(void)
{
    // arrange
    setup_mocks();
    setup_time_call();
    setup_snprintf_call();
    setup_log_context_get_property_value_pair_count_call();
    setup_log_context_get_property_value_pairs_call();

    setup_snprintf_call(); // context opening
    setup_snprintf_call(); // context opening
    setup_snprintf_call(); // property
    setup_printf_call();

    expected_calls[0].u.time_call.override_result = true;
    expected_calls[0].u.time_call.call_result = (time_t)-1;

    char* dummy_file_name = malloc(LOG_MAX_MESSAGE_LENGTH);
    POOR_MANS_ASSERT(dummy_file_name != NULL);

    char string_without_property_name[LOG_MAX_MESSAGE_LENGTH];
    int line_no = __LINE__;
    // obtain length without the property name (this is how many chars we have to substract from the max log message length)
    int string_without_property_name_length = snprintf(string_without_property_name, sizeof(string_without_property_name), "\x1b[37m%s Time:NULL File::%d Func:%s { ", MU_ENUM_TO_STRING(LOG_LEVEL, LOG_LEVEL_VERBOSE),
        line_no, __FUNCTION__);

    // build a filename of LOG_MAX_MESSAGE_LENGTH - string_without_property_name_length - 1
    // this will leave only one char space for the property name, but we will use 2 chars when time comes for it
    (void)memset(dummy_file_name, 'x', LOG_MAX_MESSAGE_LENGTH - string_without_property_name_length - 1 - 1);
    dummy_file_name[LOG_MAX_MESSAGE_LENGTH - string_without_property_name_length - 1 - 1] = '\0';

    LOG_CONTEXT_HANDLE context_1;
    // 2 chars property name
    LOG_CONTEXT_CREATE(context_1, NULL, LOG_CONTEXT_STRING_PROPERTY(ab, "x"));

    // act
    log_sink_console.log_sink_log(LOG_LEVEL_VERBOSE, context_1, dummy_file_name, __FUNCTION__, line_no, "a");

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(expected_calls[0].u.time_call.captured__time == NULL);
    POOR_MANS_ASSERT(actual_and_expected_match);

    char expected_string[LOG_MAX_MESSAGE_LENGTH];
    char expected_logged_printf_string[LOG_MAX_MESSAGE_LENGTH * 2];
    (void)snprintf(expected_string, sizeof(expected_string), "\x1b[37m%s Time:NULL File:%s:%d Func:%s { ab=x } a", MU_ENUM_TO_STRING(LOG_LEVEL, LOG_LEVEL_VERBOSE),
        dummy_file_name, line_no, __FUNCTION__);
    (void)snprintf(expected_logged_printf_string, sizeof(expected_logged_printf_string), "%s\x1b[0m\r\n", expected_string);
    POOR_MANS_ASSERT(strcmp(expected_calls[actual_call_count - 1].u.printf_call.captured_output, expected_logged_printf_string) == 0);

    // cleanup
    free(dummy_file_name);
    LOG_CONTEXT_DESTROY(context_1);
}

/* very "poor man's" way of testing, as no test harness and mocking framework are available */
int main(void)
{
    log_sink_console_log_with_NULL_message_format_returns();
    log_sink_console_log_prints_one_CRITICAL_log_line();
    log_sink_console_log_prints_one_ERROR_log_line();
    log_sink_console_log_prints_one_WARNING_log_line();
    log_sink_console_log_prints_one_INFO_log_line();
    log_sink_console_log_prints_one_VERBOSE_log_line();
    
    when_snprintf_fails_log_sink_console_log_prints_error_formatting();
    when_vsnprintf_fails_log_sink_console_log_prints_error_formatting();
    when_time_fails_log_sink_console_log_prints_time_as_NULL();
    when_ctime_returns_NULL_log_sink_console_log_prints_time_as_NULL();
    
    log_sink_console_log_with_non_NULL_context_prints_one_property();
    log_sink_console_log_with_non_NULL_context_prints_2_properties();
    log_sink_console_log_with_non_NULL_context_with_2_levels_works();
    
    log_sink_console_log_with_non_NULL_named_contexts_with_2_levels_works();
    
    log_sink_console_log_with_non_NULL_empty_context_works();
    
    log_sink_console_log_with_non_NULL_dynamically_allocated_context();
    
    when_snprintf_fails_for_context_open_brace_log_sink_console_log_prints_error_formatting();
    when_snprintf_fails_for_inner_context_open_brace_log_sink_console_log_prints_error_formatting();
    when_snprintf_fails_for_property_in_inner_context_log_sink_console_log_prints_error_formatting();
    when_snprintf_fails_for_closing_of_inner_context_log_sink_console_log_prints_error_formatting();
    when_snprintf_fails_for_property_in_outer_context_log_sink_console_log_prints_error_formatting();
    when_snprintf_fails_for_closing_of_outer_context_log_sink_console_log_prints_error_formatting();
    
    when_printing_the_message_exceeds_log_line_size_it_is_truncated();
    when_printing_the_file_exceeds_log_line_size_it_is_truncated();
    when_printing_a_property_value_exceeds_log_line_size_it_is_truncated();
    when_printing_a_property_name_exceeds_log_line_size_it_is_truncated();

    return asserts_failed;
}
