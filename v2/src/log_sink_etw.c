// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdio.h>
#include <stdarg.h>

#include "windows.h"

#include "TraceLoggingProvider.h"
#include "evntrace.h"

#include "macro_utils/macro_utils.h"

#include "c_logging/logger.h"
#include "c_logging/log_sink_if.h"
#include "c_logging/log_sink_etw.h"

#define LOG_MAX_ETW_PROPERTY_VALUE_PAIR_COUNT 64

TRACELOGGING_DEFINE_PROVIDER(
    g_my_component_provider,
    "block_storage_2",
    (0xDAD29F36, 0x0A48, 0x4DEF, 0x9D, 0x50, 0x8E, 0xF9, 0x03, 0x6B, 0x92, 0xB4));
/*DAD29F36-0A48-4DEF-9D50-8EF9036B92B4*/

/* Codes_SRS_LOG_SINK_ETW_01_002: [ `log_sink_etw_log` shall maintain the state of whether `TraceLoggingRegister` was called in a variable accessed via `InterlockedXXX` APIs, which shall have 3 possible values: `NOT_REGISTERED` (1), `REGISTERING` (2), `REGISTERED`(3). ]*/
#define PROVIDER_STATE_VALUES \
    PROVIDER_STATE_NOT_REGISTERED, \
    PROVIDER_STATE_REGISTERING, \
    PROVIDER_STATE_REGISTERED \

MU_DEFINE_ENUM(PROVIDER_STATE, PROVIDER_STATE_VALUES)

static volatile LONG etw_provider_state = PROVIDER_STATE_NOT_REGISTERED;

static void log_sink_etw_log(LOG_LEVEL log_level, LOG_CONTEXT_HANDLE log_context, const char* file, const char* func, int line, const char* message_format, ...);

static void internal_log_sink_etw_lazy_register_provider(void)
{
    LONG state;

    /* Codes_SRS_LOG_SINK_ETW_01_003: [ `log_sink_etw_log` shall perform the below actions until the provider is registered or an error is encountered: ]*/
    /* Codes_SRS_LOG_SINK_ETW_01_004: [ If the state is `NOT_REGISTERED`: ]*/
    /* Codes_SRS_LOG_SINK_ETW_01_005: [ `log_sink_etw_log` shall switch the state to `REGISTERING`. ]*/
    /* Codes_SRS_LOG_SINK_ETW_01_009: [ Checking and changing the variable that maintains whether `TraceLoggingRegister` was called shall be done using `InterlockedCompareExchange` and `InterlockedExchange`. ]*/
    while ((state = InterlockedCompareExchange(&etw_provider_state, PROVIDER_STATE_REGISTERING, PROVIDER_STATE_NOT_REGISTERED)) != PROVIDER_STATE_REGISTERED)
    {
        if (state == PROVIDER_STATE_NOT_REGISTERED)
        {
            /* Codes_SRS_LOG_SINK_ETW_01_006: [ `log_sink_etw_log` shall register the ETW TraceLogging provider by calling `TraceLoggingRegister` (`TraceLoggingRegister_EventRegister_EventSetInformation`). ]*/
            TLG_STATUS register_result = TraceLoggingRegister(g_my_component_provider);
            if (SUCCEEDED(register_result))
            {
                /* Codes_SRS_LOG_SINK_ETW_01_007: [ `log_sink_etw_log` shall switch the state to `REGISTERED`. ]*/
                (void)InterlockedExchange(&etw_provider_state, PROVIDER_STATE_REGISTERED);
                /* Codes_SRS_LOG_SINK_ETW_01_008: [ `log_sink_etw_log` shall `printf` the fact that the provider was registered and from which executable (as obtained by calling `_pgmptr`). ]*/

                char* executable_full_path_name;
                if (_get_pgmptr(&executable_full_path_name) != 0)
                {
                    executable_full_path_name = NULL;
                }

                /* Codes_SRS_LOG_SINK_ETW_01_008: [ `log_sink_etw_log` shall emit a `LOG_LEVEL_INFO` event as a self test , printing the fact that the provider was registered and from which executable (as obtained by calling `_get_pgmptr`). ]*/
                //log_sink_etw_log(LOG_LEVEL_INFO, NULL, __FILE__, __FUNCTION__, __LINE__, "ETW provider was registered succesfully (self test). Executable file full path name = %s", MU_P_OR_NULL(executable_full_path_name));

                break;
            }
            else
            {
                (void)printf("ETW provider was NOT registered.");
                (void)InterlockedExchange(&etw_provider_state, PROVIDER_STATE_NOT_REGISTERED);
            }
        }
    }
}

