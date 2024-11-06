// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#ifdef _MSC_VER
#include "windows.h"
#endif

#include "macro_utils/macro_utils.h"

#include "c_logging/log_context_property_type.h"
#include "c_logging/log_context_property_basic_types.h"
#include "c_logging/log_context_property_type_ascii_char_ptr.h"
#include "c_logging/log_context_property_type_if.h"
#include "c_logging/log_context_property_value_pair.h"
#include "c_logging/log_level.h"
#include "c_logging/log_sink_if.h"
#include "c_logging/log_context.h"

#include "c_logging/logger.h"

// defines how many mock calls we can have
#define MAX_MOCK_CALL_COUNT (128)

#define MOCK_CALL_TYPE_VALUES \
    MOCK_CALL_TYPE_log_sink1_init, \
    MOCK_CALL_TYPE_log_sink1_deinit, \
    MOCK_CALL_TYPE_log_sink1_log, \
    MOCK_CALL_TYPE_log_sink2_init, \
    MOCK_CALL_TYPE_log_sink2_deinit, \
    MOCK_CALL_TYPE_log_sink2_log, \
    MOCK_CALL_TYPE_abort

MU_DEFINE_ENUM(MOCK_CALL_TYPE, MOCK_CALL_TYPE_VALUES)

// very poor mans mocks :-(

#define MAX_FILE_STRING_LENGTH  256
#define MAX_FUNC_STRING_LENGTH  256
#define MAX_MESSAGE_STRING_LENGTH  1024

#define MAX_PRINTF_CAPTURED_OUPUT_SIZE (LOG_MAX_MESSAGE_LENGTH * 2)

typedef struct log_sink1_init_CALL_TAG
{
    bool override_result;
    int call_result;
} log_sink1_init_CALL;

typedef struct log_sink1_deinit_CALL_TAG
{
    int dummy;
} log_sink1_deinit_CALL;

typedef struct log_sink1_log_CALL_TAG
{
    LOG_LEVEL captured_log_level;
    LOG_CONTEXT_HANDLE captured_log_context;
    char captured_file[MAX_FILE_STRING_LENGTH];
    char captured_func[MAX_FUNC_STRING_LENGTH];
    int captured_line;
    char captured_message[MAX_MESSAGE_STRING_LENGTH];
} log_sink1_log_CALL;

typedef struct log_sink2_init_CALL_TAG
{
    bool override_result;
    int call_result;
} log_sink2_init_CALL;

typedef struct log_sink2_deinit_CALL_TAG
{
    int dummy;
} log_sink2_deinit_CALL;

typedef struct log_sink2_log_CALL_TAG
{
    LOG_LEVEL captured_log_level;
    LOG_CONTEXT_HANDLE captured_log_context;
    char captured_file[MAX_FILE_STRING_LENGTH];
    char captured_func[MAX_FUNC_STRING_LENGTH];
    int captured_line;
    char captured_message[MAX_MESSAGE_STRING_LENGTH];
} log_sink2_log_CALL;

typedef struct MOCK_CALL_TAG
{
    MOCK_CALL_TYPE mock_call_type;
    union
    {
        log_sink1_init_CALL log_sink1_init_call;
        log_sink1_deinit_CALL log_sink1_deinit_call;
        log_sink1_log_CALL log_sink1_log_call;
        log_sink2_init_CALL log_sink2_init_call;
        log_sink2_deinit_CALL log_sink2_deinit_call;
        log_sink2_log_CALL log_sink2_log_call;
    };
} MOCK_CALL;

static MOCK_CALL expected_calls[MAX_MOCK_CALL_COUNT];
static size_t expected_call_count;
static size_t actual_call_count;
static bool actual_and_expected_match;

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

static void cleanup_calls(void)
{
    for (uint32_t i = 0; i < expected_call_count; i++)
    {
        if (
            (expected_calls[i].mock_call_type == MOCK_CALL_TYPE_log_sink1_log) &&
            (expected_calls[i].log_sink1_log_call.captured_log_context != NULL)
            )
        {
            LOG_CONTEXT_DESTROY(expected_calls[i].log_sink1_log_call.captured_log_context);
        }

        if (
            (expected_calls[i].mock_call_type == MOCK_CALL_TYPE_log_sink2_log) &&
            (expected_calls[i].log_sink2_log_call.captured_log_context != NULL)
            )
        {
            LOG_CONTEXT_DESTROY(expected_calls[i].log_sink2_log_call.captured_log_context);
        }
    }
}

static int log_sink1_init(void)
{
    int result;
    
    if ((actual_call_count == expected_call_count) ||
        (expected_calls[actual_call_count].mock_call_type != MOCK_CALL_TYPE_log_sink1_init))
    {
        actual_and_expected_match = false;
        result = MU_FAILURE;
    }
    else
    {
        if (expected_calls[actual_call_count].log_sink1_init_call.override_result)
        {
            result = expected_calls[actual_call_count].log_sink1_init_call.call_result;
        }
        else
        {
            result = 0;
        }
    
        actual_call_count++;
    }
    
    return result;
}

static void log_sink1_deinit(void)
{
    if ((actual_call_count == expected_call_count) ||
        (expected_calls[actual_call_count].mock_call_type != MOCK_CALL_TYPE_log_sink1_deinit))
    {
        actual_and_expected_match = false;
    }
    else
    {
        actual_call_count++;
    }
}

static void log_sink1_log(LOG_LEVEL log_level, LOG_CONTEXT_HANDLE log_context, const char* file, const char* func, int line, const char* message_format, va_list args)
{
    if ((actual_call_count == expected_call_count) ||
        (expected_calls[actual_call_count].mock_call_type != MOCK_CALL_TYPE_log_sink1_log))
    {
        actual_and_expected_match = false;
    }
    else
    {
        expected_calls[actual_call_count].log_sink1_log_call.captured_log_level = log_level;
        if (log_context == NULL)
        {
            expected_calls[actual_call_count].log_sink1_log_call.captured_log_context = NULL;
        }
        else
        {
            LOG_CONTEXT_CREATE(expected_calls[actual_call_count].log_sink1_log_call.captured_log_context, log_context);
        }
        int snprintf_result = snprintf(expected_calls[actual_call_count].log_sink1_log_call.captured_file, sizeof(expected_calls[actual_call_count].log_sink1_log_call.captured_file), "%s", file);
        POOR_MANS_ASSERT((snprintf_result >= 0) && (snprintf_result < sizeof(expected_calls[actual_call_count].log_sink1_log_call.captured_file)));
        snprintf_result = snprintf(expected_calls[actual_call_count].log_sink1_log_call.captured_func, sizeof(expected_calls[actual_call_count].log_sink1_log_call.captured_func), "%s", func);
        POOR_MANS_ASSERT((snprintf_result >= 0) && (snprintf_result < sizeof(expected_calls[actual_call_count].log_sink1_log_call.captured_func)));
        expected_calls[actual_call_count].log_sink1_log_call.captured_line = line;
        snprintf_result = vsnprintf(expected_calls[actual_call_count].log_sink1_log_call.captured_message, sizeof(expected_calls[actual_call_count].log_sink1_log_call.captured_message), message_format, args);
        POOR_MANS_ASSERT((snprintf_result >= 0) && (snprintf_result < sizeof(expected_calls[actual_call_count].log_sink1_log_call.captured_message)));

        actual_call_count++;
    }
}

static const LOG_SINK_IF log_sink1 =
{
    .init = log_sink1_init,
    .deinit = log_sink1_deinit,
    .log = log_sink1_log
};

