// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <string.h>
#include <stdio.h>
#include <inttypes.h>

#include "windows.h"
#include "TraceLoggingProvider.h"
#include "evntrace.h"
#include "evntcons.h"

#include "macro_utils/macro_utils.h"

#include "timer.h"

#include "c_logging/logger.h"

#define TEST_TIME 5000 // ms

// The following dimensions are used to test various cases:
//
// LOGGER_TYPE:
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
MU_DEFINE_ENUM_STRINGS_WITHOUT_INVALID(LOGGER_TYPE, LOGGER_TYPE_VALUES);

MU_DEFINE_ENUM_WITHOUT_INVALID(CONSUMER_ENABLE_TYPE, CONSUMER_ENABLE_TYPE_VALUES);
MU_DEFINE_ENUM_STRINGS_WITHOUT_INVALID(CONSUMER_ENABLE_TYPE, CONSUMER_ENABLE_TYPE_VALUES);

typedef struct PERF_RESULT_TAG
{
    uint64_t log_count;
    double time;
} PERF_RESULT;

static PERF_RESULT results[MU_ENUM_VALUE_COUNT_WITHOUT_INVALID(LOGGER_TYPE_VALUES)][MU_ENUM_VALUE_COUNT_WITHOUT_INVALID(CONSUMER_ENABLE_TYPE_VALUES)];

static void print_results(LOGGER_TYPE logger_type, CONSUMER_ENABLE_TYPE consumer_enable_type, PERF_RESULT* perf_result)
{
    (void)printf("results[%" PRI_MU_ENUM "][%" PRI_MU_ENUM "]: %" PRIu64 " logs in %.00lf s (%" PRIu64 " us), %.00lf logs/s\r\n",
        MU_ENUM_VALUE(LOGGER_TYPE, logger_type),
        MU_ENUM_VALUE(CONSUMER_ENABLE_TYPE, consumer_enable_type),
        perf_result->log_count,
        perf_result->time / 1000000,
        (uint64_t)perf_result->time, (double)perf_result->log_count / (perf_result->time / 1000000));
}

#define ITERATION_COUNT 10000

