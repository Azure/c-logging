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
#define MAX_PROPERTY_NAME_LENGTH 256 // this should be enough for our tests
#define MAX_PROPERTY_COUNT 64
#define MAX_STRING_VALUE_LENGTH 512

// This is a more human(e) representation for our tests
// We don't care that we're using quite a bit of memory frankly, this is just for tests
typedef struct PARSED_EVENT_PROPERTY_TAG
{
    char property_name[MAX_PROPERTY_NAME_LENGTH];
    uint8_t property_type;
    union
    {
        char ascii_char_ptr_value[MAX_STRING_VALUE_LENGTH];
        uint8_t uint8_t_value;
        int8_t int8_t_value;
        uint16_t uint16_t_value;
        int16_t int16_t_value;
        uint32_t uint32_t_value;
        int32_t int32_t_value;
        uint64_t uint64_t_value;
        int64_t int64_t_value;
        uint8_t struct_field_count;
    };
} PARSED_EVENT_PROPERTY;

typedef struct PARSED_EVENT_TAG
{
    char event_name[MAX_EVENT_NAME_LENGTH];
    char content[MAX_CONTENT_LENGTH];
    char file[MAX_FILE_LENGTH];
    char func[MAX_FUNC_LENGTH];
    int line;
    uint32_t property_count;
    PARSED_EVENT_PROPERTY properties[MAX_PROPERTY_COUNT];
} PARSED_EVENT;

#define MAX_EVENTS 128

static uint32_t parsed_event_count;
static PARSED_EVENT parsed_events[MAX_EVENTS];

// This callback is needed
// Without this callback events are not properly processed
static ULONG event_trace_buffer_callback(PEVENT_TRACE_LOGFILEA log_file)
{
    (void)log_file;

    return TRUE;
}

