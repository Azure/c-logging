// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdio.h>
#include <inttypes.h>

#include "windows.h"
#include "TraceLoggingProvider.h"
#include "evntrace.h"
#include "evntcons.h"

#include "macro_utils/macro_utils.h"

#include "logger_perf_common.h"

MU_DEFINE_ENUM_STRINGS_WITHOUT_INVALID(LOGGER_TYPE, LOGGER_TYPE_VALUES);
MU_DEFINE_ENUM_STRINGS_WITHOUT_INVALID(CONSUMER_ENABLE_TYPE, CONSUMER_ENABLE_TYPE_VALUES);

PERF_RESULT results[MU_ENUM_VALUE_COUNT_WITHOUT_INVALID(LOGGER_TYPE_VALUES)][MU_ENUM_VALUE_COUNT_WITHOUT_INVALID(CONSUMER_ENABLE_TYPE_VALUES)];

void print_results(LOGGER_TYPE logger_type, CONSUMER_ENABLE_TYPE consumer_enable_type, PERF_RESULT* perf_result)
{
    (void)printf("results[%" PRI_MU_ENUM "][%" PRI_MU_ENUM "]: %" PRIu64 " logs in %.00lf s (%" PRIu64 " us), %.00lf logs/s\r\n",
        MU_ENUM_VALUE(LOGGER_TYPE, logger_type),
        MU_ENUM_VALUE(CONSUMER_ENABLE_TYPE, consumer_enable_type),
        perf_result->log_count,
        perf_result->time / 1000000,
        (uint64_t)perf_result->time, (double)perf_result->log_count / (perf_result->time / 1000000));
}

typedef struct EVENT_TRACE_PROPERTY_DATA_TAG
{
    EVENT_TRACE_PROPERTIES props;
    char logger_name[128];
} EVENT_TRACE_PROPERTY_DATA;

static GUID provider_guid = { 0xDAD29F36, 0x0A48, 0x4DEF, { 0x9D, 0x50, 0x8E, 0xF9, 0x03, 0x6B, 0x92, 0xB4 } };

TRACEHANDLE start_trace(TEST_CONTEXT* test_context, uint8_t trace_level)
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

void stop_trace(TEST_CONTEXT* test_context, TRACEHANDLE trace_session_handle)
{
    EVENT_TRACE_PROPERTY_DATA stop_event_trace_property_data = { 0 };

    stop_event_trace_property_data.props.Wnode.BufferSize = sizeof(stop_event_trace_property_data);
    stop_event_trace_property_data.props.Wnode.Flags = WNODE_FLAG_TRACED_GUID;
    stop_event_trace_property_data.props.LoggerNameOffset = offsetof(EVENT_TRACE_PROPERTY_DATA, logger_name);
    stop_event_trace_property_data.props.LogFileNameOffset = 0;

    (void)printf("Stopping trace session %s\r\n", test_context->trace_session_name);
    POOR_MANS_ASSERT(StopTraceA(trace_session_handle, NULL, &stop_event_trace_property_data.props) == ERROR_SUCCESS);
}

void generate_trace_session(TEST_CONTEXT* test_context, const char* function_name)
{
    UUID uuid;
    POOR_MANS_ASSERT(UuidCreate(&uuid) == RPC_S_OK);
    int snprintf_result = snprintf(test_context->trace_session_name, sizeof(test_context->trace_session_name), "%s-%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
        function_name, uuid.Data1, uuid.Data2, uuid.Data3, uuid.Data4[0], uuid.Data4[1], uuid.Data4[2], uuid.Data4[3], uuid.Data4[4], uuid.Data4[5], uuid.Data4[6], uuid.Data4[7]);
    POOR_MANS_ASSERT((snprintf_result >= 0) && (snprintf_result < sizeof(test_context->trace_session_name)));
}

TEST_CONTEXT* test_context_create(void)
{
    TEST_CONTEXT* result = malloc(sizeof(TEST_CONTEXT));
    POOR_MANS_ASSERT(result != NULL);

    return result;
}

void test_context_destroy(TEST_CONTEXT* test_context)
{
    free(test_context);
}