static int log_sink2_init(void)
{
    int result;

    if ((actual_call_count == expected_call_count) ||
        (expected_calls[actual_call_count].mock_call_type != MOCK_CALL_TYPE_log_sink2_init))
    {
        actual_and_expected_match = false;
        result = MU_FAILURE;
    }
    else
    {
        if (expected_calls[actual_call_count].log_sink2_init_call.override_result)
        {
            result = expected_calls[actual_call_count].log_sink2_init_call.call_result;
        }
        else
        {
            result = 0;
        }

        actual_call_count++;
    }

    return result;
}

static void log_sink2_deinit(void)
{
    if ((actual_call_count == expected_call_count) ||
        (expected_calls[actual_call_count].mock_call_type != MOCK_CALL_TYPE_log_sink2_deinit))
    {
        actual_and_expected_match = false;
    }
    else
    {
        actual_call_count++;
    }
}

static void log_sink2_log(LOG_LEVEL log_level, LOG_CONTEXT_HANDLE log_context, const char* file, const char* func, int line, const char* message_format, va_list args)
{
    if ((actual_call_count == expected_call_count) ||
        (expected_calls[actual_call_count].mock_call_type != MOCK_CALL_TYPE_log_sink2_log))
    {
        actual_and_expected_match = false;
    }
    else
    {
        expected_calls[actual_call_count].log_sink2_log_call.captured_log_level = log_level;
        if (log_context == NULL)
        {
            expected_calls[actual_call_count].log_sink2_log_call.captured_log_context = NULL;
        }
        else
        {
            LOG_CONTEXT_CREATE(expected_calls[actual_call_count].log_sink2_log_call.captured_log_context, log_context);
        }
        int snprintf_result = snprintf(expected_calls[actual_call_count].log_sink2_log_call.captured_file, sizeof(expected_calls[actual_call_count].log_sink2_log_call.captured_file), "%s", file);
        POOR_MANS_ASSERT((snprintf_result >= 0) && (snprintf_result < sizeof(expected_calls[actual_call_count].log_sink2_log_call.captured_file)));
        snprintf_result = snprintf(expected_calls[actual_call_count].log_sink2_log_call.captured_func, sizeof(expected_calls[actual_call_count].log_sink2_log_call.captured_func), "%s", func);
        POOR_MANS_ASSERT((snprintf_result >= 0) && (snprintf_result < sizeof(expected_calls[actual_call_count].log_sink2_log_call.captured_func)));
        expected_calls[actual_call_count].log_sink2_log_call.captured_line = line;
        snprintf_result = vsnprintf(expected_calls[actual_call_count].log_sink2_log_call.captured_message, sizeof(expected_calls[actual_call_count].log_sink2_log_call.captured_message), message_format, args);
        POOR_MANS_ASSERT((snprintf_result >= 0) && (snprintf_result < sizeof(expected_calls[actual_call_count].log_sink2_log_call.captured_message)));

        actual_call_count++;
    }
}

void mock_abort(void)
{
    if ((actual_call_count == expected_call_count) ||
        (expected_calls[actual_call_count].mock_call_type != MOCK_CALL_TYPE_abort))
    {
        actual_and_expected_match = false;
    }
    else
    {
        actual_call_count++;
    }
}

static const LOG_SINK_IF log_sink2 =
{
    .init = log_sink2_init,
    .deinit = log_sink2_deinit,
    .log = log_sink2_log
};

// test config
static const LOG_SINK_IF* test_log_sinks[] =
{
    &log_sink1,
    &log_sink2
};

const LOG_SINK_IF** log_sinks = test_log_sinks;
uint32_t log_sink_count = MU_COUNT_ARRAY_ITEMS(test_log_sinks);

static void setup_log_sink1_init_call(void)
{
    expected_calls[expected_call_count].mock_call_type = MOCK_CALL_TYPE_log_sink1_init;
    expected_calls[expected_call_count].log_sink1_init_call.override_result = false;
    expected_call_count++;
}

static void setup_log_sink1_deinit_call(void)
{
    expected_calls[expected_call_count].mock_call_type = MOCK_CALL_TYPE_log_sink1_deinit;
    expected_call_count++;
}

static void setup_log_sink1_log_call(void)
{
    expected_calls[expected_call_count].mock_call_type = MOCK_CALL_TYPE_log_sink1_log;
    expected_calls[expected_call_count].log_sink1_log_call.captured_log_context = NULL;
    expected_call_count++;
}

static void setup_log_sink2_init_call(void)
{
    expected_calls[expected_call_count].mock_call_type = MOCK_CALL_TYPE_log_sink2_init;
    expected_calls[expected_call_count].log_sink2_init_call.override_result = false;
    expected_call_count++;
}

static void setup_log_sink2_deinit_call(void)
{
    expected_calls[expected_call_count].mock_call_type = MOCK_CALL_TYPE_log_sink2_deinit;
    expected_call_count++;
}

static void setup_log_sink2_log_call(void)
{
    expected_calls[expected_call_count].mock_call_type = MOCK_CALL_TYPE_log_sink2_log;
    expected_calls[expected_call_count].log_sink2_log_call.captured_log_context = NULL;
    expected_call_count++;
}

static void setup_abort(void)
{
    expected_calls[expected_call_count].mock_call_type = MOCK_CALL_TYPE_abort;
    expected_call_count++;
}

/* Tests_SRS_LOGGER_01_006: [ If logger is not initialized, logger_deinit shall return. ] */
static void logger_deinit_when_not_initialized_returns(void)
{
    // arrange
    setup_mocks();

    // act
    logger_deinit();

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);

    // cleanup
    cleanup_calls();
}

/* logger_init */

/* Tests_SRS_LOGGER_01_004: [ If init fails, all sinks already initialized shall have their deinit function called and logger_init shall fail and return a non-zero value. ] */
static void when_the_1st_sink_init_fails_logger_init_fails(void)
{
    // arrange
    setup_mocks();
    setup_log_sink1_init_call();
    expected_calls[0].log_sink1_init_call.override_result = true;
    expected_calls[0].log_sink1_init_call.call_result = MU_FAILURE;

    // act
    int result = logger_init();

    // assert
    POOR_MANS_ASSERT(result != 0);
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);

    // cleanup
    cleanup_calls();
}

/* Tests_SRS_LOGGER_01_004: [ If init fails, all sinks already initialized shall have their deinit function called and logger_init shall fail and return a non-zero value. ] */
static void when_the_2nd_sink_init_fails_logger_init_fails(void)
{
    // arrange
    setup_mocks();
    setup_log_sink1_init_call();
    setup_log_sink2_init_call();
    expected_calls[1].log_sink2_init_call.override_result = true;
    expected_calls[1].log_sink2_init_call.call_result = MU_FAILURE;
    setup_log_sink1_deinit_call();

    // act
    int result = logger_init();

    // assert
    POOR_MANS_ASSERT(result != 0);
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOGGER_01_019: [ If logger is not already initialized: ] */
/* Tests_SRS_LOGGER_01_020: [ logger_init shall set the logger initialization counter to 1. ] */
/* Tests_SRS_LOGGER_01_003: [ logger_init shall call the init function of every sink that is configured to be used. ] */
/* Tests_SRS_LOGGER_01_005: [ Otherwise, logger_init shall succeed and return 0. ] */
static void logger_init_initializes_sinks(void)
{
    // arrange
    setup_mocks();
    setup_log_sink1_init_call();
    setup_log_sink2_init_call();

    // act
    int result = logger_init();

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);

    // cleanup
    logger_deinit();
    cleanup_calls();
}

static void test_logger_init(void)
{
    setup_mocks();
    setup_log_sink1_init_call();
    setup_log_sink2_init_call();
    POOR_MANS_ASSERT(logger_init() == 0);
}