static void test_all_with_consumer_enable_type(CONSUMER_ENABLE_TYPE consumer_enable_type)
{
    double start_time = timer_global_get_elapsed_us();
    uint64_t iterations = 0;
    LOGGER_TYPE logger_type;
    double current_time;

    // now have all the tests with the old v1 logger
    logger_type = LOGGER_TYPE_V1;
    (void)printf("Starting test with logger_type=%" PRI_MU_ENUM", consumer_enable_type=%" PRI_MU_ENUM "\r\n", MU_ENUM_VALUE(LOGGER_TYPE, logger_type), MU_ENUM_VALUE(CONSUMER_ENABLE_TYPE, consumer_enable_type));

    start_time = timer_global_get_elapsed_us();
    iterations = 0;

    while ((current_time = timer_global_get_elapsed_us()) - start_time < TEST_TIME * 1000)
    {
        uint32_t i;
        for (i = 0; i < ITERATION_COUNT; i++)
        {
            LogCritical("hello world %" PRI_MU_ENUM " i=%" PRIu32 "!", MU_ENUM_VALUE(LOGGER_TYPE, logger_type), i);
        }

        iterations += i;
    }

    results[logger_type][consumer_enable_type].log_count = iterations;
    results[logger_type][consumer_enable_type].time = current_time - start_time;

    print_results(logger_type, consumer_enable_type, &results[logger_type][consumer_enable_type]);

    // v1 with LastError
    logger_type = LOGGER_TYPE_V1_LASTERROR;
    (void)printf("Starting test with logger_type=%" PRI_MU_ENUM", consumer_enable_type=%" PRI_MU_ENUM "\r\n", MU_ENUM_VALUE(LOGGER_TYPE, logger_type), MU_ENUM_VALUE(CONSUMER_ENABLE_TYPE, consumer_enable_type));

    start_time = timer_global_get_elapsed_us();
    iterations = 0;

    while ((current_time = timer_global_get_elapsed_us()) - start_time < TEST_TIME * 1000)
    {
        uint32_t i;
        for (i = 0; i < ITERATION_COUNT; i++)
        {
            LogLastError("hello world %" PRI_MU_ENUM " i=%" PRIu32 "!", MU_ENUM_VALUE(LOGGER_TYPE, logger_type), i);
        }

        iterations += i;
    }

    results[logger_type][consumer_enable_type].log_count = iterations;
    results[logger_type][consumer_enable_type].time = current_time - start_time;

    print_results(logger_type, consumer_enable_type, &results[logger_type][consumer_enable_type]);

    /* v2 no context */
    logger_type = LOGGER_TYPE_V2_NO_CONTEXT;
    (void)printf("Starting test with logger_type=%" PRI_MU_ENUM", consumer_enable_type=%" PRI_MU_ENUM "\r\n", MU_ENUM_VALUE(LOGGER_TYPE, logger_type), MU_ENUM_VALUE(CONSUMER_ENABLE_TYPE, consumer_enable_type));

    start_time = timer_global_get_elapsed_us();
    iterations = 0;

    while ((current_time = timer_global_get_elapsed_us()) - start_time < TEST_TIME * 1000)
    {
        uint32_t i;
        for (i = 0; i < ITERATION_COUNT; i++)
        {
            LOGGER_LOG(LOG_LEVEL_CRITICAL, NULL, "hello world %" PRI_MU_ENUM " i=%" PRIu32 "!", MU_ENUM_VALUE(LOGGER_TYPE, logger_type), i);
        }

        iterations += i;
    }

    results[logger_type][consumer_enable_type].log_count = iterations;
    results[logger_type][consumer_enable_type].time = current_time - start_time;

    print_results(logger_type, consumer_enable_type, &results[logger_type][consumer_enable_type]);

    // now have the test run with local context created once
    logger_type = LOGGER_TYPE_V2_LOCAL_CONTEXT_CREATED_ONCE;
    (void)printf("Starting test with logger_type=%" PRI_MU_ENUM", consumer_enable_type=%" PRI_MU_ENUM "\r\n", MU_ENUM_VALUE(LOGGER_TYPE, logger_type), MU_ENUM_VALUE(CONSUMER_ENABLE_TYPE, consumer_enable_type));

    start_time = timer_global_get_elapsed_us();
    iterations = 0;

    LOG_CONTEXT_LOCAL_DEFINE(log_context, NULL, LOG_CONTEXT_STRING_PROPERTY(block_id, "%s", "pachoo"));

    current_time;
    while ((current_time = timer_global_get_elapsed_us()) - start_time < TEST_TIME * 1000)
    {
        uint32_t i;
        for (i = 0; i < ITERATION_COUNT; i++)
        {
            LOGGER_LOG(LOG_LEVEL_CRITICAL, &log_context, "hello world %" PRI_MU_ENUM " i=%" PRIu32 "!", MU_ENUM_VALUE(LOGGER_TYPE, logger_type), i);
        }

        iterations += i;
    }

    results[logger_type][consumer_enable_type].log_count = iterations;
    results[logger_type][consumer_enable_type].time = current_time - start_time;

    print_results(logger_type, consumer_enable_type, &results[logger_type][consumer_enable_type]);

    // now have the test run with dynamic context created once
    logger_type = LOGGER_TYPE_V2_DYNAMIC_CONTEXT_CREATED_ONCE;
    (void)printf("Starting test with logger_type=%" PRI_MU_ENUM", consumer_enable_type=%" PRI_MU_ENUM "\r\n", MU_ENUM_VALUE(LOGGER_TYPE, logger_type), MU_ENUM_VALUE(CONSUMER_ENABLE_TYPE, consumer_enable_type));

    start_time = timer_global_get_elapsed_us();
    iterations = 0;

    LOG_CONTEXT_HANDLE dynamic_log_context;
    LOG_CONTEXT_CREATE(dynamic_log_context, NULL, LOG_CONTEXT_NAME(dynamic_log_context), LOG_CONTEXT_STRING_PROPERTY(block_id, "%s", "pachoo"));

    current_time;
    while ((current_time = timer_global_get_elapsed_us()) - start_time < TEST_TIME * 1000)
    {
        uint32_t i;
        for (i = 0; i < ITERATION_COUNT; i++)
        {
            LOGGER_LOG(LOG_LEVEL_CRITICAL, dynamic_log_context, "hello world %" PRI_MU_ENUM " i=%" PRIu32 "!", MU_ENUM_VALUE(LOGGER_TYPE, logger_type), i);
        }

        iterations += i;
    }

    LOG_CONTEXT_DESTROY(dynamic_log_context);

    results[logger_type][consumer_enable_type].log_count = iterations;
    results[logger_type][consumer_enable_type].time = current_time - start_time;

    print_results(logger_type, consumer_enable_type, &results[logger_type][consumer_enable_type]);

    // now have the test run with local context created every time
    logger_type = LOGGER_TYPE_V2_LOCAL_CONTEXT_CREATED_EVERY_TIME;
    (void)printf("Starting test with logger_type=%" PRI_MU_ENUM", consumer_enable_type=%" PRI_MU_ENUM "\r\n", MU_ENUM_VALUE(LOGGER_TYPE, logger_type), MU_ENUM_VALUE(CONSUMER_ENABLE_TYPE, consumer_enable_type));

    start_time = timer_global_get_elapsed_us();
    iterations = 0;

    current_time;
    while ((current_time = timer_global_get_elapsed_us()) - start_time < TEST_TIME * 1000)
    {
        uint32_t i;
        for (i = 0; i < ITERATION_COUNT; i++)
        {
            LOG_CONTEXT_LOCAL_DEFINE(log_context_2, NULL, LOG_CONTEXT_STRING_PROPERTY(block_id, "%s", "pachoo"));

            LOGGER_LOG(LOG_LEVEL_CRITICAL, &log_context_2, "hello world %" PRI_MU_ENUM " i=%" PRIu32 "!", MU_ENUM_VALUE(LOGGER_TYPE, logger_type), i);
        }

        iterations += i;
    }

    results[logger_type][consumer_enable_type].log_count = iterations;
    results[logger_type][consumer_enable_type].time = current_time - start_time;

    print_results(logger_type, consumer_enable_type, &results[logger_type][consumer_enable_type]);

    // now have the test run with dynamic context created every time
    logger_type = LOGGER_TYPE_V2_DYNAMIC_CONTEXT_CREATED_EVERY_TIME;
    (void)printf("Starting test with logger_type=%" PRI_MU_ENUM", consumer_enable_type=%" PRI_MU_ENUM "\r\n", MU_ENUM_VALUE(LOGGER_TYPE, logger_type), MU_ENUM_VALUE(CONSUMER_ENABLE_TYPE, consumer_enable_type));

    start_time = timer_global_get_elapsed_us();
    iterations = 0;

    current_time;
    while ((current_time = timer_global_get_elapsed_us()) - start_time < TEST_TIME * 1000)
    {
        uint32_t i;
        for (i = 0; i < ITERATION_COUNT; i++)
        {
            LOG_CONTEXT_HANDLE dynamic_log_context_x;
            LOG_CONTEXT_CREATE(dynamic_log_context_x, NULL, LOG_CONTEXT_NAME(dynamic_log_context_x), LOG_CONTEXT_STRING_PROPERTY(block_id, "%s", "pachoo"));

            LOGGER_LOG(LOG_LEVEL_CRITICAL, dynamic_log_context_x, "hello world %" PRI_MU_ENUM " i=%" PRIu32 "!", MU_ENUM_VALUE(LOGGER_TYPE, logger_type), i);

            LOG_CONTEXT_DESTROY(dynamic_log_context_x);
        }

        iterations += i;
    }

    results[logger_type][consumer_enable_type].log_count = iterations;
    results[logger_type][consumer_enable_type].time = current_time - start_time;

    print_results(logger_type, consumer_enable_type, &results[logger_type][consumer_enable_type]);

    // now have the test run with LOGGER_LOG_EX with a message only
    logger_type = LOGGER_TYPE_V2_LOGGER_LOG_EX_ONLY_MESSAGE;
    (void)printf("Starting test with logger_type=%" PRI_MU_ENUM", consumer_enable_type=%" PRI_MU_ENUM "\r\n", MU_ENUM_VALUE(LOGGER_TYPE, logger_type), MU_ENUM_VALUE(CONSUMER_ENABLE_TYPE, consumer_enable_type));

    start_time = timer_global_get_elapsed_us();
    iterations = 0;

    current_time;
    while ((current_time = timer_global_get_elapsed_us()) - start_time < TEST_TIME * 1000)
    {
        uint32_t i;
        for (i = 0; i < ITERATION_COUNT; i++)
        {
            LOG_CONTEXT_HANDLE dynamic_log_context_x;
            LOG_CONTEXT_CREATE(dynamic_log_context_x, NULL, LOG_CONTEXT_NAME(dynamic_log_context_x), LOG_CONTEXT_STRING_PROPERTY(block_id, "%s", "pachoo"));

            LOGGER_LOG_EX(LOG_LEVEL_CRITICAL, LOG_MESSAGE("hello world %" PRI_MU_ENUM " i=%" PRIu32 "!", MU_ENUM_VALUE(LOGGER_TYPE, logger_type), i));

            LOG_CONTEXT_DESTROY(dynamic_log_context_x);
        }

        iterations += i;
    }

    results[logger_type][consumer_enable_type].log_count = iterations;
    results[logger_type][consumer_enable_type].time = current_time - start_time;

    print_results(logger_type, consumer_enable_type, &results[logger_type][consumer_enable_type]);

    // now have the test run with LOGGER_LOG_EX with properties only
    logger_type = LOGGER_TYPE_V2_LOGGER_LOG_EX_ONLY_PROPERTY;
    (void)printf("Starting test with logger_type=%" PRI_MU_ENUM", consumer_enable_type=%" PRI_MU_ENUM "\r\n", MU_ENUM_VALUE(LOGGER_TYPE, logger_type), MU_ENUM_VALUE(CONSUMER_ENABLE_TYPE, consumer_enable_type));

    start_time = timer_global_get_elapsed_us();
    iterations = 0;

    current_time;
    while ((current_time = timer_global_get_elapsed_us()) - start_time < TEST_TIME * 1000)
    {
        uint32_t i;
        for (i = 0; i < ITERATION_COUNT; i++)
        {
            LOG_CONTEXT_HANDLE dynamic_log_context_x;
            LOG_CONTEXT_CREATE(dynamic_log_context_x, NULL, LOG_CONTEXT_NAME(dynamic_log_context_x), LOG_CONTEXT_STRING_PROPERTY(block_id, "%s", "pachoo"));

            LOGGER_LOG_EX(LOG_LEVEL_CRITICAL, LOG_CONTEXT_PROPERTY(uint32_t, i, i));

            LOG_CONTEXT_DESTROY(dynamic_log_context_x);
        }

        iterations += i;
    }

    results[logger_type][consumer_enable_type].log_count = iterations;
    results[logger_type][consumer_enable_type].time = current_time - start_time;

    print_results(logger_type, consumer_enable_type, &results[logger_type][consumer_enable_type]);

    // now have the test run with v2 LastError property
    logger_type = LOGGER_TYPE_V2_LASTERROR;
    (void)printf("Starting test with logger_type=%" PRI_MU_ENUM", consumer_enable_type=%" PRI_MU_ENUM "\r\n", MU_ENUM_VALUE(LOGGER_TYPE, logger_type), MU_ENUM_VALUE(CONSUMER_ENABLE_TYPE, consumer_enable_type));

    start_time = timer_global_get_elapsed_us();
    iterations = 0;

    current_time;
    while ((current_time = timer_global_get_elapsed_us()) - start_time < TEST_TIME * 1000)
    {
        uint32_t i;
        for (i = 0; i < ITERATION_COUNT; i++)
        {
            LOGGER_LOG_EX(LOG_LEVEL_CRITICAL, LOG_LASTERROR(), LOG_MESSAGE("hello world %" PRI_MU_ENUM " i=%" PRIu32 "!", MU_ENUM_VALUE(LOGGER_TYPE, logger_type), i));
        }

        iterations += i;
    }

    results[logger_type][consumer_enable_type].log_count = iterations;
    results[logger_type][consumer_enable_type].time = current_time - start_time;

    print_results(logger_type, consumer_enable_type, &results[logger_type][consumer_enable_type]);

    // now have the test run with v2 HRESULT property
    logger_type = LOGGER_TYPE_V2_HRESULT;
    (void)printf("Starting test with logger_type=%" PRI_MU_ENUM", consumer_enable_type=%" PRI_MU_ENUM "\r\n", MU_ENUM_VALUE(LOGGER_TYPE, logger_type), MU_ENUM_VALUE(CONSUMER_ENABLE_TYPE, consumer_enable_type));

    start_time = timer_global_get_elapsed_us();
    iterations = 0;

    current_time;
    while ((current_time = timer_global_get_elapsed_us()) - start_time < TEST_TIME * 1000)
    {
        uint32_t i;
        for (i = 0; i < ITERATION_COUNT; i++)
        {
            LOGGER_LOG_EX(LOG_LEVEL_CRITICAL, LOG_HRESULT(E_POINTER), LOG_MESSAGE("hello world %" PRI_MU_ENUM " i=%" PRIu32 "!", MU_ENUM_VALUE(LOGGER_TYPE, logger_type), i));
        }

        iterations += i;
    }

    results[logger_type][consumer_enable_type].log_count = iterations;
    results[logger_type][consumer_enable_type].time = current_time - start_time;

    print_results(logger_type, consumer_enable_type, &results[logger_type][consumer_enable_type]);
}