static void WINAPI event_trace_record_callback(EVENT_RECORD* pEventRecord)
{
    if (IsEqualGUID(&pEventRecord->EventHeader.ProviderId, &provider_guid))
    {
        const uint8_t* metadata = NULL;

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
                    (void)strncpy(parsed_events[parsed_event_count].event_name, event_name, sizeof(parsed_events[parsed_event_count].event_name));
                    parsed_events[parsed_event_count].event_name[sizeof(parsed_events[parsed_event_count].event_name) - 1] = '\0';
                    metadata = (const uint8_t*)(event_name + strlen(event_name) + 1);
                }

                break;
            }
        }
       
        POOR_MANS_ASSERT(pEventRecord->UserData != NULL);

        const char* current_user_data_field = pEventRecord->UserData;
        if (current_user_data_field != NULL)
        {
            (void)strncpy(parsed_events[parsed_event_count].content, current_user_data_field, sizeof(parsed_events[parsed_event_count].content));
            parsed_events[parsed_event_count].content[sizeof(parsed_events[parsed_event_count].content) - 1] = '\0';
            current_user_data_field += strlen(current_user_data_field) + 1;
        }
        POOR_MANS_ASSERT(strcmp((const char*)metadata, "content") == 0);
        metadata += sizeof("content") + 1;

        if (current_user_data_field != NULL)
        {
            (void)strncpy(parsed_events[parsed_event_count].file, current_user_data_field, sizeof(parsed_events[parsed_event_count].file));
            parsed_events[parsed_event_count].file[sizeof(parsed_events[parsed_event_count].file) - 1] = '\0';
            current_user_data_field += strlen(current_user_data_field) + 1;
        }
        POOR_MANS_ASSERT(strcmp((const char*)metadata, "file") == 0);
        metadata += sizeof("file") + 1;

        if (current_user_data_field != NULL)
        {
            (void)strncpy(parsed_events[parsed_event_count].func, current_user_data_field, sizeof(parsed_events[parsed_event_count].func));
            parsed_events[parsed_event_count].func[sizeof(parsed_events[parsed_event_count].func) - 1] = '\0';
            current_user_data_field += strlen(current_user_data_field) + 1;
        }
        POOR_MANS_ASSERT(strcmp((const char*)metadata, "func") == 0);
        metadata += sizeof("func") + 1;

        parsed_events[parsed_event_count].line = *(int32_t*)current_user_data_field;
        current_user_data_field+= sizeof(int32_t);

        POOR_MANS_ASSERT(strcmp((const char*)metadata, "line") == 0);
        metadata += sizeof("line") + 1;

        uint32_t property_index = 0;

        // get all other properties
        while (current_user_data_field - (const char*)pEventRecord->UserData < pEventRecord->UserDataLength)
        {
            // get name and type from metadata
            (void)strncpy(parsed_events[parsed_event_count].properties[property_index].property_name, (const char*)metadata, sizeof(parsed_events[parsed_event_count].properties[property_index].property_name));
            parsed_events[parsed_event_count].properties[property_index].property_name[sizeof(parsed_events[parsed_event_count].properties[property_index].property_name) - 1] = '\0';
            metadata += strlen(parsed_events[parsed_event_count].properties[property_index].property_name) + 1;
            parsed_events[parsed_event_count].properties[property_index].property_type = *metadata;
            metadata++;

            switch (parsed_events[parsed_event_count].properties[property_index].property_type)
            {
            default:
                POOR_MANS_ASSERT(0);
                break;
            case _TlgInSTRUCT | _TlgInChain:
                parsed_events[parsed_event_count].properties[property_index].struct_field_count = *(uint8_t*)metadata;
                metadata++;
                break;
            case TlgInANSISTRING:
                (void)strncpy(parsed_events[parsed_event_count].properties[property_index].ascii_char_ptr_value, current_user_data_field, sizeof(parsed_events[parsed_event_count].properties[property_index].ascii_char_ptr_value));
                parsed_events[parsed_event_count].properties[property_index].ascii_char_ptr_value[sizeof(parsed_events[parsed_event_count].properties[property_index].ascii_char_ptr_value) - 1] = '\0';
                current_user_data_field += strlen(current_user_data_field) + 1;
                break;
            case TlgInUINT8:
                parsed_events[parsed_event_count].properties[property_index].uint8_t_value = *(uint8_t*)current_user_data_field;
                current_user_data_field += sizeof(uint8_t);
                break;
            case TlgInINT8:
                parsed_events[parsed_event_count].properties[property_index].int8_t_value = *(int8_t*)current_user_data_field;
                current_user_data_field += sizeof(int8_t);
                break;
            case TlgInUINT16:
                parsed_events[parsed_event_count].properties[property_index].uint16_t_value = *(uint16_t*)current_user_data_field;
                current_user_data_field += sizeof(uint16_t);
                break;
            case TlgInINT16:
                parsed_events[parsed_event_count].properties[property_index].int16_t_value = *(int16_t*)current_user_data_field;
                current_user_data_field += sizeof(int16_t);
                break;
            case TlgInUINT32:
                parsed_events[parsed_event_count].properties[property_index].uint32_t_value = *(uint32_t*)current_user_data_field;
                current_user_data_field += sizeof(uint32_t);
                break;
            case TlgInINT32:
                parsed_events[parsed_event_count].properties[property_index].int32_t_value = *(int32_t*)current_user_data_field;
                current_user_data_field += sizeof(int32_t);
                break;
            case TlgInUINT64:
                parsed_events[parsed_event_count].properties[property_index].uint64_t_value = *(uint64_t*)current_user_data_field;
                current_user_data_field += sizeof(uint64_t);
                break;
            case TlgInINT64:
                parsed_events[parsed_event_count].properties[property_index].int64_t_value = *(int64_t*)current_user_data_field;
                current_user_data_field += sizeof(int64_t);
                break;
            }

            property_index++;
        }

        parsed_events[parsed_event_count].property_count = property_index;

        parsed_event_count++;
    }
}

