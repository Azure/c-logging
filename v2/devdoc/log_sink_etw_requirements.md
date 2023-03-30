# `log_sink_etw` requirements

`log_sink_etw` implements a log sink interface that logs the events as ETW events.

## References

[Event Tracing For Windows](https://learn.microsoft.com/en-us/windows-hardware/drivers/devtest/event-tracing-for-windows--etw-)

## Exposed API

```
    extern const LOG_SINK_IF log_sink_etw;
```

### log_sink_etw.log_sink_log

The signature of `log_sink_etw.log_sink_log` is:

```c
typedef void (*LOG_SINK_LOG_FUNC)(LOG_LEVEL log_level, LOG_CONTEXT_HANDLE log_context, const char* file, const char* func, int line, const char* message_format, ...);
```

`log_sink_etw.log_sink_log` implements logging to an ETW provider.

If `message_format` is `NULL`, `log_sink_etw.log_sink_log` shall fail and return.

`log_sink_etw_log` shall register the ETW TraceLogging provider by calling `TraceLoggingRegister` if it was not already done.

`log_sink_etw_log` shall use as provider GUID `DAD29F36-0A48-4DEF-9D50-8EF9036B92B4`.

Note this can (and should) be improved to be configurable later.

`log_sink_etw_log` shall emit a self described event that shall have the name of the event as follows:

  - If `log_level` is `LOG_LEVEL_CRITICAL` the event name shall be `LogCritical`.

  - If `log_level` is `LOG_LEVEL_ERROR` the event name shall be `LogError`.

  - If `log_level` is `LOG_LEVEL_WARNING` the event name shall be `LogWarning`.

  - If `log_level` is `LOG_LEVEL_INFO` the event name shall be `LogInfo`.

  - If `log_level` is `LOG_LEVEL_VERBOSE` the event name shall be `LogVerbose`.

  - Otherwise the event name shall be `Unknown`.

- Logging level:

  - If `log_level` is `LOG_LEVEL_CRITICAL` the ETW logging level shall be `TRACE_LEVEL_CRITICAL`.

  - If `log_level` is `LOG_LEVEL_ERROR` the ETW logging level shall be `TRACE_LEVEL_ERROR`.

  - If `log_level` is `LOG_LEVEL_WARNING` the ETW logging level shall be `TRACE_LEVEL_WARNING`.

  - If `log_level` is `LOG_LEVEL_INFO` the ETW logging level shall be `TRACE_LEVEL_INFO`.

  - If `log_level` is `LOG_LEVEL_VERBOSE` the ETW logging level shall be `TRACE_LEVEL_VERBOSE`.

  - Otherwise the ETW logging level shall be `TRACE_LEVEL_NONE`.

- If `log_context` is `NULL` only the fields `content`, `file`, `func` and `line` shall be added to the ETW event.

- If `log_context` is not `NULL`:

`log_sink_etw.log_sink_log` shall call `log_context_get_property_value_pair_count` to obtain the count of properties that are to be added to the ETW event.

`log_sink_etw.log_sink_log` shall call `log_context_get_property_value_pairs` to obtain the properties that are to be added to the ETW event.

`log_sink_etw.log_sink_log` shall limit the number of properties that are emitted in the ETW event to 64.

`log_sink_etw.log_sink_log` shall compute the metadata size for the self described event metadata as follows:

- Length of the event name (excluding zero terminator) + 1.

- Length of the `content` field name + 1.

- Length of the `file` field name + 1.

- Length of the `func` field name + 1.

- Length of the `line` field name + 1.

- For each property in `log_context`, the length of the property name + 1 and one extra byte for the type of the field.

- For struct properties one extra byte shall be added for the field count.

`log_sink_etw.log_sink_log` shall fill a `SELF_DESCRIBED_EVENT` structure, setting the following fields:

- `_tlgBlobTyp` shall be set to `_TlgBlobEvent4`.

- `_tlgChannel` shall be set to 11.

- `_tlgLevel` shall be set to the appropriate logging level.

- `_tlgOpcode` shall be set to 0.

- `_tlgKeyword` shall be set to 0.

- `_tlgEvtMetaSize` shall be set to the computed metadata size + 4.

- `_tlgEvtTag` shall be set to 128.

`log_sink_etw.log_sink_log` shall fill the event metadata:

- The string `content` (as field name, excluding zero terminator), followed by one byte with the value `TlgInANSISTRING`.

- The string `file` (as field name, excluding zero terminator), followed by one byte with the value `TlgInANSISTRING`.

- The string `func` (as field name, excluding zero terminator), followed by one byte with the value `TlgInANSISTRING`.

- The string `line` (as field name, excluding zero terminator), followed by one byte with the value `TlgInINT32`.

For each property in `log_context` the following shall be added to the event metadata:

- A string with the property name (excluding zero terminator)

- A byte with the type of property, as follows:

  - If the property type is `LOG_CONTEXT_PROPERTY_TYPE_ascii_char_ptr`, a byte with the value `TlgInANSISTRING` shall be added in the metadata.

  - If the property type is `LOG_CONTEXT_PROPERTY_TYPE_int64_t`, a byte with the value `TlgInINT64` shall be added in the metadata.

  - If the property type is `LOG_CONTEXT_PROPERTY_TYPE_uint64_t`, a byte with the value `TlgInUINT64` shall be added in the metadata.

  - If the property type is `LOG_CONTEXT_PROPERTY_TYPE_int32_t`, a byte with the value `TlgInINT32` shall be added in the metadata.

  - If the property type is `LOG_CONTEXT_PROPERTY_TYPE_uint32_t`, a byte with the value `TlgInUINT32` shall be added in the metadata.

  - If the property type is `LOG_CONTEXT_PROPERTY_TYPE_int16_t`, a byte with the value `TlgInINT16` shall be added in the metadata.

  - If the property type is `LOG_CONTEXT_PROPERTY_TYPE_uint16_t`, a byte with the value `TlgInUINT16` shall be added in the metadata.

  - If the property type is `LOG_CONTEXT_PROPERTY_TYPE_int8_t`, a byte with the value `TlgInINT8` shall be added in the metadata.

  - If the property type is `LOG_CONTEXT_PROPERTY_TYPE_uint8_t`, a byte with the value `TlgInUINT8` shall be added in the metadata.

  - If the property type is `LOG_CONTEXT_PROPERTY_TYPE_struct`, a byte with the value `_TlgInSTRUCT | _TlgInChain` shall be added in the metadata.

  - If the property type is any other value, it shall be ignored.

- If the property is a struct, an extra byte shall be added in the metadata containing the number of fields in the structure.

`log_sink_etw.log_sink_log` shall fill an `EVENT_DATA_DESCRIPTOR` array of size `2 + 1 + 1 + 1 + 1 + property count`.

Note: 2 entries are for the event descriptor and metadata respectively, 4 entries for the well common fields (content, file, func, line) and the rest are 1 for each of the properties.

`log_sink_etw.log_sink_log` shall set event data descriptor at index 2 by calling `_tlgCreate1Sz_char` with the value of the formatted message as obtained by using `printf` with the messages format `message_format` and the arguments in `...`.

`log_sink_etw.log_sink_log` shall set event data descriptor at index 3 by calling `_tlgCreate1Sz_char` with `file`.

`log_sink_etw.log_sink_log` shall set event data descriptor at index 4 by calling `_tlgCreate1Sz_char` with `func`.

`log_sink_etw.log_sink_log` shall set event data descriptor at index 5 by calling `EventDataDescCreate` with `line`.

For each property in `log_context`:

- If the property type is `LOG_CONTEXT_PROPERTY_TYPE_int64_t`, the event data descriptor shall be filled with the value of the property by calling `EventDataDescCreate`.

- If the property type is `LOG_CONTEXT_PROPERTY_TYPE_uint64_t`, the event data descriptor shall be filled with the value of the property by calling `EventDataDescCreate`.

- If the property type is `LOG_CONTEXT_PROPERTY_TYPE_int32_t`, the event data descriptor shall be filled with the value of the property by calling `EventDataDescCreate`.

- If the property type is `LOG_CONTEXT_PROPERTY_TYPE_uint32_t`, the event data descriptor shall be filled with the value of the property by calling `EventDataDescCreate`.

- If the property type is `LOG_CONTEXT_PROPERTY_TYPE_int16_t`, the event data descriptor shall be filled with the value of the property by calling `EventDataDescCreate`.

- If the property type is `LOG_CONTEXT_PROPERTY_TYPE_uint16_t`, the event data descriptor shall be filled with the value of the property by calling `EventDataDescCreate`.

- If the property type is `LOG_CONTEXT_PROPERTY_TYPE_int8_t`, the event data descriptor shall be filled with the value of the property by calling `EventDataDescCreate`.

- If the property type is `LOG_CONTEXT_PROPERTY_TYPE_uint8_t`, the event data descriptor shall be filled with the value of the property by calling `EventDataDescCreate`.

- If the property type is `LOG_CONTEXT_PROPERTY_TYPE_struct`, no event data descriptor shall be used.

- If the property type is any other value, it shall be ignored.

`log_sink_etw.log_sink_log` shall emit the event by calling `_tlgWriteTransfer_EventWriteTransfer` passing the provider, channel, number of event data descriptors and the data descriptor array.

If any error occurs `log_sink_etw.log_sink_log` shall print `Error emitting ETW event` and return.
