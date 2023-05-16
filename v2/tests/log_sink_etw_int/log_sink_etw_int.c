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
    char logger_name[128];
    char log_file_name[1024];
} EVENT_TRACE_PROPERTY_DATA;

#define MAX_EVENT_NAME_LENGTH 512
#define MAX_CONTENT_LENGTH 4096
#define MAX_FILE_LENGTH 512
#define MAX_FUNC_LENGTH 512

// This is a more human(e) representation for our tests
typedef struct PARSED_EVENT_TAG
{
    char event_name[MAX_EVENT_NAME_LENGTH];
    char content[MAX_CONTENT_LENGTH];
    char file[MAX_FILE_LENGTH];
    char func[MAX_FUNC_LENGTH];
    int line;
} PARSED_EVENT;

#define MAX_EVENTS 128

static uint32_t parsed_event_count;
static PARSED_EVENT parsed_events[MAX_EVENTS];

static void get_event_name(EVENT_HEADER_EXTENDED_DATA_ITEM* extended_data_items, unsigned short count, const char** event_name)
{
    for (unsigned short i = 0; i < count; i++)
    {
        if (extended_data_items[i].ExtType == EVENT_HEADER_EXT_TYPE_EVENT_SCHEMA_TL)
        {
            // This one has the event name
            // Not really documented, but the Windows source code seems to indicate I can just skip over a few bytes that hold a length and version
            *event_name = (const char*)extended_data_items[i].DataPtr + 4;
            break;
        }
    }
}

static ULONG event_trace_buffer_callback(PEVENT_TRACE_LOGFILEA log_file)
{
    (void)log_file;

    return TRUE;
}

static void WINAPI event_trace_record_callback(EVENT_RECORD* pEventRecord)
{
    if (IsEqualGUID(&pEventRecord->EventHeader.ProviderId, &provider_guid))
    {
        (void)memset(&parsed_events[parsed_event_count], 0, sizeof(PARSED_EVENT));

        EVENT_HEADER_EXTENDED_DATA_ITEM* extended_data_items = pEventRecord->ExtendedData;

        for (unsigned short i = 0; i < pEventRecord->ExtendedDataCount; i++)
        {
            if (extended_data_items[i].ExtType == EVENT_HEADER_EXT_TYPE_EVENT_SCHEMA_TL)
            {
                // This one has the event name
                // Not really documented, but the Windows source code seems to indicate I can just skip over a few bytes that hold a length and version
                const char* event_name = (const char*)extended_data_items[i].DataPtr + 4;

                if (event_name != NULL)
                {
                    (void)strcpy(parsed_events[parsed_event_count].event_name, event_name);
                }

                break;
            }
        }
       
        POOR_MANS_ASSERT(pEventRecord->UserData != NULL);
        const char* current_user_data_field = pEventRecord->UserData;
        if (current_user_data_field != NULL)
        {
            (void)strcpy(parsed_events[parsed_event_count].content, current_user_data_field);
            current_user_data_field += strlen(current_user_data_field) + 1;
        }

        if (current_user_data_field != NULL)
        {
            (void)strcpy(parsed_events[parsed_event_count].file, current_user_data_field);
            current_user_data_field += strlen(current_user_data_field) + 1;
        }

        if (current_user_data_field != NULL)
        {
            (void)strcpy(parsed_events[parsed_event_count].func, current_user_data_field);
            current_user_data_field += strlen(current_user_data_field) + 1;
        }

        parsed_events[parsed_event_count].line = *(int32_t*)current_user_data_field;
        current_user_data_field++;

        parsed_event_count++;
    }
}

static char test_path[MAX_PATH];

