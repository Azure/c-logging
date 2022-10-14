// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "macro_utils/macro_utils.h"

#include "c_logging/logger.h"
#include "c_logging/log_sink_console.h"
#include "c_logging/log_context.h"

static size_t asserts_failed = 0;

// defines how many mock calls we can have
#define MAX_MOCK_CALL_COUNT (128)

#define MOCK_CALL_TYPE_VALUES \
    MOCK_CALL_TYPE_printf, \
    MOCK_CALL_TYPE_time, \
    MOCK_CALL_TYPE_ctime, \
    MOCK_CALL_TYPE_vsnprintf \

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
    int call_result;
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

typedef struct MOCK_CALL_TAG
{
    MOCK_CALL_TYPE mock_call_type;
    union
    {
        printf_CALL printf_call;
        time_CALL time_call;
        ctime_CALL ctime_call;
        vsnprintf_CALL vsnprintf_call;
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
            expected_calls[actual_call_count].u.vsnprintf_call.captured_format_arg = format;
            expected_calls[actual_call_count].u.vsnprintf_call.captured_format_arg = format;

            result = vsnprintf(s, n, format, args);

            va_end(args);
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

static void setup_ctime_call(void)
{
    expected_calls[expected_call_count].mock_call_type = MOCK_CALL_TYPE_ctime;
    expected_calls[expected_call_count].u.printf_call.override_result = false;
    expected_call_count++;
}

static void setup_vsnprintf_call(void)
{
    expected_calls[expected_call_count].mock_call_type = MOCK_CALL_TYPE_vsnprintf;
    expected_calls[expected_call_count].u.printf_call.override_result = false;
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
    //    char final_char;
    char reset_color_code[10];
    char anything_else[200];
    int scanned_values = sscanf(actual_string, expected_string, day_of_week, month, &day, &hour, &minute, &second, &year, reset_color_code, anything_else);
    POOR_MANS_ASSERT(scanned_values == 8); // the last one should not get scanned as there should be nothing past \r\n
    size_t actual_string_length = strlen(actual_string);
    POOR_MANS_ASSERT(actual_string[actual_string_length - 2] == '\r');
    POOR_MANS_ASSERT(actual_string[actual_string_length - 1] == '\n');
    POOR_MANS_ASSERT(strcmp(reset_color_code, "\x1b[0m") == 0);
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

/* Tests_SRS_LOG_SINK_CONSOLE_01_002: [ `log_sink_console.log_sink_log` shall obtain the time by calling `time`. ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_003: [ `log_sink_console.log_sink_log` shall convert the time to string by calling `ctime`. ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_004: [ `log_sink_console.log_sink_log` shall print a line in the format: `{log_level} Time: {formatted time} File:{file}:{line} Func:{func} {optional context information} {formatted message}` ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_005: [ In order to not break the line in multiple parts when displayed on the console, `log_sink_console.log_sink_log` shall print the line in such a way that only one `printf` call is made. ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_006: [ `log_sink_console.log_sink_log` shall color the lines using ANSI color codes (https://en.wikipedia.org/wiki/ANSI_escape_code#Colors), as follows: ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_007: [ `LOG_LEVEL_CRITICAL` shall be displayed with bright red `\x1b[31;1m`. ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_012: [ At the end of each line that is printed, the color shall be reset by using the `\x1b[0m` code. ]*/
static void log_sink_console_log_prints_one_CRITICAL_log_line(void)
{
    // arrange
    setup_mocks();
    setup_time_call();
    setup_ctime_call();
    setup_vsnprintf_call();
    setup_printf_call();

    // act
    int line_no = __LINE__;
    log_sink_console.log_sink_log(LOG_LEVEL_CRITICAL, NULL, __FILE__, __FUNCTION__, line_no, "test");

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(expected_calls[0].u.time_call.captured__time == NULL);
    POOR_MANS_ASSERT(actual_and_expected_match);
    validate_log_line(expected_calls[3].u.printf_call.captured_output, "\x1b[31;1m%s Time:%%s %%s %%d %%d:%%d:%%d %%d File:%s:%d Func:%s %s%%s\r\n%%s", MU_ENUM_TO_STRING(LOG_LEVEL, LOG_LEVEL_CRITICAL), __FILE__, line_no, __FUNCTION__, "test");
}

/* Tests_SRS_LOG_SINK_CONSOLE_01_002: [ `log_sink_console.log_sink_log` shall obtain the time by calling `time`. ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_003: [ `log_sink_console.log_sink_log` shall convert the time to string by calling `ctime`. ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_004: [ `log_sink_console.log_sink_log` shall print a line in the format: `{log_level} Time: {formatted time} File:{file}:{line} Func:{func} {optional context information} {formatted message}` ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_005: [ In order to not break the line in multiple parts when displayed on the console, `log_sink_console.log_sink_log` shall print the line in such a way that only one `printf` call is made. ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_006: [ `log_sink_console.log_sink_log` shall color the lines using ANSI color codes (https://en.wikipedia.org/wiki/ANSI_escape_code#Colors), as follows: ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_008: [ `LOG_LEVEL_ERROR` shall be displayed with red `\x1b[31m`. ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_012: [ At the end of each line that is printed, the color shall be reset by using the `\x1b[0m` code. ]*/
static void log_sink_console_log_prints_one_ERROR_log_line(void)
{
    // arrange
    setup_mocks();
    setup_time_call();
    setup_ctime_call();
    setup_vsnprintf_call();
    setup_printf_call();

    // act
    int line_no = __LINE__;
    log_sink_console.log_sink_log(LOG_LEVEL_ERROR, NULL, __FILE__, __FUNCTION__, line_no, "test");

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(expected_calls[0].u.time_call.captured__time == NULL);
    POOR_MANS_ASSERT(actual_and_expected_match);
    validate_log_line(expected_calls[3].u.printf_call.captured_output, "\x1b[31m%s Time:%%s %%s %%d %%d:%%d:%%d %%d File:%s:%d Func:%s %s%%s\r\n%%s", MU_ENUM_TO_STRING(LOG_LEVEL, LOG_LEVEL_ERROR), __FILE__, line_no, __FUNCTION__, "test");
}

/* Tests_SRS_LOG_SINK_CONSOLE_01_002: [ `log_sink_console.log_sink_log` shall obtain the time by calling `time`. ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_003: [ `log_sink_console.log_sink_log` shall convert the time to string by calling `ctime`. ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_004: [ `log_sink_console.log_sink_log` shall print a line in the format: `{log_level} Time: {formatted time} File:{file}:{line} Func:{func} {optional context information} {formatted message}` ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_005: [ In order to not break the line in multiple parts when displayed on the console, `log_sink_console.log_sink_log` shall print the line in such a way that only one `printf` call is made. ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_006: [ `log_sink_console.log_sink_log` shall color the lines using ANSI color codes (https://en.wikipedia.org/wiki/ANSI_escape_code#Colors), as follows: ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_009: [ `LOG_LEVEL_WARNING` shall be displayed with bright yellow `\x1b[33;1m`. ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_012: [ At the end of each line that is printed, the color shall be reset by using the `\x1b[0m` code. ]*/
static void log_sink_console_log_prints_one_WARNING_log_line(void)
{
    // arrange
    setup_mocks();
    setup_time_call();
    setup_ctime_call();
    setup_vsnprintf_call();
    setup_printf_call();

    // act
    int line_no = __LINE__;
    log_sink_console.log_sink_log(LOG_LEVEL_WARNING, NULL, __FILE__, __FUNCTION__, line_no, "test");

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(expected_calls[0].u.time_call.captured__time == NULL);
    POOR_MANS_ASSERT(actual_and_expected_match);
    validate_log_line(expected_calls[3].u.printf_call.captured_output, "\x1b[33;1m%s Time:%%s %%s %%d %%d:%%d:%%d %%d File:%s:%d Func:%s %s%%s\r\n%%s", MU_ENUM_TO_STRING(LOG_LEVEL, LOG_LEVEL_WARNING), __FILE__, line_no, __FUNCTION__, "test");
}

/* Tests_SRS_LOG_SINK_CONSOLE_01_002: [ `log_sink_console.log_sink_log` shall obtain the time by calling `time`. ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_003: [ `log_sink_console.log_sink_log` shall convert the time to string by calling `ctime`. ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_004: [ `log_sink_console.log_sink_log` shall print a line in the format: `{log_level} Time: {formatted time} File:{file}:{line} Func:{func} {optional context information} {formatted message}` ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_005: [ In order to not break the line in multiple parts when displayed on the console, `log_sink_console.log_sink_log` shall print the line in such a way that only one `printf` call is made. ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_006: [ `log_sink_console.log_sink_log` shall color the lines using ANSI color codes (https://en.wikipedia.org/wiki/ANSI_escape_code#Colors), as follows: ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_010: [ `LOG_LEVEL_INFO` shall be displayed with yellow `\x1b[33m`. ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_012: [ At the end of each line that is printed, the color shall be reset by using the `\x1b[0m` code. ]*/
static void log_sink_console_log_prints_one_INFO_log_line(void)
{
    // arrange
    setup_mocks();
    setup_time_call();
    setup_ctime_call();
    setup_vsnprintf_call();
    setup_printf_call();

    // act
    int line_no = __LINE__;
    log_sink_console.log_sink_log(LOG_LEVEL_INFO, NULL, __FILE__, __FUNCTION__, line_no, "test");

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(expected_calls[0].u.time_call.captured__time == NULL);
    POOR_MANS_ASSERT(actual_and_expected_match);
    validate_log_line(expected_calls[3].u.printf_call.captured_output, "\x1b[33m%s Time:%%s %%s %%d %%d:%%d:%%d %%d File:%s:%d Func:%s %s%%s\r\n%%s", MU_ENUM_TO_STRING(LOG_LEVEL, LOG_LEVEL_INFO), __FILE__, line_no, __FUNCTION__, "test");
}

/* Tests_SRS_LOG_SINK_CONSOLE_01_002: [ `log_sink_console.log_sink_log` shall obtain the time by calling `time`. ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_003: [ `log_sink_console.log_sink_log` shall convert the time to string by calling `ctime`. ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_004: [ `log_sink_console.log_sink_log` shall print a line in the format: `{log_level} Time: {formatted time} File:{file}:{line} Func:{func} {optional context information} {formatted message}` ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_005: [ In order to not break the line in multiple parts when displayed on the console, `log_sink_console.log_sink_log` shall print the line in such a way that only one `printf` call is made. ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_006: [ `log_sink_console.log_sink_log` shall color the lines using ANSI color codes (https://en.wikipedia.org/wiki/ANSI_escape_code#Colors), as follows: ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_011: [ `LOG_LEVEL_VERBOSE` shall be displayed with white `\x1b[37m`. ]*/
/* Tests_SRS_LOG_SINK_CONSOLE_01_012: [ At the end of each line that is printed, the color shall be reset by using the `\x1b[0m` code. ]*/
static void log_sink_console_log_prints_one_VERBOSE_log_line(void)
{
    // arrange
    setup_mocks();
    setup_time_call();
    setup_ctime_call();
    setup_vsnprintf_call();
    setup_printf_call();

    // act
    int line_no = __LINE__;
    log_sink_console.log_sink_log(LOG_LEVEL_VERBOSE, NULL, __FILE__, __FUNCTION__, line_no, "test");

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(expected_calls[0].u.time_call.captured__time == NULL);
    POOR_MANS_ASSERT(actual_and_expected_match);
    validate_log_line(expected_calls[3].u.printf_call.captured_output, "\x1b[37m%s Time:%%s %%s %%d %%d:%%d:%%d %%d File:%s:%d Func:%s %s%%s\r\n%%s", MU_ENUM_TO_STRING(LOG_LEVEL, LOG_LEVEL_VERBOSE), __FILE__, line_no, __FUNCTION__, "test");
}

/* very "poor man's" way of testing, as no test harness and mocking framework are available */
int main(void)
{
    //log_sink_console_log_with_NULL_message_format_returns();
    log_sink_console_log_prints_one_CRITICAL_log_line();
    log_sink_console_log_prints_one_ERROR_log_line();
    log_sink_console_log_prints_one_WARNING_log_line();
    log_sink_console_log_prints_one_INFO_log_line();
    log_sink_console_log_prints_one_VERBOSE_log_line();

    return asserts_failed;
}
