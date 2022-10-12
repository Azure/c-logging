// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdio.h>
#include <stdarg.h>

#include "windows.h"

#include "TraceLoggingProvider.h"
#include "evntrace.h"

#include "macro_utils/macro_utils.h"

#include "c_logging/logger.h"
#include "c_logging/log_sink_etw.h"

TRACELOGGING_DEFINE_PROVIDER(
    g_my_component_provider,
    "block_storage_2",
    (0xDAD29F36, 0x0A48, 0x4DEF, 0x9D, 0x50, 0x8E, 0xF9, 0x03, 0x6B, 0x92, 0xB4));
/*DAD29F36-0A48-4DEF-9D50-8EF9036B92B4*/

static volatile LONG isETWLoggerRegistered = 0;

static void lazyRegisterEventProvider(void)
{
    /*lazily init the logger*/
    LONG state;
    while ((state = InterlockedCompareExchange(&isETWLoggerRegistered, 1, 0)) != 2) /*0 - not init, 1 - initializing, 2 - initialized*/
    {
        if (state == 0)
        {
            /* register the provider*/
            TLG_STATUS t = TraceLoggingRegister(g_my_component_provider);
            if (SUCCEEDED(t))
            {
                (void)InterlockedExchange(&isETWLoggerRegistered, 2);
                printf("block_storage_2 ETW provider was registered succesfully (self test). Executable file full path name = %s", _pgmptr); /*_pgmptr comes from https://docs.microsoft.com/en-us/windows/win32/api/libloaderapi/nf-libloaderapi-getmodulefilenamea */
            }
            else
            {
                (void)printf("block_storage_2 ETW provider was NOT registered.");
                (void)InterlockedExchange(&isETWLoggerRegistered, 0);
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

// This function is done with a little bit of reverse engineering of TraceLogging and guidance from 
// the TraceLogging.h header about the format of the self described events
static void emit_self_described_event(const char* event_name, uint32_t trace_level, LOG_CONTEXT_PROPERTY_VALUE_PAIR* context_property_value_pairs, uint32_t property_value_count, const char* message, const char* file, const char* func, int line)
{
    (void)context_property_value_pairs;

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
        EVENT_DATA_DESCRIPTOR _tlgData[2 + 1 + 1 + 1 + 1 + LOG_MAX_PROPERTY_VALUE_PAIR_COUNT];
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
            case LOG_CONTEXT_PROPERTY_TYPE_ansi_charptr:
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

static void log_sink_etw_log(LOG_LEVEL log_level, LOG_CONTEXT_HANDLE log_context, const char* message, const char* file, const char* func, int line)
{
    lazyRegisterEventProvider();

    LOG_CONTEXT_PROPERTY_VALUE_PAIR* value_pairs = log_context_get_property_value_pairs(log_context);
    uint16_t values_count = (uint16_t)log_context_get_property_value_pair_count(log_context);

    if (log_context == NULL)
    {
        switch (log_level)
        {
        default:
            ETW_TRACE_LOGGING_WRAPPER("Unknown", TRACE_LEVEL_NONE, message, file, func, line);
            break;
        case LOG_LEVEL_CRITICAL:
            ETW_TRACE_LOGGING_WRAPPER("LogCritical", TRACE_LEVEL_CRITICAL, message, file, func, line);
            break;
        case LOG_LEVEL_ERROR:
            ETW_TRACE_LOGGING_WRAPPER("LogError", TRACE_LEVEL_ERROR, message, file, func, line);
            break;
        case LOG_LEVEL_WARNING:
            ETW_TRACE_LOGGING_WRAPPER("LogWarning", TRACE_LEVEL_WARNING, message, file, func, line);
            break;
        case LOG_LEVEL_INFO:
            ETW_TRACE_LOGGING_WRAPPER("LogInfo", TRACE_LEVEL_INFORMATION, message, file, func, line);
            break;
        case LOG_LEVEL_VERBOSE:
            ETW_TRACE_LOGGING_WRAPPER("LogVerbose", TRACE_LEVEL_VERBOSE, message, file, func, line);
            break;
        }
    }
    else
    {
        switch (log_level)
        {
        default:
            emit_self_described_event("Unknown", TRACE_LEVEL_NONE, value_pairs, values_count, message, file, func, line);
            break;
        case LOG_LEVEL_CRITICAL:
            emit_self_described_event("LogCritical", TRACE_LEVEL_CRITICAL, value_pairs, values_count, message, file, func, line);
            break;
        case LOG_LEVEL_ERROR:
            emit_self_described_event("LogError", TRACE_LEVEL_ERROR, value_pairs, values_count, message, file, func, line);
            break;
        case LOG_LEVEL_WARNING:
            emit_self_described_event("LogWarning", TRACE_LEVEL_WARNING, value_pairs, values_count, message, file, func, line);
            break;
        case LOG_LEVEL_INFO:
            emit_self_described_event("LogInfo", TRACE_LEVEL_INFORMATION, value_pairs, values_count, message, file, func, line);
            break;
        case LOG_LEVEL_VERBOSE:
            emit_self_described_event("LogVerbose", TRACE_LEVEL_VERBOSE, value_pairs, values_count, message, file, func, line);
            break;
        }
    }
}

const LOG_SINK etw_log_sink = { .log_sink_log = log_sink_etw_log };