static void log_sink_etw_log_with_LOG_LEVEL_ERROR_succeeds(void)
{
    // arrange
    parsed_event_count = 0;

    // setup the etw consumer
    TRACEHANDLE trace_session_handle;
    UUID uuid;
    POOR_MANS_ASSERT(UuidCreate(&uuid) == RPC_S_OK);
    char trace_session_name[128];
    int session_name_length = sprintf(trace_session_name, "%s-%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
        __FUNCTION__, uuid.Data1, uuid.Data2, uuid.Data3, uuid.Data4[0], uuid.Data4[1], uuid.Data4[2], uuid.Data4[3], uuid.Data4[4], uuid.Data4[5], uuid.Data4[6], uuid.Data4[7]);
    POOR_MANS_ASSERT(session_name_length > 0);
    (void)printf("Starting trace session %s\r\n", trace_session_name);

    EVENT_TRACE_PROPERTY_DATA start_event_trace_property_data = { 0 };

    start_event_trace_property_data.props.Wnode.BufferSize = sizeof(start_event_trace_property_data);
    start_event_trace_property_data.props.Wnode.Flags = WNODE_FLAG_TRACED_GUID;
    start_event_trace_property_data.props.LogFileMode = EVENT_TRACE_FILE_MODE_SEQUENTIAL;
    start_event_trace_property_data.props.MinimumBuffers = 4;
    start_event_trace_property_data.props.MaximumBuffers = 4;
    start_event_trace_property_data.props.BufferSize = 64 * 1024;
    start_event_trace_property_data.props.FlushTimer = 1;
    start_event_trace_property_data.props.LogFileNameOffset = offsetof(EVENT_TRACE_PROPERTY_DATA, log_file_name);
    start_event_trace_property_data.props.LoggerNameOffset = offsetof(EVENT_TRACE_PROPERTY_DATA, logger_name);

    // fill in a file to use
    int snprintf_result = snprintf(start_event_trace_property_data.log_file_name, sizeof(start_event_trace_property_data.log_file_name), "%s\\%s.etl",
        test_path, trace_session_name);
    POOR_MANS_ASSERT((snprintf_result > 0) && (snprintf_result < sizeof(start_event_trace_property_data.log_file_name)));

    POOR_MANS_ASSERT(StartTraceA(&trace_session_handle, trace_session_name, &start_event_trace_property_data.props) == ERROR_SUCCESS);

    // enable our desired provider
    ULONG enable_trace_result = EnableTraceEx2(trace_session_handle, (LPCGUID)&provider_guid, EVENT_CONTROL_CODE_ENABLE_PROVIDER,
        TRACE_LEVEL_INFORMATION, 0, 0, 0, NULL
    );
    POOR_MANS_ASSERT(enable_trace_result == ERROR_SUCCESS);

    int captured_line = __LINE__;

    // act
    log_sink_etw.log_sink_log(LOG_LEVEL_ERROR, NULL, __FILE__, __FUNCTION__, captured_line, "test");

    // assert
    EVENT_TRACE_PROPERTY_DATA stop_event_trace_property_data = { 0 };

    stop_event_trace_property_data.props.Wnode.BufferSize = sizeof(stop_event_trace_property_data);
    stop_event_trace_property_data.props.Wnode.Flags = WNODE_FLAG_TRACED_GUID;
    stop_event_trace_property_data.props.LogFileNameOffset = offsetof(EVENT_TRACE_PROPERTY_DATA, log_file_name);
    stop_event_trace_property_data.props.LoggerNameOffset = offsetof(EVENT_TRACE_PROPERTY_DATA, logger_name);

    (void)printf("Stopping trace session %s\r\n", trace_session_name);
    POOR_MANS_ASSERT(StopTraceA(trace_session_handle, NULL, &stop_event_trace_property_data.props) == ERROR_SUCCESS);

    EVENT_TRACE_LOGFILEA log_file = { 0 };

    log_file.ProcessTraceMode = PROCESS_TRACE_MODE_EVENT_RECORD;
    log_file.EventRecordCallback = event_trace_record_callback;
    log_file.BufferCallback = event_trace_buffer_callback;
    log_file.LogFileName = &start_event_trace_property_data.log_file_name[0];
    log_file.Context = NULL;

    TRACEHANDLE trace = OpenTraceA(&log_file);
    POOR_MANS_ASSERT(trace != INVALID_PROCESSTRACE_HANDLE);

    // open the trace and process all events
    ULONG process_trace_result = ProcessTrace(&trace, 1, NULL, NULL);
    POOR_MANS_ASSERT(process_trace_result == ERROR_SUCCESS);

    // cleanup
    (void)CloseTrace(trace);

    // 2 events expected: one self and one for the actual event
    POOR_MANS_ASSERT(parsed_event_count == 2);
    // self test event
    POOR_MANS_ASSERT(strcmp(parsed_events[0].event_name, "LogInfo") == 0);
    POOR_MANS_ASSERT(strlen(parsed_events[0].content) > 0);
    POOR_MANS_ASSERT(strlen(parsed_events[0].file) > 0);
    POOR_MANS_ASSERT(strlen(parsed_events[0].func) > 0);
    POOR_MANS_ASSERT(parsed_events[0].line != 0);

    // error event
    POOR_MANS_ASSERT(strcmp(parsed_events[1].event_name, "LogError") == 0);
    POOR_MANS_ASSERT(strcmp(parsed_events[1].content, "test") == 0);
    POOR_MANS_ASSERT(strcmp(parsed_events[1].file, __FILE__) == 0);
    POOR_MANS_ASSERT(strcmp(parsed_events[1].func, __FUNCTION__) == 0);
    POOR_MANS_ASSERT(parsed_events[1].line == captured_line);

    // delete the trace file if created
    // allocate enough
    char delete_command[MAX_PATH + 4];
    snprintf_result = sprintf(delete_command, "del %s", start_event_trace_property_data.log_file_name);
    POOR_MANS_ASSERT((snprintf_result > 0) && (snprintf_result < sizeof(delete_command)));
    (void)system(delete_command);
}