__pragma(pack(push, 1))
typedef struct SELF_DESCRIBED_EVENT_TAG
{
    uint8_t _tlgBlobTyp;
    uint8_t _tlgChannel;
    uint8_t _tlgLevel;
    uint8_t _tlgOpcode;
    uint64_t _tlgKeyword;
    uint16_t _tlgEvtMetaSize;
    uint16_t _tlgEvtTag;
    uint8_t metadata[];
} SELF_DESCRIBED_EVENT;
__pragma(pack(pop))

#define MAX_SELF_DESCRIBED_EVENT_WITH_METADATA (sizeof(SELF_DESCRIBED_EVENT) + 4096)

// This function was written with a little bit of reverse engineering of TraceLogging and guidance from 
// the TraceLogging.h header about the format of the self described events
static void emit_self_described_event(const char* event_name, uint32_t trace_level, const LOG_CONTEXT_PROPERTY_VALUE_PAIR* context_property_value_pairs, uint32_t property_value_count, const char* message, const char* file, const char* func, int line)
{
    __pragma(warning(push))
    __pragma(warning(disable:4127 4132 6001))
    __pragma(warning(error:4047))
    __pragma(pack(push, 1))
    __pragma(execution_character_set(push, "UTF-8"))
    enum { _tlgTagConst = (0) };
    enum { _tlgLevelConst = 5 & 0 | (5) };

    TraceLoggingHProvider const _tlgProv = (g_my_component_provider);
    if (trace_level < _tlgProv->LevelPlus1 && _tlgKeywordOn(_tlgProv, 0))
    {
        // have a stack allocated buffer where we construct the event metadata and
        uint8_t _tlgEvent[MAX_SELF_DESCRIBED_EVENT_WITH_METADATA];

        // compute event metadata size
        size_t event_name_length = strlen(event_name);
        uint16_t metadata_size = (uint16_t)(event_name_length + 1);
        metadata_size += sizeof("content") + 1 + sizeof("file") + 1 + sizeof("func") + 1 + sizeof("line") + 1;
        for (uint32_t i = 0; i < property_value_count; i++)
        {
            size_t name_length = strlen(context_property_value_pairs[i].name);
            metadata_size += (uint16_t)name_length + 1 + 1;
            if (context_property_value_pairs[i].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_struct)
            {
                metadata_size++;
            }
        }

        // alias to the event bytes
        SELF_DESCRIBED_EVENT* self_described_event = (SELF_DESCRIBED_EVENT*)_tlgEvent;
        self_described_event->_tlgBlobTyp = _TlgBlobEvent4;
        self_described_event->_tlgChannel = 11;
        self_described_event->_tlgLevel = _tlgLevelConst;
        self_described_event->_tlgOpcode = 0;
        self_described_event->_tlgKeyword = 0;
        self_described_event->_tlgEvtMetaSize = metadata_size + 4;
        self_described_event->_tlgEvtTag = 128 | ((0xfe00000 & (UINT32)_tlgTagConst) >> 21) | ((0x01fc000 & (UINT32)_tlgTagConst) >> 6) | ((0x0003fff & (UINT32)_tlgTagConst) << 16) | (~(0x0FFFFFFF | ~(UINT32)_tlgTagConst));

        // at this point we filled all the event information, now we need to provide the metadata bytes
        // first one is the event name, followed by metadata for all the fields
        // copy event name 
        uint8_t* pos = &self_described_event->metadata[0];
        (void)memcpy(pos, event_name, event_name_length + 1); pos += event_name_length + 1;

        // copy the field metadata (name and in type)
        (void)memcpy(pos, "content", sizeof("content")); pos += sizeof("content");
        *pos = TlgInANSISTRING;  pos++;
        (void)memcpy(pos, "file", sizeof("file")); pos += sizeof("file");
        *pos = TlgInANSISTRING;  pos++;
        (void)memcpy(pos, "func", sizeof("func")); pos += sizeof("func");
        *pos = TlgInANSISTRING;  pos++;
        (void)memcpy(pos, "line", sizeof("line")); pos += sizeof("line");
        *pos = TlgInINT32;  pos++;
        for (uint32_t i = 0; i < property_value_count; i++)
        {
            size_t name_length = strlen(context_property_value_pairs[i].name);
            (void)memcpy(pos, context_property_value_pairs[i].name, name_length + 1); pos += name_length + 1;
            switch (context_property_value_pairs[i].type->get_type())
            {
            default:
                *pos = TlgInANSISTRING;
                break;
            case LOG_CONTEXT_PROPERTY_TYPE_int32_t:
                *pos = TlgInINT32;
                break;
            case LOG_CONTEXT_PROPERTY_TYPE_int64_t:
                *pos = TlgInINT64;
                break;
            case LOG_CONTEXT_PROPERTY_TYPE_struct:
                *pos = _TlgInSTRUCT | _TlgInChain;
                pos++;
                // add the struct field count
                *pos = (uint8_t)(uintptr_t)context_property_value_pairs[i].value;
                break;
            }
            pos++;
        }

        // now we need to fill in the event data descriptors
        // first 2 are actually reserved for the event descriptor and metadata respectively
        // all the rest starting at index 2 are actual data payloads in the event
        EVENT_DATA_DESCRIPTOR _tlgData[2 + 1 + 1 + 1 + 1 + LOG_MAX_ETW_PROPERTY_VALUE_PAIR_COUNT];
        UINT32 _tlgIdx = 2;
        _tlgCreate1Sz_char(&_tlgData[_tlgIdx], (message));
        _tlgIdx += 1;
        _tlgCreate1Sz_char(&_tlgData[_tlgIdx], (file));
        _tlgIdx += 1;
        _tlgCreate1Sz_char(&_tlgData[_tlgIdx], (func));
        _tlgIdx += 1;
        INT32 const _tlgTemp7 = (line);
        EventDataDescCreate(&_tlgData[_tlgIdx], &_tlgTemp7, sizeof(INT32));
        _tlgIdx += 1;

        for (uint32_t i = 0; i < property_value_count; i++)
        {
            switch (context_property_value_pairs[i].type->get_type())
            {
            default:
                break;
            case LOG_CONTEXT_PROPERTY_TYPE_int32_t:
                EventDataDescCreate(&_tlgData[_tlgIdx], context_property_value_pairs[i].value, sizeof(INT32));
                break;
            case LOG_CONTEXT_PROPERTY_TYPE_int64_t:
                EventDataDescCreate(&_tlgData[_tlgIdx], context_property_value_pairs[i].value, sizeof(INT64));
                break;
            case LOG_CONTEXT_PROPERTY_TYPE_ascii_char_ptr:
                _tlgCreate1Sz_char(&_tlgData[_tlgIdx], context_property_value_pairs[i].value);
                break;
            case LOG_CONTEXT_PROPERTY_TYPE_struct:
                _tlgIdx--;
                break;
            }
            _tlgIdx++;
        }

        // ... AND drumrolls, emit the event
        _tlgWriteTransfer_EventWriteTransfer(_tlgProv, &self_described_event->_tlgChannel, ((void*)0), ((void*)0), _tlgIdx, _tlgData);
    }
    __pragma(execution_character_set(pop))
    __pragma(pack(pop))
    __pragma(warning(pop))
}

