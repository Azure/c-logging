// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <inttypes.h>

#include "windows.h"

// this is needed for debug builds to not fire asserts about the segment used for the event metadata
#undef TLG_RAISEASSERTIONFAILURE
#define TLG_RAISEASSERTIONFAILURE() (void)0

#include "TraceLoggingProvider.h"
#include "evntrace.h"

#include "macro_utils/macro_utils.h"

#include "c_logging/logger.h"
#include "c_logging/log_sink_if.h"
#include "c_logging/log_sink_etw.h"
#include "c_logging/log_context_property_type.h"

// The following is copy/paste from TraceLoggingProvider.h

/*
*** How to send a TraceLogging event to ETW :

-Determine N = the number of EVENT_DATA_DESCRIPTORs you need for your data.
- Allocate N + 2 EVENT_DATA_DESCRIPTORs.
- Use the first EVENT_DATA_DESCRIPTOR for provider metadata.
- Use the second EVENT_DATA_DESCRIPTOR for event metadata.
- Use the remaining EVENT_DATA_DESCRIPTORs for your data.

Example:

EVENT_DATA_DESCRIPTOR DataDescriptors[N + 2];
DataDescriptors[0].Ptr = (ULONGLONG)(ULONG_PTR)&ProviderMetadata; // Defined below
DataDescriptors[0].Size = ProviderMetadata.Size;
DataDescriptors[0].Reserved = EVENT_DATA_DESCRIPTOR_TYPE_PROVIDER_METADATA;
DataDescriptors[1].Ptr = (ULONGLONG)(ULONG_PTR)&EventMetadata; // Defined below
DataDescriptors[1].Size = EventMetadata.Size;
DataDescriptors[1].Reserved = EVENT_DATA_DESCRIPTOR_TYPE_EVENT_METADATA;
// ...
// Use EventDataDescCreate(...) to populate the remaining N DataDescriptors
// with the data for your event, e.g.
// EventDataDescCreate(&DataDescriptors[2], &myData, sizeof(myData));
// Note that the data layout in the remaining N DataDescriptors must match up
// with the fields defined in EventMetadata.
// ...
EventWrite(RegHandle, &EventDescriptor, N + 2, DataDescriptors);

***How to encode metadata :

// ProviderMetadata:
// This pseudo-structure is the layout of the "provider traits" referenced by
// EventProviderSetTraits, EtwGetTraitFromProviderTraits,
// EVENT_HEADER_EXT_TYPE_PROV_TRAITS, and
// EVENT_DATA_DESCRIPTOR_TYPE_PROVIDER_METADATA.
// It provides the provider's name, plus other optional information such as group ID.
struct ProviderMetadata // Variable-length pseudo-structure, byte-aligned, tightly-packed.
{
    UINT16 Size; // = sizeof(ProviderMetadata)
    char Name[]; // UTF-8 nul-terminated provider name
    ProviderMetadataChunk AdditionalProviderInfo[]; // 0 or more chunks of data.
};

// ProviderMetadataChunk:
struct ProviderMetadataChunk // Variable-length pseudo-structure, byte-aligned, tightly-packed.
{
    UINT16 Size; // = sizeof(ProviderMetadataChunk)
    UINT8 Type; // Value from the ETW_PROVIDER_TRAIT_TYPE enumeration.
    AnyType Data;
};

// EventMetadata:
// This pseudo-structure is the layout of the "event metadata" referenced by
// EVENT_DATA_DESCRIPTOR_TYPE_EVENT_METADATA.
// It provides the event's name, event tags, and field information.
struct EventMetadata // Variable-length pseudo-structure, byte-aligned, tightly-packed.
{
    UINT16 Size; // = sizeof(EventMetadata)
    UINT8 Extension[]; // 1 or more bytes. Read until you hit a byte with high bit unset.
    char Name[]; // UTF-8 nul-terminated event name
    FieldMetadata Fields[]; // 0 or more field definitions.
};

// FieldMetadata:
struct FieldMetadata // Variable-length pseudo-structure, byte-aligned, tightly-packed.
{
    char Name[]; // UTF-8 nul-terminated field name
    UINT8 InType; // Values from the TlgIn enumeration.
    UINT8 OutType; // TlgOut enumeration. Only present if (InType & 128) == 128.
    UINT8 Extension[]; // Only present if OutType is present and (OutType & 128) == 128. Read until you hit a byte with high bit unset.
    UINT16 ValueCount;  // Only present if (InType & CountMask) == Ccount.
    UINT16 TypeInfoSize; // Only present if (InType & CountMask) == Custom.
    char TypeInfo[TypeInfoSize]; // Only present if (InType & CountMask) == Custom.
};

Each field has InType and OutType.The InType controls how the field's size is
determined and also implies a default formatting behavior that should be used
if OutType is 0 or not present.For example, TlgInINT32 indicates that the
field's size is 4 and that if no OutType is given (or if the decoder does not
understand the given OutType), the field formatting should be "signed decimal".

Some InType values have special behaviors or limitations:

-TlgInNULL : This means the field has no data(size = 0).Array of NULL is
illegal.In addition, many decoders do not correctly support NULL fields.
- TlgInBINARY : Array of BINARY should not be used.Array of BINARY is legal,
but TDH cannot decode a field with type array of BINARY.
- _TlgInPOINTER_unsupported : Do not use.This value is reserved because the
corresponding TDH value is TDH_INTYPE_POINTER.TraceLogging does not directly
support a POINTER type.Instead, TraceLogging defines TlgInPOINTER as
TlgInHEXINT32 for 32 - bit binaries and defines TlgInPOINTER as TlgInHEXINT64
for 64 - bit binaries.
- A struct has no data for itself, but it groups the following N logical fields
together into a single logical field.The value N is encoded in the OutType.
Arrays of struct are allowed, and structs can contain other structs(or
    arrays of structs).

    The EventMetadata and FieldMetadata structures contain an Extension field.The
    size of the Extension field is variable and unlimited.The decoder should
    consume bytes until it has consumed a byte with the high bit set to 0.

    At present, only the first 4 bytes of Extension are defined(the remaining
        bytes of the Extension field are reserved for future use and should be
        ignored).The first 4 bytes(if present) each contribute 7 bits of "Tag" data
    to the event or field, for up to 28 bits of "Tag" data.Tags are encoded
    high - bits first.For example, if the first byte of Extension is 0x12, that
    means the Extension field is 1 byte in length(high bit is unset), and value of
    the Tag data is 0x02400000 (0x12 low 7 bits are 0010010, used as bits 21..27 of
        the Tag data).

    * **Encoding data :

Each scalar field's data is appended end-to-end with no alignment or padding.

A fixed - length array is encoded by appending the data for N values end - to - end
with no alignment or padding, where N is the number of values as encoded in
the field's metadata. This rule applies even if the data is variable-length or
complex(e.g.it applies even if the field is a structure).Note that a length
of 0 for a fixed - length array can cause problems with some decoders.

A variable - length array is encoded as a UINT16 containing the number of values
followed by the data for those values, end - to - end.

Most value types are fixed size(e.g. sizeof(INT32) == 4), but the following
value types are variable - size:

-SID : size is 8 + 4 * SubAuthorityCount.
- ANSISTRING : size is strlen(value) + 1. (Use strnlen for safe parsing.)
- UNICODESTRING : size is wcslen(value) * 2 + 2. (Use wcsnlen for safe parsing.)
- BINARY, COUNTEDSTRING, COUNTEDANSISTRING : first two bytes are UINT16
bytecount.
*/

