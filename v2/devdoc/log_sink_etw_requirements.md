# `log_sink_etw` requirements

`log_sink_etw` implements a log sink interface that logs the events as ETW events.

## References

[Event Tracing For Windows](https://learn.microsoft.com/en-us/windows-hardware/drivers/devtest/event-tracing-for-windows--etw-)

## Exposed API

```
    extern const LOG_SINK_IF log_sink_etw;
```

### log_sink_etw.init

The signature of `log_sink_etw.init` is:

```c
typedef int (*LOG_SINK_INIT_FUNC)(void);
```

Note: No other APIs (`deinit`, `log`) should be called while `init` executes.

**SRS_LOG_SINK_ETW_01_006: [** `log_sink_etw.init` shall register the ETW TraceLogging provider by calling `TraceLoggingRegister` (`TraceLoggingRegister_EventRegister_EventSetInformation`). **]**

**SRS_LOG_SINK_ETW_01_008: [** `log_sink_etw.init` shall emit a `LOG_LEVEL_INFO` event as a self test, printing the fact that the provider was registered and from which executable (as obtained by calling `_get_pgmptr`). **]**

**SRS_LOG_SINK_ETW_01_084: [** `log_sink_etw_log` shall use as provider GUID `DAD29F36-0A48-4DEF-9D50-8EF9036B92B4`. **]**

**SRS_LOG_SINK_ETW_01_091: [** `log_sink_etw.init` shall succeed and return 0. **]**

**SRS_LOG_SINK_ETW_01_088: [** If `TraceLoggingRegister` fails, `log_sink_etw.init` shall fail and return a non-zero value. **]**

Note: More interesting initialization for the module will come in a subsequent PR.

### log_sink_etw.deinit

The signature of `log_sink_etw.deinit` is:

```c
typedef void (*LOG_SINK_DEINIT_FUNC)(void);
```

Note: No other APIs (`init`, `log`) should be called while `deinit` executes.

**SRS_LOG_SINK_ETW_01_090: [** `log_sink_etw.deinit` shall call `TraceLoggingUnregister` to unregister the provider. **]**

### log_sink_etw.log

The signature of `log_sink_etw.log` is:

```c
typedef void (*LOG_SINK_LOG_FUNC)(LOG_LEVEL log_level, LOG_CONTEXT_HANDLE log_context, const char* file, const char* func, int line, const char* message_format, ...);
```

`log_sink_etw.log` implements logging as an ETW provider.

**SRS_LOG_SINK_ETW_01_001: [** If `message_format` is `NULL`, `log_sink_etw.log` shall return. **]**

Note: `_get_pgmptr` is documented [here](https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/get-pgmptr?view=msvc-170).

**SRS_LOG_SINK_ETW_01_083: [** If `_get_pgmptr` fails, the executable shall be printed as `UNKNOWN`. **]**

Note this can (and should) be improved to be configurable in a subsequent task.

**SRS_LOG_SINK_ETW_01_010: [** `log_sink_etw_log` shall emit a self described event that shall have the name of the event as follows: **]**

  - **SRS_LOG_SINK_ETW_01_012: [** If `log_level` is `LOG_LEVEL_CRITICAL` the event name shall be `LogCritical`. **]**

  - **SRS_LOG_SINK_ETW_01_013: [** If `log_level` is `LOG_LEVEL_ERROR` the event name shall be `LogError`. **]**

  - **SRS_LOG_SINK_ETW_01_014: [** If `log_level` is `LOG_LEVEL_WARNING` the event name shall be `LogWarning`. **]**

  - **SRS_LOG_SINK_ETW_01_015: [** If `log_level` is `LOG_LEVEL_INFO` the event name shall be `LogInfo`. **]**

  - **SRS_LOG_SINK_ETW_01_016: [** If `log_level` is `LOG_LEVEL_VERBOSE` the event name shall be `LogVerbose`. **]**

  - **SRS_LOG_SINK_ETW_01_017: [** Otherwise the event name shall be `Unknown`. **]**

- **SRS_LOG_SINK_ETW_01_018: [** Logging level: **]**

  - **SRS_LOG_SINK_ETW_01_019: [** If `log_level` is `LOG_LEVEL_CRITICAL` the ETW logging level shall be `TRACE_LEVEL_CRITICAL`. **]**

  - **SRS_LOG_SINK_ETW_01_020: [** If `log_level` is `LOG_LEVEL_ERROR` the ETW logging level shall be `TRACE_LEVEL_ERROR`. **]**

  - **SRS_LOG_SINK_ETW_01_021: [** If `log_level` is `LOG_LEVEL_WARNING` the ETW logging level shall be `TRACE_LEVEL_WARNING`. **]**

  - **SRS_LOG_SINK_ETW_01_022: [** If `log_level` is `LOG_LEVEL_INFO` the ETW logging level shall be `TRACE_LEVEL_INFO`. **]**

  - **SRS_LOG_SINK_ETW_01_023: [** If `log_level` is `LOG_LEVEL_VERBOSE` the ETW logging level shall be `TRACE_LEVEL_VERBOSE`. **]**

  - **SRS_LOG_SINK_ETW_01_024: [** Otherwise the ETW logging level shall be `TRACE_LEVEL_NONE`. **]**

- **SRS_LOG_SINK_ETW_01_025: [** If `log_context` is `NULL` only the fields `content`, `file`, `func` and `line` shall be added to the ETW event. **]**

- **SRS_LOG_SINK_ETW_01_048: [** If `log_context` is not `NULL`: **]**

  - **SRS_LOG_SINK_ETW_01_049: [** `log_sink_etw.log` shall call `log_context_get_property_value_pair_count` to obtain the count of properties that are to be added to the ETW event. **]**

  - **SRS_LOG_SINK_ETW_01_050: [** `log_sink_etw.log` shall call `log_context_get_property_value_pairs` to obtain the properties that are to be added to the ETW event. **]**

  - **SRS_LOG_SINK_ETW_01_082: [** If more than 64 properties are given in `log_context`, `log_sink_etw.log` shall not add any properties to the event. **]**

**SRS_LOG_SINK_ETW_01_042: [** `log_sink_etw.log` shall compute the metadata size for the self described event metadata as follows: **]**

- **SRS_LOG_SINK_ETW_01_043: [** Size of the event name. **]**

- **SRS_LOG_SINK_ETW_01_044: [** Size of the `content` field name. **]**

- **SRS_LOG_SINK_ETW_01_045: [** Size of the `file` field name. **]**

- **SRS_LOG_SINK_ETW_01_046: [** Size of the `func` field name. **]**

- **SRS_LOG_SINK_ETW_01_047: [** Size of the `line` field name. **]**

- **SRS_LOG_SINK_ETW_01_051: [** For each property in `log_context`, the length of the property name + 1 and one extra byte for the type of the field. **]**

- **SRS_LOG_SINK_ETW_01_052: [** For struct properties one extra byte shall be added for the field count. **]**

**SRS_LOG_SINK_ETW_01_085: [** If the size of the metadata and the formatted message exceeds 4096 bytes, `log_sink_etw.log` shall not add any properties to the event. **]**

**SRS_LOG_SINK_ETW_01_026: [** `log_sink_etw.log` shall fill a `SELF_DESCRIBED_EVENT` structure, setting the following fields: **]**

- **SRS_LOG_SINK_ETW_01_027: [** `_tlgBlobTyp` shall be set to `_TlgBlobEvent4`. **]**

Note: `_tlgBlobTyp` represents the metadata blob type in the binary form described in TraceLogging.h. It is internal to TraceLogging, and we take a dependency on it because we want to carve our own events.

- **SRS_LOG_SINK_ETW_01_028: [** `_tlgChannel` shall be set to 11. **]**

- **SRS_LOG_SINK_ETW_01_029: [** `_tlgLevel` shall be set to the appropriate logging level. **]**

- **SRS_LOG_SINK_ETW_01_030: [** `_tlgOpcode` shall be set to 0. **]**

- **SRS_LOG_SINK_ETW_01_031: [** `_tlgKeyword` shall be set to 0. **]**

- **SRS_LOG_SINK_ETW_01_032: [** `_tlgEvtMetaSize` shall be set to the computed metadata size + 4. **]**

- **SRS_LOG_SINK_ETW_01_033: [** `_tlgEvtTag` shall be set to 128. **]**

**SRS_LOG_SINK_ETW_01_034: [** `log_sink_etw.log` shall fill the event metadata: **]**

- **SRS_LOG_SINK_ETW_01_035: [** The string `content` (as field name, excluding zero terminator), followed by one byte with the value `TlgInANSISTRING`. **]**

- **SRS_LOG_SINK_ETW_01_036: [** The string `file` (as field name, excluding zero terminator), followed by one byte with the value `TlgInANSISTRING`. **]**

- **SRS_LOG_SINK_ETW_01_037: [** The string `func` (as field name, excluding zero terminator), followed by one byte with the value `TlgInANSISTRING`. **]**

- **SRS_LOG_SINK_ETW_01_038: [** The string `line` (as field name, excluding zero terminator), followed by one byte with the value `TlgInINT32`. **]**

**SRS_LOG_SINK_ETW_01_053: [** For each property in `log_context` the following shall be added to the event metadata: **]**

- **SRS_LOG_SINK_ETW_01_054: [** A string with the property name (including zero terminator) **]**

- **SRS_LOG_SINK_ETW_01_055: [** A byte with the type of property, as follows: **]**

  - **SRS_LOG_SINK_ETW_01_063: [** If the property type is `LOG_CONTEXT_PROPERTY_TYPE_ascii_char_ptr`, a byte with the value `TlgInANSISTRING` shall be added in the metadata. **]**

  - **SRS_LOG_SINK_ETW_01_064: [** If the property type is `LOG_CONTEXT_PROPERTY_TYPE_int64_t`, a byte with the value `TlgInINT64` shall be added in the metadata. **]**

  - **SRS_LOG_SINK_ETW_01_065: [** If the property type is `LOG_CONTEXT_PROPERTY_TYPE_uint64_t`, a byte with the value `TlgInUINT64` shall be added in the metadata. **]**

  - **SRS_LOG_SINK_ETW_01_069: [** If the property type is `LOG_CONTEXT_PROPERTY_TYPE_int32_t`, a byte with the value `TlgInINT32` shall be added in the metadata. **]**

  - **SRS_LOG_SINK_ETW_01_071: [** If the property type is `LOG_CONTEXT_PROPERTY_TYPE_uint32_t`, a byte with the value `TlgInUINT32` shall be added in the metadata. **]**

  - **SRS_LOG_SINK_ETW_01_073: [** If the property type is `LOG_CONTEXT_PROPERTY_TYPE_int16_t`, a byte with the value `TlgInINT16` shall be added in the metadata. **]**

  - **SRS_LOG_SINK_ETW_01_075: [** If the property type is `LOG_CONTEXT_PROPERTY_TYPE_uint16_t`, a byte with the value `TlgInUINT16` shall be added in the metadata. **]**

  - **SRS_LOG_SINK_ETW_01_077: [** If the property type is `LOG_CONTEXT_PROPERTY_TYPE_int8_t`, a byte with the value `TlgInINT8` shall be added in the metadata. **]**

  - **SRS_LOG_SINK_ETW_01_079: [** If the property type is `LOG_CONTEXT_PROPERTY_TYPE_uint8_t`, a byte with the value `TlgInUINT8` shall be added in the metadata. **]**

  - **SRS_LOG_SINK_ETW_01_056: [** If the property type is `LOG_CONTEXT_PROPERTY_TYPE_struct`, a byte with the value `_TlgInSTRUCT | _TlgInChain` shall be added in the metadata. **]**

  - **SRS_LOG_SINK_ETW_01_081: [** If the property type is any other value, no property data shall be added to the event. **]**

- **SRS_LOG_SINK_ETW_01_057: [** If the property is a struct, an extra byte shall be added in the metadata containing the number of fields in the structure. **]**

**SRS_LOG_SINK_ETW_01_039: [** `log_sink_etw.log` shall fill an `EVENT_DATA_DESCRIPTOR` array of size `2 + 1 + 1 + 1 + 1 + property count`. **]**

Note: 2 entries are for the event descriptor and metadata respectively, 4 entries for the common fields (content, file, func, line) and the rest are 1 for each of the properties.

**SRS_LOG_SINK_ETW_01_040: [** `log_sink_etw.log` shall set event data descriptor at index 2 by calling `_tlgCreate1Sz_char` with the value of the formatted message as obtained by using `printf` with the messages format `message_format` and the arguments in `...`. **]**

**SRS_LOG_SINK_ETW_01_058: [** `log_sink_etw.log` shall set event data descriptor at index 3 by calling `_tlgCreate1Sz_char` with `file`. **]**

**SRS_LOG_SINK_ETW_01_059: [** `log_sink_etw.log` shall set event data descriptor at index 4 by calling `_tlgCreate1Sz_char` with `func`. **]**

**SRS_LOG_SINK_ETW_01_060: [** `log_sink_etw.log` shall set event data descriptor at index 5 by calling `EventDataDescCreate` with `line`. **]**

**SRS_LOG_SINK_ETW_01_061: [** For each property in `log_context`: **]**

- **SRS_LOG_SINK_ETW_01_067: [** If the property type is `LOG_CONTEXT_PROPERTY_TYPE_ascii_char_ptr`, the event data descriptor shall be filled with the value of the property by calling `_tlgCreate1Sz_char`. **]**

- **SRS_LOG_SINK_ETW_01_066: [** If the property type is `LOG_CONTEXT_PROPERTY_TYPE_int64_t`, the event data descriptor shall be filled with the value of the property by calling `EventDataDescCreate`. **]**

- **SRS_LOG_SINK_ETW_01_068: [** If the property type is `LOG_CONTEXT_PROPERTY_TYPE_uint64_t`, the event data descriptor shall be filled with the value of the property by calling `EventDataDescCreate`. **]**

- **SRS_LOG_SINK_ETW_01_070: [** If the property type is `LOG_CONTEXT_PROPERTY_TYPE_int32_t`, the event data descriptor shall be filled with the value of the property by calling `EventDataDescCreate`. **]**

- **SRS_LOG_SINK_ETW_01_072: [** If the property type is `LOG_CONTEXT_PROPERTY_TYPE_uint32_t`, the event data descriptor shall be filled with the value of the property by calling `EventDataDescCreate`. **]**

- **SRS_LOG_SINK_ETW_01_074: [** If the property type is `LOG_CONTEXT_PROPERTY_TYPE_int16_t`, the event data descriptor shall be filled with the value of the property by calling `EventDataDescCreate`. **]**

- **SRS_LOG_SINK_ETW_01_076: [** If the property type is `LOG_CONTEXT_PROPERTY_TYPE_uint16_t`, the event data descriptor shall be filled with the value of the property by calling `EventDataDescCreate`. **]**

- **SRS_LOG_SINK_ETW_01_078: [** If the property type is `LOG_CONTEXT_PROPERTY_TYPE_int8_t`, the event data descriptor shall be filled with the value of the property by calling `EventDataDescCreate`. **]**

- **SRS_LOG_SINK_ETW_01_080: [** If the property type is `LOG_CONTEXT_PROPERTY_TYPE_uint8_t`, the event data descriptor shall be filled with the value of the property by calling `EventDataDescCreate`. **]**

- **SRS_LOG_SINK_ETW_01_062: [** If the property type is `LOG_CONTEXT_PROPERTY_TYPE_struct`, no event data descriptor shall be used. **]**

**SRS_LOG_SINK_ETW_01_041: [** `log_sink_etw.log` shall emit the event by calling `_tlgWriteTransfer_EventWriteTransfer` passing the provider, channel, number of event data descriptors and the data descriptor array. **]**

**SRS_LOG_SINK_ETW_01_086: [** If any error occurs `log_sink_etw.log` shall print `Error emitting ETW event` and return. **]**
