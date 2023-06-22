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

#include "c_logging/xlogging.h"

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
}

void logger_perf_v1_run_tests(void)
{
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
}
