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
    MOCK_CALL_TYPE_abort

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

typedef struct MOCK_CALL_TAG
{
    MOCK_CALL_TYPE mock_call_type;
    union
    {
        log_sink1_init_CALL log_sink1_init_call;
        log_sink1_deinit_CALL log_sink1_deinit_call;
        log_sink1_log_CALL log_sink1_log_call;
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

// test config
static const LOG_SINK_IF* test_log_sinks[] =
{
    &log_sink1
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

static void setup_abort(void)
{
    expected_calls[expected_call_count].mock_call_type = MOCK_CALL_TYPE_abort;
    expected_call_count++;
}

/* LOGGER_LOG */

/* Tests_SRS_LOGGER_01_017: [ If logger is not initialized, LOGGER_LOG shall abort the program. ] */
static void LOGGER_LOG_when_not_initialized_aborts(void)
{
    // arrange
    setup_mocks();
    setup_abort();

    LOGGER_LOG(LOG_LEVEL_CRITICAL, NULL, "gigi duru");

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOGGER_01_018: [ If logger is not initialized, LOGGER_LOG shall abort the program. ] */
static void LOGGER_LOG_WITH_CONFIG_when_not_initialized_aborts(void)
{
    // arrange
    setup_mocks();
    setup_abort();

    LOGGER_CONFIG custom_config =
    {
        .log_sinks = NULL,
        .log_sink_count = 0
    };
    LOGGER_LOG_WITH_CONFIG(custom_config, LOG_LEVEL_CRITICAL, NULL, "gigi duru");

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

    LOGGER_LOG_when_not_initialized_aborts();
    LOGGER_LOG_WITH_CONFIG_when_not_initialized_aborts();

    return 0;
}