/* Tests_SRS_LOGGER_01_002: [ If logger is already initialized, logger_init shall increment the logger initialization counter, succeed and return 0. ] */
static void logger_init_after_init_succeeds(void)
{
    // arrange
    test_logger_init();
    setup_mocks();

    // act
    int result = logger_init();

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);

    // cleanup
    logger_deinit();
    logger_deinit();
    cleanup_calls();
}

/* LOGGER_LOG */

static void test_with_log_level(LOG_LEVEL log_level)
{
    // arrange
    test_logger_init();
    setup_mocks();
    setup_log_sink1_log_call();
    setup_log_sink2_log_call();

    // act
    // capture the line no of the error
    int expected_line = __LINE__; LOGGER_LOG(log_level, NULL, "gigi duru");

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);

    // 1st sink
    POOR_MANS_ASSERT(expected_calls[0].log_sink1_log_call.captured_log_level == log_level);
    POOR_MANS_ASSERT(expected_calls[0].log_sink1_log_call.captured_log_context == NULL);
    POOR_MANS_ASSERT(strcmp(expected_calls[0].log_sink1_log_call.captured_file, __FILE__) == 0);
    POOR_MANS_ASSERT(strcmp(expected_calls[0].log_sink1_log_call.captured_func, __FUNCTION__) == 0);
    POOR_MANS_ASSERT(expected_calls[0].log_sink1_log_call.captured_line == expected_line);
    POOR_MANS_ASSERT(strcmp(expected_calls[0].log_sink1_log_call.captured_message, "gigi duru") == 0);

    // 2nd sink
    POOR_MANS_ASSERT(expected_calls[1].log_sink2_log_call.captured_log_level == log_level);
    POOR_MANS_ASSERT(expected_calls[1].log_sink2_log_call.captured_log_context == NULL);
    POOR_MANS_ASSERT(strcmp(expected_calls[1].log_sink2_log_call.captured_file, __FILE__) == 0);
    POOR_MANS_ASSERT(strcmp(expected_calls[1].log_sink2_log_call.captured_func, __FUNCTION__) == 0);
    POOR_MANS_ASSERT(expected_calls[1].log_sink2_log_call.captured_line == expected_line);
    POOR_MANS_ASSERT(strcmp(expected_calls[1].log_sink2_log_call.captured_message, "gigi duru") == 0);

    // cleanup
    logger_deinit();
    cleanup_calls();
}

/* Tests_SRS_LOGGER_01_001: [ LOGGER_LOG shall call the log function of every sink that is configured to be used. ] */
static void LOGGER_LOG_with_CRITICAL_works(void)
{
    test_with_log_level(LOG_LEVEL_CRITICAL);
}

/* Tests_SRS_LOGGER_01_001: [ LOGGER_LOG shall call the log function of every sink that is configured to be used. ] */
static void LOGGER_LOG_with_ERROR_works(void)
{
    test_with_log_level(LOG_LEVEL_ERROR);
}

/* Tests_SRS_LOGGER_01_001: [ LOGGER_LOG shall call the log function of every sink that is configured to be used. ] */
static void LOGGER_LOG_with_INFO_works(void)
{
    test_with_log_level(LOG_LEVEL_INFO);
}

/* Tests_SRS_LOGGER_01_001: [ LOGGER_LOG shall call the log function of every sink that is configured to be used. ] */
static void LOGGER_LOG_with_WARNING_works(void)
{
    test_with_log_level(LOG_LEVEL_WARNING);
}

/* Tests_SRS_LOGGER_01_001: [ LOGGER_LOG shall call the log function of every sink that is configured to be used. ] */
static void LOGGER_LOG_with_VERBOSE_works(void)
{
    test_with_log_level(LOG_LEVEL_VERBOSE);
}

/* Tests_SRS_LOGGER_01_001: [ LOGGER_LOG shall call the log function of every sink that is configured to be used. ] */
static void LOGGER_LOG_with_non_NULL_context(void)
{
    // arrange
    LOG_CONTEXT_HANDLE log_context;
    LOG_CONTEXT_CREATE(log_context, NULL);

    test_logger_init();
    setup_mocks();
    setup_log_sink1_log_call();
    setup_log_sink2_log_call();

    // act
    // capture the line no of the error
    int expected_line = __LINE__; LOGGER_LOG(LOG_LEVEL_ERROR, log_context, "u lala %d", 42);

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);

    // 1st sink
    POOR_MANS_ASSERT(expected_calls[0].log_sink1_log_call.captured_log_level == LOG_LEVEL_ERROR);
    POOR_MANS_ASSERT(expected_calls[0].log_sink1_log_call.captured_log_context != NULL);
    POOR_MANS_ASSERT(strcmp(expected_calls[0].log_sink1_log_call.captured_file, __FILE__) == 0);
    POOR_MANS_ASSERT(strcmp(expected_calls[0].log_sink1_log_call.captured_func, __FUNCTION__) == 0);
    POOR_MANS_ASSERT(expected_calls[0].log_sink1_log_call.captured_line == expected_line);
    POOR_MANS_ASSERT(strcmp(expected_calls[0].log_sink1_log_call.captured_message, "u lala 42") == 0);

    // 2nd sink
    POOR_MANS_ASSERT(expected_calls[1].log_sink2_log_call.captured_log_level == LOG_LEVEL_ERROR);
    POOR_MANS_ASSERT(expected_calls[1].log_sink2_log_call.captured_log_context != NULL);
    POOR_MANS_ASSERT(strcmp(expected_calls[1].log_sink2_log_call.captured_file, __FILE__) == 0);
    POOR_MANS_ASSERT(strcmp(expected_calls[1].log_sink2_log_call.captured_func, __FUNCTION__) == 0);
    POOR_MANS_ASSERT(expected_calls[1].log_sink2_log_call.captured_line == expected_line);
    POOR_MANS_ASSERT(strcmp(expected_calls[1].log_sink2_log_call.captured_message, "u lala 42") == 0);

    //cleanup
    LOG_CONTEXT_DESTROY(log_context);
    logger_deinit();
    cleanup_calls();
}

/* LOGGER_LOG_EX */

