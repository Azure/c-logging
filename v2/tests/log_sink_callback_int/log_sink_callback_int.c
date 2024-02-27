// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdarg.h>
#include <stdio.h> // IWYU pragma: keep
#include <stdint.h>
#include <stdlib.h> // IWYU pragma: keep
#include <stdbool.h>
#include <string.h>

#include "macro_utils/macro_utils.h"

#include "c_logging/log_context.h"
#include "c_logging/log_context_property_basic_types.h"
#include "c_logging/log_context_property_type_ascii_char_ptr.h"
#include "c_logging/log_context_property_bool_type.h"
#include "c_logging/log_context_property_type_wchar_t_ptr.h"
#include "c_logging/log_level.h"
#include "c_logging/log_sink_if.h"
#include "c_logging/logger.h"

#include "c_logging/log_sink_callback.h"

typedef struct LOG_CALLBACK_CONTEXT_TAG
{
    uint32_t message_count;
    LOG_LEVEL last_log_level;
    char last_message[LOG_MAX_MESSAGE_LENGTH + 1];
} LOG_CALLBACK_CONTEXT;

static LOG_CALLBACK_CONTEXT log_callback_context;

static void test_log_callback(void* context, LOG_LEVEL log_level, const char* message)
{
    LOG_CALLBACK_CONTEXT* temp_log_callback_context = context;

    temp_log_callback_context->last_log_level = log_level;
    (void)strncpy(temp_log_callback_context->last_message, message, MU_COUNT_ARRAY_ITEMS(temp_log_callback_context->last_message));
    temp_log_callback_context->message_count++;
}

static void test_log_sink_callback_log(LOG_LEVEL log_level, LOG_CONTEXT_HANDLE log_context, const char* file, const char* func, int line, const char* message_format, ...)
{
    va_list args;
    va_start(args, message_format);
    log_sink_callback.log(log_level, log_context, file, func, line, message_format, args);
    va_end(args);
}

#define POOR_MANS_ASSERT(cond) \
    if (!(cond)) \
    { \
        (void)printf("%s:%d test failed\r\n", __FUNCTION__, __LINE__); \
        abort(); \
    } \

static void assert_log_message_received(uint32_t message_count_before, LOG_LEVEL log_level, const char* file, const char* func, const char* log_context_string, const char* message)
{
    POOR_MANS_ASSERT(log_callback_context.message_count == message_count_before + 1);
    POOR_MANS_ASSERT(log_callback_context.last_log_level == log_level);
    if (file != NULL)
    {
        POOR_MANS_ASSERT(strstr(log_callback_context.last_message, file) != NULL);
    }
    if (func != NULL)
    {
        POOR_MANS_ASSERT(strstr(log_callback_context.last_message, func) != NULL);
    }
    if (log_context_string != NULL)
    {
        POOR_MANS_ASSERT(strstr(log_callback_context.last_message, log_context_string) != NULL);
    }
    if (message != NULL)
    {
        POOR_MANS_ASSERT(strstr(log_callback_context.last_message, message) != NULL);
    }
}

static void log_sink_callback_prints_an_empty_string(void)
{
    // arrange
    uint32_t count_before = log_callback_context.message_count;

    // act
    test_log_sink_callback_log(LOG_LEVEL_CRITICAL, NULL, __FILE__, __FUNCTION__, __LINE__, "");

    // assert
    assert_log_message_received(count_before, LOG_LEVEL_CRITICAL, __FILE__, __FUNCTION__, NULL, NULL);
}

static void log_sink_callback_prints_with_CRITICAL_a_simple_log(void)
{
    // arrange
    uint32_t count_before = log_callback_context.message_count;

    // act
    test_log_sink_callback_log(LOG_LEVEL_CRITICAL, NULL, __FILE__, __FUNCTION__, __LINE__, "a");

    // assert
    assert_log_message_received(count_before, LOG_LEVEL_CRITICAL, __FILE__, __FUNCTION__, NULL, "a");
}

static void log_sink_callback_prints_with_ERROR_a_simple_log(void)
{
    // arrange
    uint32_t count_before = log_callback_context.message_count;

    // act
    test_log_sink_callback_log(LOG_LEVEL_ERROR, NULL, __FILE__, __FUNCTION__, __LINE__, "a");

    // assert
    assert_log_message_received(count_before, LOG_LEVEL_ERROR, __FILE__, __FUNCTION__, NULL, "a");
}