static char test_path[MAX_PATH];

static void generate_trace_session_and_file_name(char* trace_session_name, size_t trace_session_name_buffer_size, char* log_file_name, size_t log_file_name_buffer_size)
{
    UUID uuid;
    POOR_MANS_ASSERT(UuidCreate(&uuid) == RPC_S_OK);
    int snprintf_result = snprintf(trace_session_name, trace_session_name_buffer_size, "%s-%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
        __FUNCTION__, uuid.Data1, uuid.Data2, uuid.Data3, uuid.Data4[0], uuid.Data4[1], uuid.Data4[2], uuid.Data4[3], uuid.Data4[4], uuid.Data4[5], uuid.Data4[6], uuid.Data4[7]);
    POOR_MANS_ASSERT((snprintf_result > 0) && (snprintf_result < trace_session_name_buffer_size));

    // fill in a file to use
    snprintf_result = snprintf(log_file_name, log_file_name_buffer_size, "%s\\%s.etl",
        test_path, trace_session_name);
    POOR_MANS_ASSERT((snprintf_result > 0) && (snprintf_result < log_file_name_buffer_size));
}

static TRACEHANDLE start_trace(const char* trace_session_name, const char* log_file_name, uint8_t trace_level)
{
    // setup the etw consumer
    TRACEHANDLE trace_session_handle;

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

    (void)strcpy(start_event_trace_property_data.log_file_name, log_file_name);

    POOR_MANS_ASSERT(StartTraceA(&trace_session_handle, trace_session_name, &start_event_trace_property_data.props) == ERROR_SUCCESS);

    // enable our desired provider
    ULONG enable_trace_result = EnableTraceEx2(trace_session_handle, (LPCGUID)&provider_guid, EVENT_CONTROL_CODE_ENABLE_PROVIDER,
        trace_level, 0, 0, 0, NULL
    );
    POOR_MANS_ASSERT(enable_trace_result == ERROR_SUCCESS);

    return trace_session_handle;
}

static void stop_trace(TRACEHANDLE trace_session_handle, const char* trace_session_name)
{
    EVENT_TRACE_PROPERTY_DATA stop_event_trace_property_data = { 0 };

    stop_event_trace_property_data.props.Wnode.BufferSize = sizeof(stop_event_trace_property_data);
    stop_event_trace_property_data.props.Wnode.Flags = WNODE_FLAG_TRACED_GUID;
    stop_event_trace_property_data.props.LogFileNameOffset = offsetof(EVENT_TRACE_PROPERTY_DATA, log_file_name);
    stop_event_trace_property_data.props.LoggerNameOffset = offsetof(EVENT_TRACE_PROPERTY_DATA, logger_name);

    // give the trace some time to have a chance to get the events
    Sleep(5000);

    (void)printf("Stopping trace session %s\r\n", trace_session_name);
    POOR_MANS_ASSERT(StopTraceA(trace_session_handle, NULL, &stop_event_trace_property_data.props) == ERROR_SUCCESS);
}

static void parse_events(const char* log_file_name)
{
    EVENT_TRACE_LOGFILEA log_file = { 0 };

    parsed_event_count = 0;

    log_file.ProcessTraceMode = PROCESS_TRACE_MODE_EVENT_RECORD;
    log_file.EventRecordCallback = event_trace_record_callback;
    log_file.BufferCallback = event_trace_buffer_callback;
    log_file.LogFileName = (char*)log_file_name;
    log_file.Context = NULL;

    TRACEHANDLE trace = OpenTraceA(&log_file);
    POOR_MANS_ASSERT(trace != INVALID_PROCESSTRACE_HANDLE);

    // open the trace and process all events
    ULONG process_trace_result = ProcessTrace(&trace, 1, NULL, NULL);
    POOR_MANS_ASSERT(process_trace_result == ERROR_SUCCESS);

    // cleanup
    (void)CloseTrace(trace);
}