/* Tests_SRS_LOGGER_01_008: [ LOGGER_LOG_EX shall call the log function of every sink that is configured to be used. ]*/
/* Tests_SRS_LOGGER_01_010: [ Otherwise, LOGGER_LOG_EX shall construct a log context with all the properties specified in .... ] */
/* Tests_SRS_LOGGER_01_011: [ Each LOG_CONTEXT_STRING_PROPERTY and LOG_CONTEXT_PROPERTY entry in ... shall be added as a property in the context that is passed to log. ] */
static void LOGGER_LOG_EX_works(void)
{
    // arrange
    test_logger_init();

    setup_mocks();
    setup_log_sink1_log_call();
    setup_log_sink2_log_call();

    // act
    // capture the line no of the error
    int expected_line = __LINE__; LOGGER_LOG_EX(LOG_LEVEL_INFO, LOG_CONTEXT_PROPERTY(int32_t, prop1, 42));

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);

    // 1st sink
    POOR_MANS_ASSERT(expected_calls[0].log_sink1_log_call.captured_log_level == LOG_LEVEL_INFO);
    POOR_MANS_ASSERT(expected_calls[0].log_sink1_log_call.captured_log_context != NULL);
    POOR_MANS_ASSERT(strcmp(expected_calls[0].log_sink1_log_call.captured_file, __FILE__) == 0);
    POOR_MANS_ASSERT(strcmp(expected_calls[0].log_sink1_log_call.captured_func, __FUNCTION__) == 0);
    POOR_MANS_ASSERT(expected_calls[0].log_sink1_log_call.captured_line == expected_line);
    POOR_MANS_ASSERT(strcmp(expected_calls[0].log_sink1_log_call.captured_message, "") == 0);

    // check properties
    uint32_t captured_context_property_count = log_context_get_property_value_pair_count(expected_calls[0].log_sink1_log_call.captured_log_context);
    const LOG_CONTEXT_PROPERTY_VALUE_PAIR* captured_context_properties = log_context_get_property_value_pairs(expected_calls[0].log_sink1_log_call.captured_log_context);
    // we expect one extra property as we're "copying" the context by creating a parent one
    POOR_MANS_ASSERT(captured_context_property_count == 3); 
    POOR_MANS_ASSERT(strcmp(captured_context_properties[1].name, "") == 0);
    POOR_MANS_ASSERT(captured_context_properties[1].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_struct);
    POOR_MANS_ASSERT(*(uint8_t*)captured_context_properties[1].value == 1);
    POOR_MANS_ASSERT(strcmp(captured_context_properties[2].name, "prop1") == 0);
    POOR_MANS_ASSERT(captured_context_properties[2].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_int32_t);
    POOR_MANS_ASSERT(*(int32_t*)captured_context_properties[2].value == 42);

    // 2nd sink
    POOR_MANS_ASSERT(expected_calls[1].log_sink2_log_call.captured_log_level == LOG_LEVEL_INFO);
    POOR_MANS_ASSERT(expected_calls[1].log_sink2_log_call.captured_log_context != NULL);
    POOR_MANS_ASSERT(strcmp(expected_calls[1].log_sink2_log_call.captured_file, __FILE__) == 0);
    POOR_MANS_ASSERT(strcmp(expected_calls[1].log_sink2_log_call.captured_func, __FUNCTION__) == 0);
    POOR_MANS_ASSERT(expected_calls[1].log_sink2_log_call.captured_line == expected_line);
    POOR_MANS_ASSERT(strcmp(expected_calls[1].log_sink2_log_call.captured_message, "") == 0);

    // check properties
    captured_context_property_count = log_context_get_property_value_pair_count(expected_calls[0].log_sink2_log_call.captured_log_context);
    captured_context_properties = log_context_get_property_value_pairs(expected_calls[0].log_sink2_log_call.captured_log_context);
    // we expect one extra property as we're "copying" the context by creating a parent one
    POOR_MANS_ASSERT(captured_context_property_count == 3);
    POOR_MANS_ASSERT(strcmp(captured_context_properties[1].name, "") == 0);
    POOR_MANS_ASSERT(captured_context_properties[1].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_struct);
    POOR_MANS_ASSERT(*(uint8_t*)captured_context_properties[1].value == 1);
    POOR_MANS_ASSERT(strcmp(captured_context_properties[2].name, "prop1") == 0);
    POOR_MANS_ASSERT(captured_context_properties[2].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_int32_t);
    POOR_MANS_ASSERT(*(int32_t*)captured_context_properties[2].value == 42);

    //cleanup
    logger_deinit();
    cleanup_calls();
}

/* Tests_SRS_LOGGER_01_008: [ LOGGER_LOG_EX shall call the log function of every sink that is configured to be used. ]*/
/* Tests_SRS_LOGGER_01_010: [ Otherwise, LOGGER_LOG_EX shall construct a log context with all the properties specified in .... ] */
/* Tests_SRS_LOGGER_01_011: [ Each LOG_CONTEXT_STRING_PROPERTY and LOG_CONTEXT_PROPERTY entry in ... shall be added as a property in the context that is passed to log. ] */
static void LOGGER_LOG_EX_works_with_2_properties(void)
{
    // arrange
    test_logger_init();

    setup_mocks();
    setup_log_sink1_log_call();
    setup_log_sink2_log_call();

    // act
    // capture the line no of the error
    int expected_line = __LINE__; LOGGER_LOG_EX(LOG_LEVEL_INFO, LOG_CONTEXT_PROPERTY(int8_t, prop1, 42), LOG_CONTEXT_PROPERTY(uint8_t, prop2, 43));

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);

    // 1st sink
    POOR_MANS_ASSERT(expected_calls[0].log_sink1_log_call.captured_log_level == LOG_LEVEL_INFO);
    POOR_MANS_ASSERT(expected_calls[0].log_sink1_log_call.captured_log_context != NULL);
    POOR_MANS_ASSERT(strcmp(expected_calls[0].log_sink1_log_call.captured_file, __FILE__) == 0);
    POOR_MANS_ASSERT(strcmp(expected_calls[0].log_sink1_log_call.captured_func, __FUNCTION__) == 0);
    POOR_MANS_ASSERT(expected_calls[0].log_sink1_log_call.captured_line == expected_line);
    POOR_MANS_ASSERT(strcmp(expected_calls[0].log_sink1_log_call.captured_message, "") == 0);

    // check properties
    uint32_t captured_context_property_count = log_context_get_property_value_pair_count(expected_calls[0].log_sink1_log_call.captured_log_context);
    const LOG_CONTEXT_PROPERTY_VALUE_PAIR* captured_context_properties = log_context_get_property_value_pairs(expected_calls[0].log_sink1_log_call.captured_log_context);
    // we expect one extra property as we're "copying" the context by creating a parent one
    POOR_MANS_ASSERT(captured_context_property_count == 4);
    POOR_MANS_ASSERT(strcmp(captured_context_properties[1].name, "") == 0);
    POOR_MANS_ASSERT(captured_context_properties[1].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_struct);
    POOR_MANS_ASSERT(*(uint8_t*)captured_context_properties[1].value == 2);
    POOR_MANS_ASSERT(strcmp(captured_context_properties[2].name, "prop1") == 0);
    POOR_MANS_ASSERT(captured_context_properties[2].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_int8_t);
    POOR_MANS_ASSERT(*(int8_t*)captured_context_properties[2].value == 42);
    POOR_MANS_ASSERT(strcmp(captured_context_properties[3].name, "prop2") == 0);
    POOR_MANS_ASSERT(captured_context_properties[3].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_uint8_t);
    POOR_MANS_ASSERT(*(uint8_t*)captured_context_properties[3].value == 43);

    // 2nd sink
    POOR_MANS_ASSERT(expected_calls[1].log_sink2_log_call.captured_log_level == LOG_LEVEL_INFO);
    POOR_MANS_ASSERT(expected_calls[1].log_sink2_log_call.captured_log_context != NULL);
    POOR_MANS_ASSERT(strcmp(expected_calls[1].log_sink2_log_call.captured_file, __FILE__) == 0);
    POOR_MANS_ASSERT(strcmp(expected_calls[1].log_sink2_log_call.captured_func, __FUNCTION__) == 0);
    POOR_MANS_ASSERT(expected_calls[1].log_sink2_log_call.captured_line == expected_line);
    POOR_MANS_ASSERT(strcmp(expected_calls[1].log_sink2_log_call.captured_message, "") == 0);

    // check properties
    captured_context_property_count = log_context_get_property_value_pair_count(expected_calls[0].log_sink2_log_call.captured_log_context);
    captured_context_properties = log_context_get_property_value_pairs(expected_calls[0].log_sink2_log_call.captured_log_context);
    // we expect one extra property as we're "copying" the context by creating a parent one
    POOR_MANS_ASSERT(captured_context_property_count == 4);
    POOR_MANS_ASSERT(strcmp(captured_context_properties[1].name, "") == 0);
    POOR_MANS_ASSERT(captured_context_properties[1].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_struct);
    POOR_MANS_ASSERT(*(uint8_t*)captured_context_properties[1].value == 2);
    POOR_MANS_ASSERT(strcmp(captured_context_properties[2].name, "prop1") == 0);
    POOR_MANS_ASSERT(captured_context_properties[2].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_int8_t);
    POOR_MANS_ASSERT(*(int8_t*)captured_context_properties[2].value == 42);
    POOR_MANS_ASSERT(strcmp(captured_context_properties[3].name, "prop2") == 0);
    POOR_MANS_ASSERT(captured_context_properties[3].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_uint8_t);
    POOR_MANS_ASSERT(*(uint8_t*)captured_context_properties[3].value == 43);

    //cleanup
    logger_deinit();
    cleanup_calls();
}