static GUID provider_guid = { 0xDAD29F36, 0x0A48, 0x4DEF, { 0x9D, 0x50, 0x8E, 0xF9, 0x03, 0x6B, 0x92, 0xB4 } };

typedef struct TEST_CONTEXT_TAG
{
    HANDLE process_thread_handle;
    char trace_session_name[128];
} TEST_CONTEXT;

typedef struct EVENT_TRACE_PROPERTY_DATA_TAG
{
    EVENT_TRACE_PROPERTIES props;
    char logger_name[128];
} EVENT_TRACE_PROPERTY_DATA;

#define POOR_MANS_ASSERT(cond) \
    if (!(cond)) \
    { \
        (void)printf("%s:%d test failed\r\n", __FUNCTION__, __LINE__); \
        abort(); \
    } \

static TEST_CONTEXT* test_context_create(void)
{
    TEST_CONTEXT* result = malloc(sizeof(TEST_CONTEXT));
    POOR_MANS_ASSERT(result != NULL);

    return result;
}

static void generate_trace_session(TEST_CONTEXT* test_context, const char* function_name)
{
    UUID uuid;
    POOR_MANS_ASSERT(UuidCreate(&uuid) == RPC_S_OK);
    int snprintf_result = snprintf(test_context->trace_session_name, sizeof(test_context->trace_session_name), "%s-%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
        function_name, uuid.Data1, uuid.Data2, uuid.Data3, uuid.Data4[0], uuid.Data4[1], uuid.Data4[2], uuid.Data4[3], uuid.Data4[4], uuid.Data4[5], uuid.Data4[6], uuid.Data4[7]);
    POOR_MANS_ASSERT((snprintf_result >= 0) && (snprintf_result < sizeof(test_context->trace_session_name)));
}