static void log_sink_etw_log_with_LOG_LEVEL_ERROR_succeeds(void)
{
    // arrange
    char trace_session_name[128];
    char log_file_name[MAX_PATH];

    generate_trace_session_and_file_name(trace_session_name, sizeof(trace_session_name), log_file_name, sizeof(log_file_name));

    TRACEHANDLE trace_session_handle = start_trace(trace_session_name, log_file_name, TRACE_LEVEL_VERBOSE);

    int captured_line = __LINE__;

    // act
    log_sink_etw.log_sink_log(LOG_LEVEL_ERROR, NULL, __FILE__, __FUNCTION__, captured_line, "test");

    stop_trace(trace_session_handle, trace_session_name);

    // assert
    parse_events(log_file_name);

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
    int snprintf_result = sprintf(delete_command, "del %s", log_file_name);
    POOR_MANS_ASSERT((snprintf_result > 0) && (snprintf_result < sizeof(delete_command)));
    (void)system(delete_command);
}

static void log_sink_etw_log_all_levels_when_all_levels_enabled_succeeds(void)
{
    // arrange
    char trace_session_name[128];
    char log_file_name[MAX_PATH];

    generate_trace_session_and_file_name(trace_session_name, sizeof(trace_session_name), log_file_name, sizeof(log_file_name));

    TRACEHANDLE trace_session_handle = start_trace(trace_session_name, log_file_name, TRACE_LEVEL_VERBOSE);

    int captured_line = __LINE__;

    // act
    log_sink_etw.log_sink_log(LOG_LEVEL_CRITICAL, NULL, __FILE__, __FUNCTION__, captured_line, "test_critical");
    log_sink_etw.log_sink_log(LOG_LEVEL_ERROR, NULL, __FILE__, __FUNCTION__, captured_line + 1, "test_error");
    log_sink_etw.log_sink_log(LOG_LEVEL_WARNING, NULL, __FILE__, __FUNCTION__, captured_line + 2, "test_warning");
    log_sink_etw.log_sink_log(LOG_LEVEL_INFO, NULL, __FILE__, __FUNCTION__, captured_line + 3, "test_info");
    log_sink_etw.log_sink_log(LOG_LEVEL_VERBOSE, NULL, __FILE__, __FUNCTION__, captured_line + 4, "test_verbose");

    // assert
    stop_trace(trace_session_handle, trace_session_name);

    parse_events(log_file_name);

    // 5 events expected: all actual events
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
    int snprintf_result = sprintf(delete_command, "del %s", log_file_name);
    POOR_MANS_ASSERT((snprintf_result > 0) && (snprintf_result < sizeof(delete_command)));
    (void)system(delete_command);
}

