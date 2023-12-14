// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <wchar.h>

#include "windows.h"
#include "TraceLoggingProvider.h"
#include "evntrace.h"
#include "evntcons.h"

#include "macro_utils/macro_utils.h"

#include "c_logging/log_context_property_basic_types.h"
#include "c_logging/log_context_property_bool_type.h"
#include "c_logging/log_context_property_type_ascii_char_ptr.h"
#include "c_logging/log_context_property_type_wchar_t_ptr.h"
#include "c_logging/log_context_property_value_pair.h"
#include "c_logging/log_level.h"
#include "c_logging/log_sink_if.h"
#include "c_logging/log_context.h"
#include "c_logging/logger.h"

#include "c_logging/log_sink_etw.h"

// helper functions to avoid reading unaligned fields
static void read_uint8_t(const void* source, uint8_t* destination)
{
    *destination = *(const uint8_t*)source;
}

static void read_int8_t(const void* source, int8_t* destination)
{
    *destination = *(const int8_t*)source;
}

static void read_uint16_t(const void* source, uint16_t* destination)
{
    (void)memcpy(destination, source, sizeof(uint16_t));
}

static void read_int16_t(const void* source, int16_t* destination)
{
    (void)memcpy(destination, source, sizeof(int16_t));
}

static void read_uint32_t(const void* source, uint32_t* destination)
{
    (void)memcpy(destination, source, sizeof(uint32_t));
}

static void read_int32_t(const void* source, int32_t* destination)
{
    (void)memcpy(destination, source, sizeof(int32_t));
}

static void read_uint64_t(const void* source, uint64_t* destination)
{
    (void)memcpy(destination, source, sizeof(uint64_t));
}

static void read_int64_t(const void* source, int64_t* destination)
{
    (void)memcpy(destination, source, sizeof(int64_t));
}

static void read_bool(const void* source, int32_t* destination)
{
    (void)memcpy(destination, source, sizeof(int32_t));
}

static GUID provider_guid = { 0xDAD29F36, 0x0A48, 0x4DEF, { 0x9D, 0x50, 0x8E, 0xF9, 0x03, 0x6B, 0x92, 0xB4 } };

MU_DEFINE_ENUM_STRINGS(LOG_CONTEXT_PROPERTY_TYPE, LOG_CONTEXT_PROPERTY_TYPE_VALUES)

#define POOR_MANS_ASSERT(cond) \
    if (!(cond)) \
    { \
        (void)printf("%s:%d test failed\r\n", __FUNCTION__, __LINE__); \
        abort(); \
    } \