/* Tests_SRS_LOGGER_01_008: [ LOGGER_LOG_EX shall call the log function of every sink that is configured to be used. ]*/
/* Tests_SRS_LOGGER_01_010: [ Otherwise, LOGGER_LOG_EX shall construct a log context with all the properties specified in .... ] */
/* Tests_SRS_LOGGER_01_011: [ Each LOG_CONTEXT_STRING_PROPERTY and LOG_CONTEXT_PROPERTY entry in ... shall be added as a property in the context that is passed to log. ] */
static void LOGGER_LOG_EX_twice_works(void)
{
    // arrange
    test_logger_init();
    setup_mocks();
    // 1st log line
    setup_log_sink1_log_call();
    setup_log_sink2_log_call();
    // 2nd log line
    setup_log_sink1_log_call();
    setup_log_sink2_log_call();

    // act
    // capture the line no of the error
    int expected_line_1 = __LINE__; LOGGER_LOG_EX(LOG_LEVEL_INFO, LOG_CONTEXT_PROPERTY(int8_t, prop1, 42));
    int expected_line_2 = __LINE__; LOGGER_LOG_EX(LOG_LEVEL_CRITICAL, LOG_CONTEXT_PROPERTY(uint8_t, prop2, 43));

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);

    // 1st sink
    POOR_MANS_ASSERT(expected_calls[0].log_sink1_log_call.captured_log_level == LOG_LEVEL_INFO);
    POOR_MANS_ASSERT(expected_calls[0].log_sink1_log_call.captured_log_context != NULL);
    POOR_MANS_ASSERT(strcmp(expected_calls[0].log_sink1_log_call.captured_file, __FILE__) == 0);
    POOR_MANS_ASSERT(strcmp(expected_calls[0].log_sink1_log_call.captured_func, __FUNCTION__) == 0);
    POOR_MANS_ASSERT(expected_calls[0].log_sink1_log_call.captured_line == expected_line_1);
    POOR_MANS_ASSERT(strcmp(expected_calls[0].log_sink1_log_call.captured_message, "") == 0);

    // check properties
    uint32_t captured_context_property_count = log_context_get_property_value_pair_count(expected_calls[0].log_sink1_log_call.captured_log_context);
    const LOG_CONTEXT_PROPERTY_VALUE_PAIR* captured_context_properties = log_context_get_property_value_pairs(expected_calls[0].log_sink1_log_call.captured_log_context);
    // we expect one extra property as we're "copying" the context by creating a parent one
    POOR_MANS_ASSERT(captured_context_property_count == 3);
    POOR_MANS_ASSERT(strcmp(captured_context_properties[1].name, "") == 0);
    POOR_MANS_ASSERT(captured_context_properties[1].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_struct);
    POOR_MANS_ASSERT(*(uint8_t*)captured_context_properties[1].value == 1);
    POOR_MANS_ASSERT(strcmp(captured_context_properties[2].name, "prop1") == 0);
    POOR_MANS_ASSERT(captured_context_properties[2].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_int8_t);
    POOR_MANS_ASSERT(*(int8_t*)captured_context_properties[2].value == 42);

    // 2nd sink
    POOR_MANS_ASSERT(expected_calls[1].log_sink2_log_call.captured_log_level == LOG_LEVEL_INFO);
    POOR_MANS_ASSERT(expected_calls[1].log_sink2_log_call.captured_log_context != NULL);
    POOR_MANS_ASSERT(strcmp(expected_calls[1].log_sink2_log_call.captured_file, __FILE__) == 0);
    POOR_MANS_ASSERT(strcmp(expected_calls[1].log_sink2_log_call.captured_func, __FUNCTION__) == 0);
    POOR_MANS_ASSERT(expected_calls[1].log_sink2_log_call.captured_line == expected_line_1);
    POOR_MANS_ASSERT(strcmp(expected_calls[1].log_sink2_log_call.captured_message, "") == 0);

    // check properties
    captured_context_property_count = log_context_get_property_value_pair_count(expected_calls[0].log_sink2_log_call.captured_log_context);
    captured_context_properties = log_context_get_property_value_pairs(expected_calls[0].log_sink2_log_call.captured_log_context);
    // we expect one extra property as we're "copying" the context by creating a parent one
    POOR_MANS_ASSERT(captured_context_property_count == 3);
    POOR_MANS_ASSERT(strcmp(captured_context_properties[1].name, "") == 0);
    POOR_MANS_ASSERT(captured_context_properties[1].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_struct);
    POOR_MANS_ASSERT(*(uint8_t*)captured_context_properties[1].value == 1);
    POOR_MANS_ASSERT(strcmp(captured_context_properties[2].name, "prop1") == 0);
    POOR_MANS_ASSERT(captured_context_properties[2].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_int8_t);
    POOR_MANS_ASSERT(*(int8_t*)captured_context_properties[2].value == 42);

    // second call

    // 1st sink
    POOR_MANS_ASSERT(expected_calls[2].log_sink1_log_call.captured_log_level == LOG_LEVEL_CRITICAL);
    POOR_MANS_ASSERT(expected_calls[2].log_sink1_log_call.captured_log_context != NULL);
    POOR_MANS_ASSERT(strcmp(expected_calls[2].log_sink1_log_call.captured_file, __FILE__) == 0);
    POOR_MANS_ASSERT(strcmp(expected_calls[2].log_sink1_log_call.captured_func, __FUNCTION__) == 0);
    POOR_MANS_ASSERT(expected_calls[2].log_sink1_log_call.captured_line == expected_line_2);
    POOR_MANS_ASSERT(strcmp(expected_calls[2].log_sink1_log_call.captured_message, "") == 0);

    // check properties
    log_context_get_property_value_pair_count(expected_calls[0].log_sink1_log_call.captured_log_context);
    captured_context_properties = log_context_get_property_value_pairs(expected_calls[0].log_sink1_log_call.captured_log_context);
    // we expect one extra property as we're "copying" the context by creating a parent one
    POOR_MANS_ASSERT(captured_context_property_count == 3);
    POOR_MANS_ASSERT(strcmp(captured_context_properties[1].name, "") == 0);
    POOR_MANS_ASSERT(captured_context_properties[1].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_struct);
    POOR_MANS_ASSERT(*(uint8_t*)captured_context_properties[1].value == 1);
    POOR_MANS_ASSERT(strcmp(captured_context_properties[2].name, "prop1") == 0);
    POOR_MANS_ASSERT(captured_context_properties[2].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_int8_t);
    POOR_MANS_ASSERT(*(int8_t*)captured_context_properties[2].value == 42);

    // 2nd sink
    POOR_MANS_ASSERT(expected_calls[3].log_sink2_log_call.captured_log_level == LOG_LEVEL_CRITICAL);
    POOR_MANS_ASSERT(expected_calls[3].log_sink2_log_call.captured_log_context != NULL);
    POOR_MANS_ASSERT(strcmp(expected_calls[3].log_sink2_log_call.captured_file, __FILE__) == 0);
    POOR_MANS_ASSERT(strcmp(expected_calls[3].log_sink2_log_call.captured_func, __FUNCTION__) == 0);
    POOR_MANS_ASSERT(expected_calls[3].log_sink2_log_call.captured_line == expected_line_2);
    POOR_MANS_ASSERT(strcmp(expected_calls[3].log_sink2_log_call.captured_message, "") == 0);

    // check properties
    captured_context_property_count = log_context_get_property_value_pair_count(expected_calls[0].log_sink2_log_call.captured_log_context);
    captured_context_properties = log_context_get_property_value_pairs(expected_calls[0].log_sink2_log_call.captured_log_context);
    // we expect one extra property as we're "copying" the context by creating a parent one
    POOR_MANS_ASSERT(captured_context_property_count == 3);
    POOR_MANS_ASSERT(strcmp(captured_context_properties[1].name, "") == 0);
    POOR_MANS_ASSERT(captured_context_properties[1].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_struct);
    POOR_MANS_ASSERT(*(uint8_t*)captured_context_properties[1].value == 1);
    POOR_MANS_ASSERT(strcmp(captured_context_properties[2].name, "prop1") == 0);
    POOR_MANS_ASSERT(captured_context_properties[2].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_int8_t);
    POOR_MANS_ASSERT(*(int8_t*)captured_context_properties[2].value == 42);

    //cleanup
    logger_deinit();
    cleanup_calls();
}