static TRACEHANDLE start_trace(TEST_CONTEXT* test_context, uint8_t trace_level)
{
    // setup the etw consumer
    TRACEHANDLE trace_session_handle;

    (void)printf("Starting trace session %s\r\n", test_context->trace_session_name);

    EVENT_TRACE_PROPERTY_DATA start_event_trace_property_data = { 0 };

    start_event_trace_property_data.props.Wnode.BufferSize = sizeof(start_event_trace_property_data);
    start_event_trace_property_data.props.Wnode.Flags = WNODE_FLAG_TRACED_GUID;
    start_event_trace_property_data.props.LogFileMode = EVENT_TRACE_REAL_TIME_MODE;
    start_event_trace_property_data.props.MinimumBuffers = 4;
    start_event_trace_property_data.props.MaximumBuffers = 4;
    start_event_trace_property_data.props.BufferSize = 64 * 1024;
    start_event_trace_property_data.props.FlushTimer = 1;
    start_event_trace_property_data.props.LogFileNameOffset = 0;
    start_event_trace_property_data.props.LoggerNameOffset = offsetof(EVENT_TRACE_PROPERTY_DATA, logger_name);

    POOR_MANS_ASSERT(StartTraceA(&trace_session_handle, test_context->trace_session_name, &start_event_trace_property_data.props) == ERROR_SUCCESS);

    // enable our desired provider
    ULONG enable_trace_result = EnableTraceEx2(trace_session_handle, (LPCGUID)&provider_guid, EVENT_CONTROL_CODE_ENABLE_PROVIDER,
        trace_level, 0, 0, 0, NULL
    );
    POOR_MANS_ASSERT(enable_trace_result == ERROR_SUCCESS);

    return trace_session_handle;
}