typedef struct EVENT_TRACE_PROPERTY_DATA_TAG
{
    EVENT_TRACE_PROPERTIES props;
    char logger_name[128];
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
        wchar_t wchar_t_ptr_value[MAX_STRING_VALUE_LENGTH];
        uint8_t uint8_t_value;
        int8_t int8_t_value;
        uint16_t uint16_t_value;
        int16_t int16_t_value;
        uint32_t uint32_t_value;
        int32_t int32_t_value;
        uint64_t uint64_t_value;
        int64_t int64_t_value;
        int32_t bool_value;
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

typedef struct TEST_CONTEXT_TAG
{
    volatile LONG parsed_event_count;
    volatile LONG parsed_event_index;
    PARSED_EVENT parsed_events[MAX_EVENTS];
    HANDLE process_thread_handle;
    TRACEHANDLE trace;
    char logger_name[128];
    char trace_session_name[128];
} TEST_CONTEXT;

static TEST_CONTEXT* test_context_create(void)
{
    TEST_CONTEXT* result = malloc(sizeof(TEST_CONTEXT));
    POOR_MANS_ASSERT(result != NULL);

    return result;
}

static void test_context_destroy(TEST_CONTEXT* test_context)
{
    free(test_context);
}

// This callback is needed
// Without this callback events are not properly processed
static ULONG WINAPI event_trace_buffer_callback(PEVENT_TRACE_LOGFILEA log_file)
{
    (void)log_file;

    return TRUE;
}

static void copy_string(char* destination, size_t destination_size, const char* source, size_t source_length)
{
    if (source_length >= destination_size)
    {
        source_length = destination_size - 1;
    }
    (void)memcpy(destination, source, source_length);
    destination[source_length] = '\0';
}

static void copy_wstring(wchar_t* destination, size_t destination_size, const wchar_t* source, size_t source_length)
{
    if (source_length >= destination_size)
    {
        source_length = destination_size - 2;
    }
    (void)memcpy(destination, source, source_length);
    destination[source_length] = L'\0';
}

static void WINAPI event_trace_record_callback(EVENT_RECORD* pEventRecord)
{
    if (IsEqualGUID(&pEventRecord->EventHeader.ProviderId, &provider_guid))
    {
        TEST_CONTEXT* test_context = pEventRecord->UserContext;

        const char* metadata = NULL;

        LONG current_index = InterlockedIncrement(&test_context->parsed_event_index);
        (void)memset(&test_context->parsed_events[current_index], 0, sizeof(PARSED_EVENT));

        EVENT_HEADER_EXTENDED_DATA_ITEM* extended_data_items = pEventRecord->ExtendedData;

        for (unsigned short i = 0; i < pEventRecord->ExtendedDataCount; i++)
        {
            if (extended_data_items[i].ExtType == EVENT_HEADER_EXT_TYPE_EVENT_SCHEMA_TL)
            {
                // This one has the event name
                // Documented in TraceLoggingProvider.h in the structure of TraceLogging that we should skip 4 bytes
                const char* event_name = (const char*)(uintptr_t)(extended_data_items[i].DataPtr + 4);

                if (event_name != NULL)
                {
                    size_t event_name_length = strlen(event_name);
                    copy_string(test_context->parsed_events[current_index].event_name, sizeof(test_context->parsed_events[current_index].event_name), event_name, event_name_length);
                    metadata = event_name + event_name_length + 1;
                }

                break;
            }
        }

        POOR_MANS_ASSERT(pEventRecord->UserData != NULL);

        const char* current_user_data_field = pEventRecord->UserData;

        size_t content_length = strlen(current_user_data_field);
        copy_string(test_context->parsed_events[current_index].content, sizeof(test_context->parsed_events[current_index].content), current_user_data_field, content_length);
        current_user_data_field += content_length + 1;

        POOR_MANS_ASSERT(strcmp(metadata, "content") == 0);
        // sizeof the string and 1 byte for the type
        metadata += sizeof("content") + 1;

        size_t file_length = strlen(current_user_data_field);
        copy_string(test_context->parsed_events[current_index].file, sizeof(test_context->parsed_events[current_index].file), current_user_data_field, file_length);
        current_user_data_field += file_length + 1;

        POOR_MANS_ASSERT(strcmp(metadata, "file") == 0);
        // sizeof the string and 1 byte for the type
        metadata += sizeof("file") + 1;

        size_t func_length = strlen(current_user_data_field);
        copy_string(test_context->parsed_events[current_index].func, sizeof(test_context->parsed_events[current_index].func), current_user_data_field, func_length);
        current_user_data_field += func_length + 1;

        POOR_MANS_ASSERT(strcmp(metadata, "func") == 0);
        // sizeof the string and 1 byte for the type
        metadata += sizeof("func") + 1;

        test_context->parsed_events[current_index].line = *(int32_t*)current_user_data_field;
        current_user_data_field+= sizeof(int32_t);

        POOR_MANS_ASSERT(strcmp(metadata, "line") == 0);
        // sizeof the string and 1 byte for the type
        metadata += sizeof("line") + 1;

        uint32_t property_index = 0;

        // get all other properties
        while (current_user_data_field - (const char*)pEventRecord->UserData < pEventRecord->UserDataLength)
        {
            // get name and type from metadata
            size_t property_name_length = strlen(metadata);
            copy_string(test_context->parsed_events[current_index].properties[property_index].property_name, sizeof(test_context->parsed_events[current_index].properties[property_index].property_name), metadata, property_name_length);
            metadata += property_name_length + 1;

            test_context->parsed_events[current_index].properties[property_index].property_type = *metadata;
            metadata++;

            switch (test_context->parsed_events[current_index].properties[property_index].property_type)
            {
            default:
                POOR_MANS_ASSERT(0);
                break;
            case _TlgInSTRUCT | _TlgInChain:
                read_uint8_t(metadata, &test_context->parsed_events[current_index].properties[property_index].struct_field_count);
                metadata++;
                break;
            case TlgInANSISTRING:
            {
                size_t property_string_value_length = strlen(current_user_data_field);
                copy_string(test_context->parsed_events[current_index].properties[property_index].ascii_char_ptr_value, sizeof(test_context->parsed_events[current_index].properties[property_index].ascii_char_ptr_value), current_user_data_field, property_string_value_length);
                current_user_data_field += property_string_value_length + 1;
                break;
            }
            case TlgInUNICODESTRING:
            {
                size_t property_string_value_length = wcslen((wchar_t*)current_user_data_field)*2;
                copy_wstring(test_context->parsed_events[current_index].properties[property_index].wchar_t_ptr_value, sizeof(test_context->parsed_events[current_index].properties[property_index].wchar_t_ptr_value), (wchar_t*)current_user_data_field, property_string_value_length);
                current_user_data_field += property_string_value_length + 2;
                break;
            }
            case TlgInUINT8:
                read_uint8_t(current_user_data_field, &test_context->parsed_events[current_index].properties[property_index].uint8_t_value);
                current_user_data_field += sizeof(uint8_t);
                break;
            case TlgInINT8:
                read_int8_t(current_user_data_field, &test_context->parsed_events[current_index].properties[property_index].int8_t_value);
                current_user_data_field += sizeof(int8_t);
                break;
            case TlgInUINT16:
                read_uint16_t(current_user_data_field, &test_context->parsed_events[current_index].properties[property_index].uint16_t_value);
                current_user_data_field += sizeof(uint16_t);
                break;
            case TlgInINT16:
                read_int16_t(current_user_data_field, &test_context->parsed_events[current_index].properties[property_index].int16_t_value);
                current_user_data_field += sizeof(int16_t);
                break;
            case TlgInUINT32:
                read_uint32_t(current_user_data_field, &test_context->parsed_events[current_index].properties[property_index].uint32_t_value);
                current_user_data_field += sizeof(uint32_t);
                break;
            case TlgInINT32:
                read_int32_t(current_user_data_field, &test_context->parsed_events[current_index].properties[property_index].int32_t_value);
                current_user_data_field += sizeof(int32_t);
                break;
            case TlgInUINT64:
                read_uint64_t(current_user_data_field, &test_context->parsed_events[current_index].properties[property_index].uint64_t_value);
                current_user_data_field += sizeof(uint64_t);
                break;
            case TlgInINT64:
                read_int64_t(current_user_data_field, &test_context->parsed_events[current_index].properties[property_index].int64_t_value);
                current_user_data_field += sizeof(int64_t);
                break;
            case TlgInBOOL32:
                read_bool(current_user_data_field, &test_context->parsed_events[current_index].properties[property_index].bool_value);
                current_user_data_field += sizeof(int32_t);
                break;
            }

            property_index++;
        }

        test_context->parsed_events[current_index].property_count = property_index;

        (void)InterlockedIncrement(&test_context->parsed_event_count);
        WakeByAddressAll((void*)&test_context->parsed_event_count);
    }
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

    size_t logger_name_length = strlen(start_event_trace_property_data.logger_name);
    POOR_MANS_ASSERT(logger_name_length < sizeof(test_context->logger_name));
    (void)memcpy(test_context->logger_name, start_event_trace_property_data.logger_name, logger_name_length + 1);

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

static DWORD WINAPI process_trace_thread_func(void* arg)
{
    TEST_CONTEXT* test_context = arg;

    // open the trace and process all events
    ULONG process_trace_result = ProcessTrace(&test_context->trace, 1, NULL, NULL);
    POOR_MANS_ASSERT(process_trace_result == ERROR_SUCCESS);

    return 0;
}

#define TEST_PARSE_START_DELAY 5000 // ms
#define TEST_WAITTIME_NO_EVENT 20000 // ms

static void start_parse_events(TEST_CONTEXT* test_context)
{
    EVENT_TRACE_LOGFILEA log_file = { 0 };

    (void)InterlockedExchange(&test_context->parsed_event_index, -1);
    (void)InterlockedExchange(&test_context->parsed_event_count, 0);

    log_file.ProcessTraceMode = PROCESS_TRACE_MODE_REAL_TIME | PROCESS_TRACE_MODE_EVENT_RECORD;
    log_file.EventRecordCallback = event_trace_record_callback;
    log_file.BufferCallback = event_trace_buffer_callback;
    log_file.LoggerName = test_context->logger_name;
    log_file.Context = test_context;

    test_context->trace = OpenTraceA(&log_file);
    POOR_MANS_ASSERT(test_context->trace != INVALID_PROCESSTRACE_HANDLE);

    DWORD thread_id;
    test_context->process_thread_handle = CreateThread(NULL, 0, process_trace_thread_func, test_context, 0, &thread_id);
    POOR_MANS_ASSERT(test_context->process_thread_handle != NULL);

    // Give the thread some time to start
    Sleep(TEST_PARSE_START_DELAY);
}

static void stop_parse_events(TEST_CONTEXT* test_context)
{
    POOR_MANS_ASSERT(WaitForSingleObject(test_context->process_thread_handle, INFINITE) == WAIT_OBJECT_0);

    (void)CloseHandle(test_context->process_thread_handle);
    (void)CloseTrace(test_context->trace);
}

static void wait_for_event_count(TEST_CONTEXT* test_context, int32_t expected_event_count)
{
    do
    {
        int32_t current_index = InterlockedAdd(&test_context->parsed_event_count, 0);
        if (current_index == expected_event_count)
        {
            break;
        }

        (void)WaitOnAddress(&test_context->parsed_event_count, &current_index, sizeof(int32_t), INFINITE);
    } while (1);
}

static void test_log_sink_etw_log(LOG_LEVEL log_level, LOG_CONTEXT_HANDLE log_context, const char* file, const char* func, int line, const char* message_format, ...)
{
    va_list args;
    va_start(args, message_format);
    log_sink_etw.log(log_level, log_context, file, func, line, message_format, args);
    va_end(args);
}

// This test is a bit special because it is the first event and thus the time the provider gets registered.
// So instead of expecting 1 event we'll be expecting a self test event and the actual logging event.
static void log_sink_etw_log_with_LOG_LEVEL_ERROR_succeeds(void)
{
    // arrange
    TEST_CONTEXT* test_context = test_context_create();

    generate_trace_session(test_context, __FUNCTION__);

    TRACEHANDLE trace_session_handle = start_trace(test_context, TRACE_LEVEL_VERBOSE);

    int captured_line = __LINE__;

    start_parse_events(test_context);

    // act
    test_log_sink_etw_log(LOG_LEVEL_ERROR, NULL, __FILE__, __FUNCTION__, captured_line, "test");

    // assert

    // 1 events expected
    wait_for_event_count(test_context, 1);

    stop_trace(test_context, trace_session_handle);
    stop_parse_events(test_context);

    // error event
    POOR_MANS_ASSERT(strcmp(test_context->parsed_events[0].event_name, "LogError") == 0);
    POOR_MANS_ASSERT(strcmp(test_context->parsed_events[0].content, "test") == 0);
    POOR_MANS_ASSERT(strcmp(test_context->parsed_events[0].file, __FILE__) == 0);
    POOR_MANS_ASSERT(strcmp(test_context->parsed_events[0].func, __FUNCTION__) == 0);
    POOR_MANS_ASSERT(test_context->parsed_events[0].line == captured_line);

    test_context_destroy(test_context);
}

static void log_sink_etw_log_all_levels_when_all_levels_enabled_succeeds(void)
{
    // arrange
    TEST_CONTEXT* test_context = test_context_create();

    generate_trace_session(test_context, __FUNCTION__);

    TRACEHANDLE trace_session_handle = start_trace(test_context, TRACE_LEVEL_VERBOSE);

    int captured_line = __LINE__;

    start_parse_events(test_context);

    // act
    test_log_sink_etw_log(LOG_LEVEL_CRITICAL, NULL, __FILE__, __FUNCTION__, captured_line, "test_critical");
    test_log_sink_etw_log(LOG_LEVEL_ERROR, NULL, __FILE__, __FUNCTION__, captured_line + 1, "test_error");
    test_log_sink_etw_log(LOG_LEVEL_WARNING, NULL, __FILE__, __FUNCTION__, captured_line + 2, "test_warning");
    test_log_sink_etw_log(LOG_LEVEL_INFO, NULL, __FILE__, __FUNCTION__, captured_line + 3, "test_info");
    test_log_sink_etw_log(LOG_LEVEL_VERBOSE, NULL, __FILE__, __FUNCTION__, captured_line + 4, "test_verbose");

    // assert

    // 5 events expected: all actual events
    wait_for_event_count(test_context, 5);

    stop_trace(test_context, trace_session_handle);
    stop_parse_events(test_context);

    // critical event
    POOR_MANS_ASSERT(strcmp(test_context->parsed_events[0].event_name, "LogCritical") == 0);
    POOR_MANS_ASSERT(strcmp(test_context->parsed_events[0].content, "test_critical") == 0);
    POOR_MANS_ASSERT(strcmp(test_context->parsed_events[0].file, __FILE__) == 0);
    POOR_MANS_ASSERT(strcmp(test_context->parsed_events[0].func, __FUNCTION__) == 0);
    POOR_MANS_ASSERT(test_context->parsed_events[0].line == captured_line);

    // error event
    POOR_MANS_ASSERT(strcmp(test_context->parsed_events[1].event_name, "LogError") == 0);
    POOR_MANS_ASSERT(strcmp(test_context->parsed_events[1].content, "test_error") == 0);
    POOR_MANS_ASSERT(strcmp(test_context->parsed_events[1].file, __FILE__) == 0);
    POOR_MANS_ASSERT(strcmp(test_context->parsed_events[1].func, __FUNCTION__) == 0);
    POOR_MANS_ASSERT(test_context->parsed_events[1].line == captured_line + 1);

    // warning event
    POOR_MANS_ASSERT(strcmp(test_context->parsed_events[2].event_name, "LogWarning") == 0);
    POOR_MANS_ASSERT(strcmp(test_context->parsed_events[2].content, "test_warning") == 0);
    POOR_MANS_ASSERT(strcmp(test_context->parsed_events[2].file, __FILE__) == 0);
    POOR_MANS_ASSERT(strcmp(test_context->parsed_events[2].func, __FUNCTION__) == 0);
    POOR_MANS_ASSERT(test_context->parsed_events[2].line == captured_line + 2);

    // info event
    POOR_MANS_ASSERT(strcmp(test_context->parsed_events[3].event_name, "LogInfo") == 0);
    POOR_MANS_ASSERT(strcmp(test_context->parsed_events[3].content, "test_info") == 0);
    POOR_MANS_ASSERT(strcmp(test_context->parsed_events[3].file, __FILE__) == 0);
    POOR_MANS_ASSERT(strcmp(test_context->parsed_events[3].func, __FUNCTION__) == 0);
    POOR_MANS_ASSERT(test_context->parsed_events[3].line == captured_line + 3);

    // verbose event
    POOR_MANS_ASSERT(strcmp(test_context->parsed_events[4].event_name, "LogVerbose") == 0);
    POOR_MANS_ASSERT(strcmp(test_context->parsed_events[4].content, "test_verbose") == 0);
    POOR_MANS_ASSERT(strcmp(test_context->parsed_events[4].file, __FILE__) == 0);
    POOR_MANS_ASSERT(strcmp(test_context->parsed_events[4].func, __FUNCTION__) == 0);
    POOR_MANS_ASSERT(test_context->parsed_events[4].line == captured_line + 4);

    test_context_destroy(test_context);
}

static void log_sink_etw_log_each_individual_level(void)
{
    // arrange
    for (LOG_LEVEL event_log_level = LOG_LEVEL_CRITICAL; event_log_level <= LOG_LEVEL_VERBOSE; event_log_level++)
    {
        for (LOG_LEVEL trace_level = TRACE_LEVEL_CRITICAL; trace_level <= TRACE_LEVEL_VERBOSE; trace_level++)
        {
            TEST_CONTEXT* test_context = test_context_create();

            generate_trace_session(test_context, __FUNCTION__);

            TRACEHANDLE trace_session_handle = start_trace(test_context, trace_level);

            int captured_line = __LINE__;
            uint8_t expected_event_count = (trace_level - TRACE_LEVEL_CRITICAL) >= (event_log_level - LOG_LEVEL_CRITICAL) ? 1 : 0;

            start_parse_events(test_context);

            // act
            test_log_sink_etw_log(event_log_level, NULL, __FILE__, __FUNCTION__, captured_line, "test_event");

            // assert
            wait_for_event_count(test_context, expected_event_count);

            // if we are not expecting an event, wait for some time just to make sure no event is seen
            if (expected_event_count == 0)
            {
                (void)printf("Waiting for %.02f seconds to make sure no events are published ...", (double)TEST_WAITTIME_NO_EVENT / 1000);
                Sleep(TEST_WAITTIME_NO_EVENT);
            }

            stop_trace(test_context, trace_session_handle);
            stop_parse_events(test_context);

            POOR_MANS_ASSERT(test_context->parsed_event_count == expected_event_count);

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

                POOR_MANS_ASSERT(strcmp(test_context->parsed_events[0].event_name, expected_event_name) == 0);
                POOR_MANS_ASSERT(strcmp(test_context->parsed_events[0].content, "test_event") == 0);
                POOR_MANS_ASSERT(strcmp(test_context->parsed_events[0].file, __FILE__) == 0);
                POOR_MANS_ASSERT(strcmp(test_context->parsed_events[0].func, __FUNCTION__) == 0);
                POOR_MANS_ASSERT(test_context->parsed_events[0].line == captured_line);
            }

            test_context_destroy(test_context);
        }
    }
}

static void log_sink_etw_log_with_context_with_properties(void)
{
    // arrange
    TEST_CONTEXT* test_context = test_context_create();

    generate_trace_session(test_context, __FUNCTION__);

    TRACEHANDLE trace_session_handle = start_trace(test_context, TRACE_LEVEL_VERBOSE);

    int captured_line = __LINE__;

    LOG_CONTEXT_HANDLE log_context;
    LOG_CONTEXT_CREATE(log_context, NULL,
        LOG_CONTEXT_STRING_PROPERTY(gigi, "duru"),
        LOG_CONTEXT_PROPERTY(int8_t, prop1, 0x10),
        LOG_CONTEXT_PROPERTY(uint8_t, prop2, 0x20),
        LOG_CONTEXT_PROPERTY(int16_t, prop3, 0x3031),
        LOG_CONTEXT_PROPERTY(uint16_t, prop4, 0x4041),
        LOG_CONTEXT_PROPERTY(int32_t, prop5, 0x50515253),
        LOG_CONTEXT_PROPERTY(uint32_t, prop6, 0x60616263),
        LOG_CONTEXT_PROPERTY(int64_t, prop7, 0x7071727374757677),
        LOG_CONTEXT_PROPERTY(uint64_t, prop8, 0x8081828384858687),
        LOG_CONTEXT_PROPERTY(bool, prop9, true),
        LOG_CONTEXT_PROPERTY(bool, prop10, false),
        LOG_CONTEXT_WSTRING_PROPERTY(wstring, L"duru")
    );

    start_parse_events(test_context);

    // act
    test_log_sink_etw_log(LOG_LEVEL_CRITICAL, log_context, __FILE__, __FUNCTION__, captured_line, "test_with_context");

    // assert

    // 1 event expected which has att the properties
    wait_for_event_count(test_context, 1);

    stop_trace(test_context, trace_session_handle);
    stop_parse_events(test_context);

    POOR_MANS_ASSERT(strcmp(test_context->parsed_events[0].event_name, "LogCritical") == 0);
    POOR_MANS_ASSERT(strcmp(test_context->parsed_events[0].content, "test_with_context") == 0);
    POOR_MANS_ASSERT(strcmp(test_context->parsed_events[0].file, __FILE__) == 0);
    POOR_MANS_ASSERT(strcmp(test_context->parsed_events[0].func, __FUNCTION__) == 0);
    POOR_MANS_ASSERT(test_context->parsed_events[0].line == captured_line);

    POOR_MANS_ASSERT(test_context->parsed_events[0].property_count == 13);
    POOR_MANS_ASSERT(test_context->parsed_events[0].properties[0].property_type == (_TlgInSTRUCT | _TlgInChain));
    POOR_MANS_ASSERT(test_context->parsed_events[0].properties[0].struct_field_count == 12);
    POOR_MANS_ASSERT(strcmp(test_context->parsed_events[0].properties[0].property_name, "") == 0);

    POOR_MANS_ASSERT(test_context->parsed_events[0].properties[1].property_type == TlgInANSISTRING);
    POOR_MANS_ASSERT(strcmp(test_context->parsed_events[0].properties[1].ascii_char_ptr_value, "duru") == 0);
    POOR_MANS_ASSERT(strcmp(test_context->parsed_events[0].properties[1].property_name, "gigi") == 0);
    POOR_MANS_ASSERT(test_context->parsed_events[0].properties[2].property_type == TlgInINT8);
    POOR_MANS_ASSERT(test_context->parsed_events[0].properties[2].int8_t_value == 0x10);
    POOR_MANS_ASSERT(strcmp(test_context->parsed_events[0].properties[2].property_name, "prop1") == 0);
    POOR_MANS_ASSERT(test_context->parsed_events[0].properties[3].property_type == TlgInUINT8);
    POOR_MANS_ASSERT(test_context->parsed_events[0].properties[3].uint8_t_value == 0x20);
    POOR_MANS_ASSERT(strcmp(test_context->parsed_events[0].properties[3].property_name, "prop2") == 0);
    POOR_MANS_ASSERT(test_context->parsed_events[0].properties[4].property_type == TlgInINT16);
    POOR_MANS_ASSERT(test_context->parsed_events[0].properties[4].int16_t_value == 0x3031);
    POOR_MANS_ASSERT(strcmp(test_context->parsed_events[0].properties[4].property_name, "prop3") == 0);
    POOR_MANS_ASSERT(test_context->parsed_events[0].properties[5].property_type == TlgInUINT16);
    POOR_MANS_ASSERT(test_context->parsed_events[0].properties[5].uint16_t_value == 0x4041);
    POOR_MANS_ASSERT(strcmp(test_context->parsed_events[0].properties[5].property_name, "prop4") == 0);
    POOR_MANS_ASSERT(test_context->parsed_events[0].properties[6].property_type == TlgInINT32);
    POOR_MANS_ASSERT(test_context->parsed_events[0].properties[6].int32_t_value == 0x50515253);
    POOR_MANS_ASSERT(strcmp(test_context->parsed_events[0].properties[6].property_name, "prop5") == 0);
    POOR_MANS_ASSERT(test_context->parsed_events[0].properties[7].property_type == TlgInUINT32);
    POOR_MANS_ASSERT(test_context->parsed_events[0].properties[7].uint32_t_value == 0x60616263);
    POOR_MANS_ASSERT(strcmp(test_context->parsed_events[0].properties[7].property_name, "prop6") == 0);
    POOR_MANS_ASSERT(test_context->parsed_events[0].properties[8].property_type == TlgInINT64);
    POOR_MANS_ASSERT(test_context->parsed_events[0].properties[8].int64_t_value == 0x7071727374757677);
    POOR_MANS_ASSERT(strcmp(test_context->parsed_events[0].properties[8].property_name, "prop7") == 0);
    POOR_MANS_ASSERT(test_context->parsed_events[0].properties[9].property_type == TlgInUINT64);
    POOR_MANS_ASSERT(test_context->parsed_events[0].properties[9].uint64_t_value == 0x8081828384858687);
    POOR_MANS_ASSERT(strcmp(test_context->parsed_events[0].properties[9].property_name, "prop8") == 0);
    POOR_MANS_ASSERT(test_context->parsed_events[0].properties[10].property_type == TlgInBOOL32);
    POOR_MANS_ASSERT(test_context->parsed_events[0].properties[10].bool_value == true);
    POOR_MANS_ASSERT(strcmp(test_context->parsed_events[0].properties[10].property_name, "prop9") == 0);
    POOR_MANS_ASSERT(test_context->parsed_events[0].properties[11].property_type == TlgInBOOL32);
    POOR_MANS_ASSERT(test_context->parsed_events[0].properties[11].bool_value == false);
    POOR_MANS_ASSERT(strcmp(test_context->parsed_events[0].properties[11].property_name, "prop10") == 0);
    POOR_MANS_ASSERT(test_context->parsed_events[0].properties[12].property_type == TlgInUNICODESTRING);
    POOR_MANS_ASSERT(wcscmp(test_context->parsed_events[0].properties[12].wchar_t_ptr_value, L"duru") == 0);
    POOR_MANS_ASSERT(strcmp(test_context->parsed_events[0].properties[12].property_name, "wstring") == 0);

    LOG_CONTEXT_DESTROY(log_context);
    test_context_destroy(test_context);
}

static void log_sink_etw_log_with_context_with_nested_structs(void)
{
    // arrange
    TEST_CONTEXT* test_context = test_context_create();

    generate_trace_session(test_context, __FUNCTION__);

    TRACEHANDLE trace_session_handle = start_trace(test_context, TRACE_LEVEL_VERBOSE);

    int captured_line = __LINE__;

    LOG_CONTEXT_HANDLE log_context_1;
    LOG_CONTEXT_CREATE(log_context_1, NULL,
        LOG_CONTEXT_PROPERTY(int8_t, prop1, 42)
    );

    LOG_CONTEXT_HANDLE log_context_2;
    LOG_CONTEXT_CREATE(log_context_2, log_context_1,
        LOG_CONTEXT_PROPERTY(uint8_t, prop2, 43)
    );

    start_parse_events(test_context);

    // act
    test_log_sink_etw_log(LOG_LEVEL_CRITICAL, log_context_2, __FILE__, __FUNCTION__, captured_line, "test_with_nested");

    // assert
    // 1 event expected which has att the properties
    wait_for_event_count(test_context, 1);

    stop_trace(test_context, trace_session_handle);
    stop_parse_events(test_context);

    POOR_MANS_ASSERT(strcmp(test_context->parsed_events[0].event_name, "LogCritical") == 0);
    POOR_MANS_ASSERT(strcmp(test_context->parsed_events[0].content, "test_with_nested") == 0);
    POOR_MANS_ASSERT(strcmp(test_context->parsed_events[0].file, __FILE__) == 0);
    POOR_MANS_ASSERT(strcmp(test_context->parsed_events[0].func, __FUNCTION__) == 0);
    POOR_MANS_ASSERT(test_context->parsed_events[0].line == captured_line);

    POOR_MANS_ASSERT(test_context->parsed_events[0].property_count == 4);
    POOR_MANS_ASSERT(test_context->parsed_events[0].properties[0].property_type == (_TlgInSTRUCT | _TlgInChain));
    POOR_MANS_ASSERT(test_context->parsed_events[0].properties[0].struct_field_count == 2); // First struct is the struct that contains the nested struct and prop2
    POOR_MANS_ASSERT(strcmp(test_context->parsed_events[0].properties[0].property_name, "") == 0);

    // log_context_1 properties
    POOR_MANS_ASSERT(test_context->parsed_events[0].properties[1].property_type == (_TlgInSTRUCT | _TlgInChain));
    POOR_MANS_ASSERT(test_context->parsed_events[0].properties[1].struct_field_count == 1); // Second struct is the nested struct that contains only prop1
    POOR_MANS_ASSERT(strcmp(test_context->parsed_events[0].properties[1].property_name, "") == 0);

    POOR_MANS_ASSERT(test_context->parsed_events[0].properties[2].property_type == TlgInINT8);
    POOR_MANS_ASSERT(test_context->parsed_events[0].properties[2].uint8_t_value == 42);
    POOR_MANS_ASSERT(strcmp(test_context->parsed_events[0].properties[2].property_name, "prop1") == 0);

    // log_context_2 properties
    POOR_MANS_ASSERT(test_context->parsed_events[0].properties[3].property_type == TlgInUINT8);
    POOR_MANS_ASSERT(test_context->parsed_events[0].properties[3].uint8_t_value == 43);
    POOR_MANS_ASSERT(strcmp(test_context->parsed_events[0].properties[3].property_name, "prop2") == 0);

    LOG_CONTEXT_DESTROY(log_context_2);
    LOG_CONTEXT_DESTROY(log_context_1);

    test_context_destroy(test_context);
}

/* very "poor man's" way of testing, as no test harness available */
int main(void)
{
    // make abort not popup
    _set_abort_behavior(_CALL_REPORTFAULT, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);

    POOR_MANS_ASSERT(log_sink_etw.init() == 0);

    log_sink_etw_log_with_LOG_LEVEL_ERROR_succeeds();
    log_sink_etw_log_all_levels_when_all_levels_enabled_succeeds();
    log_sink_etw_log_each_individual_level();
    log_sink_etw_log_with_context_with_properties();
    log_sink_etw_log_with_context_with_nested_structs();

    log_sink_etw.deinit();

    return 0;
}