/* Tests_SRS_LOGGER_01_008: [ LOGGER_LOG_EX shall call the log function of every sink that is configured to be used. ]*/
/* Tests_SRS_LOGGER_01_010: [ Otherwise, LOGGER_LOG_EX shall construct a log context with all the properties specified in .... ] */
/* Tests_SRS_LOGGER_01_011: [ Each LOG_CONTEXT_STRING_PROPERTY and LOG_CONTEXT_PROPERTY entry in ... shall be added as a property in the context that is passed to log. ] */
static void LOGGER_LOG_EX_with_string_property_works(void)
{
    // arrange
    test_logger_init();

    setup_mocks();
    // 1st log line
    setup_log_sink1_log_call();
    setup_log_sink2_log_call();

    // act
    // capture the line no of the error
    int expected_line = __LINE__; LOGGER_LOG_EX(LOG_LEVEL_INFO, LOG_CONTEXT_STRING_PROPERTY(prop1, "gigi %s", "duru"));

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);

    // 1st sink
    POOR_MANS_ASSERT(expected_calls[0].log_sink1_log_call.captured_log_level == LOG_LEVEL_INFO);
    POOR_MANS_ASSERT(expected_calls[0].log_sink1_log_call.captured_log_context != NULL);
    POOR_MANS_ASSERT(strcmp(expected_calls[0].log_sink1_log_call.captured_file, __FILE__) == 0);
    POOR_MANS_ASSERT(strcmp(expected_calls[0].log_sink1_log_call.captured_func, __FUNCTION__) == 0);
    POOR_MANS_ASSERT(expected_calls[0].log_sink1_log_call.captured_line == expected_line);
    POOR_MANS_ASSERT(strcmp(expected_calls[0].log_sink1_log_call.captured_message, "") == 0);

    // check properties
    uint32_t captured_context_property_count = log_context_get_property_value_pair_count(expected_calls[0].log_sink1_log_call.captured_log_context);
    const LOG_CONTEXT_PROPERTY_VALUE_PAIR* captured_context_properties = log_context_get_property_value_pairs(expected_calls[0].log_sink1_log_call.captured_log_context);
    // we expect one extra property as we're "copying" the context by creating a parent one
    POOR_MANS_ASSERT(captured_context_property_count == 3);
    POOR_MANS_ASSERT(strcmp(captured_context_properties[1].name, "") == 0);
    POOR_MANS_ASSERT(captured_context_properties[1].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_struct);
    POOR_MANS_ASSERT(*(uint8_t*)captured_context_properties[1].value == 1);
    POOR_MANS_ASSERT(strcmp(captured_context_properties[2].name, "prop1") == 0);
    POOR_MANS_ASSERT(captured_context_properties[2].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_ascii_char_ptr);
    POOR_MANS_ASSERT(strcmp(captured_context_properties[2].value, "gigi duru") == 0);

    // 2nd sink
    POOR_MANS_ASSERT(expected_calls[1].log_sink2_log_call.captured_log_level == LOG_LEVEL_INFO);
    POOR_MANS_ASSERT(expected_calls[1].log_sink2_log_call.captured_log_context != NULL);
    POOR_MANS_ASSERT(strcmp(expected_calls[1].log_sink2_log_call.captured_file, __FILE__) == 0);
    POOR_MANS_ASSERT(strcmp(expected_calls[1].log_sink2_log_call.captured_func, __FUNCTION__) == 0);
    POOR_MANS_ASSERT(expected_calls[1].log_sink2_log_call.captured_line == expected_line);
    POOR_MANS_ASSERT(strcmp(expected_calls[1].log_sink2_log_call.captured_message, "") == 0);

    // check properties
    captured_context_property_count = log_context_get_property_value_pair_count(expected_calls[0].log_sink2_log_call.captured_log_context);
    captured_context_properties = log_context_get_property_value_pairs(expected_calls[0].log_sink2_log_call.captured_log_context);
    // we expect one extra property as we're "copying" the context by creating a parent one
    POOR_MANS_ASSERT(captured_context_property_count == 3);
    POOR_MANS_ASSERT(strcmp(captured_context_properties[1].name, "") == 0);
    POOR_MANS_ASSERT(captured_context_properties[1].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_struct);
    POOR_MANS_ASSERT(*(uint8_t*)captured_context_properties[1].value == 1);
    POOR_MANS_ASSERT(strcmp(captured_context_properties[2].name, "prop1") == 0);
    POOR_MANS_ASSERT(captured_context_properties[2].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_ascii_char_ptr);
    POOR_MANS_ASSERT(strcmp(captured_context_properties[2].value, "gigi duru") == 0);

    //cleanup
    logger_deinit();
    cleanup_calls();
}

/* Tests_SRS_LOGGER_01_009: [ If no properties are specified in ..., LOGGER_LOG_EX shall call log with log_context being NULL. ] */
static void LOGGER_LOG_EX_with_no_properties_works(void)
{
    // arrange
    test_logger_init();
    setup_mocks();
    setup_log_sink1_log_call();
    setup_log_sink2_log_call();

    // act
    // capture the line no of the error
    int expected_line = __LINE__; LOGGER_LOG_EX(LOG_LEVEL_INFO);

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);

    // 1st sink
    POOR_MANS_ASSERT(expected_calls[0].log_sink1_log_call.captured_log_level == LOG_LEVEL_INFO);
    POOR_MANS_ASSERT(expected_calls[0].log_sink1_log_call.captured_log_context == NULL);
    POOR_MANS_ASSERT(strcmp(expected_calls[0].log_sink1_log_call.captured_file, __FILE__) == 0);
    POOR_MANS_ASSERT(strcmp(expected_calls[0].log_sink1_log_call.captured_func, __FUNCTION__) == 0);
    POOR_MANS_ASSERT(expected_calls[0].log_sink1_log_call.captured_line == expected_line);
    POOR_MANS_ASSERT(strcmp(expected_calls[0].log_sink1_log_call.captured_message, "") == 0);

    // 2nd sink
    POOR_MANS_ASSERT(expected_calls[1].log_sink2_log_call.captured_log_level == LOG_LEVEL_INFO);
    POOR_MANS_ASSERT(expected_calls[1].log_sink2_log_call.captured_log_context == NULL);
    POOR_MANS_ASSERT(strcmp(expected_calls[1].log_sink2_log_call.captured_file, __FILE__) == 0);
    POOR_MANS_ASSERT(strcmp(expected_calls[1].log_sink2_log_call.captured_func, __FUNCTION__) == 0);
    POOR_MANS_ASSERT(expected_calls[1].log_sink2_log_call.captured_line == expected_line);
    POOR_MANS_ASSERT(strcmp(expected_calls[1].log_sink2_log_call.captured_message, "") == 0);

    //cleanup
    logger_deinit();
    cleanup_calls();
}