static void stop_trace(TEST_CONTEXT* test_context, TRACEHANDLE trace_session_handle)
{
    EVENT_TRACE_PROPERTY_DATA stop_event_trace_property_data = { 0 };

    stop_event_trace_property_data.props.Wnode.BufferSize = sizeof(stop_event_trace_property_data);
    stop_event_trace_property_data.props.Wnode.Flags = WNODE_FLAG_TRACED_GUID;
    stop_event_trace_property_data.props.LoggerNameOffset = offsetof(EVENT_TRACE_PROPERTY_DATA, logger_name);
    stop_event_trace_property_data.props.LogFileNameOffset = 0;

    (void)printf("Stopping trace session %s\r\n", test_context->trace_session_name);
    POOR_MANS_ASSERT(StopTraceA(trace_session_handle, NULL, &stop_event_trace_property_data.props) == ERROR_SUCCESS);
}

void logger_perf_v2_run_tests(void)
{
    POOR_MANS_ASSERT(logger_init() == 0);

    // run once without any ETW consumer
    test_all_with_consumer_enable_type(CONSUMER_DISABLED);

    TEST_CONTEXT* test_context = test_context_create();

    generate_trace_session(test_context, __FUNCTION__);

    TRACEHANDLE trace_session_handle = start_trace(test_context, TRACE_LEVEL_VERBOSE);

    // run once with an ETW consumer
    test_all_with_consumer_enable_type(CONSUMER_ENABLED);

    stop_trace(test_context, trace_session_handle);

    (void)printf("Test summary:\r\n");
    (void)printf("=============\r\n");

    for (int logger_type = 0; logger_type < MU_ENUM_VALUE_COUNT_WITHOUT_INVALID(LOGGER_TYPE_VALUES); logger_type++)
    {
        for (int consumer_enable_type = 0; consumer_enable_type < MU_ENUM_VALUE_COUNT_WITHOUT_INVALID(CONSUMER_ENABLE_TYPE_VALUES); consumer_enable_type++)
        {
            print_results(logger_type, consumer_enable_type, &results[logger_type][consumer_enable_type]);
        }
    }

    logger_deinit();
}
