// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdarg.h>
#include <stdio.h> // IWYU pragma: keep
#include <stdint.h>
#include <stdlib.h> // IWYU pragma: keep
#include <stdbool.h>

#include "macro_utils/macro_utils.h"

#include "c_logging/log_context.h"
#include "c_logging/log_context_property_basic_types.h"
#include "c_logging/log_context_property_type_ascii_char_ptr.h"
#include "c_logging/log_context_property_bool_type.h"
#include "c_logging/log_context_property_type_wchar_t_ptr.h"
#include "c_logging/log_level.h"
#include "c_logging/log_sink_if.h"

#include "c_logging/log_sink_console.h"

static void test_log_sink_console_log(LOG_LEVEL log_level, LOG_CONTEXT_HANDLE log_context, const char* file, const char* func, int line, const char* message_format, ...)
{
    va_list args;
    va_start(args, message_format);
    log_sink_console.log(log_level, log_context, file, func, line, message_format, args);
    va_end(args);
}

#define POOR_MANS_ASSERT(cond) \
    if (!(cond)) \
    { \
        (void)printf("%s:%d test failed\r\n", __FUNCTION__, __LINE__); \
        abort(); \
    } \

static void log_sink_console_prints_an_empty_string(void)
{
    // arrange

    // act
    test_log_sink_console_log(LOG_LEVEL_CRITICAL, NULL, __FILE__, __FUNCTION__, __LINE__, "");

    // assert
    // no explicit assert, no crash expected
}

static void log_sink_console_prints_with_CRITICAL_a_simple_log(void)
{
    // arrange

    // act
    test_log_sink_console_log(LOG_LEVEL_CRITICAL, NULL, __FILE__, __FUNCTION__, __LINE__, "a");

    // assert
    // no explicit assert, no crash expected
}

static void log_sink_console_prints_with_ERROR_a_simple_log(void)
{
    // arrange

    // act
    test_log_sink_console_log(LOG_LEVEL_ERROR, NULL, __FILE__, __FUNCTION__, __LINE__, "a");

    // assert
    // no explicit assert, no crash expected
}

static void log_sink_console_prints_with_WARNING_a_simple_log(void)
{
    // arrange

    // act
    test_log_sink_console_log(LOG_LEVEL_WARNING, NULL, __FILE__, __FUNCTION__, __LINE__, "a");

    // assert
    // no explicit assert, no crash expected
}

static void log_sink_console_prints_with_INFO_a_simple_log(void)
{
    // arrange

    // act
    test_log_sink_console_log(LOG_LEVEL_INFO, NULL, __FILE__, __FUNCTION__, __LINE__, "a");

    // assert
    // no explicit assert, no crash expected
}

static void log_sink_console_prints_with_VERBOSE_a_simple_log(void)
{
    // arrange

    // act
    test_log_sink_console_log(LOG_LEVEL_VERBOSE, NULL, __FILE__, __FUNCTION__, __LINE__, "a");

    // assert
    // no explicit assert, no crash expected
}

static void log_sink_console_prints_with_several_arguments_for_the_message(void)
{
    // arrange

    // act
    test_log_sink_console_log(LOG_LEVEL_VERBOSE, NULL, __FILE__, __FUNCTION__, __LINE__, "%s is %d years old", "Gogu", 100);

    // assert
    // no explicit assert, no crash expected
}

static void log_sink_console_prints_with_NULL_file_works(void)
{
    // arrange

    // act
    test_log_sink_console_log(LOG_LEVEL_VERBOSE, NULL, NULL, __FUNCTION__, __LINE__, "");

    // assert
    // no explicit assert, no crash expected
}

static void log_sink_console_prints_with_NULL_function_works(void)
{
    // arrange

    // act
    test_log_sink_console_log(LOG_LEVEL_VERBOSE, NULL, __FILE__, NULL, __LINE__, "");

    // assert
    // no explicit assert, no crash expected
}