static void log_sink_callback_prints_with_WARNING_a_simple_log(void)
{
    // arrange
    uint32_t count_before = log_callback_context.message_count;

    // act
    test_log_sink_callback_log(LOG_LEVEL_WARNING, NULL, __FILE__, __FUNCTION__, __LINE__, "a");

    // assert
    assert_log_message_received(count_before, LOG_LEVEL_WARNING, __FILE__, __FUNCTION__, NULL, "a");
}

static void log_sink_callback_prints_with_INFO_a_simple_log(void)
{
    // arrange
    uint32_t count_before = log_callback_context.message_count;

    // act
    test_log_sink_callback_log(LOG_LEVEL_INFO, NULL, __FILE__, __FUNCTION__, __LINE__, "a");

    // assert
    assert_log_message_received(count_before, LOG_LEVEL_INFO, __FILE__, __FUNCTION__, NULL, "a");
}

static void log_sink_callback_prints_with_VERBOSE_a_simple_log(void)
{
    // arrange
    uint32_t count_before = log_callback_context.message_count;

    // act
    test_log_sink_callback_log(LOG_LEVEL_VERBOSE, NULL, __FILE__, __FUNCTION__, __LINE__, "a");

    // assert
    assert_log_message_received(count_before, LOG_LEVEL_VERBOSE, __FILE__, __FUNCTION__, NULL, "a");
}

static void log_sink_callback_prints_with_several_arguments_for_the_message(void)
{
    // arrange
    uint32_t count_before = log_callback_context.message_count;

    // act
    test_log_sink_callback_log(LOG_LEVEL_VERBOSE, NULL, __FILE__, __FUNCTION__, __LINE__, "%s is %d years old", "Gogu", 100);

    // assert
    assert_log_message_received(count_before, LOG_LEVEL_VERBOSE, __FILE__, __FUNCTION__, NULL, "Gogu is 100 years old");
}

static void log_sink_callback_prints_with_NULL_file_works(void)
{
    // arrange
    uint32_t count_before = log_callback_context.message_count;

    // act
    test_log_sink_callback_log(LOG_LEVEL_VERBOSE, NULL, NULL, __FUNCTION__, __LINE__, "");

    // assert
    assert_log_message_received(count_before, LOG_LEVEL_VERBOSE, NULL, __FUNCTION__, NULL, NULL);
}

static void log_sink_callback_prints_with_NULL_function_works(void)
{
    // arrange
    uint32_t count_before = log_callback_context.message_count;

    // act
    test_log_sink_callback_log(LOG_LEVEL_VERBOSE, NULL, __FILE__, NULL, __LINE__, "");

    // assert
    assert_log_message_received(count_before, LOG_LEVEL_VERBOSE, __FILE__, NULL, NULL, NULL);
}

static void log_sink_callback_with_empty_context_works(void)
{
    // arrange
    uint32_t count_before = log_callback_context.message_count;
    LOG_CONTEXT_LOCAL_DEFINE(test_context, NULL);

    // act
    test_log_sink_callback_log(LOG_LEVEL_VERBOSE, &test_context, __FILE__, __FUNCTION__, __LINE__, "");

    // assert
    assert_log_message_received(count_before, LOG_LEVEL_VERBOSE, __FILE__, __FUNCTION__, "{ }", NULL);
}

static void log_sink_callback_with_a_context_with_one_property_works(void)
{
    // arrange
    uint32_t count_before = log_callback_context.message_count;
    LOG_CONTEXT_LOCAL_DEFINE(test_context, NULL, LOG_CONTEXT_PROPERTY(int32_t, x, 42));

    // act
    test_log_sink_callback_log(LOG_LEVEL_VERBOSE, &test_context, __FILE__, __FUNCTION__, __LINE__, "");

    // assert
    assert_log_message_received(count_before, LOG_LEVEL_VERBOSE, __FILE__, __FUNCTION__, "{ x=42 }", NULL);
}

static void log_sink_callback_with_a_context_with_one_property_and_a_message_works(void)
{
    // arrange
    uint32_t count_before = log_callback_context.message_count;
    LOG_CONTEXT_LOCAL_DEFINE(test_context, NULL, LOG_CONTEXT_PROPERTY(int32_t, x, 42));

    // act
    test_log_sink_callback_log(LOG_LEVEL_VERBOSE, &test_context, __FILE__, __FUNCTION__, __LINE__, "This is the message to log");

    // assert
    assert_log_message_received(count_before, LOG_LEVEL_VERBOSE, __FILE__, __FUNCTION__, "{ x=42 }", "This is the message to log");
}

