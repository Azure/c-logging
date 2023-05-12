// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>

#include "windows.h"

#include "TraceLoggingProvider.h"
#include "evntrace.h"

#include "macro_utils/macro_utils.h"

#include "c_logging/logger.h"
#include "c_logging/log_sink_if.h"
#include "c_logging/log_sink_etw.h"

// max number of properties we want to have in the event
#define LOG_MAX_ETW_PROPERTY_VALUE_PAIR_COUNT 64

// max metadata size
#define MAX_METADATA_SIZE 4096
#define MAX_SELF_DESCRIBED_EVENT_WITH_METADATA (sizeof(SELF_DESCRIBED_EVENT) + MAX_METADATA_SIZE)

// ID Work Item Type Title State Assigned To Effort Remaining Work Original Estimate Value Area Iteration Path Tags
// 17751591 Task Have configurable provider To Do           One\Custom\AzureMessaging\Gallium\GaM2

/* Codes_SRS_LOG_SINK_ETW_01_084: [ log_sink_etw_log shall use as provider GUID DAD29F36-0A48-4DEF-9D50-8EF9036B92B4. ]*/
TRACELOGGING_DEFINE_PROVIDER(
    g_my_component_provider,
    "block_storage_2",
    (0xDAD29F36, 0x0A48, 0x4DEF, 0x9D, 0x50, 0x8E, 0xF9, 0x03, 0x6B, 0x92, 0xB4));
/*DAD29F36-0A48-4DEF-9D50-8EF9036B92B4*/

/* Codes_SRS_LOG_SINK_ETW_01_002: [ log_sink_etw_log shall maintain the state of whether TraceLoggingRegister was called in a variable accessed via InterlockedXXX APIs, which shall have 3 possible values: NOT_REGISTERED (1), REGISTERING (2), REGISTERED(3). ]*/
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

    /* Codes_SRS_LOG_SINK_ETW_01_003: [ log_sink_etw_log shall perform the below actions until the provider is registered or an error is encountered: ]*/
    /* Codes_SRS_LOG_SINK_ETW_01_004: [ If the state is NOT_REGISTERED (1): ]*/
    /* Codes_SRS_LOG_SINK_ETW_01_005: [ log_sink_etw_log shall switch the state to REGISTERING (2). ]*/
    /* Codes_SRS_LOG_SINK_ETW_01_009: [ Checking and changing the variable that maintains whether TraceLoggingRegister was called shall be done using InterlockedCompareExchange and InterlockedExchange. ]*/
    /* Codes_SRS_LOG_SINK_ETW_01_011: [ If the state is REGISTERED (3), log_sink_etw_log shall proceed to log the ETW event. ]*/
    /* Codes_SRS_LOG_SINK_ETW_01_087: [ If the state is REGISTERING (2) log_sink_etw_log shall wait until the state is not REGISTERING (2). ]*/
    while ((state = InterlockedCompareExchange(&etw_provider_state, PROVIDER_STATE_REGISTERING, PROVIDER_STATE_NOT_REGISTERED)) != PROVIDER_STATE_REGISTERED)
    {
        if (state == PROVIDER_STATE_NOT_REGISTERED)
        {
            /* Codes_SRS_LOG_SINK_ETW_01_006: [ log_sink_etw_log shall register the ETW TraceLogging provider by calling TraceLoggingRegister (TraceLoggingRegister_EventRegister_EventSetInformation). ]*/
            TLG_STATUS register_result = TraceLoggingRegister(g_my_component_provider);
            if (SUCCEEDED(register_result))
            {
                /* Codes_SRS_LOG_SINK_ETW_01_007: [ log_sink_etw_log shall switch the state to REGISTERED. ]*/
                (void)InterlockedExchange(&etw_provider_state, PROVIDER_STATE_REGISTERED);
                /* Codes_SRS_LOG_SINK_ETW_01_008: [ log_sink_etw_log shall printf the fact that the provider was registered and from which executable (as obtained by calling _pgmptr). ]*/

                char* executable_full_path_name;
                if (_get_pgmptr(&executable_full_path_name) != 0)
                {
                    /* Codes_SRS_LOG_SINK_ETW_01_083: [ If _get_pgmptr fails, the executable shall be printed as UNKNOWN. ]*/
                    executable_full_path_name = "UNKNOWN";
                }

                /* Codes_SRS_LOG_SINK_ETW_01_008: [ log_sink_etw_log shall emit a LOG_LEVEL_INFO event as a self test , printing the fact that the provider was registered and from which executable (as obtained by calling _get_pgmptr). ]*/
                log_sink_etw_log(LOG_LEVEL_INFO, NULL, __FILE__, __FUNCTION__, __LINE__, "ETW provider was registered succesfully (self test). Executable file full path name = %s", MU_P_OR_NULL(executable_full_path_name));

                break;
            }
            else
            {
                (void)printf("ETW provider was NOT registered.\r\n");
                (void)InterlockedExchange(&etw_provider_state, PROVIDER_STATE_NOT_REGISTERED);
            }
        }
    }
}