static void log_sink_console_with_empty_context_works(void)
{
    // arrange
    LOG_CONTEXT_LOCAL_DEFINE(test_context, NULL);

    // act
    test_log_sink_console_log(LOG_LEVEL_VERBOSE, &test_context, __FILE__, __FUNCTION__, __LINE__, "");

    // assert
    // no explicit assert, no crash expected
}

static void log_sink_console_with_a_context_with_one_property_works(void)
{
    // arrange
    LOG_CONTEXT_LOCAL_DEFINE(test_context, NULL, LOG_CONTEXT_PROPERTY(int32_t, x, 42));

    // act
    test_log_sink_console_log(LOG_LEVEL_VERBOSE, &test_context, __FILE__, __FUNCTION__, __LINE__, "");

    // assert
    // no explicit assert, no crash expected
}

static void log_sink_console_with_a_context_with_2_properties_works(void)
{
    // arrange
    LOG_CONTEXT_LOCAL_DEFINE(test_context, NULL,
        LOG_CONTEXT_PROPERTY(int32_t, x, 42),
        LOG_CONTEXT_PROPERTY(uint32_t, y, 4242));

    // act
    test_log_sink_console_log(LOG_LEVEL_VERBOSE, &test_context, __FILE__, __FUNCTION__, __LINE__, "");

    // assert
    // no explicit assert, no crash expected
}

static void log_sink_console_with_a_context_with_3_properties_works(void)
{
    // arrange
    LOG_CONTEXT_LOCAL_DEFINE(test_context, NULL,
        LOG_CONTEXT_PROPERTY(int32_t, x, 42),
        LOG_CONTEXT_PROPERTY(bool, z, false),
        LOG_CONTEXT_PROPERTY(bool, y, true));

    // act
    test_log_sink_console_log(LOG_LEVEL_VERBOSE, &test_context, __FILE__, __FUNCTION__, __LINE__, "");

    // assert
    // no explicit assert, no crash expected
}

static void log_sink_console_with_context_with_parent_works(void)
{
    // arrange
    LOG_CONTEXT_LOCAL_DEFINE(test_context_1, NULL,
        LOG_CONTEXT_PROPERTY(int32_t, x, 42));

    LOG_CONTEXT_LOCAL_DEFINE(test_context_2, &test_context_1,
        LOG_CONTEXT_PROPERTY(uint32_t, y, 4242));

    // act
    test_log_sink_console_log(LOG_LEVEL_VERBOSE, &test_context_2, __FILE__, __FUNCTION__, __LINE__, "");

    // assert
    // no explicit assert, no crash expected
}

static void log_sink_console_with_a_chain_of_3_contexts_works(void)
{
    // arrange
    LOG_CONTEXT_LOCAL_DEFINE(test_context_1, NULL,
        LOG_CONTEXT_PROPERTY(int32_t, x, 42));

    LOG_CONTEXT_LOCAL_DEFINE(test_context_2, &test_context_1,
        LOG_CONTEXT_PROPERTY(uint32_t, y, 4242));

    LOG_CONTEXT_LOCAL_DEFINE(test_context_3, &test_context_2,
        LOG_CONTEXT_PROPERTY(uint32_t, z, 1));

    // act
    test_log_sink_console_log(LOG_LEVEL_VERBOSE, &test_context_3, __FILE__, __FUNCTION__, __LINE__, "");

    // assert
    // no explicit assert, no crash expected
}

static void log_sink_console_with_a_dynamically_allocated_context_works(void)
{
    // arrange
    LOG_CONTEXT_HANDLE test_context;
    LOG_CONTEXT_CREATE(test_context, NULL,
        LOG_CONTEXT_PROPERTY(int32_t, x, 42));

    // act
    test_log_sink_console_log(LOG_LEVEL_VERBOSE, test_context, __FILE__, __FUNCTION__, __LINE__, "");

    // assert
    // no explicit assert, no crash expected

    // cleanup
    LOG_CONTEXT_DESTROY(test_context);
}

