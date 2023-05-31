// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <string.h>
#include <stdio.h>
#include <inttypes.h>

#include "macro_utils/macro_utils.h"

#include "timer.h"

#include "c_logging/logger.h"
#include "c_logging/xlogging.h"

#define TEST_TIME 5000 // ms

// The following dimensions are used to test various cases:
//
// LOGGER_TYPE:
// - LOGGER_TYPE_V1
// - LOGGER_TYPE_V2_LOCAL_CONTEXT_CREATED_ONCE
// - LOGGER_TYPE_V2_DYNAMIC_CONTEXT_CREATED_ONCE
// - LOGGER_TYPE_V2_LOCAL_CONTEXT_CREATED_EVERY_TIME
// - LOGGER_TYPE_V2_DYNAMIC_CONTEXT_CREATED_EVERY_TIME
//
// CONSUMER_ENABLE_TYPE:
// - CONSUMER_DISABLED
// - CONSUMER_ENABLED

#define LOGGER_TYPE_VALUES \
    LOGGER_TYPE_V1, \
    LOGGER_TYPE_V2_LOCAL_CONTEXT_CREATED_ONCE, \
    LOGGER_TYPE_V2_DYNAMIC_CONTEXT_CREATED_ONCE, \
    LOGGER_TYPE_V2_LOCAL_CONTEXT_CREATED_EVERY_TIME, \
    LOGGER_TYPE_V2_DYNAMIC_CONTEXT_CREATED_EVERY_TIME

#define CONSUMER_ENABLE_TYPE_VALUES \
    CONSUMER_DISABLED, \
    CONSUMER_ENABLED

MU_DEFINE_ENUM_WITHOUT_INVALID(LOGGER_TYPE, LOGGER_TYPE_VALUES_VALUES);
MU_DEFINE_ENUM_STRINGS_WITHOUT_INVALID(LOGGER_TYPE, LOGGER_TYPE_VALUES_VALUES_VALUES);

MU_DEFINE_ENUM_WITHOUT_INVALID(CONSUMER_ENABLE_TYPE, CONSUMER_ENABLE_TYPE_VALUES);
MU_DEFINE_ENUM_STRINGS_WITHOUT_INVALID(CONSUMER_ENABLE_TYPE, CONSUMER_ENABLE_TYPE_VALUES);

PERF_RESULT v1_results[MU_ENUM_VALUE_COUNT_WITHOUT_INVALID(PERF_TEST_LIST)];
PERF_RESULT v2_results[MU_ENUM_VALUE_COUNT_WITHOUT_INVALID(PERF_TEST_LIST)];
PERF_RESULT v2_results_local_context_once[MU_ENUM_VALUE_COUNT_WITHOUT_INVALID(PERF_TEST_LIST)];
PERF_RESULT v2_results_dynamic_context_once[MU_ENUM_VALUE_COUNT_WITHOUT_INVALID(PERF_TEST_LIST)];
PERF_RESULT v2_results_local_context_every_time[MU_ENUM_VALUE_COUNT_WITHOUT_INVALID(PERF_TEST_LIST)];
PERF_RESULT v2_results_dynamic_context_every_time[MU_ENUM_VALUE_COUNT_WITHOUT_INVALID(PERF_TEST_LIST)];