static void log_sink_etw_log_each_individual_level(void)
{
    // arrange
    for (LOG_LEVEL event_log_level = LOG_LEVEL_CRITICAL; event_log_level <= LOG_LEVEL_VERBOSE; event_log_level++)
    {
        for (LOG_LEVEL trace_level = TRACE_LEVEL_CRITICAL; trace_level <= TRACE_LEVEL_VERBOSE; trace_level++)
        {
            char trace_session_name[128];
            char log_file_name[MAX_PATH];

            generate_trace_session_and_file_name(trace_session_name, sizeof(trace_session_name), log_file_name, sizeof(log_file_name));

            TRACEHANDLE trace_session_handle = start_trace(trace_session_name, log_file_name, trace_level);

            int captured_line = __LINE__;

            // act

            log_sink_etw.log_sink_log(event_log_level, NULL, __FILE__, __FUNCTION__, captured_line, "test_event");

            // assert
            stop_trace(trace_session_handle, trace_session_name);

            parse_events(log_file_name);

            uint8_t expected_event_count = (trace_level - TRACE_LEVEL_CRITICAL) >= (event_log_level - LOG_LEVEL_CRITICAL) ? 1 : 0;
            POOR_MANS_ASSERT(parsed_event_count == expected_event_count);

            if (expected_event_count > 0)
            {
                const char* expected_event_name;
                switch (event_log_level)
                {
                default:
                    expected_event_name = NULL;
                    break;

                case LOG_LEVEL_CRITICAL:
                    expected_event_name = "LogCritical";
                    break;
                case LOG_LEVEL_ERROR:
                    expected_event_name = "LogError";
                    break;
                case LOG_LEVEL_WARNING:
                    expected_event_name = "LogWarning";
                    break;
                case LOG_LEVEL_INFO:
                    expected_event_name = "LogInfo";
                    break;
                case LOG_LEVEL_VERBOSE:
                    expected_event_name = "LogVerbose";
                    break;
                }

                POOR_MANS_ASSERT(strcmp(parsed_events[0].event_name, expected_event_name) == 0);
                POOR_MANS_ASSERT(strcmp(parsed_events[0].content, "test_event") == 0);
                POOR_MANS_ASSERT(strcmp(parsed_events[0].file, __FILE__) == 0);
                POOR_MANS_ASSERT(strcmp(parsed_events[0].func, __FUNCTION__) == 0);
                POOR_MANS_ASSERT(parsed_events[0].line == captured_line);
            }

            // delete the trace file if created
            // allocate enough
            char delete_command[MAX_PATH + 4];
            int snprintf_result = sprintf(delete_command, "del %s", log_file_name);
            POOR_MANS_ASSERT((snprintf_result > 0) && (snprintf_result < sizeof(delete_command)));
            (void)system(delete_command);
        }
    }
}

