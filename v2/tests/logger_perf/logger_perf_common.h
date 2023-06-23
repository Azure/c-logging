// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef LOGGER_PERF_COMMON_H
#define LOGGER_PERF_COMMON_H

#include <stdint.h>
#include <stdio.h>

#include "windows.h"
#include "TraceLoggingProvider.h"
#include "evntrace.h"
#include "evntcons.h"

#include "macro_utils/macro_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

// The following dimensions are used to test various cases:
//
// LOGGER_TYPE:
// - LOGGER_TYPE_V1
// - LOGGER_TYPE_V1_LASTERROR
// - LOGGER_TYPE_V2_NO_CONTEXT
// - LOGGER_TYPE_V2_LOCAL_CONTEXT_CREATED_ONCE
// - LOGGER_TYPE_V2_DYNAMIC_CONTEXT_CREATED_ONCE
// - LOGGER_TYPE_V2_LOCAL_CONTEXT_CREATED_EVERY_TIME
// - LOGGER_TYPE_V2_DYNAMIC_CONTEXT_CREATED_EVERY_TIME
// - LOGGER_TYPE_V2_LOGGER_LOG_EX_ONLY_MESSAGE
// - LOGGER_TYPE_V2_LOGGER_LOG_EX_ONLY_PROPERTY
// - LOGGER_TYPE_V2_LASTERROR
// - LOGGER_TYPE_V2_HRESULT
//
// CONSUMER_ENABLE_TYPE:
// - CONSUMER_DISABLED
// - CONSUMER_ENABLED

#define LOGGER_TYPE_VALUES \
    LOGGER_TYPE_V1, \
    LOGGER_TYPE_V1_LASTERROR, \
    LOGGER_TYPE_V2_NO_CONTEXT, \
    LOGGER_TYPE_V2_LOCAL_CONTEXT_CREATED_ONCE, \
    LOGGER_TYPE_V2_DYNAMIC_CONTEXT_CREATED_ONCE, \
    LOGGER_TYPE_V2_LOCAL_CONTEXT_CREATED_EVERY_TIME, \
    LOGGER_TYPE_V2_DYNAMIC_CONTEXT_CREATED_EVERY_TIME, \
    LOGGER_TYPE_V2_LOGGER_LOG_EX_ONLY_MESSAGE, \
    LOGGER_TYPE_V2_LOGGER_LOG_EX_ONLY_PROPERTY, \
    LOGGER_TYPE_V2_LASTERROR, \
    LOGGER_TYPE_V2_HRESULT \

// The tests assume that there is no other consumer and no tracing session capturing the events
// except for the one that is started by this test
#define CONSUMER_ENABLE_TYPE_VALUES \
    CONSUMER_DISABLED, \
    CONSUMER_ENABLED

MU_DEFINE_ENUM_WITHOUT_INVALID(LOGGER_TYPE, LOGGER_TYPE_VALUES);

MU_DEFINE_ENUM_WITHOUT_INVALID(CONSUMER_ENABLE_TYPE, CONSUMER_ENABLE_TYPE_VALUES);

#define TEST_TIME 5000 // ms
#define ITERATION_COUNT 10000

typedef struct PERF_RESULT_TAG
{
    uint64_t log_count;
    double time;
} PERF_RESULT;

PERF_RESULT results[MU_ENUM_VALUE_COUNT_WITHOUT_INVALID(LOGGER_TYPE_VALUES)][MU_ENUM_VALUE_COUNT_WITHOUT_INVALID(CONSUMER_ENABLE_TYPE_VALUES)];

void print_results(LOGGER_TYPE logger_type, CONSUMER_ENABLE_TYPE consumer_enable_type, PERF_RESULT* perf_result);

typedef struct TEST_CONTEXT_TAG
{
    HANDLE process_thread_handle;
    char trace_session_name[128];
} TEST_CONTEXT;

#define POOR_MANS_ASSERT(cond) \
    if (!(cond)) \
    { \
        (void)printf("%s:%d test failed\r\n", __FUNCTION__, __LINE__); \
        abort(); \
    } \

TRACEHANDLE start_trace(TEST_CONTEXT* test_context, uint8_t trace_level);
void stop_trace(TEST_CONTEXT* test_context, TRACEHANDLE trace_session_handle);
void generate_trace_session(TEST_CONTEXT* test_context, const char* function_name);

TEST_CONTEXT* test_context_create(void);
void test_context_destroy(TEST_CONTEXT* test_context);

#ifdef __cplusplus
}
#endif

#endif // LOGGER_PERF_COMMON_H