static void setup_test(char* trace_session_name)
{
    parsed_event_count = 0;

    // setup the etw consumer
    TRACEHANDLE trace_session_handle;
    UUID uuid;
    POOR_MANS_ASSERT(UuidCreate(&uuid) == RPC_S_OK);
    int session_name_length = sprintf(trace_session_name, "%s-%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
        __FUNCTION__, uuid.Data1, uuid.Data2, uuid.Data3, uuid.Data4[0], uuid.Data4[1], uuid.Data4[2], uuid.Data4[3], uuid.Data4[4], uuid.Data4[5], uuid.Data4[6], uuid.Data4[7]);
    POOR_MANS_ASSERT(session_name_length > 0);
    (void)printf("Starting trace session %s\r\n", trace_session_name);

    EVENT_TRACE_PROPERTY_DATA start_event_trace_property_data = { 0 };

    start_event_trace_property_data.props.Wnode.BufferSize = sizeof(start_event_trace_property_data);
    start_event_trace_property_data.props.Wnode.Flags = WNODE_FLAG_TRACED_GUID;
    start_event_trace_property_data.props.LogFileMode = EVENT_TRACE_FILE_MODE_SEQUENTIAL;
    start_event_trace_property_data.props.MinimumBuffers = 4;
    start_event_trace_property_data.props.MaximumBuffers = 4;
    start_event_trace_property_data.props.BufferSize = 64 * 1024;
    start_event_trace_property_data.props.FlushTimer = 1;
    start_event_trace_property_data.props.LogFileNameOffset = offsetof(EVENT_TRACE_PROPERTY_DATA, log_file_name);
    start_event_trace_property_data.props.LoggerNameOffset = offsetof(EVENT_TRACE_PROPERTY_DATA, logger_name);

    // fill in a file to use
    int snprintf_result = snprintf(start_event_trace_property_data.log_file_name, sizeof(start_event_trace_property_data.log_file_name), "%s\\%s.etl",
        test_path, trace_session_name);
    POOR_MANS_ASSERT((snprintf_result > 0) && (snprintf_result < sizeof(start_event_trace_property_data.log_file_name)));

    POOR_MANS_ASSERT(StartTraceA(&trace_session_handle, trace_session_name, &start_event_trace_property_data.props) == ERROR_SUCCESS);

    // enable our desired provider
    ULONG enable_trace_result = EnableTraceEx2(trace_session_handle, (LPCGUID)&provider_guid, EVENT_CONTROL_CODE_ENABLE_PROVIDER,
        TRACE_LEVEL_VERBOSE, 0, 0, 0, NULL
    );
    POOR_MANS_ASSERT(enable_trace_result == ERROR_SUCCESS);
}

static void stop_trace(const char* trace_session_name)
{
}

