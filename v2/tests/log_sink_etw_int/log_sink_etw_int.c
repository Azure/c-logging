// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "windows.h"
#include "TraceLoggingProvider.h"
#include "evntrace.h"
#include "evntcons.h"

#include "macro_utils/macro_utils.h"

#include "c_logging/log_context_property_basic_types.h"
#include "c_logging/log_context_property_type_ascii_char_ptr.h"
#include "c_logging/log_context_property_value_pair.h"
#include "c_logging/log_level.h"
#include "c_logging/log_sink_if.h"
#include "c_logging/log_context.h"
#include "c_logging/logger.h"

#include "c_logging/log_sink_etw.h"

static size_t asserts_failed = 0;

static GUID provider_guid = { 0xDAD29F36, 0x0A48, 0x4DEF, { 0x9D, 0x50, 0x8E, 0xF9, 0x03, 0x6B, 0x92, 0xB4 } };

MU_DEFINE_ENUM_STRINGS(LOG_CONTEXT_PROPERTY_TYPE, LOG_CONTEXT_PROPERTY_TYPE_VALUES)

#define POOR_MANS_ASSERT(cond) \
    if (!(cond)) \
    { \
        (void)printf("%s:%d test failed\r\n", __FUNCTION__, __LINE__); \
        asserts_failed++; \
    } \

typedef struct EVENT_TRACE_PROPERTY_DATA_TAG
{
    EVENT_TRACE_PROPERTIES props;
    WCHAR logger_name[128];
    WCHAR log_file_name[1024];
} EVENT_TRACE_PROPERTY_DATA;

static void WINAPI EventRecordCallback(
    _In_ EVENT_RECORD* pEventRecord)
{
    (void)printf("xxx");
    (void)pEventRecord;
}

static void log_sink_etw_log_with_LOG_LEVEL_ERROR_succeeds(void)
{
    // arrange
    // setup the etw consumer
    TRACEHANDLE trace_session_handle;
    UUID uuid;
    POOR_MANS_ASSERT(UuidCreate(&uuid) == RPC_S_OK);
    wchar_t trace_session_name[128];
    int session_name_length = _swprintf(trace_session_name, L"%S-%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
        __FUNCTION__, uuid.Data1, uuid.Data2, uuid.Data3, uuid.Data4[0], uuid.Data4[1], uuid.Data4[2], uuid.Data4[3], uuid.Data4[4], uuid.Data4[5], uuid.Data4[6], uuid.Data4[7]);
    POOR_MANS_ASSERT(session_name_length > 0);
    (void)printf("Starting trace session %ls\r\n", trace_session_name);

    EVENT_TRACE_PROPERTY_DATA start_event_trace_property_data = { 0 };

    start_event_trace_property_data.props.Wnode.BufferSize = sizeof(start_event_trace_property_data);
    start_event_trace_property_data.props.Wnode.Flags = WNODE_FLAG_TRACED_GUID;
    start_event_trace_property_data.props.LogFileMode = EVENT_TRACE_FILE_MODE_NONE /*| EVENT_TRACE_REAL_TIME_MODE*/;
    start_event_trace_property_data.props.MinimumBuffers = 1;
    start_event_trace_property_data.props.BufferSize = 64 * 1024;
    start_event_trace_property_data.props.LogFileNameOffset = offsetof(EVENT_TRACE_PROPERTY_DATA, log_file_name);
    start_event_trace_property_data.props.LoggerNameOffset = offsetof(EVENT_TRACE_PROPERTY_DATA, logger_name);

    (void)wcscpy(start_event_trace_property_data.log_file_name, L"d:\\x.etl");

    POOR_MANS_ASSERT(StartTraceW(&trace_session_handle, trace_session_name, &start_event_trace_property_data.props) == ERROR_SUCCESS);

    ULONG enable_trace_result = EnableTraceEx2(
        trace_session_handle,
        (LPCGUID)&provider_guid,
        EVENT_CONTROL_CODE_ENABLE_PROVIDER,
        TRACE_LEVEL_INFORMATION,
        0,
        0,
        0,
        NULL
    );

    POOR_MANS_ASSERT(enable_trace_result == ERROR_SUCCESS);

    //EVENT_TRACE_LOGFILEW log_file = { 0 };
    //
    //log_file.ProcessTraceMode = PROCESS_TRACE_MODE_EVENT_RECORD | PROCESS_TRACE_MODE_REAL_TIME;
    //log_file.EventRecordCallback = &EventRecordCallback;
    //log_file.LogFileMode = EVENT_TRACE_REAL_TIME_MODE;
    //log_file.LoggerName = &start_event_trace_property_data.logger_name[0];
    //log_file.Context = NULL;
    //
    //TRACEHANDLE trace = OpenTraceW(&log_file);
    //POOR_MANS_ASSERT(trace != INVALID_PROCESSTRACE_HANDLE);
    //if (trace == INVALID_PROCESSTRACE_HANDLE)
    //{
    //    (void)printf("OpenTraceA failed, LE=%lu", GetLastError());
    //}

    // act
    log_sink_etw.log_sink_log(LOG_LEVEL_ERROR, NULL, __FILE__, __FUNCTION__, __LINE__, "test");

    //ULONG process_trace_result = ProcessTrace(&trace, 1, NULL, NULL);
    //POOR_MANS_ASSERT(process_trace_result == ERROR_SUCCESS);
    //if (process_trace_result != ERROR_SUCCESS)
    //{
    //    (void)printf("ProcessTrace failed, process_trace_result=%lu", process_trace_result);
    //}

    // assert

    // cleanup
    //(void)CloseTrace(trace);

    EVENT_TRACE_PROPERTY_DATA stop_event_trace_property_data = { 0 };

    stop_event_trace_property_data.props.Wnode.BufferSize = sizeof(stop_event_trace_property_data);
    stop_event_trace_property_data.props.Wnode.Flags = WNODE_FLAG_TRACED_GUID;
    stop_event_trace_property_data.props.LogFileNameOffset = offsetof(EVENT_TRACE_PROPERTY_DATA, log_file_name);
    stop_event_trace_property_data.props.LoggerNameOffset = offsetof(EVENT_TRACE_PROPERTY_DATA, logger_name);

    (void)printf("Stopping trace session %ls\r\n", trace_session_name);
    POOR_MANS_ASSERT(StopTraceW(trace_session_handle, NULL, &stop_event_trace_property_data.props) == ERROR_SUCCESS);
}

/* very "poor man's" way of testing, as no test harness available */
int main(void)
{
    log_sink_etw_log_with_LOG_LEVEL_ERROR_succeeds();

    return asserts_failed;
}