static const char event_name_critical[] = "LogCritical";
static const char event_name_error[] = "LogError";
static const char event_name_warning[] = "LogWarning";
static const char event_name_info[] = "LogInfo";
static const char event_name_verbose[] = "LogVerbose";
static const char event_name_unknown[] = "Unknown";

// max number of properties we want to have in the event
#define LOG_MAX_ETW_PROPERTY_VALUE_PAIR_COUNT 64

// max metadata size
#define MAX_METADATA_SIZE 4096

// ID Work Item Type Title State Assigned To Effort Remaining Work Original Estimate Value Area Iteration Path Tags
// 17751591 Task Have configurable provider To Do           One\Custom\AzureMessaging\Gallium\GaM2

/* Codes_SRS_LOG_SINK_ETW_01_084: [ log_sink_etw.init shall use as provider GUID DAD29F36-0A48-4DEF-9D50-8EF9036B92B4. ]*/
TRACELOGGING_DEFINE_PROVIDER(
    log_sink_etw_provider,
    "block_storage_2",
    (0xDAD29F36, 0x0A48, 0x4DEF, 0x9D, 0x50, 0x8E, 0xF9, 0x03, 0x6B, 0x92, 0xB4));
/*DAD29F36-0A48-4DEF-9D50-8EF9036B92B4*/

