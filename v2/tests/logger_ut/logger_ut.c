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
    MOCK_CALL_TYPE_printf, \
    MOCK_CALL_TYPE_log_context_get_property_value_pair_count, \
    MOCK_CALL_TYPE_log_context_get_property_value_pairs, \
    MOCK_CALL_TYPE_TraceLoggingRegister_EventRegister_EventSetInformation, \
    MOCK_CALL_TYPE__get_pgmptr, \
    MOCK_CALL_TYPE__tlgCreate1Sz_char, \
    MOCK_CALL_TYPE_EventDataDescCreate, \
    MOCK_CALL_TYPE__tlgWriteTransfer_EventWriteTransfer, \
    MOCK_CALL_TYPE_log_context_property_if_get_type, \
    MOCK_CALL_TYPE_vsnprintf, \
    MOCK_CALL_TYPE_TraceLoggingUnregister

MU_DEFINE_ENUM(MOCK_CALL_TYPE, MOCK_CALL_TYPE_VALUES)

MU_DEFINE_ENUM_STRINGS(LOG_CONTEXT_PROPERTY_TYPE, LOG_CONTEXT_PROPERTY_TYPE_VALUES)

// very poor mans mocks :-(

#define MAX_PRINTF_CAPTURED_OUPUT_SIZE (LOG_MAX_MESSAGE_LENGTH * 2)

//typedef struct vsnprintf_CALL_TAG
//{
//    bool override_result;
//    int call_result;
//    char captured_output[MAX_PRINTF_CAPTURED_OUPUT_SIZE];
//} vsnprintf_CALL;

//typedef struct MOCK_CALL_TAG
//{
//    MOCK_CALL_TYPE mock_call_type;
//    union
//    {
//        vsnprintf_CALL vsnprintf_call;
//    };
//} MOCK_CALL;

//static MOCK_CALL expected_calls[MAX_MOCK_CALL_COUNT];
//static size_t expected_call_count;
//static size_t actual_call_count;
//static bool actual_and_expected_match;

//static void setup_mocks(void)
//{
//    expected_call_count = 0;
//    actual_call_count = 0;
//    actual_and_expected_match = true;
//}

//int mock_vsnprintf(char* restrict s, size_t n, const char* restrict format, va_list args)
//{
//    int result;
//
//    if ((actual_call_count == expected_call_count) ||
//        (expected_calls[actual_call_count].mock_call_type != MOCK_CALL_TYPE_vsnprintf))
//    {
//        actual_and_expected_match = false;
//        result = -1;
//    }
//    else
//    {
//        if (expected_calls[actual_call_count].printf_call.override_result)
//        {
//            result = expected_calls[actual_call_count].printf_call.call_result;
//        }
//        else
//        {
//            // also capture the result in a variable for comparisons in tests
//            (void)vsnprintf(expected_calls[actual_call_count].printf_call.captured_output, sizeof(expected_calls[actual_call_count].printf_call.captured_output),
//                format, args);
//
//            // call "real" function
//            result = vsnprintf(s, n, format, args);
//        }
//
//        actual_call_count++;
//    }
//
//    return result;
//}

#define POOR_MANS_ASSERT(cond) \
    if (!(cond)) \
    { \
        (void)printf("%s:%d test failed\r\n", __FUNCTION__, __LINE__); \
        abort(); \
    } \

//static void setup_vsnprintf_call(void)
//{
//    expected_calls[expected_call_count].mock_call_type = MOCK_CALL_TYPE_vsnprintf;
//    expected_calls[expected_call_count].vsnprintf_call.override_result = false;
//    expected_call_count++;
//}

/* log_sink_etw.init */

/* Tests_SRS_LOG_SINK_ETW_01_088: [ If TraceLoggingRegister fails, log_sink_etw.init shall fail and return a non-zero value. ]*/
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

    LOGGER_LOG_with_ERROR_works();

    return 0;
}

