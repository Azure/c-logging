// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>

#ifdef _MSC_VER
#include "windows.h"
#endif

#include "macro_utils/macro_utils.h"

#include "c_logging/log_context_property_basic_types.h"
#include "c_logging/log_context_property_type_ascii_char_ptr.h"
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
    MOCK_CALL_TYPE_log_sink2_log \

MU_DEFINE_ENUM(MOCK_CALL_TYPE, MOCK_CALL_TYPE_VALUES)

MU_DEFINE_ENUM_STRINGS(LOG_CONTEXT_PROPERTY_TYPE, LOG_CONTEXT_PROPERTY_TYPE_VALUES)

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
        expected_calls[actual_call_count].log_sink1_log_call.captured_log_context = log_context;
        int snprintf_result = snprintf(expected_calls[actual_call_count].log_sink1_log_call.captured_file, sizeof(expected_calls[actual_call_count].log_sink1_log_call.captured_file), "%s", file);
        POOR_MANS_ASSERT((snprintf_result > 0) && (snprintf_result < sizeof(expected_calls[actual_call_count].log_sink1_log_call.captured_file)));
        snprintf_result = snprintf(expected_calls[actual_call_count].log_sink1_log_call.captured_func, sizeof(expected_calls[actual_call_count].log_sink1_log_call.captured_func), "%s", func);
        POOR_MANS_ASSERT((snprintf_result > 0) && (snprintf_result < sizeof(expected_calls[actual_call_count].log_sink1_log_call.captured_func)));
        expected_calls[actual_call_count].log_sink1_log_call.captured_line = line;
        snprintf_result = vsnprintf(expected_calls[actual_call_count].log_sink1_log_call.captured_message, sizeof(expected_calls[actual_call_count].log_sink1_log_call.captured_message), message_format, args);
        POOR_MANS_ASSERT((snprintf_result > 0) && (snprintf_result < sizeof(expected_calls[actual_call_count].log_sink1_log_call.captured_message)));

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
        expected_calls[actual_call_count].log_sink2_log_call.captured_log_context = log_context;
        int snprintf_result = snprintf(expected_calls[actual_call_count].log_sink2_log_call.captured_file, sizeof(expected_calls[actual_call_count].log_sink2_log_call.captured_file), "%s", file);
        POOR_MANS_ASSERT((snprintf_result > 0) && (snprintf_result < sizeof(expected_calls[actual_call_count].log_sink2_log_call.captured_file)));
        snprintf_result = snprintf(expected_calls[actual_call_count].log_sink2_log_call.captured_func, sizeof(expected_calls[actual_call_count].log_sink2_log_call.captured_func), "%s", func);
        POOR_MANS_ASSERT((snprintf_result > 0) && (snprintf_result < sizeof(expected_calls[actual_call_count].log_sink2_log_call.captured_func)));
        expected_calls[actual_call_count].log_sink2_log_call.captured_line = line;
        snprintf_result = vsnprintf(expected_calls[actual_call_count].log_sink2_log_call.captured_message, sizeof(expected_calls[actual_call_count].log_sink2_log_call.captured_message), message_format, args);
        POOR_MANS_ASSERT((snprintf_result > 0) && (snprintf_result < sizeof(expected_calls[actual_call_count].log_sink2_log_call.captured_message)));

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
const LOG_SINK_IF* log_sinks[] =
{
    &log_sink1,
    &log_sink2
};

const uint32_t log_sink_count = MU_COUNT_ARRAY_ITEMS(log_sinks);

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
}

/* Tests_SRS_LOGGER_01_002: [ If logger is already initialized, logger_init shall fail and return a non-zero value. ] */
static void logger_init_after_init_fails(void)
{
    // arrange
    setup_mocks();

    // act
    int result = logger_init();

    // assert
    POOR_MANS_ASSERT(result != 0);
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* LOGGER_LOG */

static void test_with_log_level(LOG_LEVEL log_level)
{
    // arrange
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
    POOR_MANS_ASSERT(expected_calls[0].log_sink2_log_call.captured_log_context == NULL);
    POOR_MANS_ASSERT(strcmp(expected_calls[1].log_sink2_log_call.captured_file, __FILE__) == 0);
    POOR_MANS_ASSERT(strcmp(expected_calls[1].log_sink2_log_call.captured_func, __FUNCTION__) == 0);
    POOR_MANS_ASSERT(expected_calls[1].log_sink2_log_call.captured_line == expected_line);
    POOR_MANS_ASSERT(strcmp(expected_calls[1].log_sink2_log_call.captured_message, "gigi duru") == 0);
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
    POOR_MANS_ASSERT(expected_calls[0].log_sink1_log_call.captured_log_context == log_context);
    POOR_MANS_ASSERT(strcmp(expected_calls[0].log_sink1_log_call.captured_file, __FILE__) == 0);
    POOR_MANS_ASSERT(strcmp(expected_calls[0].log_sink1_log_call.captured_func, __FUNCTION__) == 0);
    POOR_MANS_ASSERT(expected_calls[0].log_sink1_log_call.captured_line == expected_line);
    POOR_MANS_ASSERT(strcmp(expected_calls[0].log_sink1_log_call.captured_message, "u lala 42") == 0);

    // 2nd sink
    POOR_MANS_ASSERT(expected_calls[1].log_sink2_log_call.captured_log_level == LOG_LEVEL_ERROR);
    POOR_MANS_ASSERT(expected_calls[0].log_sink2_log_call.captured_log_context == log_context);
    POOR_MANS_ASSERT(strcmp(expected_calls[1].log_sink2_log_call.captured_file, __FILE__) == 0);
    POOR_MANS_ASSERT(strcmp(expected_calls[1].log_sink2_log_call.captured_func, __FUNCTION__) == 0);
    POOR_MANS_ASSERT(expected_calls[1].log_sink2_log_call.captured_line == expected_line);
    POOR_MANS_ASSERT(strcmp(expected_calls[1].log_sink2_log_call.captured_message, "u lala 42") == 0);

    LOG_CONTEXT_DESTROY(log_context);
}

/* logger_deinit */

/* Tests_SRS_LOGGER_01_007: [ logger_deinit shall call the deinit function of every sink that is configured to be used. ] */
static void logger_deinit_deinitialized_all_sinks(void)
{
    // arrange
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
static void logger_deinit_after_deinit_returns(void)
{
    // arrange
    setup_mocks();

    // act
    logger_deinit();

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

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
    logger_init_after_init_fails();

    LOGGER_LOG_with_ERROR_works();

    logger_deinit_deinitialized_all_sinks();
    logger_deinit_after_deinit_returns();

    return 0;
}