static void log_sink_callback_with_a_context_with_2_properties_works(void)
{
    // arrange
    uint32_t count_before = log_callback_context.message_count;
    LOG_CONTEXT_LOCAL_DEFINE(test_context, NULL,
        LOG_CONTEXT_PROPERTY(int32_t, x, 42),
        LOG_CONTEXT_PROPERTY(uint32_t, y, 4242));

    // act
    test_log_sink_callback_log(LOG_LEVEL_VERBOSE, &test_context, __FILE__, __FUNCTION__, __LINE__, "");

    // assert
    assert_log_message_received(count_before, LOG_LEVEL_VERBOSE, __FILE__, __FUNCTION__, "{ x=42 y=4242 }", NULL);
}

static void log_sink_callback_with_a_context_with_3_properties_works(void)
{
    // arrange
    uint32_t count_before = log_callback_context.message_count;
    LOG_CONTEXT_LOCAL_DEFINE(test_context, NULL,
        LOG_CONTEXT_PROPERTY(int32_t, x, 42),
        LOG_CONTEXT_PROPERTY(bool, z, false),
        LOG_CONTEXT_PROPERTY(bool, y, true));

    // act
    test_log_sink_callback_log(LOG_LEVEL_VERBOSE, &test_context, __FILE__, __FUNCTION__, __LINE__, "");

    // assert
    assert_log_message_received(count_before, LOG_LEVEL_VERBOSE, __FILE__, __FUNCTION__, "{ x=42 z=false (0) y=true (1) }", NULL);
}

static void log_sink_callback_with_context_with_parent_works(void)
{
    // arrange
    uint32_t count_before = log_callback_context.message_count;
    LOG_CONTEXT_LOCAL_DEFINE(test_context_1, NULL,
        LOG_CONTEXT_PROPERTY(int32_t, x, 42));

    LOG_CONTEXT_LOCAL_DEFINE(test_context_2, &test_context_1,
        LOG_CONTEXT_PROPERTY(uint32_t, y, 4242));

    // act
    test_log_sink_callback_log(LOG_LEVEL_VERBOSE, &test_context_2, __FILE__, __FUNCTION__, __LINE__, "");

    // assert
    assert_log_message_received(count_before, LOG_LEVEL_VERBOSE, __FILE__, __FUNCTION__, "{ { x=42 } y=4242 }", NULL);
}

static void log_sink_callback_with_a_chain_of_3_contexts_works(void)
{
    // arrange
    uint32_t count_before = log_callback_context.message_count;
    LOG_CONTEXT_LOCAL_DEFINE(test_context_1, NULL,
        LOG_CONTEXT_PROPERTY(int32_t, x, 42));

    LOG_CONTEXT_LOCAL_DEFINE(test_context_2, &test_context_1,
        LOG_CONTEXT_PROPERTY(uint32_t, y, 4242));

    LOG_CONTEXT_LOCAL_DEFINE(test_context_3, &test_context_2,
        LOG_CONTEXT_PROPERTY(uint32_t, z, 1));

    // act
    test_log_sink_callback_log(LOG_LEVEL_VERBOSE, &test_context_3, __FILE__, __FUNCTION__, __LINE__, "");

    // assert
    assert_log_message_received(count_before, LOG_LEVEL_VERBOSE, __FILE__, __FUNCTION__, "{ { { x=42 } y=4242 } z=1 }", NULL);
}

static void log_sink_callback_with_a_dynamically_allocated_context_works(void)
{
    // arrange
    uint32_t count_before = log_callback_context.message_count;
    LOG_CONTEXT_HANDLE test_context;
    LOG_CONTEXT_CREATE(test_context, NULL,
        LOG_CONTEXT_PROPERTY(int32_t, x, 42));

    // act
    test_log_sink_callback_log(LOG_LEVEL_VERBOSE, test_context, __FILE__, __FUNCTION__, __LINE__, "");

    // assert
    assert_log_message_received(count_before, LOG_LEVEL_VERBOSE, __FILE__, __FUNCTION__, "{ x=42 }", NULL);

    // cleanup
    LOG_CONTEXT_DESTROY(test_context);
}