int main(void)
{
    double start_time = timer_global_get_elapsed_us();
    uint64_t iterations = 0;

    // now have all the tests with the old v1 logger

    start_time = timer_global_get_elapsed_us();
    iterations = 0;

    /*LogCritical (old way)*/
    double current_time;
    while ((current_time = timer_global_get_elapsed_us()) - start_time < TEST_TIME * 1000)
    {
        uint32_t i;
        for (i = 0; i < 10000; i++)
        {
            LogCritical("LogCritical: hello world!");
        }

        iterations += i;
    }

    v1_results[PERF_TEST_LOG_CRITICAL].log_count = iterations;
    v1_results[PERF_TEST_LOG_CRITICAL].time = current_time - start_time;

    /*LogCritical*/
    while ((current_time = timer_global_get_elapsed_us()) - start_time < TEST_TIME * 1000)
    {
        uint32_t i;
        for (i = 0; i < 10000; i++)
        {
            LOGGER_LOG(LOG_LEVEL_CRITICAL, NULL, "LogCritical: hello world!");
        }

        iterations += i;
    }

    v2_results[PERF_TEST_LOG_CRITICAL].log_count = iterations;
    v2_results[PERF_TEST_LOG_CRITICAL].time = current_time - start_time;

    // now have the test run with local context created once
    start_time = timer_global_get_elapsed_us();
    iterations = 0;
    
    LOG_CONTEXT_LOCAL_DEFINE(log_context, NULL, LOG_CONTEXT_STRING_PROPERTY("block_id", "%s", "pachoo"));
    
    current_time;
    while ((current_time = timer_global_get_elapsed_us()) - start_time < TEST_TIME * 1000)
    {
        uint32_t i;
        for (i = 0; i < 10000; i++)
        {
            logger_log(LOG_LEVEL_CRITICAL, &log_context, "LogCritical: hello world!");
        }
    
        iterations += i;
    }
    
    v2_results_local_context_once[PERF_TEST_LOG_CRITICAL].log_count = iterations;
    v2_results_local_context_once[PERF_TEST_LOG_CRITICAL].time = current_time - start_time;

    // now have the test run with dynamic context created once
    start_time = timer_global_get_elapsed_us();
    iterations = 0;

    LOG_CONTEXT_HANDLE dynamic_log_context;
    LOG_CONTEXT_CREATE(dynamic_log_context, NULL, LOG_CONTEXT_NAME(dynamic_log_context), LOG_CONTEXT_STRING_PROPERTY("block_id", "%s", "pachoo"));

    current_time;
    while ((current_time = timer_global_get_elapsed_us()) - start_time < TEST_TIME * 1000)
    {
        uint32_t i;
        for (i = 0; i < 10000; i++)
        {
            LOGGER_LOG(LOG_LEVEL_CRITICAL, dynamic_log_context, "LogCritical: hello world!");
        }

        iterations += i;
    }

    LOG_CONTEXT_DESTROY(dynamic_log_context);

    v2_results_dynamic_context_once[PERF_TEST_LOG_CRITICAL].log_count = iterations;
    v2_results_dynamic_context_once[PERF_TEST_LOG_CRITICAL].time = current_time - start_time;

    // now have the test run with local context created every time
    //start_time = timer_global_get_elapsed_us();
    //iterations = 0;
    //
    //current_time;
    //while ((current_time = timer_global_get_elapsed_us()) - start_time < TEST_TIME * 1000)
    //{
    //    uint32_t i;
    //    for (i = 0; i < 10000; i++)
    //    {
    //        LOG_CONTEXT_LOCAL_DEFINE(log_context_2, NULL, LOG_CONTEXT_STRING_PROPERTY("block_id", "%s", "pachoo"));
    //
    //        logger_log(LOG_LEVEL_CRITICAL, &log_context_2, "LogCritical: hello world!");
    //    }
    //
    //    iterations += i;
    //}
    //
    //v2_results_local_context_every_time[PERF_TEST_LOG_CRITICAL].log_count = iterations;
    //v2_results_local_context_every_time[PERF_TEST_LOG_CRITICAL].time = current_time - start_time;

    // now have the test run with dynamic context created every time
    start_time = timer_global_get_elapsed_us();
    iterations = 0;

    current_time;
    while ((current_time = timer_global_get_elapsed_us()) - start_time < TEST_TIME * 1000)
    {
        uint32_t i;
        for (i = 0; i < 10000; i++)
        {
            LOG_CONTEXT_HANDLE dynamic_log_context_x;
            LOG_CONTEXT_CREATE(dynamic_log_context_x, NULL, LOG_CONTEXT_NAME(dynamic_log_context_x), LOG_CONTEXT_STRING_PROPERTY("block_id", "%s", "pachoo"));

            LOGGER_LOG(LOG_LEVEL_CRITICAL, dynamic_log_context_x, "LogCritical: hello world!");

            LOG_CONTEXT_DESTROY(dynamic_log_context_x);
        }

        iterations += i;
    }

    v2_results_local_context_every_time[PERF_TEST_LOG_CRITICAL].log_count = iterations;
    v2_results_local_context_every_time[PERF_TEST_LOG_CRITICAL].time = current_time - start_time;

#define PRINT_RESULTS(title, results_var) \
    (void)printf(title " %" PRI_MU_ENUM " %" PRIu64 " logs in %.00lf s (%" PRIu64 " us), %.00lf logs/us\r\n", MU_ENUM_VALUE(PERF_TEST_LIST, i), results_var[i].log_count, results_var[i].time / 1000000, (uint64_t)results_var[i].time, (double)results_var[i].log_count / (results_var[i].time / 1000000));

    for (int i = 0; i < MU_ENUM_VALUE_COUNT_WITHOUT_INVALID(PERF_TEST_LIST); i++)
    {
        PRINT_RESULTS("v1", v1_results);
        PRINT_RESULTS("v2", v2_results);
        PRINT_RESULTS("v2 with local context created once", v2_results_local_context_once);
        PRINT_RESULTS("v2 with dynamic context created once", v2_results_dynamic_context_once);
        PRINT_RESULTS("v2 with local context created every time", v2_results_local_context_every_time);
        PRINT_RESULTS("v2 with dynamic context created every time", v2_results_local_context_every_time);
    }
}