static void log_sink_etw_log_all_levels_when_all_levels_enabled_succeeds(void)
{
    // arrange
    char trace_session_name[128];
    setup_test(trace_session_name);

    int captured_line = __LINE__;

    // act
    log_sink_etw.log_sink_log(LOG_LEVEL_CRITICAL, NULL, __FILE__, __FUNCTION__, captured_line, "test_critical");
    log_sink_etw.log_sink_log(LOG_LEVEL_ERROR, NULL, __FILE__, __FUNCTION__, captured_line + 1, "test_error");
    log_sink_etw.log_sink_log(LOG_LEVEL_WARNING, NULL, __FILE__, __FUNCTION__, captured_line + 2, "test_warning");
    log_sink_etw.log_sink_log(LOG_LEVEL_INFO, NULL, __FILE__, __FUNCTION__, captured_line + 3, "test_info");
    log_sink_etw.log_sink_log(LOG_LEVEL_VERBOSE, NULL, __FILE__, __FUNCTION__, captured_line + 4, "test_verbose");

    // assert
    stop_trace(trace_session_name);
    EVENT_TRACE_PROPERTY_DATA stop_event_trace_property_data = { 0 };

    stop_event_trace_property_data.props.Wnode.BufferSize = sizeof(stop_event_trace_property_data);
    stop_event_trace_property_data.props.Wnode.Flags = WNODE_FLAG_TRACED_GUID;
    stop_event_trace_property_data.props.LogFileNameOffset = offsetof(EVENT_TRACE_PROPERTY_DATA, log_file_name);
    stop_event_trace_property_data.props.LoggerNameOffset = offsetof(EVENT_TRACE_PROPERTY_DATA, logger_name);

    (void)printf("Stopping trace session %s\r\n", trace_session_name);
    POOR_MANS_ASSERT(StopTraceA(trace_session_handle, NULL, &stop_event_trace_property_data.props) == ERROR_SUCCESS);

    EVENT_TRACE_LOGFILEA log_file = { 0 };

    log_file.ProcessTraceMode = PROCESS_TRACE_MODE_EVENT_RECORD;
    log_file.EventRecordCallback = event_trace_record_callback;
    log_file.BufferCallback = event_trace_buffer_callback;
    log_file.LogFileName = &start_event_trace_property_data.log_file_name[0];
    log_file.Context = NULL;

    TRACEHANDLE trace = OpenTraceA(&log_file);
    POOR_MANS_ASSERT(trace != INVALID_PROCESSTRACE_HANDLE);

    // open the trace and process all events
    ULONG process_trace_result = ProcessTrace(&trace, 1, NULL, NULL);
    POOR_MANS_ASSERT(process_trace_result == ERROR_SUCCESS);

    // cleanup
    (void)CloseTrace(trace);

    // 2 events expected: one self and one for the actual event
    POOR_MANS_ASSERT(parsed_event_count == 5);

    // critical event
    POOR_MANS_ASSERT(strcmp(parsed_events[0].event_name, "LogCritical") == 0);
    POOR_MANS_ASSERT(strcmp(parsed_events[0].content, "test_critical") == 0);
    POOR_MANS_ASSERT(strcmp(parsed_events[0].file, __FILE__) == 0);
    POOR_MANS_ASSERT(strcmp(parsed_events[0].func, __FUNCTION__) == 0);
    POOR_MANS_ASSERT(parsed_events[0].line == captured_line);

    // error event
    POOR_MANS_ASSERT(strcmp(parsed_events[1].event_name, "LogError") == 0);
    POOR_MANS_ASSERT(strcmp(parsed_events[1].content, "test_error") == 0);
    POOR_MANS_ASSERT(strcmp(parsed_events[1].file, __FILE__) == 0);
    POOR_MANS_ASSERT(strcmp(parsed_events[1].func, __FUNCTION__) == 0);
    POOR_MANS_ASSERT(parsed_events[1].line == captured_line + 1);

    // warning event
    POOR_MANS_ASSERT(strcmp(parsed_events[2].event_name, "LogWarning") == 0);
    POOR_MANS_ASSERT(strcmp(parsed_events[2].content, "test_warning") == 0);
    POOR_MANS_ASSERT(strcmp(parsed_events[2].file, __FILE__) == 0);
    POOR_MANS_ASSERT(strcmp(parsed_events[2].func, __FUNCTION__) == 0);
    POOR_MANS_ASSERT(parsed_events[2].line == captured_line + 2);

    // info event
    POOR_MANS_ASSERT(strcmp(parsed_events[3].event_name, "LogInfo") == 0);
    POOR_MANS_ASSERT(strcmp(parsed_events[3].content, "test_info") == 0);
    POOR_MANS_ASSERT(strcmp(parsed_events[3].file, __FILE__) == 0);
    POOR_MANS_ASSERT(strcmp(parsed_events[3].func, __FUNCTION__) == 0);
    POOR_MANS_ASSERT(parsed_events[3].line == captured_line + 3);

    // verbose event
    POOR_MANS_ASSERT(strcmp(parsed_events[4].event_name, "LogVerbose") == 0);
    POOR_MANS_ASSERT(strcmp(parsed_events[4].content, "test_verbose") == 0);
    POOR_MANS_ASSERT(strcmp(parsed_events[4].file, __FILE__) == 0);
    POOR_MANS_ASSERT(strcmp(parsed_events[4].func, __FUNCTION__) == 0);
    POOR_MANS_ASSERT(parsed_events[4].line == captured_line + 4);

    // delete the trace file if created
    // allocate enough
    char delete_command[MAX_PATH + 4];
    snprintf_result = sprintf(delete_command, "del %s", start_event_trace_property_data.log_file_name);
    POOR_MANS_ASSERT((snprintf_result > 0) && (snprintf_result < sizeof(delete_command)));
    (void)system(delete_command);
}

/* very "poor man's" way of testing, as no test harness available */
int main(void)
{
    int result;

    if (GetTempPathA(sizeof(test_path), test_path) == 0)
    {
        (void)printf("GetTempPathA failed, last error = %lu\r\n", GetLastError());
        result = MU_FAILURE;
    }
    else
    {
        log_sink_etw_log_with_LOG_LEVEL_ERROR_succeeds();
        log_sink_etw_log_all_levels_when_all_levels_enabled_succeeds();

        result = (int)asserts_failed;
    }

    return result;
}