static void log_sink_etw_log_with_context_with_properties(void)
{
    // arrange
    char trace_session_name[128];
    char log_file_name[MAX_PATH];

    generate_trace_session_and_file_name(trace_session_name, sizeof(trace_session_name), log_file_name, sizeof(log_file_name));

    TRACEHANDLE trace_session_handle = start_trace(trace_session_name, log_file_name, TRACE_LEVEL_VERBOSE);

    int captured_line = __LINE__;

    LOG_CONTEXT_HANDLE log_context;
    LOG_CONTEXT_CREATE(log_context, NULL,
        LOG_CONTEXT_STRING_PROPERTY(gigi, "duru"),
        LOG_CONTEXT_PROPERTY(int8_t, prop1, 42),
        LOG_CONTEXT_PROPERTY(uint8_t, prop2, 43),
        LOG_CONTEXT_PROPERTY(int16_t, prop3, 44),
        LOG_CONTEXT_PROPERTY(uint16_t, prop4, 45),
        LOG_CONTEXT_PROPERTY(int32_t, prop5, 46),
        LOG_CONTEXT_PROPERTY(uint32_t, prop6, 47),
        LOG_CONTEXT_PROPERTY(int64_t, prop7, 48),
        LOG_CONTEXT_PROPERTY(uint64_t, prop8, 49)
    );

    // act
    log_sink_etw.log_sink_log(LOG_LEVEL_CRITICAL, log_context, __FILE__, __FUNCTION__, captured_line, "test_with_context");

    // assert
    stop_trace(trace_session_handle, trace_session_name);

    parse_events(log_file_name);

    // 1 event expected which has att the properties
    POOR_MANS_ASSERT(parsed_event_count == 1);

    POOR_MANS_ASSERT(strcmp(parsed_events[0].event_name, "LogCritical") == 0);
    POOR_MANS_ASSERT(strcmp(parsed_events[0].content, "test_with_context") == 0);
    POOR_MANS_ASSERT(strcmp(parsed_events[0].file, __FILE__) == 0);
    POOR_MANS_ASSERT(strcmp(parsed_events[0].func, __FUNCTION__) == 0);
    POOR_MANS_ASSERT(parsed_events[0].line == captured_line);

    POOR_MANS_ASSERT(parsed_events[0].property_count == 10);
    POOR_MANS_ASSERT(parsed_events[0].properties[0].property_type == (_TlgInSTRUCT | _TlgInChain));
    POOR_MANS_ASSERT(parsed_events[0].properties[0].struct_field_count == 9);
    POOR_MANS_ASSERT(strcmp(parsed_events[0].properties[0].property_name, "") == 0);

    POOR_MANS_ASSERT(parsed_events[0].properties[1].property_type == TlgInANSISTRING);
    POOR_MANS_ASSERT(strcmp(parsed_events[0].properties[1].ascii_char_ptr_value, "duru") == 0);
    POOR_MANS_ASSERT(strcmp(parsed_events[0].properties[1].property_name, "gigi") == 0);
    POOR_MANS_ASSERT(parsed_events[0].properties[2].property_type == TlgInINT8);
    POOR_MANS_ASSERT(parsed_events[0].properties[2].int8_t_value == 42);
    POOR_MANS_ASSERT(strcmp(parsed_events[0].properties[2].property_name, "prop1") == 0);
    POOR_MANS_ASSERT(parsed_events[0].properties[3].property_type == TlgInUINT8);
    POOR_MANS_ASSERT(parsed_events[0].properties[3].uint8_t_value == 43);
    POOR_MANS_ASSERT(strcmp(parsed_events[0].properties[3].property_name, "prop2") == 0);
    POOR_MANS_ASSERT(parsed_events[0].properties[4].property_type == TlgInINT16);
    POOR_MANS_ASSERT(parsed_events[0].properties[4].int16_t_value == 44);
    POOR_MANS_ASSERT(strcmp(parsed_events[0].properties[4].property_name, "prop3") == 0);
    POOR_MANS_ASSERT(parsed_events[0].properties[5].property_type == TlgInUINT16);
    POOR_MANS_ASSERT(parsed_events[0].properties[5].uint16_t_value == 45);
    POOR_MANS_ASSERT(strcmp(parsed_events[0].properties[5].property_name, "prop4") == 0);
    POOR_MANS_ASSERT(parsed_events[0].properties[6].property_type == TlgInINT32);
    POOR_MANS_ASSERT(parsed_events[0].properties[6].int32_t_value == 46);
    POOR_MANS_ASSERT(strcmp(parsed_events[0].properties[6].property_name, "prop5") == 0);
    POOR_MANS_ASSERT(parsed_events[0].properties[7].property_type == TlgInUINT32);
    POOR_MANS_ASSERT(parsed_events[0].properties[7].uint32_t_value == 47);
    POOR_MANS_ASSERT(strcmp(parsed_events[0].properties[7].property_name, "prop6") == 0);
    POOR_MANS_ASSERT(parsed_events[0].properties[8].property_type == TlgInINT64);
    POOR_MANS_ASSERT(parsed_events[0].properties[8].int64_t_value == 48);
    POOR_MANS_ASSERT(strcmp(parsed_events[0].properties[8].property_name, "prop7") == 0);
    POOR_MANS_ASSERT(parsed_events[0].properties[9].property_type == TlgInUINT64);
    POOR_MANS_ASSERT(parsed_events[0].properties[9].uint64_t_value == 49);
    POOR_MANS_ASSERT(strcmp(parsed_events[0].properties[9].property_name, "prop8") == 0);

    // delete the trace file if created
    // allocate enough
    char delete_command[MAX_PATH + 4];
    int snprintf_result = sprintf(delete_command, "del %s", log_file_name);
    POOR_MANS_ASSERT((snprintf_result > 0) && (snprintf_result < sizeof(delete_command)));
    (void)system(delete_command);

    LOG_CONTEXT_DESTROY(log_context);
}