// want these aligned to 1 byte (copy/pasted from Tracelogging code)
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

// This function was written with a little bit of reverse engineering of TraceLogging and guidance from 
// the TraceLogging.h header about the format of the self described events
static void internal_emit_self_described_event(const char* event_name, uint16_t event_name_length, uint8_t trace_level, const LOG_CONTEXT_PROPERTY_VALUE_PAIR* context_property_value_pairs, uint32_t property_value_count, const char* message, const char* file, const char* func, int32_t line, va_list args)
{
    TraceLoggingHProvider const _tlgProv = g_my_component_provider;
    if (trace_level < _tlgProv->LevelPlus1)
    {
        // have a stack allocated buffer where we construct the event metadata and
        uint8_t _tlgEvent[MAX_SELF_DESCRIBED_EVENT_WITH_METADATA];
        bool add_properties = true;

        // compute event metadata size
        // event_name_length includes NULL terminator
        /* Codes_SRS_LOG_SINK_ETW_01_042: [ log_sink_etw.log_sink_log shall compute the metadata size for the self described event metadata as follows: ]*/
        /* Codes_SRS_LOG_SINK_ETW_01_043: [ Length of the event name (determined at compile time, excluding zero terminator) + 1. ]*/
        uint32_t i;

        // alias to the event bytes
        SELF_DESCRIBED_EVENT* self_described_event = (SELF_DESCRIBED_EVENT*)_tlgEvent;

        uint8_t* pos = &self_described_event->metadata[0];

        // at this point we filled all the event information, now we need to provide the metadata bytes
        // first one is the event name, followed by metadata for all the fields
        // copy event name 
        (void)memcpy(pos, event_name, event_name_length); pos += event_name_length;

        // copy the field metadata (name and in type)

        /* Codes_SRS_LOG_SINK_ETW_01_034: [ log_sink_etw.log_sink_log shall fill the event metadata: ]*/

        /* Codes_SRS_LOG_SINK_ETW_01_035: [ The string content (as field name, excluding zero terminator), followed by one byte with the value TlgInANSISTRING. ]*/
        /* Codes_SRS_LOG_SINK_ETW_01_044: [ Length of the content field name (determined at compile time, excluding zero terminator) + 1. ]*/
        (void)memcpy(pos, "content", sizeof("content")); pos += sizeof("content");
        *pos = TlgInANSISTRING;  pos++;

        /* Codes_SRS_LOG_SINK_ETW_01_036: [ The string file (as field name, excluding zero terminator), followed by one byte with the value TlgInANSISTRING. ]*/
        /* Codes_SRS_LOG_SINK_ETW_01_045: [ Length of the file field name (determined at compile time, excluding zero terminator) + 1. ]*/
        (void)memcpy(pos, "file", sizeof("file")); pos += sizeof("file");
        *pos = TlgInANSISTRING;  pos++;

        /* Codes_SRS_LOG_SINK_ETW_01_037: [ The string func (as field name, excluding zero terminator), followed by one byte with the value TlgInANSISTRING. ]*/
        /* Codes_SRS_LOG_SINK_ETW_01_046: [ Length of the func field name (determined at compile time, excluding zero terminator) + 1. ]*/
        (void)memcpy(pos, "func", sizeof("func")); pos += sizeof("func");
        *pos = TlgInANSISTRING;  pos++;

        /* Codes_SRS_LOG_SINK_ETW_01_038: [ The string line (as field name, excluding zero terminator), followed by one byte with the value TlgInINT32. ]*/
        /* Codes_SRS_LOG_SINK_ETW_01_047: [ Length of the line field name (determined at compile time, excluding zero terminator) + 1. ]*/
        (void)memcpy(pos, "line", sizeof("line")); pos += sizeof("line");
        *pos = TlgInINT32;  pos++;

        uint16_t metadata_size_without_properties = (uint16_t)(pos - &self_described_event->metadata[0]);

        if (property_value_count <= LOG_MAX_ETW_PROPERTY_VALUE_PAIR_COUNT)
        {
            /* Codes_SRS_LOG_SINK_ETW_01_053: [ For each property in log_context the following shall be added to the event metadata: ]*/
            for (i = 0; i < property_value_count; i++)
            {
                /* Codes_SRS_LOG_SINK_ETW_01_051: [ For each property in log_context, the length of the property name + 1 and one extra byte for the type of the field. ]*/
                size_t name_length = strlen(context_property_value_pairs[i].name);
                /* Codes_SRS_LOG_SINK_ETW_01_085: [ If the size of the metadata and the formatted message exceeds 4096 bytes, log_sink_etw.log_sink_log shall not add any properties to the event. ]*/
                if ((uint16_t)(&self_described_event->metadata[MAX_METADATA_SIZE] - pos) < name_length + 1 + 1)
                {
                    add_properties = false;
                    break;
                }
                else
                {
                    /* Codes_SRS_LOG_SINK_ETW_01_054: [ A string with the property name (including zero terminator) ]*/
                    (void)memcpy(pos, context_property_value_pairs[i].name, name_length + 1);
                    pos += name_length + 1;
                }

                /* Codes_SRS_LOG_SINK_ETW_01_055: [ A byte with the type of property, as follows: ]*/
                switch (context_property_value_pairs[i].type->get_type())
                {
                default:
                    /* Codes_SRS_LOG_SINK_ETW_01_081: [ If the property type is any other value, no property data shall be added to the event. ]*/
                    add_properties = false;
                    break;

                case LOG_CONTEXT_PROPERTY_TYPE_ascii_char_ptr:
                    /* Codes_SRS_LOG_SINK_ETW_01_063: [ If the property type is LOG_CONTEXT_PROPERTY_TYPE_ascii_char_ptr, a byte with the value TlgInANSISTRING shall be added in the metadata. ]*/
                    *pos = TlgInANSISTRING;
                    break;
                case LOG_CONTEXT_PROPERTY_TYPE_int64_t:
                    /* Codes_SRS_LOG_SINK_ETW_01_064: [ If the property type is LOG_CONTEXT_PROPERTY_TYPE_int64_t, a byte with the value TlgInINT64 shall be added in the metadata. ]*/
                    *pos = TlgInINT64;
                    break;
                case LOG_CONTEXT_PROPERTY_TYPE_uint64_t:
                    /* Codes_SRS_LOG_SINK_ETW_01_065: [ If the property type is LOG_CONTEXT_PROPERTY_TYPE_uint64_t, a byte with the value TlgInUINT64 shall be added in the metadata. ]*/
                    *pos = TlgInUINT64;
                    break;
                case LOG_CONTEXT_PROPERTY_TYPE_int32_t:
                    /* Codes_SRS_LOG_SINK_ETW_01_069: [ If the property type is LOG_CONTEXT_PROPERTY_TYPE_int32_t, a byte with the value TlgInINT32 shall be added in the metadata. ]*/
                    *pos = TlgInINT32;
                    break;
                case LOG_CONTEXT_PROPERTY_TYPE_uint32_t:
                    /* Codes_SRS_LOG_SINK_ETW_01_071: [ If the property type is LOG_CONTEXT_PROPERTY_TYPE_uint32_t, a byte with the value TlgInUINT32 shall be added in the metadata. ]*/
                    *pos = TlgInUINT32;
                    break;
                case LOG_CONTEXT_PROPERTY_TYPE_int16_t:
                    /* Codes_SRS_LOG_SINK_ETW_01_073: [ If the property type is LOG_CONTEXT_PROPERTY_TYPE_int16_t, a byte with the value TlgInINT16 shall be added in the metadata. ]*/
                    *pos = TlgInINT16;
                    break;
                case LOG_CONTEXT_PROPERTY_TYPE_uint16_t:
                    /* Codes_SRS_LOG_SINK_ETW_01_075: [ If the property type is LOG_CONTEXT_PROPERTY_TYPE_uint16_t, a byte with the value TlgInUINT16 shall be added in the metadata. ]*/
                    *pos = TlgInUINT16;
                    break;
                case LOG_CONTEXT_PROPERTY_TYPE_int8_t:
                    /* Codes_SRS_LOG_SINK_ETW_01_077: [ If the property type is LOG_CONTEXT_PROPERTY_TYPE_int8_t, a byte with the value TlgInINT8 shall be added in the metadata. ]*/
                    *pos = TlgInINT8;
                    break;
                case LOG_CONTEXT_PROPERTY_TYPE_uint8_t:
                    /* Codes_SRS_LOG_SINK_ETW_01_079: [ If the property type is LOG_CONTEXT_PROPERTY_TYPE_uint8_t, a byte with the value TlgInUINT8 shall be added in the metadata. ]*/
                    *pos = TlgInUINT8;
                    break;

                case LOG_CONTEXT_PROPERTY_TYPE_struct:
                    /* Codes_SRS_LOG_SINK_ETW_01_052: [ For struct properties one extra byte shall be added for the field count. ]*/
                    /* Codes_SRS_LOG_SINK_ETW_01_056: [ If the property type is LOG_CONTEXT_PROPERTY_TYPE_struct, a byte with the value _TlgInSTRUCT | _TlgInChain shall be added in the metadata. ]*/
                    *pos = _TlgInSTRUCT | _TlgInChain;
                    pos++;

                    if (pos == &self_described_event->metadata[MAX_METADATA_SIZE])
                    {
                        add_properties = false;
                    }
                    else
                    {
                        /* Codes_SRS_LOG_SINK_ETW_01_057: [ If the property is a struct, an extra byte shall be added in the metadata containing the number of fields in the structure. ]*/
                        *pos = *((uint8_t*)context_property_value_pairs[i].value);
                    }
                    break;
                }

                if (!add_properties)
                {
                    break;
                }
                else
                {
                    pos++;
                }
            }
        }
        else
        {
            /* Codes_SRS_LOG_SINK_ETW_01_082: [ If more than 64 properties are given in log_context, log_sink_etw.log_sink_log shall not add any properties to the event. ]*/
            add_properties = false;
        }

        /* Codes_SRS_LOG_SINK_ETW_01_026: [ log_sink_etw.log_sink_log shall fill a SELF_DESCRIBED_EVENT structure, setting the following fields: ]*/
        uint16_t metadata_size;

        if (!add_properties)
        {
            metadata_size = metadata_size_without_properties;
            pos = &self_described_event->metadata[metadata_size_without_properties];
        }
        else
        {
            metadata_size = (uint16_t)(pos - &self_described_event->metadata[0]);
        }

        char* formatted_message = (char*)pos;
        size_t available_bytes = sizeof(_tlgEvent) - (formatted_message - (char*)self_described_event);

        int formatted_message_length = vsnprintf(formatted_message, available_bytes, message, args);
        if (formatted_message_length < 0)
        {
            /* Codes_SRS_LOG_SINK_ETW_01_086: [ If any error occurs log_sink_etw.log_sink_log shall print Error emitting ETW event and return. ]*/
            (void)printf("Error emitting ETW event.\r\n");
        }
        else
        {
            if ((size_t)formatted_message_length >= available_bytes)
            {
                if (add_properties)
                {
                    // did not fit, print again without properties and limit it
                    /* Codes_SRS_LOG_SINK_ETW_01_085: [ If the size of the metadata and the formatted message exceeds 4096 bytes, log_sink_etw.log_sink_log shall not add any properties to the event. ]*/
                    add_properties = false;
                    metadata_size = metadata_size_without_properties;

                    formatted_message = (char*)&self_described_event->metadata[metadata_size_without_properties];
                    available_bytes = sizeof(_tlgEvent) - (formatted_message - (char*)self_described_event);
                    formatted_message_length = vsnprintf(formatted_message, available_bytes, message, args);
                    if (formatted_message_length < 0)
                    {
                        /* Codes_SRS_LOG_SINK_ETW_01_086: [ If any error occurs log_sink_etw.log_sink_log shall print Error emitting ETW event and return. ]*/
                        (void)printf("Error emitting ETW event.\r\n");
                    }
                    else
                    {
                        // Ok as is
                    }
                }
            }
            else
            {
                // Ok as is
            }

            if (formatted_message_length >= 0)
            {
                /* Codes_SRS_LOG_SINK_ETW_01_027: [ _tlgBlobTyp shall be set to _TlgBlobEvent4. ]*/
                self_described_event->_tlgBlobTyp = _TlgBlobEvent4;

                /* Codes_SRS_LOG_SINK_ETW_01_028: [ _tlgChannel shall be set to 11. ]*/
                self_described_event->_tlgChannel = 11;

                /* Codes_SRS_LOG_SINK_ETW_01_018: [ Logging level: ]*/
                /* Codes_SRS_LOG_SINK_ETW_01_029: [ _tlgLevel shall be set to the appropriate logging level. ]*/
                self_described_event->_tlgLevel = trace_level;

                /* Codes_SRS_LOG_SINK_ETW_01_030: [ _tlgOpcode shall be set to 0. ]*/
                self_described_event->_tlgOpcode = 0;

                /* Codes_SRS_LOG_SINK_ETW_01_031: [ _tlgKeyword shall be set to 0. ]*/
                self_described_event->_tlgKeyword = 0;

                /* Codes_SRS_LOG_SINK_ETW_01_032: [ _tlgEvtMetaSize shall be set to the computed metadata size + 4. ]*/
                self_described_event->_tlgEvtMetaSize = metadata_size + 4;

                /* Codes_SRS_LOG_SINK_ETW_01_033: [ _tlgEvtTag shall be set to 128. ]*/
                self_described_event->_tlgEvtTag = 128;

                // now we need to fill in the event data descriptors
                // first 2 are actually reserved for the event descriptor and metadata respectively
                // all the rest starting at index 2 are actual data payloads in the event

                /* Codes_SRS_LOG_SINK_ETW_01_039: [ log_sink_etw.log_sink_log shall fill an EVENT_DATA_DESCRIPTOR array of size 2 + 1 + 1 + 1 + 1 + property count. ]*/
                EVENT_DATA_DESCRIPTOR _tlgData[2 + 1 + 1 + 1 + 1 + LOG_MAX_ETW_PROPERTY_VALUE_PAIR_COUNT];

                uint32_t _tlgIdx = 2;

                /* Codes_SRS_LOG_SINK_ETW_01_040: [ log_sink_etw.log_sink_log shall set event data descriptor at index 2 by calling _tlgCreate1Sz_char with the value of the formatted message as obtained by using printf with the messages format message_format and the arguments in .... ]*/
                _tlgCreate1Sz_char(&_tlgData[_tlgIdx], formatted_message);
                _tlgIdx++;
                /* Codes_SRS_LOG_SINK_ETW_01_058: [ log_sink_etw.log_sink_log shall set event data descriptor at index 3 by calling _tlgCreate1Sz_char with file. ]*/
                _tlgCreate1Sz_char(&_tlgData[_tlgIdx], file);
                _tlgIdx++;
                /* Codes_SRS_LOG_SINK_ETW_01_059: [ log_sink_etw.log_sink_log shall set event data descriptor at index 4 by calling _tlgCreate1Sz_char with func. ]*/
                _tlgCreate1Sz_char(&_tlgData[_tlgIdx], func);
                _tlgIdx++;
                /* Codes_SRS_LOG_SINK_ETW_01_060: [ log_sink_etw.log_sink_log shall set event data descriptor at index 5 by calling EventDataDescCreate with line. ]*/
                EventDataDescCreate(&_tlgData[_tlgIdx], &line, sizeof(int32_t));
                _tlgIdx++;

                if (add_properties)
                {
                    /* Codes_SRS_LOG_SINK_ETW_01_061: [ For each property in log_context: ]*/
                    for (i = 0; i < property_value_count; i++)
                    {
                        switch (context_property_value_pairs[i].type->get_type())
                        {
                        default:
                            break;
                        case LOG_CONTEXT_PROPERTY_TYPE_int64_t:
                            /* Codes_SRS_LOG_SINK_ETW_01_066: [ If the property type is LOG_CONTEXT_PROPERTY_TYPE_int64_t, the event data descriptor shall be filled with the value of the property by calling EventDataDescCreate. ]*/
                            EventDataDescCreate(&_tlgData[_tlgIdx], context_property_value_pairs[i].value, sizeof(int64_t));
                            break;
                        case LOG_CONTEXT_PROPERTY_TYPE_uint64_t:
                            /* Codes_SRS_LOG_SINK_ETW_01_068: [ If the property type is LOG_CONTEXT_PROPERTY_TYPE_uint64_t, the event data descriptor shall be filled with the value of the property by calling EventDataDescCreate. ]*/
                            EventDataDescCreate(&_tlgData[_tlgIdx], context_property_value_pairs[i].value, sizeof(uint64_t));
                            break;
                        case LOG_CONTEXT_PROPERTY_TYPE_int32_t:
                            /* Codes_SRS_LOG_SINK_ETW_01_070: [ If the property type is LOG_CONTEXT_PROPERTY_TYPE_int32_t, the event data descriptor shall be filled with the value of the property by calling EventDataDescCreate. ]*/
                            EventDataDescCreate(&_tlgData[_tlgIdx], context_property_value_pairs[i].value, sizeof(int32_t));
                            break;
                        case LOG_CONTEXT_PROPERTY_TYPE_uint32_t:
                            /* Codes_SRS_LOG_SINK_ETW_01_072: [ If the property type is LOG_CONTEXT_PROPERTY_TYPE_uint32_t, the event data descriptor shall be filled with the value of the property by calling EventDataDescCreate. ]*/
                            EventDataDescCreate(&_tlgData[_tlgIdx], context_property_value_pairs[i].value, sizeof(uint32_t));
                            break;
                        case LOG_CONTEXT_PROPERTY_TYPE_int16_t:
                            /* Codes_SRS_LOG_SINK_ETW_01_074: [ If the property type is LOG_CONTEXT_PROPERTY_TYPE_int16_t, the event data descriptor shall be filled with the value of the property by calling EventDataDescCreate. ]*/
                            EventDataDescCreate(&_tlgData[_tlgIdx], context_property_value_pairs[i].value, sizeof(int16_t));
                            break;
                        case LOG_CONTEXT_PROPERTY_TYPE_uint16_t:
                            /* Codes_SRS_LOG_SINK_ETW_01_076: [ If the property type is LOG_CONTEXT_PROPERTY_TYPE_uint16_t, the event data descriptor shall be filled with the value of the property by calling EventDataDescCreate. ]*/
                            EventDataDescCreate(&_tlgData[_tlgIdx], context_property_value_pairs[i].value, sizeof(uint16_t));
                            break;
                        case LOG_CONTEXT_PROPERTY_TYPE_int8_t:
                            /* Codes_SRS_LOG_SINK_ETW_01_078: [ If the property type is LOG_CONTEXT_PROPERTY_TYPE_int8_t, the event data descriptor shall be filled with the value of the property by calling EventDataDescCreate. ]*/
                            EventDataDescCreate(&_tlgData[_tlgIdx], context_property_value_pairs[i].value, sizeof(int8_t));
                            break;
                        case LOG_CONTEXT_PROPERTY_TYPE_uint8_t:
                            /* Codes_SRS_LOG_SINK_ETW_01_080: [ If the property type is LOG_CONTEXT_PROPERTY_TYPE_uint8_t, the event data descriptor shall be filled with the value of the property by calling EventDataDescCreate. ]*/
                            EventDataDescCreate(&_tlgData[_tlgIdx], context_property_value_pairs[i].value, sizeof(uint8_t));
                            break;
                        case LOG_CONTEXT_PROPERTY_TYPE_ascii_char_ptr:
                            /* Codes_SRS_LOG_SINK_ETW_01_067: [ If the property type is LOG_CONTEXT_PROPERTY_TYPE_ascii_char_ptr, the event data descriptor shall be filled with the value of the property by calling _tlgCreate1Sz_char. ]*/
                            _tlgCreate1Sz_char(&_tlgData[_tlgIdx], context_property_value_pairs[i].value);
                            break;
                        case LOG_CONTEXT_PROPERTY_TYPE_struct:
                            /* Codes_SRS_LOG_SINK_ETW_01_062: [ If the property type is LOG_CONTEXT_PROPERTY_TYPE_struct, no event data descriptor shall be used. ]*/
                            _tlgIdx--;
                            break;
                        }

                        _tlgIdx++;
                    }
                }

                // ... AND drumrolls, emit the event
                /* Codes_SRS_LOG_SINK_ETW_01_041: [ log_sink_etw.log_sink_log shall emit the event by calling _tlgWriteTransfer_EventWriteTransfer passing the provider, channel, number of event data descriptors and the data descriptor array. ]*/
                _tlgWriteTransfer_EventWriteTransfer(_tlgProv, &self_described_event->_tlgChannel, ((void*)0), ((void*)0), _tlgIdx, _tlgData);
            }
        }
    }
}

