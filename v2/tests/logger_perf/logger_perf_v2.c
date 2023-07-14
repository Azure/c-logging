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
#include "logger_perf_common.h"

#include "c_logging/logger.h"

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

    test_context_destroy(test_context);

    logger_deinit();
}
