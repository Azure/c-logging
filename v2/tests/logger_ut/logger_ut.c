// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>

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
    int dummy;
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
    int dummy;
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
        result = -1;
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
}

static void log_sink1_log(LOG_LEVEL log_level, LOG_CONTEXT_HANDLE log_context, const char* file, const char* func, int line, const char* message_format, ...)
{
    (void)log_level;
    (void)log_context;
    (void)file;
    (void)func;
    (void)line;
    (void)message_format;
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
        result = -1;
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
}

static void log_sink2_log(LOG_LEVEL log_level, LOG_CONTEXT_HANDLE log_context, const char* file, const char* func, int line, const char* message_format, ...)
{
    (void)log_level;
    (void)log_context;
    (void)file;
    (void)func;
    (void)line;
    (void)message_format;
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

#define POOR_MANS_ASSERT(cond) \
    if (!(cond)) \
    { \
        (void)printf("%s:%d test failed\r\n", __FUNCTION__, __LINE__); \
        abort(); \
    } \

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

/* logger_init */

/* Tests_SRS_LOGGER_01_003: [ logger_init shall call the init function of every sink that is configured to be used. ] */
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

static void LOGGER_LOG_with_ERROR_works(void)
{
    // arrange
    //setup_mocks();

    // act
    LOGGER_LOG(LOG_LEVEL_ERROR, NULL, "gigi duru");

    // assert
    //POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    //POOR_MANS_ASSERT(actual_and_expected_match);
}

/* very "poor man's" way of testing, as no test harness and mocking framework are available */
int main(void)
{
    // make abort not popup
    _set_abort_behavior(_CALL_REPORTFAULT, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);

    logger_init_initializes_sinks();

    return 0;
}