static const char event_name_critical[] = "LogCritical";
static const char event_name_error[] = "LogError";
static const char event_name_warning[] = "LogWarning";
static const char event_name_info[] = "LogInfo";
static const char event_name_verbose[] = "LogVerbose";
static const char event_name_unknown[] = "Unknown";

static void log_sink_etw_log(LOG_LEVEL log_level, LOG_CONTEXT_HANDLE log_context, const char* file, const char* func, int line, const char* message_format, ...)
{
    if (message_format == NULL)
    {
        /* Codes_SRS_LOG_SINK_ETW_01_001: [ If message_format is NULL, log_sink_etw.log_sink_log shall return. ]*/
        (void)printf("Invalid arguments: LOG_LEVEL log_level=%" PRI_MU_ENUM ", LOG_CONTEXT_HANDLE log_context=%p, const char* file=%s, const char* func=%s, int line=%d, const char* message_format=%s\r\n",
            MU_ENUM_VALUE(LOG_LEVEL, log_level), log_context, file, func, line, message_format);
    }
    else
    {
        const LOG_CONTEXT_PROPERTY_VALUE_PAIR* value_pairs;
        uint16_t values_count;

        internal_log_sink_etw_lazy_register_provider();

        /* Codes_SRS_LOG_SINK_ETW_01_010: [ log_sink_etw_log shall emit a self described event that shall have the name of the event as follows: ]*/

        if (log_context != NULL)
        {
            /* Codes_SRS_LOG_SINK_ETW_01_048: [ If log_context is not NULL: ]*/

            /* Codes_SRS_LOG_SINK_ETW_01_049: [ log_sink_etw.log_sink_log shall call log_context_get_property_value_pair_count to obtain the count of properties that are to be added to the ETW event. ]*/
            value_pairs = log_context_get_property_value_pairs(log_context);
            /* Codes_SRS_LOG_SINK_ETW_01_050: [ log_sink_etw.log_sink_log shall call log_context_get_property_value_pairs to obtain the properties that are to be added to the ETW event. ]*/
            values_count = (uint16_t)log_context_get_property_value_pair_count(log_context);
        }
        else
        {
            /* Codes_SRS_LOG_SINK_ETW_01_025: [ If log_context is NULL only the fields content, file, func and line shall be added to the ETW event. ]*/
            value_pairs = NULL;
            values_count = 0;
        }

        va_list args;
        va_start(args, message_format);

        switch (log_level)
        {
        default:
            /* Codes_SRS_LOG_SINK_ETW_01_017: [ Otherwise the event name shall be Unknown. ]*/
            /* Codes_SRS_LOG_SINK_ETW_01_024: [ Otherwise the ETW logging level shall be TRACE_LEVEL_NONE. ]*/
            internal_emit_self_described_event(event_name_unknown, sizeof(event_name_unknown), TRACE_LEVEL_NONE, value_pairs, values_count, message_format, file, func, line, args);
            break;
        case LOG_LEVEL_CRITICAL:
            /* Codes_SRS_LOG_SINK_ETW_01_012: [ If log_level is LOG_LEVEL_CRITICAL the event name shall be LogCritical. ]*/
            /* Codes_SRS_LOG_SINK_ETW_01_019: [ If log_level is LOG_LEVEL_CRITICAL the ETW logging level shall be TRACE_LEVEL_CRITICAL. ]*/
            internal_emit_self_described_event(event_name_critical, sizeof(event_name_critical), TRACE_LEVEL_CRITICAL, value_pairs, values_count, message_format, file, func, line, args);
            break;
        case LOG_LEVEL_ERROR:
            /* Codes_SRS_LOG_SINK_ETW_01_013: [ If log_level is LOG_LEVEL_ERROR the event name shall be LogError. ]*/
            /* Codes_SRS_LOG_SINK_ETW_01_020: [ If log_level is LOG_LEVEL_ERROR the ETW logging level shall be TRACE_LEVEL_ERROR. ]*/
            internal_emit_self_described_event(event_name_error, sizeof(event_name_error), TRACE_LEVEL_ERROR, value_pairs, values_count, message_format, file, func, line, args);
            break;
        case LOG_LEVEL_WARNING:
            /* Codes_SRS_LOG_SINK_ETW_01_014: [ If log_level is LOG_LEVEL_WARNING the event name shall be LogWarning. ]*/
            /* Codes_SRS_LOG_SINK_ETW_01_021: [ If log_level is LOG_LEVEL_WARNING the ETW logging level shall be TRACE_LEVEL_WARNING. ]*/
            internal_emit_self_described_event(event_name_warning, sizeof(event_name_warning), TRACE_LEVEL_WARNING, value_pairs, values_count, message_format, file, func, line, args);
            break;
        case LOG_LEVEL_INFO:
            /* Codes_SRS_LOG_SINK_ETW_01_015: [ If log_level is LOG_LEVEL_INFO the event name shall be LogInfo. ]*/
            /* Codes_SRS_LOG_SINK_ETW_01_022: [ If log_level is LOG_LEVEL_INFO the ETW logging level shall be TRACE_LEVEL_INFO. ]*/
            internal_emit_self_described_event(event_name_info, sizeof(event_name_info), TRACE_LEVEL_INFORMATION, value_pairs, values_count, message_format, file, func, line, args);
            break;
        case LOG_LEVEL_VERBOSE:
            /* Codes_SRS_LOG_SINK_ETW_01_016: [ If log_level is LOG_LEVEL_VERBOSE the event name shall be LogVerbose. ]*/
            /* Codes_SRS_LOG_SINK_ETW_01_023: [ If log_level is LOG_LEVEL_VERBOSE the ETW logging level shall be TRACE_LEVEL_VERBOSE. ]*/
            internal_emit_self_described_event(event_name_verbose, sizeof(event_name_verbose), TRACE_LEVEL_VERBOSE, value_pairs, values_count, message_format, file, func, line, args);
            break;
        }

        va_end(args);
    }
}

const LOG_SINK_IF log_sink_etw = { .log_sink_log = log_sink_etw_log };