#define LOG_SINK_ETW_STATE_VALUES \
    LOG_SINK_ETW_STATE_NOT_INITIALIZED, \
    LOG_SINK_ETW_STATE_INITIALIZED

MU_DEFINE_ENUM(LOG_SINK_ETW_STATE, LOG_SINK_ETW_STATE_VALUES)
MU_DEFINE_ENUM_STRINGS(LOG_SINK_ETW_STATE, LOG_SINK_ETW_STATE_VALUES)

static LOG_SINK_ETW_STATE log_sink_etw_state = LOG_SINK_ETW_STATE_NOT_INITIALIZED;

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
    uint8_t metadata[MAX_METADATA_SIZE];
} SELF_DESCRIBED_EVENT;
__pragma(pack(pop))

static void log_sink_etw_log(LOG_LEVEL log_level, LOG_CONTEXT_HANDLE log_context, const char* file, const char* func, int line, const char* message_format, ...);

// This function was written with a little bit of reverse engineering of TraceLogging and guidance from 
// the TraceLogging.h header about the format of the self described events
static void internal_emit_self_described_event_va(const char* event_name, uint16_t event_name_length, uint8_t trace_level, const LOG_CONTEXT_PROPERTY_VALUE_PAIR* context_property_value_pairs, uint16_t property_value_count, const char* file, const char* func, int32_t line, const char* message_format, va_list args)
{
    TraceLoggingHProvider const _tlgProv = log_sink_etw_provider;
    if (trace_level < _tlgProv->LevelPlus1)
    {
        // have a stack allocated buffer where we construct the event metadata and
        SELF_DESCRIBED_EVENT _tlgEvent;
        bool add_properties = true;

        // compute event metadata size
        // event_name_length includes NULL terminator
        /* Codes_SRS_LOG_SINK_ETW_01_042: [ log_sink_etw.log shall compute the metadata size for the self described event metadata as follows: ]*/
        /* Codes_SRS_LOG_SINK_ETW_01_043: [ Size of the event name. ]*/
        uint16_t i;

        // An eve

        // alias to the event bytes
        uint8_t* pos = &_tlgEvent.metadata[0];

        // fill metadata bytes
        // first one is the event name, followed by metadata for all the fields
        // copy event name 
        (void)memcpy(pos, event_name, event_name_length); pos += event_name_length;

        // copy the field metadata (name and in type)

        /* Codes_SRS_LOG_SINK_ETW_01_034: [ log_sink_etw.log shall fill the event metadata: ]*/

        /* Codes_SRS_LOG_SINK_ETW_01_035: [ The string content (as field name, excluding zero terminator), followed by one byte with the value TlgInANSISTRING. ]*/
        /* Codes_SRS_LOG_SINK_ETW_01_044: [ Size of the content field name. ]*/
        (void)memcpy(pos, "content", sizeof("content")); pos += sizeof("content");
        *pos = TlgInANSISTRING;  pos++;

        /* Codes_SRS_LOG_SINK_ETW_01_036: [ The string file (as field name, excluding zero terminator), followed by one byte with the value TlgInANSISTRING. ]*/
        /* Codes_SRS_LOG_SINK_ETW_01_045: [ Size of the file field name. ]*/
        (void)memcpy(pos, "file", sizeof("file")); pos += sizeof("file");
        *pos = TlgInANSISTRING;  pos++;

        /* Codes_SRS_LOG_SINK_ETW_01_037: [ The string func (as field name, excluding zero terminator), followed by one byte with the value TlgInANSISTRING. ]*/
        /* Codes_SRS_LOG_SINK_ETW_01_046: [ Size of the func field name. ]*/
        (void)memcpy(pos, "func", sizeof("func")); pos += sizeof("func");
        *pos = TlgInANSISTRING;  pos++;

        /* Codes_SRS_LOG_SINK_ETW_01_038: [ The string line (as field name, excluding zero terminator), followed by one byte with the value TlgInINT32. ]*/
        /* Codes_SRS_LOG_SINK_ETW_01_047: [ Size of the line field name. ]*/
        (void)memcpy(pos, "line", sizeof("line")); pos += sizeof("line");
        *pos = TlgInINT32;  pos++;

        uint16_t metadata_size_without_properties = (uint16_t)(pos - &_tlgEvent.metadata[0]);

        if (property_value_count <= LOG_MAX_ETW_PROPERTY_VALUE_PAIR_COUNT)
        {
            /* Codes_SRS_LOG_SINK_ETW_01_053: [ For each property in log_context the following shall be added to the event metadata: ]*/
            for (i = 0; i < property_value_count; i++)
            {
                /* Codes_SRS_LOG_SINK_ETW_01_051: [ For each property in log_context, the length of the property name + 1 and one extra byte for the type of the field. ]*/
                size_t name_length = strlen(context_property_value_pairs[i].name);
                /* Codes_SRS_LOG_SINK_ETW_01_085: [ If the size of the metadata and the formatted message exceeds 4096 bytes, log_sink_etw.log shall not add any properties to the event. ]*/
                if ((name_length > UINT16_MAX) ||
                    ((uint16_t)(&_tlgEvent.metadata[MAX_METADATA_SIZE] - pos) < name_length + 1 + 1))
                {
                    (void)printf("Property %" PRIu16 "/%" PRIu16 " does not fit for ETW event, file=%s, func=%s, line=%" PRId32 ".\r\n",
                        i, property_value_count, file, func, line);
                    add_properties = false;
                    break;
                }
                else
                {
                    /* Codes_SRS_LOG_SINK_ETW_01_054: [ A string with the property name (including zero terminator) ]*/
                    (void)memcpy(pos, context_property_value_pairs[i].name, name_length + 1);
                    pos += name_length + 1;
                }

                LOG_CONTEXT_PROPERTY_TYPE property_type = context_property_value_pairs[i].type->get_type();

                /* Codes_SRS_LOG_SINK_ETW_01_055: [ A byte with the type of property, as follows: ]*/
                switch (property_type)
                {
                default:
                    /* Codes_SRS_LOG_SINK_ETW_01_081: [ If the property type is any other value, no property data shall be added to the event. ]*/
                    (void)printf("Invalid property type: %" PRI_MU_ENUM " for property %" PRIu16 "/%" PRIu16 " does not fit for ETW event, file=%s, func=%s, line=%" PRId32 ".\r\n",
                        MU_ENUM_VALUE(LOG_CONTEXT_PROPERTY_TYPE, property_type), i, property_value_count, file, func, line);
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

                    if (pos == &_tlgEvent.metadata[MAX_METADATA_SIZE])
                    {
                        (void)printf("Property %" PRIu16 "/%" PRIu16 " does not fit for ETW event, file=%s, func=%s, line=%" PRId32 ".\r\n",
                            i, property_value_count, file, func, line);
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
            /* Codes_SRS_LOG_SINK_ETW_01_082: [ If more than 64 properties are given in log_context, log_sink_etw.log shall not add any properties to the event. ]*/
            add_properties = false;
        }

        /* Codes_SRS_LOG_SINK_ETW_01_026: [ log_sink_etw.log shall fill a SELF_DESCRIBED_EVENT structure, setting the following fields: ]*/
        uint16_t metadata_size;

        if (!add_properties)
        {
            metadata_size = metadata_size_without_properties;
            pos = &_tlgEvent.metadata[metadata_size_without_properties];
        }
        else
        {
            metadata_size = (uint16_t)(pos - &_tlgEvent.metadata[0]);
        }

        char* formatted_message = (char*)pos;
        size_t available_bytes = (char*)&_tlgEvent.metadata[MAX_METADATA_SIZE] - formatted_message;

        va_list saved_args;
        va_copy(saved_args, args);

        int formatted_message_length = vsnprintf(formatted_message, available_bytes, message_format, args);
        if (formatted_message_length < 0)
        {
            /* Codes_SRS_LOG_SINK_ETW_01_086: [ If any error occurs log_sink_etw.log shall print Error emitting ETW event and return. ]*/
            (void)printf("Error emitting ETW event with %" PRIu16 " properties, file=%s, func=%s, line=%" PRId32 ".\r\n",
                property_value_count, file, func, line);
        }
        else
        {
            if ((size_t)formatted_message_length >= available_bytes)
            {
                if (add_properties)
                {
                    // did not fit, print again without properties and limit it
                    /* Codes_SRS_LOG_SINK_ETW_01_085: [ If the size of the metadata and the formatted message exceeds 4096 bytes, log_sink_etw.log shall not add any properties to the event. ]*/
                    add_properties = false;
                    metadata_size = metadata_size_without_properties;

                    formatted_message = (char*)&_tlgEvent.metadata[metadata_size_without_properties];
                    available_bytes = (char*)&_tlgEvent.metadata[MAX_METADATA_SIZE] - formatted_message;
                    formatted_message_length = vsnprintf(formatted_message, available_bytes, message_format, saved_args);
                    if (formatted_message_length < 0)
                    {
                        /* Codes_SRS_LOG_SINK_ETW_01_086: [ If any error occurs log_sink_etw.log shall print Error emitting ETW event and return. ]*/
                        (void)printf("Error emitting ETW event with %" PRIu16 " properties, file=%s, func=%s, line=%" PRId32 ".\r\n",
                            property_value_count, file, func, line);
                    }
                    else
                    {
                        // make sure it is zero terminated
                        formatted_message[available_bytes - 1] = '\0';
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
                _tlgEvent._tlgBlobTyp = _TlgBlobEvent4;

                /* Codes_SRS_LOG_SINK_ETW_01_028: [ _tlgChannel shall be set to 11. ]*/
                _tlgEvent._tlgChannel = 11;

                /* Codes_SRS_LOG_SINK_ETW_01_018: [ Logging level: ]*/
                /* Codes_SRS_LOG_SINK_ETW_01_029: [ _tlgLevel shall be set to the appropriate logging level. ]*/
                _tlgEvent._tlgLevel = trace_level;

                /* Codes_SRS_LOG_SINK_ETW_01_030: [ _tlgOpcode shall be set to 0. ]*/
                _tlgEvent._tlgOpcode = 0;

                /* Codes_SRS_LOG_SINK_ETW_01_031: [ _tlgKeyword shall be set to 0. ]*/
                _tlgEvent._tlgKeyword = 0;

                /* Codes_SRS_LOG_SINK_ETW_01_032: [ _tlgEvtMetaSize shall be set to the computed metadata size + 4. ]*/
                _tlgEvent._tlgEvtMetaSize = metadata_size + 4;

                /* Codes_SRS_LOG_SINK_ETW_01_033: [ _tlgEvtTag shall be set to 128. ]*/
                _tlgEvent._tlgEvtTag = 128;

                // now we need to fill in the event data descriptors
                // first 2 are actually reserved for the event descriptor and metadata respectively
                // all the rest starting at index 2 are actual data payloads in the event:
                // 1 for the content field (containing the formatted message)
                // 1 for the file field
                // 1 for the func field
                // 1 for the line field
                // n entries (1 for each property)
                /* Codes_SRS_LOG_SINK_ETW_01_039: [ log_sink_etw.log shall fill an EVENT_DATA_DESCRIPTOR array of size 2 + 1 + 1 + 1 + 1 + property count. ]*/
                EVENT_DATA_DESCRIPTOR _tlgData[2 + 1 + 1 + 1 + 1 + LOG_MAX_ETW_PROPERTY_VALUE_PAIR_COUNT];

                uint32_t _tlgIdx = 2;

                /* Codes_SRS_LOG_SINK_ETW_01_040: [ log_sink_etw.log shall set event data descriptor at index 2 by calling _tlgCreate1Sz_char with the value of the formatted message as obtained by using printf with the messages format message_format and the arguments in .... ]*/
                _tlgCreate1Sz_char(&_tlgData[_tlgIdx], formatted_message);
                _tlgIdx++;
                /* Codes_SRS_LOG_SINK_ETW_01_058: [ log_sink_etw.log shall set event data descriptor at index 3 by calling _tlgCreate1Sz_char with file. ]*/
                _tlgCreate1Sz_char(&_tlgData[_tlgIdx], file);
                _tlgIdx++;
                /* Codes_SRS_LOG_SINK_ETW_01_059: [ log_sink_etw.log shall set event data descriptor at index 4 by calling _tlgCreate1Sz_char with func. ]*/
                _tlgCreate1Sz_char(&_tlgData[_tlgIdx], func);
                _tlgIdx++;
                /* Codes_SRS_LOG_SINK_ETW_01_060: [ log_sink_etw.log shall set event data descriptor at index 5 by calling EventDataDescCreate with line. ]*/
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
                /* Codes_SRS_LOG_SINK_ETW_01_041: [ log_sink_etw.log shall emit the event by calling _tlgWriteTransfer_EventWriteTransfer passing the provider, channel, number of event data descriptors and the data descriptor array. ]*/
                _tlgWriteTransfer_EventWriteTransfer(_tlgProv, &_tlgEvent._tlgChannel, NULL, NULL, _tlgIdx, _tlgData);
            }
        }

        va_end(saved_args);
    }
}

static void internal_emit_self_described_event(const char* event_name, uint16_t event_name_length, uint8_t trace_level, const LOG_CONTEXT_PROPERTY_VALUE_PAIR* context_property_value_pairs, uint16_t property_value_count, const char* file, const char* func, int32_t line, const char* message_format, ...)
{
    va_list args;
    va_start(args, message_format);

    internal_emit_self_described_event_va(event_name, event_name_length, trace_level, context_property_value_pairs, property_value_count, file, func, line, message_format, args);

    va_end(args);
}

static int log_sink_etw_init(void)
{
    int result;

    if (log_sink_etw_state == LOG_SINK_ETW_STATE_NOT_INITIALIZED)
    {
        /* Codes_SRS_LOG_SINK_ETW_01_006: [ log_sink_etw.init shall register the ETW TraceLogging provider by calling TraceLoggingRegister (TraceLoggingRegister_EventRegister_EventSetInformation). ]*/
        TLG_STATUS register_result = TraceLoggingRegister(log_sink_etw_provider);
        if (!SUCCEEDED(register_result))
        {
            /* Codes_SRS_LOG_SINK_ETW_01_088: [ If TraceLoggingRegister fails, log_sink_etw.init shall fail and return a non-zero value. ]*/
            (void)printf("ETW provider was NOT registered (register_result=0x%08x).\r\n", register_result);

            result = MU_FAILURE;
        }
        else
        {
            char* temp_executable_full_path_name;
            const char* executable_full_path_name;

            if (_get_pgmptr(&temp_executable_full_path_name) != 0)
            {
                /* Codes_SRS_LOG_SINK_ETW_01_083: [ If _get_pgmptr fails, the executable shall be printed as UNKNOWN. ]*/
                executable_full_path_name = "UNKNOWN";
            }
            else
            {
                executable_full_path_name = temp_executable_full_path_name;
            }

            /* Codes_SRS_LOG_SINK_ETW_01_008: [ log_sink_etw.init shall emit a LOG_LEVEL_INFO event as a self test, printing the fact that the provider was registered and from which executable (as obtained by calling _get_pgmptr). ]*/
            internal_emit_self_described_event(event_name_info, sizeof(event_name_info), TRACE_LEVEL_INFORMATION, NULL, 0, __FILE__, __FUNCTION__, __LINE__, "ETW provider was registered succesfully (self test). Executable file full path name = %s", executable_full_path_name);

            log_sink_etw_state = LOG_SINK_ETW_STATE_INITIALIZED;

            /* Codes_SRS_LOG_SINK_ETW_01_091: [ log_sink_etw.init shall succeed and return 0. ] */
            result = 0;
        }
    }
    else
    {
        /* Codes_SRS_LOG_SINK_ETW_01_092: [ If the module is already initialized, log_sink_etw.init shall fail and return a non-zero value. ]*/
        (void)printf("log_sink_etw_init called in %" PRI_MU_ENUM "\r\n", MU_ENUM_VALUE(LOG_SINK_ETW_STATE, log_sink_etw_state));
        result = MU_FAILURE;
    }

    return result;
}

static void log_sink_etw_deinit(void)
{
    switch (log_sink_etw_state)
    {
    default:
    case LOG_SINK_ETW_STATE_NOT_INITIALIZED:
        /* Codes_SRS_LOG_SINK_ETW_01_093: [ If the module is not initialized, log_sink_etw.deinit shall return. ]*/
        (void)printf("log_sink_etw_deinit called in %" PRI_MU_ENUM "\r\n", MU_ENUM_VALUE(LOG_SINK_ETW_STATE, log_sink_etw_state));
        break;
    case LOG_SINK_ETW_STATE_INITIALIZED:
        /* Codes_SRS_LOG_SINK_ETW_01_090: [ log_sink_etw.deinit shall return. ] */
        TraceLoggingUnregister(log_sink_etw_provider);
        log_sink_etw_state = LOG_SINK_ETW_STATE_NOT_INITIALIZED;
        break;
    }
}

static void log_sink_etw_log(LOG_LEVEL log_level, LOG_CONTEXT_HANDLE log_context, const char* file, const char* func, int line, const char* message_format, ...)
{
    if (message_format == NULL)
    {
        /* Codes_SRS_LOG_SINK_ETW_01_001: [ If message_format is NULL, log_sink_etw.log shall return. ]*/
        (void)printf("Invalid arguments: LOG_LEVEL log_level=%" PRI_MU_ENUM ", LOG_CONTEXT_HANDLE log_context=%p, const char* file=%s, const char* func=%s, int line=%d, const char* message_format=%s\r\n",
            MU_ENUM_VALUE(LOG_LEVEL, log_level), log_context, file, func, line, message_format);
    }
    else
    {
        const LOG_CONTEXT_PROPERTY_VALUE_PAIR* value_pairs;
        uint16_t values_count;

        if (log_context != NULL)
        {
            /* Codes_SRS_LOG_SINK_ETW_01_048: [ If log_context is not NULL: ]*/

            /* Codes_SRS_LOG_SINK_ETW_01_049: [ log_sink_etw.log shall call log_context_get_property_value_pair_count to obtain the count of properties that are to be added to the ETW event. ]*/
            value_pairs = log_context_get_property_value_pairs(log_context);
            /* Codes_SRS_LOG_SINK_ETW_01_050: [ log_sink_etw.log shall call log_context_get_property_value_pairs to obtain the properties that are to be added to the ETW event. ]*/
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

        /* Codes_SRS_LOG_SINK_ETW_01_010: [ log_sink_etw_log shall emit a self described event that shall have the name of the event as follows: ]*/
        switch (log_level)
        {
        default:
            /* Codes_SRS_LOG_SINK_ETW_01_017: [ Otherwise the event name shall be Unknown. ]*/
            /* Codes_SRS_LOG_SINK_ETW_01_024: [ Otherwise the ETW logging level shall be TRACE_LEVEL_NONE. ]*/
            internal_emit_self_described_event_va(event_name_unknown, sizeof(event_name_unknown), TRACE_LEVEL_NONE, value_pairs, values_count, file, func, line, message_format, args);
            break;
        case LOG_LEVEL_CRITICAL:
            /* Codes_SRS_LOG_SINK_ETW_01_012: [ If log_level is LOG_LEVEL_CRITICAL the event name shall be LogCritical. ]*/
            /* Codes_SRS_LOG_SINK_ETW_01_019: [ If log_level is LOG_LEVEL_CRITICAL the ETW logging level shall be TRACE_LEVEL_CRITICAL. ]*/
            internal_emit_self_described_event_va(event_name_critical, sizeof(event_name_critical), TRACE_LEVEL_CRITICAL, value_pairs, values_count, file, func, line, message_format, args);
            break;
        case LOG_LEVEL_ERROR:
            /* Codes_SRS_LOG_SINK_ETW_01_013: [ If log_level is LOG_LEVEL_ERROR the event name shall be LogError. ]*/
            /* Codes_SRS_LOG_SINK_ETW_01_020: [ If log_level is LOG_LEVEL_ERROR the ETW logging level shall be TRACE_LEVEL_ERROR. ]*/
            internal_emit_self_described_event_va(event_name_error, sizeof(event_name_error), TRACE_LEVEL_ERROR, value_pairs, values_count, file, func, line, message_format, args);
            break;
        case LOG_LEVEL_WARNING:
            /* Codes_SRS_LOG_SINK_ETW_01_014: [ If log_level is LOG_LEVEL_WARNING the event name shall be LogWarning. ]*/
            /* Codes_SRS_LOG_SINK_ETW_01_021: [ If log_level is LOG_LEVEL_WARNING the ETW logging level shall be TRACE_LEVEL_WARNING. ]*/
            internal_emit_self_described_event_va(event_name_warning, sizeof(event_name_warning), TRACE_LEVEL_WARNING, value_pairs, values_count, file, func, line, message_format, args);
            break;
        case LOG_LEVEL_INFO:
            /* Codes_SRS_LOG_SINK_ETW_01_015: [ If log_level is LOG_LEVEL_INFO the event name shall be LogInfo. ]*/
            /* Codes_SRS_LOG_SINK_ETW_01_022: [ If log_level is LOG_LEVEL_INFO the ETW logging level shall be TRACE_LEVEL_INFO. ]*/
            internal_emit_self_described_event_va(event_name_info, sizeof(event_name_info), TRACE_LEVEL_INFORMATION, value_pairs, values_count, file, func, line, message_format, args);
            break;
        case LOG_LEVEL_VERBOSE:
            /* Codes_SRS_LOG_SINK_ETW_01_016: [ If log_level is LOG_LEVEL_VERBOSE the event name shall be LogVerbose. ]*/
            /* Codes_SRS_LOG_SINK_ETW_01_023: [ If log_level is LOG_LEVEL_VERBOSE the ETW logging level shall be TRACE_LEVEL_VERBOSE. ]*/
            internal_emit_self_described_event_va(event_name_verbose, sizeof(event_name_verbose), TRACE_LEVEL_VERBOSE, value_pairs, values_count, file, func, line, message_format, args);
            break;
        }

        va_end(args);
    }
}

const LOG_SINK_IF log_sink_etw =
{
    .init = log_sink_etw_init,
    .deinit = log_sink_etw_deinit,
    .log = log_sink_etw_log
};