static void log_sink_etw_log_with_context_with_nested_structs(void)
{
    // arrange
    char trace_session_name[128];
    char log_file_name[MAX_PATH];

    generate_trace_session_and_file_name(trace_session_name, sizeof(trace_session_name), log_file_name, sizeof(log_file_name));

    TRACEHANDLE trace_session_handle = start_trace(trace_session_name, log_file_name, TRACE_LEVEL_VERBOSE);

    int captured_line = __LINE__;

    LOG_CONTEXT_HANDLE log_context_1;
    LOG_CONTEXT_CREATE(log_context_1, NULL,
        LOG_CONTEXT_PROPERTY(int8_t, prop1, 42)
    );

    LOG_CONTEXT_HANDLE log_context_2;
    LOG_CONTEXT_CREATE(log_context_2, log_context_1,
        LOG_CONTEXT_PROPERTY(uint8_t, prop2, 43)
    );

    // act
    log_sink_etw.log_sink_log(LOG_LEVEL_CRITICAL, log_context_2, __FILE__, __FUNCTION__, captured_line, "test_with_nested");

    // assert
    stop_trace(trace_session_handle, trace_session_name);

    parse_events(log_file_name);

    // 1 event expected which has att the properties
    POOR_MANS_ASSERT(parsed_event_count == 1);

    POOR_MANS_ASSERT(strcmp(parsed_events[0].event_name, "LogCritical") == 0);
    POOR_MANS_ASSERT(strcmp(parsed_events[0].content, "test_with_nested") == 0);
    POOR_MANS_ASSERT(strcmp(parsed_events[0].file, __FILE__) == 0);
    POOR_MANS_ASSERT(strcmp(parsed_events[0].func, __FUNCTION__) == 0);
    POOR_MANS_ASSERT(parsed_events[0].line == captured_line);

    POOR_MANS_ASSERT(parsed_events[0].property_count == 4);
    POOR_MANS_ASSERT(parsed_events[0].properties[0].property_type == (_TlgInSTRUCT | _TlgInChain));
    POOR_MANS_ASSERT(parsed_events[0].properties[0].struct_field_count == 3);
    POOR_MANS_ASSERT(strcmp(parsed_events[0].properties[0].property_name, "") == 0);

    // log_context_1 properties
    POOR_MANS_ASSERT(parsed_events[0].properties[1].property_type == (_TlgInSTRUCT | _TlgInChain));
    POOR_MANS_ASSERT(parsed_events[0].properties[1].struct_field_count == 1);
    POOR_MANS_ASSERT(strcmp(parsed_events[0].properties[1].property_name, "") == 0);

    POOR_MANS_ASSERT(parsed_events[0].properties[2].property_type == TlgInINT8);
    POOR_MANS_ASSERT(parsed_events[0].properties[2].uint8_t_value == 42);
    POOR_MANS_ASSERT(strcmp(parsed_events[0].properties[2].property_name, "prop1") == 0);

    // log_context_2 properties
    POOR_MANS_ASSERT(parsed_events[0].properties[3].property_type == TlgInUINT8);
    POOR_MANS_ASSERT(parsed_events[0].properties[3].uint8_t_value == 43);
    POOR_MANS_ASSERT(strcmp(parsed_events[0].properties[3].property_name, "prop2") == 0);

    // delete the trace file if created
    // allocate enough
    char delete_command[MAX_PATH + 4];
    int snprintf_result = sprintf(delete_command, "del %s", log_file_name);
    POOR_MANS_ASSERT((snprintf_result > 0) && (snprintf_result < sizeof(delete_command)));
    (void)system(delete_command);

    LOG_CONTEXT_DESTROY(log_context_2);
    LOG_CONTEXT_DESTROY(log_context_1);
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
        log_sink_etw_log_each_individual_level();
        log_sink_etw_log_with_context_with_properties();
        log_sink_etw_log_with_context_with_nested_structs();

        result = (int)asserts_failed;
    }

    return result;
}