/* Tests_SRS_LOGGER_01_012: [ If LOG_CONTEXT_MESSAGE is specified in ..., message_format shall be passed to the log call together with a argument list made out of the ... portion of the LOG_CONTEXT_MESSAGE macro. ] */
static void LOGGER_LOG_EX_with_message_works(void)
{
    // arrange
    test_logger_init();
    setup_mocks();
    setup_log_sink1_log_call();
    setup_log_sink2_log_call();

    // act
    // capture the line no of the error
    int expected_line = __LINE__; LOGGER_LOG_EX(LOG_LEVEL_INFO, LOG_MESSAGE("baba %s %d", "cloantza", 42));

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);

    // 1st sink
    POOR_MANS_ASSERT(expected_calls[0].log_sink1_log_call.captured_log_level == LOG_LEVEL_INFO);
    POOR_MANS_ASSERT(expected_calls[0].log_sink1_log_call.captured_log_context == NULL);
    POOR_MANS_ASSERT(strcmp(expected_calls[0].log_sink1_log_call.captured_file, __FILE__) == 0);
    POOR_MANS_ASSERT(strcmp(expected_calls[0].log_sink1_log_call.captured_func, __FUNCTION__) == 0);
    POOR_MANS_ASSERT(expected_calls[0].log_sink1_log_call.captured_line == expected_line);
    POOR_MANS_ASSERT(strcmp(expected_calls[0].log_sink1_log_call.captured_message, "baba cloantza 42") == 0);

    // 2nd sink
    POOR_MANS_ASSERT(expected_calls[1].log_sink2_log_call.captured_log_level == LOG_LEVEL_INFO);
    POOR_MANS_ASSERT(expected_calls[1].log_sink2_log_call.captured_log_context == NULL);
    POOR_MANS_ASSERT(strcmp(expected_calls[1].log_sink2_log_call.captured_file, __FILE__) == 0);
    POOR_MANS_ASSERT(strcmp(expected_calls[1].log_sink2_log_call.captured_func, __FUNCTION__) == 0);
    POOR_MANS_ASSERT(expected_calls[1].log_sink2_log_call.captured_line == expected_line);
    POOR_MANS_ASSERT(strcmp(expected_calls[1].log_sink2_log_call.captured_message, "baba cloantza 42") == 0);

    //cleanup
    logger_deinit();
    cleanup_calls();
}

/* logger_deinit */

/* Tests_SRS_LOGGER_01_007: [ logger_deinit shall call the deinit function of every sink that is configured to be used. ] */
static void logger_deinit_deinitialized_all_sinks(void)
{
    // arrange
    test_logger_init();
    setup_mocks();
    setup_log_sink1_deinit_call();
    setup_log_sink2_deinit_call();

    // act
    logger_deinit();

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOGGER_01_006: [ If logger is not initialized, logger_deinit shall return. ] */
static void logger_deinit_after_deinit_with_only_one_init_returns(void)
{
    // arrange
    test_logger_init();
    logger_deinit();
    setup_mocks();

    // act
    logger_deinit();

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOGGER_01_021: [ Otherwise, logger_deinit shall decrement the initialization counter for the module. ] */
static void logger_deinit_when_initialized_twice_does_not_call_underlying_deinit(void)
{
    // arrange
    test_logger_init();
    POOR_MANS_ASSERT(logger_init() == 0);
    setup_mocks();

    // act
    logger_deinit();

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);

    // cleanup
    logger_deinit();
}

/* Tests_SRS_LOGGER_01_022: [ If the initilization counter reaches 0: ] */
/* Tests_SRS_LOGGER_01_007: [ logger_deinit shall call the deinit function of every sink that is configured to be used. ] */
static void logger_deinit_twice_after_2_inits_calls_deinit_on_underlying_modules(void)
{
    // arrange
    test_logger_init();
    POOR_MANS_ASSERT(logger_init() == 0);
    logger_deinit();
    setup_mocks();
    setup_log_sink1_deinit_call();
    setup_log_sink2_deinit_call();

    // act
    logger_deinit();

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

// logger_get_config

/* Tests_SRS_LOGGER_01_013: [ logger_get_config shall return a LOGGER_CONFIG structure with log_sink_count set to the current log sink count and log_sinks set to the array of log sink interfaces currently used. ] */
static void logger_get_config_returns_the_current_configuration(void)
{
    // arrange
    setup_mocks();

    // act
    LOGGER_CONFIG config = logger_get_config();

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(config.log_sink_count == 2);
    POOR_MANS_ASSERT(config.log_sinks[0] == &log_sink1);
    POOR_MANS_ASSERT(config.log_sinks[1] == &log_sink2);
}

// logger_set_config

/* Tests_SRS_LOGGER_01_014: [ logger_set_config set the current log sink count to new_config.log_sink_count and the array of log sink interfaces currently used to new_config.log_sinks. ] */
static void logger_set_config_sets_a_new_configuration_to_no_sinks(void)
{
    // arrange
    setup_mocks();

    logger_set_config((LOGGER_CONFIG) { .log_sinks = NULL, .log_sink_count = 0 });

    // act
    LOGGER_CONFIG config = logger_get_config();

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(config.log_sink_count == 0);
}

/* Tests_SRS_LOGGER_01_014: [ logger_set_config set the current log sink count to new_config.log_sink_count and the array of log sink interfaces currently used to new_config.log_sinks. ] */
static void logger_set_config_sets_a_new_configuration_to_1_sink(void)
{
    // arrange
    setup_mocks();

    static const LOG_SINK_IF* new_logger_config_sinks[] = { &log_sink2 };
    logger_set_config((LOGGER_CONFIG) { .log_sinks = new_logger_config_sinks, .log_sink_count = 1 });

    // act
    LOGGER_CONFIG config = logger_get_config();

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(config.log_sink_count == 1);
    POOR_MANS_ASSERT(config.log_sinks[0] == &log_sink2); // on purpose set to log_sink2
}

static void test_logger_log_with_config_on_sink_with_log_level(LOG_LEVEL log_level)
{
    // arrange
    test_logger_init();
    setup_mocks();
    setup_log_sink2_log_call();

    const LOG_SINK_IF* only_one_sink[] =
    {
        &log_sink2
    };

    const LOGGER_CONFIG custom_config =
    {
        .log_sinks = only_one_sink,
        .log_sink_count = 1
    };

    // act
    // capture the line no of the error
    int expected_line = __LINE__; LOGGER_LOG_WITH_CONFIG(custom_config, log_level, NULL, "gigi duru");

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);

    // only 2nd sink
    POOR_MANS_ASSERT(expected_calls[0].log_sink2_log_call.captured_log_level == log_level);
    POOR_MANS_ASSERT(expected_calls[0].log_sink2_log_call.captured_log_context == NULL);
    POOR_MANS_ASSERT(strcmp(expected_calls[0].log_sink2_log_call.captured_file, __FILE__) == 0);
    POOR_MANS_ASSERT(strcmp(expected_calls[0].log_sink2_log_call.captured_func, __FUNCTION__) == 0);
    POOR_MANS_ASSERT(expected_calls[0].log_sink2_log_call.captured_line == expected_line);
    POOR_MANS_ASSERT(strcmp(expected_calls[0].log_sink2_log_call.captured_message, "gigi duru") == 0);

    // cleanup
    logger_deinit();
    cleanup_calls();
}

/* Tests_SRS_LOGGER_01_016: [ Otherwise, LOGGER_LOG_WITH_CONFIG shall call the log function of every sink specified in logger_config. ] */
static void LOGGER_LOG_WITH_CONFIG_with_CRITICAL_works(void)
{
    test_logger_log_with_config_on_sink_with_log_level(LOG_LEVEL_CRITICAL);
}

/* Tests_SRS_LOGGER_01_016: [ Otherwise, LOGGER_LOG_WITH_CONFIG shall call the log function of every sink specified in logger_config. ] */
static void LOGGER_LOG_WITH_CONFIG_with_ERROR_works(void)
{
    test_logger_log_with_config_on_sink_with_log_level(LOG_LEVEL_ERROR);
}

/* Tests_SRS_LOGGER_01_016: [ Otherwise, LOGGER_LOG_WITH_CONFIG shall call the log function of every sink specified in logger_config. ] */
static void LOGGER_LOG_WITH_CONFIG_with_INFO_works(void)
{
    test_logger_log_with_config_on_sink_with_log_level(LOG_LEVEL_INFO);
}

/* Tests_SRS_LOGGER_01_016: [ Otherwise, LOGGER_LOG_WITH_CONFIG shall call the log function of every sink specified in logger_config. ] */
static void LOGGER_LOG_WITH_CONFIG_with_WARNING_works(void)
{
    test_logger_log_with_config_on_sink_with_log_level(LOG_LEVEL_WARNING);
}

/* Tests_SRS_LOGGER_01_016: [ Otherwise, LOGGER_LOG_WITH_CONFIG shall call the log function of every sink specified in logger_config. ] */
static void LOGGER_LOG_WITH_CONFIG_with_VERBOSE_works(void)
{
    test_logger_log_with_config_on_sink_with_log_level(LOG_LEVEL_VERBOSE);
}

/* Tests_SRS_LOGGER_01_015: [ If logger_config.log_sinks is NULL and logger_config.log_sink_count is greater than 0, LOGGER_LOG_WITH_CONFIG shall return. ] */
static void LOGGER_LOG_WITH_CONFIG_with_NULL_sinks_and_1_count_returns(void)
{
    // arrange
    test_logger_init();
    setup_mocks();

    // act
    // capture the line no of the error
    LOGGER_CONFIG custom_config = {
        .log_sinks = NULL, .log_sink_count = 1
    };
    LOGGER_LOG_WITH_CONFIG(custom_config, LOG_LEVEL_CRITICAL, NULL, "gigi duru");

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);

    // cleanup
    logger_deinit();
    cleanup_calls();
}