#define ETW_TRACE_LOGGING_WRAPPER(event_name, trace_level, message, file, func, line) \
    TraceLoggingWrite(g_my_component_provider, \
        event_name, \
        TraceLoggingLevel(trace_level), \
        TraceLoggingString(message, "content"), \
        TraceLoggingString(file, "file"), \
        TraceLoggingString(func, "func"), \
        TraceLoggingInt32(line, "line")) \

static void log_sink_etw_log(LOG_LEVEL log_level, LOG_CONTEXT_HANDLE log_context, const char* file, const char* func, int line, const char* message_format, ...)
{
    if (message_format == NULL)
    {
        (void)printf("Invalid arguments: LOG_LEVEL log_level=%" PRI_MU_ENUM ", LOG_CONTEXT_HANDLE log_context=%p, const char* file=%s, const char* func=%s, int line=%d, const char* message_format=%s",
            MU_ENUM_VALUE(LOG_LEVEL, log_level), log_context, file, func, line, message_format);
    }
    else
    {
        internal_log_sink_etw_lazy_register_provider();

        /* Codes_SRS_LOG_SINK_ETW_01_010: [ `log_sink_etw_log` shall emit a self described event that shall have the name of the event as follows: ]*/

        if (log_context == NULL)
        {
            switch (log_level)
            {
            default:
                ETW_TRACE_LOGGING_WRAPPER("Unknown", TRACE_LEVEL_NONE, message_format, file, func, line);
                break;
            case LOG_LEVEL_CRITICAL:
                ETW_TRACE_LOGGING_WRAPPER("LogCritical", TRACE_LEVEL_CRITICAL, message_format, file, func, line);
                break;
            case LOG_LEVEL_ERROR:
                ETW_TRACE_LOGGING_WRAPPER("LogError", TRACE_LEVEL_ERROR, message_format, file, func, line);
                break;
            case LOG_LEVEL_WARNING:
                ETW_TRACE_LOGGING_WRAPPER("LogWarning", TRACE_LEVEL_WARNING, message_format, file, func, line);
                break;
            case LOG_LEVEL_INFO:
                ETW_TRACE_LOGGING_WRAPPER("LogInfo", TRACE_LEVEL_INFORMATION, message_format, file, func, line);
                break;
            case LOG_LEVEL_VERBOSE:
                ETW_TRACE_LOGGING_WRAPPER("LogVerbose", TRACE_LEVEL_VERBOSE, message_format, file, func, line);
                break;
            }
        }
        else
        {
            const LOG_CONTEXT_PROPERTY_VALUE_PAIR* value_pairs = log_context_get_property_value_pairs(log_context);
            uint16_t values_count = (uint16_t)log_context_get_property_value_pair_count(log_context);

            switch (log_level)
            {
            default:
                emit_self_described_event("Unknown", TRACE_LEVEL_NONE, value_pairs, values_count, message_format, file, func, line);
                break;
            case LOG_LEVEL_CRITICAL:
                emit_self_described_event("LogCritical", TRACE_LEVEL_CRITICAL, value_pairs, values_count, message_format, file, func, line);
                break;
            case LOG_LEVEL_ERROR:
                emit_self_described_event("LogError", TRACE_LEVEL_ERROR, value_pairs, values_count, message_format, file, func, line);
                break;
            case LOG_LEVEL_WARNING:
                emit_self_described_event("LogWarning", TRACE_LEVEL_WARNING, value_pairs, values_count, message_format, file, func, line);
                break;
            case LOG_LEVEL_INFO:
                emit_self_described_event("LogInfo", TRACE_LEVEL_INFORMATION, value_pairs, values_count, message_format, file, func, line);
                break;
            case LOG_LEVEL_VERBOSE:
                emit_self_described_event("LogVerbose", TRACE_LEVEL_VERBOSE, value_pairs, values_count, message_format, file, func, line);
                break;
            }
        }
    }
}

const LOG_SINK_IF log_sink_etw = { .log_sink_log = log_sink_etw_log };
