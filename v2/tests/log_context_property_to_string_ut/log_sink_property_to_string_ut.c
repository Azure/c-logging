// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef USE_VLD
#include "vld.h"
#endif

#include "macro_utils/macro_utils.h"

#include "c_logging/log_context_property_basic_types.h"
#include "c_logging/log_context_property_type_ascii_char_ptr.h"
#include "c_logging/log_context_property_value_pair.h"
#include "c_logging/log_context.h"

#include "c_logging/log_context_property_to_string.h"

#define LOG_MAX_MESSAGE_LENGTH 4096

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

static void setup_mocks(void)
{
    expected_call_count = 0;
    actual_call_count = 0;
    actual_and_expected_match = true;
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

#define POOR_MANS_ASSERT(cond) \
    if (!(cond)) \
    { \
        (void)printf("%s:%d test failed\r\n", __FUNCTION__, __LINE__); \
        abort(); \
    } \

static void setup_snprintf_call(void)
{
    expected_calls[expected_call_count].mock_call_type = MOCK_CALL_TYPE_snprintf;
    expected_calls[expected_call_count].snprintf_call.override_result = false;
    expected_call_count++;
}

static void validate_log_line(const char* actual_string, const char* expected_format, const char* expected_log_level_string, const char* file, int line, const char* func, const char* expected_message)
{
    char expected_string[LOG_MAX_MESSAGE_LENGTH * 2];
    int snprintf_result = snprintf(expected_string, sizeof(expected_string), expected_format, expected_log_level_string, file, line, func, expected_message);
    POOR_MANS_ASSERT(snprintf_result >= 0);
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

static void validate_property_string(const char* actual_string, const char* expected_format)
{
    POOR_MANS_ASSERT(strcmp(actual_string, expected_format) == 0);
}

/* log_context_property_to_string */

/* Tests_SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_027: [ If buffer is NULL then log_context_property_to_string shall fail and return a negative value. ]*/
static void log_context_property_to_string_with_NULL_buffer_fails(void)
{
    // arrange
    LOG_CONTEXT_LOCAL_DEFINE(context_1, NULL, LOG_CONTEXT_PROPERTY(int32_t, x, 42));
    size_t property_value_pair_count = log_context_get_property_value_pair_count(&context_1);
    const LOG_CONTEXT_PROPERTY_VALUE_PAIR* property_value_pairs = log_context_get_property_value_pairs(&context_1);

    setup_mocks();

    // act
    int result = log_context_property_to_string(NULL, LOG_MAX_MESSAGE_LENGTH, property_value_pairs, property_value_pair_count);

    // assert
    POOR_MANS_ASSERT(result < 0);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_028: [ If buffer_size is 0 then log_context_property_to_string shall return 0. ]*/
static void log_context_property_to_string_with_0_buffer_size_fails(void)
{
    // arrange
    char buffer[LOG_MAX_MESSAGE_LENGTH];

    LOG_CONTEXT_LOCAL_DEFINE(context_1, NULL, LOG_CONTEXT_PROPERTY(int32_t, x, 42));
    size_t property_value_pair_count = log_context_get_property_value_pair_count(&context_1);
    const LOG_CONTEXT_PROPERTY_VALUE_PAIR* property_value_pairs = log_context_get_property_value_pairs(&context_1);

    setup_mocks();

    // act
    int result = log_context_property_to_string(buffer, 0, property_value_pairs, property_value_pair_count);

    // assert
    POOR_MANS_ASSERT(result < 0);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_029: [ If property_value_pairs is NULL then log_context_property_to_string shall fail and return a negative value. ]*/
static void log_context_property_to_string_with_NULL_property_value_pairs_fails(void)
{
    // arrange
    char buffer[LOG_MAX_MESSAGE_LENGTH];

    LOG_CONTEXT_LOCAL_DEFINE(context_1, NULL, LOG_CONTEXT_PROPERTY(int32_t, x, 42));
    size_t property_value_pair_count = log_context_get_property_value_pair_count(&context_1);

    setup_mocks();

    // act
    int result = log_context_property_to_string(buffer, LOG_MAX_MESSAGE_LENGTH, NULL, property_value_pair_count);

    // assert
    POOR_MANS_ASSERT(result < 0);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_030: [ If property_value_pair_count is 0 then log_context_property_to_string shall fail and return a negative value. ]*/
static void log_context_property_to_string_with_0_property_value_pair_count_fails(void)
{
    // arrange
    char buffer[LOG_MAX_MESSAGE_LENGTH];

    LOG_CONTEXT_LOCAL_DEFINE(context_1, NULL, LOG_CONTEXT_PROPERTY(int32_t, x, 42));
    const LOG_CONTEXT_PROPERTY_VALUE_PAIR* property_value_pairs = log_context_get_property_value_pairs(&context_1);

    setup_mocks();

    // act
    int result = log_context_property_to_string(buffer, LOG_MAX_MESSAGE_LENGTH, property_value_pairs, 0);

    // assert
    POOR_MANS_ASSERT(result < 0);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_016: [ For each property: ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_017: [ If the property type is struct (used as a container for context properties): ]*/
  /* Tests_SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_025: [ log_context_property_to_string shall print the struct property name and an opening brace. ]*/
  /* Tests_SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_018: [ log_context_property_to_string shall obtain the number of fields in the struct. ]*/
  /* Tests_SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_019: [ log_context_property_to_string shall print the next n properties as being the fields that are part of the struct. ]*/
  /* Tests_SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_026: [ log_context_property_to_string shall print a closing brace as end of the struct. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_020: [ Otherwise log_context_property_to_string shall call to_string for the property and print its name and value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_031: [ log_context_property_to_string shall return the number of bytes written to the buffer. ]*/
static void log_context_property_to_string_with_single_property_no_struct(void)
{
    // arrange
    char buffer[LOG_MAX_MESSAGE_LENGTH];

    uint32_t x = 42;
    size_t property_value_pair_count = 1;
    LOG_CONTEXT_PROPERTY_VALUE_PAIR property_value_pair;
    property_value_pair.name = "x";
    property_value_pair.value = &x;
    property_value_pair.type = &int32_t_log_context_property_type;

    setup_mocks();

    setup_snprintf_call(); // property

    // act
    int result = log_context_property_to_string(buffer, LOG_MAX_MESSAGE_LENGTH, &property_value_pair, property_value_pair_count);

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);
    const char expected_string[] = " x=42";
    validate_property_string(buffer, expected_string);
    POOR_MANS_ASSERT(result == sizeof(expected_string) - 1);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_016: [ For each property: ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_017: [ If the property type is struct (used as a container for context properties): ]*/
  /* Tests_SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_025: [ log_context_property_to_string shall print the struct property name and an opening brace. ]*/
  /* Tests_SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_018: [ log_context_property_to_string shall obtain the number of fields in the struct. ]*/
  /* Tests_SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_019: [ log_context_property_to_string shall print the next n properties as being the fields that are part of the struct. ]*/
  /* Tests_SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_026: [ log_context_property_to_string shall print a closing brace as end of the struct. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_020: [ Otherwise log_context_property_to_string shall call to_string for the property and print its name and value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_031: [ log_context_property_to_string shall return the number of bytes written to the buffer. ]*/
static void log_context_property_to_string_with_non_NULL_context_prints_one_property(void)
{
    // arrange
    char buffer[LOG_MAX_MESSAGE_LENGTH];

    LOG_CONTEXT_LOCAL_DEFINE(context_1, NULL, LOG_CONTEXT_PROPERTY(int32_t, x, 42));
    size_t property_value_pair_count = log_context_get_property_value_pair_count(&context_1);
    const LOG_CONTEXT_PROPERTY_VALUE_PAIR* property_value_pairs = log_context_get_property_value_pairs(&context_1);

    setup_mocks();

    setup_snprintf_call(); // context opening
    setup_snprintf_call(); // property
    setup_snprintf_call(); // context closing

    // act
    int result = log_context_property_to_string(buffer, LOG_MAX_MESSAGE_LENGTH, property_value_pairs, property_value_pair_count);

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);
    const char expected_string[] = " { x=42 }";
    validate_property_string(buffer, expected_string);
    POOR_MANS_ASSERT(result == sizeof(expected_string) - 1);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_016: [ For each property: ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_017: [ If the property type is struct (used as a container for context properties): ]*/
  /* Tests_SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_025: [ log_context_property_to_string shall print the struct property name and an opening brace. ]*/
  /* Tests_SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_018: [ log_context_property_to_string shall obtain the number of fields in the struct. ]*/
  /* Tests_SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_019: [ log_context_property_to_string shall print the next n properties as being the fields that are part of the struct. ]*/
  /* Tests_SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_026: [ log_context_property_to_string shall print a closing brace as end of the struct. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_020: [ Otherwise log_context_property_to_string shall call to_string for the property and print its name and value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_031: [ log_context_property_to_string shall return the number of bytes written to the buffer. ]*/
static void log_context_property_to_string_with_non_NULL_context_prints_2_properties(void)
{
    // arrange
    char buffer[LOG_MAX_MESSAGE_LENGTH];

    LOG_CONTEXT_LOCAL_DEFINE(context_1, NULL, LOG_CONTEXT_PROPERTY(int32_t, x, 42), LOG_CONTEXT_PROPERTY(uint32_t, y, 1));
    size_t property_value_pair_count = log_context_get_property_value_pair_count(&context_1);
    const LOG_CONTEXT_PROPERTY_VALUE_PAIR* property_value_pairs = log_context_get_property_value_pairs(&context_1);

    setup_mocks();

    setup_snprintf_call(); // context opening
    setup_snprintf_call(); // property
    setup_snprintf_call(); // property
    setup_snprintf_call(); // context closing

    // act
    int result = log_context_property_to_string(buffer, LOG_MAX_MESSAGE_LENGTH, property_value_pairs, property_value_pair_count);

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);
    const char expected_string[] = " { x=42 y=1 }";
    validate_property_string(buffer, expected_string);
    POOR_MANS_ASSERT(result == sizeof(expected_string) - 1);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_016: [ For each property: ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_017: [ If the property type is struct (used as a container for context properties): ]*/
  /* Tests_SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_025: [ log_context_property_to_string shall print the struct property name and an opening brace. ]*/
  /* Tests_SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_018: [ log_context_property_to_string shall obtain the number of fields in the struct. ]*/
  /* Tests_SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_019: [ log_context_property_to_string shall print the next n properties as being the fields that are part of the struct. ]*/
  /* Tests_SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_026: [ log_context_property_to_string shall print a closing brace as end of the struct. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_020: [ Otherwise log_context_property_to_string shall call to_string for the property and print its name and value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_031: [ log_context_property_to_string shall return the number of bytes written to the buffer. ]*/
static void log_context_property_to_string_with_non_NULL_context_with_2_levels_works(void)
{
    // arrange
    char buffer[LOG_MAX_MESSAGE_LENGTH];

    LOG_CONTEXT_LOCAL_DEFINE(context_1, NULL, LOG_CONTEXT_PROPERTY(int32_t, x, 42));
    LOG_CONTEXT_LOCAL_DEFINE(context_2, &context_1, LOG_CONTEXT_PROPERTY(uint32_t, y, 1));
    size_t property_value_pair_count = log_context_get_property_value_pair_count(&context_2);
    const LOG_CONTEXT_PROPERTY_VALUE_PAIR* property_value_pairs = log_context_get_property_value_pairs(&context_2);

    setup_mocks();

    setup_snprintf_call(); // context opening
    setup_snprintf_call(); // context opening
    setup_snprintf_call(); // property
    setup_snprintf_call(); // context closing
    setup_snprintf_call(); // property
    setup_snprintf_call(); // context closing

    // act
    int result = log_context_property_to_string(buffer, LOG_MAX_MESSAGE_LENGTH, property_value_pairs, property_value_pair_count);

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);
    const char expected_string[] = " { { x=42 } y=1 }";
    validate_property_string(buffer, expected_string);
    POOR_MANS_ASSERT(result == sizeof(expected_string) - 1);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_016: [ For each property: ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_017: [ If the property type is struct (used as a container for context properties): ]*/
  /* Tests_SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_025: [ log_context_property_to_string shall print the struct property name and an opening brace. ]*/
  /* Tests_SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_018: [ log_context_property_to_string shall obtain the number of fields in the struct. ]*/
  /* Tests_SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_019: [ log_context_property_to_string shall print the next n properties as being the fields that are part of the struct. ]*/
  /* Tests_SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_026: [ log_context_property_to_string shall print a closing brace as end of the struct. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_020: [ Otherwise log_context_property_to_string shall call to_string for the property and print its name and value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_031: [ log_context_property_to_string shall return the number of bytes written to the buffer. ]*/
static void log_context_property_to_string_with_non_NULL_named_contexts_with_2_levels_works(void)
{
    // arrange
    char buffer[LOG_MAX_MESSAGE_LENGTH];

    LOG_CONTEXT_LOCAL_DEFINE(context_1, NULL, LOG_CONTEXT_NAME(haga), LOG_CONTEXT_PROPERTY(int32_t, x, 42));
    LOG_CONTEXT_LOCAL_DEFINE(context_2, &context_1, LOG_CONTEXT_NAME(uaga), LOG_CONTEXT_PROPERTY(uint32_t, y, 1));
    size_t property_value_pair_count = log_context_get_property_value_pair_count(&context_2);
    const LOG_CONTEXT_PROPERTY_VALUE_PAIR* property_value_pairs = log_context_get_property_value_pairs(&context_2);

    setup_mocks();

    setup_snprintf_call(); // context opening
    setup_snprintf_call(); // context opening
    setup_snprintf_call(); // property
    setup_snprintf_call(); // context closing
    setup_snprintf_call(); // property
    setup_snprintf_call(); // context closing

    // act
    int result = log_context_property_to_string(buffer, LOG_MAX_MESSAGE_LENGTH, property_value_pairs, property_value_pair_count);

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);
    const char expected_string[] = " uaga={ haga={ x=42 } y=1 }";
    validate_property_string(buffer, expected_string);
    POOR_MANS_ASSERT(result == sizeof(expected_string) - 1);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_016: [ For each property: ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_017: [ If the property type is struct (used as a container for context properties): ]*/
  /* Tests_SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_025: [ log_context_property_to_string shall print the struct property name and an opening brace. ]*/
  /* Tests_SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_018: [ log_context_property_to_string shall obtain the number of fields in the struct. ]*/
  /* Tests_SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_026: [ log_context_property_to_string shall print a closing brace as end of the struct. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_020: [ Otherwise log_context_property_to_string shall call to_string for the property and print its name and value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_031: [ log_context_property_to_string shall return the number of bytes written to the buffer. ]*/
static void log_context_property_to_string_with_non_NULL_empty_context_works(void)
{
    // arrange
    char buffer[LOG_MAX_MESSAGE_LENGTH];

    LOG_CONTEXT_LOCAL_DEFINE(context_1, NULL);
    size_t property_value_pair_count = log_context_get_property_value_pair_count(&context_1);
    const LOG_CONTEXT_PROPERTY_VALUE_PAIR* property_value_pairs = log_context_get_property_value_pairs(&context_1);

    setup_mocks();

    setup_snprintf_call(); // context opening
    setup_snprintf_call(); // context closing

    // act
    int result = log_context_property_to_string(buffer, LOG_MAX_MESSAGE_LENGTH, property_value_pairs, property_value_pair_count);

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);
    const char expected_string[] = " { }";
    validate_property_string(buffer, expected_string);
    POOR_MANS_ASSERT(result == sizeof(expected_string) - 1);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_016: [ For each property: ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_017: [ If the property type is struct (used as a container for context properties): ]*/
  /* Tests_SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_025: [ log_context_property_to_string shall print the struct property name and an opening brace. ]*/
  /* Tests_SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_018: [ log_context_property_to_string shall obtain the number of fields in the struct. ]*/
  /* Tests_SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_019: [ log_context_property_to_string shall print the next n properties as being the fields that are part of the struct. ]*/
  /* Tests_SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_026: [ log_context_property_to_string shall print a closing brace as end of the struct. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_020: [ Otherwise log_context_property_to_string shall call to_string for the property and print its name and value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_031: [ log_context_property_to_string shall return the number of bytes written to the buffer. ]*/
static void log_context_property_to_string_with_non_NULL_dynamically_allocated_context(void)
{
    // arrange
    char buffer[LOG_MAX_MESSAGE_LENGTH];

    LOG_CONTEXT_HANDLE context_1;
    LOG_CONTEXT_CREATE(context_1, NULL, LOG_CONTEXT_PROPERTY(int32_t, x, 42));
    LOG_CONTEXT_HANDLE context_2;
    LOG_CONTEXT_CREATE(context_2, context_1, LOG_CONTEXT_PROPERTY(uint32_t, y, 1));
    size_t property_value_pair_count = log_context_get_property_value_pair_count(context_2);
    const LOG_CONTEXT_PROPERTY_VALUE_PAIR* property_value_pairs = log_context_get_property_value_pairs(context_2);

    setup_mocks();

    setup_snprintf_call(); // context opening
    setup_snprintf_call(); // context opening
    setup_snprintf_call(); // property
    setup_snprintf_call(); // context closing
    setup_snprintf_call(); // property
    setup_snprintf_call(); // context closing

    // act
    int result = log_context_property_to_string(buffer, LOG_MAX_MESSAGE_LENGTH, property_value_pairs, property_value_pair_count);

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);
    const char expected_string[] = " { { x=42 } y=1 }";
    validate_property_string(buffer, expected_string);
    POOR_MANS_ASSERT(result == sizeof(expected_string) - 1);

    // cleanup
    LOG_CONTEXT_DESTROY(context_1);
    LOG_CONTEXT_DESTROY(context_2);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_022: [ If any encoding error occurs during formatting of the line (i.e. if any printf class functions fails), log_context_property_to_string shall fail and return a negative value. ]*/
static void when_snprintf_fails_for_context_open_brace_log_context_property_to_string_fails(void)
{
    // arrange
    char buffer[LOG_MAX_MESSAGE_LENGTH];

    LOG_CONTEXT_LOCAL_DEFINE(context_1, NULL, LOG_CONTEXT_PROPERTY(int32_t, x, 42));
    LOG_CONTEXT_LOCAL_DEFINE(context_2, &context_1, LOG_CONTEXT_PROPERTY(uint32_t, y, 1));
    size_t property_value_pair_count = log_context_get_property_value_pair_count(&context_2);
    const LOG_CONTEXT_PROPERTY_VALUE_PAIR* property_value_pairs = log_context_get_property_value_pairs(&context_2);

    setup_mocks();

    setup_snprintf_call(); // context opening

    expected_calls[0].snprintf_call.override_result = true;
    expected_calls[0].snprintf_call.call_result = -1;

    // act
    int result = log_context_property_to_string(buffer, LOG_MAX_MESSAGE_LENGTH, property_value_pairs, property_value_pair_count);

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(result < 0);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_022: [ If any encoding error occurs during formatting of the line (i.e. if any printf class functions fails), log_context_property_to_string shall fail and return a negative value. ]*/
static void when_snprintf_fails_for_inner_context_open_brace_log_context_property_to_string_fails(void)
{
    // arrange
    char buffer[LOG_MAX_MESSAGE_LENGTH];

    LOG_CONTEXT_LOCAL_DEFINE(context_1, NULL, LOG_CONTEXT_PROPERTY(int32_t, x, 42));
    LOG_CONTEXT_LOCAL_DEFINE(context_2, &context_1, LOG_CONTEXT_PROPERTY(uint32_t, y, 1));
    size_t property_value_pair_count = log_context_get_property_value_pair_count(&context_2);
    const LOG_CONTEXT_PROPERTY_VALUE_PAIR* property_value_pairs = log_context_get_property_value_pairs(&context_2);

    setup_mocks();

    setup_snprintf_call(); // context opening
    setup_snprintf_call(); // context opening

    expected_calls[1].snprintf_call.override_result = true;
    expected_calls[1].snprintf_call.call_result = -1;

    // act
    int result = log_context_property_to_string(buffer, LOG_MAX_MESSAGE_LENGTH, property_value_pairs, property_value_pair_count);

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(result < 0);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_022: [ If any encoding error occurs during formatting of the line (i.e. if any printf class functions fails), log_context_property_to_string shall fail and return a negative value. ]*/
static void when_snprintf_fails_for_property_in_inner_context_log_context_property_to_string_fails(void)
{
    // arrange
    char buffer[LOG_MAX_MESSAGE_LENGTH];

    LOG_CONTEXT_LOCAL_DEFINE(context_1, NULL, LOG_CONTEXT_PROPERTY(int32_t, x, 42));
    LOG_CONTEXT_LOCAL_DEFINE(context_2, &context_1, LOG_CONTEXT_PROPERTY(uint32_t, y, 1));
    size_t property_value_pair_count = log_context_get_property_value_pair_count(&context_2);
    const LOG_CONTEXT_PROPERTY_VALUE_PAIR* property_value_pairs = log_context_get_property_value_pairs(&context_2);

    setup_mocks();

    setup_snprintf_call(); // context opening
    setup_snprintf_call(); // context opening
    setup_snprintf_call(); // property

    expected_calls[2].snprintf_call.override_result = true;
    expected_calls[2].snprintf_call.call_result = -1;

    // act
    int result = log_context_property_to_string(buffer, LOG_MAX_MESSAGE_LENGTH, property_value_pairs, property_value_pair_count);

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(result < 0);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_022: [ If any encoding error occurs during formatting of the line (i.e. if any printf class functions fails), log_context_property_to_string shall fail and return a negative value. ]*/
static void when_snprintf_fails_for_closing_of_inner_context_log_context_property_to_string_fails(void)
{
    // arrange
    char buffer[LOG_MAX_MESSAGE_LENGTH];

    LOG_CONTEXT_LOCAL_DEFINE(context_1, NULL, LOG_CONTEXT_PROPERTY(int32_t, x, 42));
    LOG_CONTEXT_LOCAL_DEFINE(context_2, &context_1, LOG_CONTEXT_PROPERTY(uint32_t, y, 1));
    size_t property_value_pair_count = log_context_get_property_value_pair_count(&context_2);
    const LOG_CONTEXT_PROPERTY_VALUE_PAIR* property_value_pairs = log_context_get_property_value_pairs(&context_2);

    setup_mocks();

    setup_snprintf_call(); // context opening
    setup_snprintf_call(); // context opening
    setup_snprintf_call(); // property
    setup_snprintf_call(); // context closing

    expected_calls[3].snprintf_call.override_result = true;
    expected_calls[3].snprintf_call.call_result = -1;

    // act
    int result = log_context_property_to_string(buffer, LOG_MAX_MESSAGE_LENGTH, property_value_pairs, property_value_pair_count);

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(result < 0);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_022: [ If any encoding error occurs during formatting of the line (i.e. if any printf class functions fails), log_context_property_to_string shall fail and return a negative value. ]*/
static void when_snprintf_fails_for_property_in_outer_context_log_context_property_to_string_fails(void)
{
    // arrange
    char buffer[LOG_MAX_MESSAGE_LENGTH];

    LOG_CONTEXT_LOCAL_DEFINE(context_1, NULL, LOG_CONTEXT_PROPERTY(int32_t, x, 42));
    LOG_CONTEXT_LOCAL_DEFINE(context_2, &context_1, LOG_CONTEXT_PROPERTY(uint32_t, y, 1));
    size_t property_value_pair_count = log_context_get_property_value_pair_count(&context_2);
    const LOG_CONTEXT_PROPERTY_VALUE_PAIR* property_value_pairs = log_context_get_property_value_pairs(&context_2);

    setup_mocks();

    setup_snprintf_call(); // context opening
    setup_snprintf_call(); // context opening
    setup_snprintf_call(); // property
    setup_snprintf_call(); // context closing
    setup_snprintf_call(); // property

    expected_calls[4].snprintf_call.override_result = true;
    expected_calls[4].snprintf_call.call_result = -1;

    // act
    int result = log_context_property_to_string(buffer, LOG_MAX_MESSAGE_LENGTH, property_value_pairs, property_value_pair_count);

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(result < 0);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_022: [ If any encoding error occurs during formatting of the line (i.e. if any printf class functions fails), log_context_property_to_string shall fail and return a negative value. ]*/
static void when_snprintf_fails_for_closing_of_outer_context_log_context_property_to_string_fails(void)
{
    // arrange
    char buffer[LOG_MAX_MESSAGE_LENGTH];

    LOG_CONTEXT_LOCAL_DEFINE(context_1, NULL, LOG_CONTEXT_PROPERTY(int32_t, x, 42));
    LOG_CONTEXT_LOCAL_DEFINE(context_2, &context_1, LOG_CONTEXT_PROPERTY(uint32_t, y, 1));
    size_t property_value_pair_count = log_context_get_property_value_pair_count(&context_2);
    const LOG_CONTEXT_PROPERTY_VALUE_PAIR* property_value_pairs = log_context_get_property_value_pairs(&context_2);

    setup_mocks();

    setup_snprintf_call(); // context opening
    setup_snprintf_call(); // context opening
    setup_snprintf_call(); // property
    setup_snprintf_call(); // context closing
    setup_snprintf_call(); // property
    setup_snprintf_call(); // context closing

    expected_calls[5].snprintf_call.override_result = true;
    expected_calls[5].snprintf_call.call_result = -1;

    // act
    int result = log_context_property_to_string(buffer, LOG_MAX_MESSAGE_LENGTH, property_value_pairs, property_value_pair_count);

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(result < 0);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_TO_STRING_42_021: [ log_context_property_to_string shall store at most buffer_size characters including the null terminator in buffer (the rest of the context shall be truncated). ]*/
static void when_printing_a_property_value_exceeds_log_line_size_it_is_truncated(void)
{
    // arrange
    char buffer[LOG_MAX_MESSAGE_LENGTH];

    char* string_property_value_too_big = malloc(LOG_MAX_MESSAGE_LENGTH);
    POOR_MANS_ASSERT(string_property_value_too_big != NULL);

    (void)memset(string_property_value_too_big, 'x', LOG_MAX_MESSAGE_LENGTH - 1);
    string_property_value_too_big[LOG_MAX_MESSAGE_LENGTH - 1] = '\0';

    LOG_CONTEXT_HANDLE context_1;
    LOG_CONTEXT_CREATE(context_1, NULL, LOG_CONTEXT_STRING_PROPERTY(hagauaga, "%s", string_property_value_too_big));
    size_t property_value_pair_count = log_context_get_property_value_pair_count(context_1);
    const LOG_CONTEXT_PROPERTY_VALUE_PAIR* property_value_pairs = log_context_get_property_value_pairs(context_1);

    setup_mocks();

    setup_snprintf_call(); // context opening
    setup_snprintf_call(); // context opening
    setup_snprintf_call(); // property

    // act
    int result = log_context_property_to_string(buffer, LOG_MAX_MESSAGE_LENGTH, property_value_pairs, property_value_pair_count);

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);

    char expected_string[LOG_MAX_MESSAGE_LENGTH];
    (void)snprintf(expected_string, sizeof(expected_string), " { hagauaga=%s }", string_property_value_too_big);

    validate_property_string(buffer, expected_string);
    POOR_MANS_ASSERT(result == LOG_MAX_MESSAGE_LENGTH);

    // cleanup
    free(string_property_value_too_big);
    LOG_CONTEXT_DESTROY(context_1);
}

/* very "poor man's" way of testing, as no test harness and mocking framework are available */
int main(void)
{
    log_context_property_to_string_with_NULL_buffer_fails();
    log_context_property_to_string_with_0_buffer_size_fails();
    log_context_property_to_string_with_NULL_property_value_pairs_fails();
    log_context_property_to_string_with_0_property_value_pair_count_fails();

    log_context_property_to_string_with_single_property_no_struct();
    log_context_property_to_string_with_non_NULL_context_prints_one_property();
    log_context_property_to_string_with_non_NULL_context_prints_2_properties();
    log_context_property_to_string_with_non_NULL_context_with_2_levels_works();

    log_context_property_to_string_with_non_NULL_named_contexts_with_2_levels_works();

    log_context_property_to_string_with_non_NULL_empty_context_works();

    log_context_property_to_string_with_non_NULL_dynamically_allocated_context();

    when_snprintf_fails_for_context_open_brace_log_context_property_to_string_fails();
    when_snprintf_fails_for_inner_context_open_brace_log_context_property_to_string_fails();
    when_snprintf_fails_for_property_in_inner_context_log_context_property_to_string_fails();
    when_snprintf_fails_for_closing_of_inner_context_log_context_property_to_string_fails();
    when_snprintf_fails_for_property_in_outer_context_log_context_property_to_string_fails();
    when_snprintf_fails_for_closing_of_outer_context_log_context_property_to_string_fails();

    when_printing_a_property_value_exceeds_log_line_size_it_is_truncated();

    return 0;
}