/* Tests_SRS_LOGGER_01_016: [ Otherwise, LOGGER_LOG_WITH_CONFIG shall call the log function of every sink specified in logger_config. ] */
static void LOGGER_LOG_WITH_CONFIG_with_1_sink_when_no_sinks_in_default_config(void)
{
    // arrange
    test_logger_init();
    setup_mocks();
    setup_log_sink2_log_call();

    const LOG_SINK_IF* only_one_sink[] =
    {
        &log_sink2
    };

    LOGGER_CONFIG old_config = logger_get_config();
    logger_set_config((LOGGER_CONFIG) { .log_sinks = NULL, .log_sink_count = 0 });

    // act
    // capture the line no of the error
    const LOGGER_CONFIG custom_config =
    {
        .log_sinks = only_one_sink,
        .log_sink_count = 1
    };
    LOGGER_LOG_WITH_CONFIG(custom_config, LOG_LEVEL_CRITICAL, NULL, "gigi duru");

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);

    logger_set_config(old_config);

    // cleanup
    logger_deinit();
    cleanup_calls();
}

/* This test does not compile. The thought of spinning the compiler as part of the test and checking that it compiles or not (a la cmake)
  crossed my mind, buuuut "other generations of developers" might try that */
/* Tests_SRS_LOGGER_01_023: [ LOGGER_LOG shall generate code that verifies at compile time that format and ... are suitable to be passed as arguments to printf. ] */
//static void test_format_printf_args(LOG_LEVEL log_level)
//{
//    // arrange
//    test_logger_init();
//    setup_mocks();
//    setup_log_sink1_log_call();
//    setup_log_sink2_log_call();
//
//    // act
//    // capture the line no of the error
//    int expected_line = __LINE__; LOGGER_LOG(log_level, NULL, "gigi duru %" PRIu32 "");
//
//    // assert
//    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
//    POOR_MANS_ASSERT(actual_and_expected_match);
//
//    // cleanup
//    logger_deinit();
//    cleanup_calls();
//}

/* This test does not compile. The thought of spinning the compiler as part of the test and checking that it compiles or not (a la cmake)
  crossed my mind, buuuut "other generations of developers" might try that */
/* Tests_SRS_LOGGER_01_024: [ LOGGER_LOG_WITH_CONFIG shall generate code that verifies at compile time that format and ... are suitable to be passed as arguments to printf. ] */
//static void test_format_printf_args_with_config(LOG_LEVEL log_level)
//{
//    // arrange
//    test_logger_init();
//    setup_mocks();
//    setup_log_sink1_log_call();
//    setup_log_sink2_log_call();
//
//    // act
//    // capture the line no of the error
//    int expected_line = __LINE__; LOGGER_LOG_WITH_CONFIG(custom_config, log_level, NULL, "gigi duru %" PRIu32 "");
//
//    // assert
//    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
//    POOR_MANS_ASSERT(actual_and_expected_match);
//
//    // cleanup
//    logger_deinit();
//    cleanup_calls();
//}

/* very "poor man's" way of testing, as no test harness and mocking framework are available */
int main(void)
{
#ifdef _MSC_VER
    // make abort not popup
    _set_abort_behavior(_CALL_REPORTFAULT, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
#endif

    logger_deinit_when_not_initialized_returns();

    when_the_1st_sink_init_fails_logger_init_fails();
    when_the_2nd_sink_init_fails_logger_init_fails();
    logger_init_initializes_sinks();
    logger_init_after_init_succeeds();
    
    LOGGER_LOG_with_CRITICAL_works();
    LOGGER_LOG_with_ERROR_works();
    LOGGER_LOG_with_INFO_works();
    LOGGER_LOG_with_WARNING_works();
    LOGGER_LOG_with_VERBOSE_works();
    LOGGER_LOG_with_non_NULL_context();

    LOGGER_LOG_EX_works();
    LOGGER_LOG_EX_works_with_2_properties();
    LOGGER_LOG_EX_twice_works();
    LOGGER_LOG_EX_with_no_properties_works();
    LOGGER_LOG_EX_with_string_property_works();

    LOGGER_LOG_EX_with_message_works();

    LOGGER_LOG_WITH_CONFIG_with_CRITICAL_works();
    LOGGER_LOG_WITH_CONFIG_with_ERROR_works();
    LOGGER_LOG_WITH_CONFIG_with_INFO_works();
    LOGGER_LOG_WITH_CONFIG_with_WARNING_works();
    LOGGER_LOG_WITH_CONFIG_with_VERBOSE_works();

    LOGGER_LOG_WITH_CONFIG_with_NULL_sinks_and_1_count_returns();
    LOGGER_LOG_WITH_CONFIG_with_1_sink_when_no_sinks_in_default_config();

    logger_deinit_deinitialized_all_sinks();
    logger_deinit_after_deinit_with_only_one_init_returns();
    logger_deinit_when_initialized_twice_does_not_call_underlying_deinit();
    logger_deinit_twice_after_2_inits_calls_deinit_on_underlying_modules();

    logger_get_config_returns_the_current_configuration();
    logger_set_config_sets_a_new_configuration_to_no_sinks();
    logger_set_config_sets_a_new_configuration_to_1_sink();

    return 0;
}