static void log_sink_console_with_a_context_with_string_works(void)
{
    // arrange
    LOG_CONTEXT_HANDLE test_context;
    LOG_CONTEXT_CREATE(test_context, NULL,
        LOG_CONTEXT_STRING_PROPERTY(my_string, "%s is %d years old", "Gogu", 42));

    // act
    test_log_sink_console_log(LOG_LEVEL_VERBOSE, test_context, __FILE__, __FUNCTION__, __LINE__, "");

    // assert
    // no explicit assert, no crash expected

    // cleanup
    LOG_CONTEXT_DESTROY(test_context);
}

static void log_sink_console_with_a_context_with_wstring_works(void)
{
    // arrange
    LOG_CONTEXT_HANDLE test_context;
    LOG_CONTEXT_CREATE(test_context, NULL,
        LOG_CONTEXT_WSTRING_PROPERTY(my_string, L"duru"));

    // act
    //test_log_sink_console_log(LOG_LEVEL_VERBOSE, test_context, __FILE__, __FUNCTION__, __LINE__, "");

    // assert
    // no explicit assert, no crash expected

    // cleanup
    //LOG_CONTEXT_DESTROY(test_context);
}

static void log_sink_console_with_a_context_with_boolean_true_works(void)
{
    // arrange
    LOG_CONTEXT_HANDLE test_context;
    LOG_CONTEXT_CREATE(test_context, NULL,
        LOG_CONTEXT_STRING_PROPERTY(my_string, "This statement is %" PRI_BOOL "", MU_BOOL_VALUE(true)));

    // act
    test_log_sink_console_log(LOG_LEVEL_VERBOSE, test_context, __FILE__, __FUNCTION__, __LINE__, "");

    // assert
    // no explicit assert, no crash expected

    // cleanup
    LOG_CONTEXT_DESTROY(test_context);
}

static void log_sink_console_with_a_context_with_boolean_false_works(void)
{
    // arrange
    LOG_CONTEXT_HANDLE test_context;
    LOG_CONTEXT_CREATE(test_context, NULL,
        LOG_CONTEXT_STRING_PROPERTY(my_string, "This statement is %" PRI_BOOL "", MU_BOOL_VALUE(false)));

    // act
    test_log_sink_console_log(LOG_LEVEL_VERBOSE, test_context, __FILE__, __FUNCTION__, __LINE__, "");

    // assert
    // no explicit assert, no crash expected

    // cleanup
    LOG_CONTEXT_DESTROY(test_context);
}

/* very "poor man's" way of testing, as no test harness and mocking framework are available */
int main(void)
{
    log_sink_console.init();

    log_sink_console_prints_an_empty_string();

    log_sink_console_prints_with_CRITICAL_a_simple_log();
    log_sink_console_prints_with_ERROR_a_simple_log();
    log_sink_console_prints_with_WARNING_a_simple_log();
    log_sink_console_prints_with_INFO_a_simple_log();
    log_sink_console_prints_with_VERBOSE_a_simple_log();

    log_sink_console_prints_with_several_arguments_for_the_message();

    log_sink_console_prints_with_NULL_file_works();
    log_sink_console_prints_with_NULL_function_works();

    log_sink_console_with_empty_context_works();
    log_sink_console_with_a_context_with_one_property_works();
    log_sink_console_with_a_context_with_2_properties_works();
    log_sink_console_with_a_context_with_3_properties_works();
    log_sink_console_with_context_with_parent_works();
    log_sink_console_with_a_chain_of_3_contexts_works();

    log_sink_console_with_a_dynamically_allocated_context_works();

    log_sink_console_with_a_context_with_string_works();

    log_sink_console_with_a_context_with_wstring_works();

    log_sink_console_with_a_context_with_boolean_true_works();
    log_sink_console_with_a_context_with_boolean_false_works();

    log_sink_console.deinit();

    return 0;
}