static void log_sink_callback_with_a_context_with_string_works(void)
{
    // arrange
    uint32_t count_before = log_callback_context.message_count;
    LOG_CONTEXT_HANDLE test_context;
    LOG_CONTEXT_CREATE(test_context, NULL,
        LOG_CONTEXT_STRING_PROPERTY(my_string, "%s is %d years old", "Gogu", 42));

    // act
    test_log_sink_callback_log(LOG_LEVEL_VERBOSE, test_context, __FILE__, __FUNCTION__, __LINE__, "");

    // assert
    assert_log_message_received(count_before, LOG_LEVEL_VERBOSE, __FILE__, __FUNCTION__, "{ my_string=Gogu is 42 years old }", NULL);

    // cleanup
    LOG_CONTEXT_DESTROY(test_context);
}

static void log_sink_callback_with_a_context_with_wstring_works(void)
{
    // arrange
    uint32_t count_before = log_callback_context.message_count;
    LOG_CONTEXT_HANDLE test_context;
    LOG_CONTEXT_CREATE(test_context, NULL,
        LOG_CONTEXT_WSTRING_PROPERTY(my_string, L"%ls is %d years old", L"Gogu", 42));

    // act
    test_log_sink_callback_log(LOG_LEVEL_VERBOSE, test_context, __FILE__, __FUNCTION__, __LINE__, "");

    // assert
    assert_log_message_received(count_before, LOG_LEVEL_VERBOSE, __FILE__, __FUNCTION__, "{ my_string=Gogu is 42 years old }", NULL);

    // cleanup
    LOG_CONTEXT_DESTROY(test_context);
}

static void log_sink_callback_with_a_context_with_boolean_true_works(void)
{
    // arrange
    uint32_t count_before = log_callback_context.message_count;
    LOG_CONTEXT_HANDLE test_context;
    LOG_CONTEXT_CREATE(test_context, NULL,
        LOG_CONTEXT_STRING_PROPERTY(my_string, "This statement is %" PRI_BOOL "", MU_BOOL_VALUE(true)));

    // act
    test_log_sink_callback_log(LOG_LEVEL_VERBOSE, test_context, __FILE__, __FUNCTION__, __LINE__, "");

    // assert
    assert_log_message_received(count_before, LOG_LEVEL_VERBOSE, __FILE__, __FUNCTION__, "{ my_string=This statement is true (1) }", NULL);

    // cleanup
    LOG_CONTEXT_DESTROY(test_context);
}

static void log_sink_callback_with_a_context_with_boolean_false_works(void)
{
    // arrange
    uint32_t count_before = log_callback_context.message_count;
    LOG_CONTEXT_HANDLE test_context;
    LOG_CONTEXT_CREATE(test_context, NULL,
        LOG_CONTEXT_STRING_PROPERTY(my_string, "This statement is %" PRI_BOOL "", MU_BOOL_VALUE(false)));

    // act
    test_log_sink_callback_log(LOG_LEVEL_VERBOSE, test_context, __FILE__, __FUNCTION__, __LINE__, "");

    // assert
    assert_log_message_received(count_before, LOG_LEVEL_VERBOSE, __FILE__, __FUNCTION__, "{ my_string=This statement is false (0) }", NULL);

    // cleanup
    LOG_CONTEXT_DESTROY(test_context);
}

/* very "poor man's" way of testing, as no test harness and mocking framework are available */
int main(void)
{
    log_callback_context.message_count = 0;
    POOR_MANS_ASSERT(log_sink_callback_set_callback(test_log_callback, &log_callback_context) == 0);
    log_sink_callback.init();

    log_sink_callback_prints_an_empty_string();

    log_sink_callback_prints_with_CRITICAL_a_simple_log();
    log_sink_callback_prints_with_ERROR_a_simple_log();
    log_sink_callback_prints_with_WARNING_a_simple_log();
    log_sink_callback_prints_with_INFO_a_simple_log();
    log_sink_callback_prints_with_VERBOSE_a_simple_log();

    log_sink_callback_prints_with_several_arguments_for_the_message();

    log_sink_callback_prints_with_NULL_file_works();
    log_sink_callback_prints_with_NULL_function_works();

    log_sink_callback_with_empty_context_works();
    log_sink_callback_with_a_context_with_one_property_works();
    log_sink_callback_with_a_context_with_2_properties_works();
    log_sink_callback_with_a_context_with_3_properties_works();
    log_sink_callback_with_context_with_parent_works();
    log_sink_callback_with_a_chain_of_3_contexts_works();

    log_sink_callback_with_a_dynamically_allocated_context_works();

    log_sink_callback_with_a_context_with_string_works();

    log_sink_callback_with_a_context_with_wstring_works();

    log_sink_callback_with_a_context_with_boolean_true_works();
    log_sink_callback_with_a_context_with_boolean_false_works();

    log_sink_callback.deinit();

    return 0;
}
