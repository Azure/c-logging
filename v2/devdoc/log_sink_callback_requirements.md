# `log_sink_callback` requirements

`log_sink_callback` implements a log sink interface that formats log messages to string and passes the level and string to a callback function. This creates a pluggable log sink that can forward log messages on to other logging systems.

Note that this assumes the other logging system cares about only the level and the log message. The log message will encode the original message as well as the timestamp, file name, function name, line number, and additional context information.

## Exposed API

```c
    typedef void (*LOG_SINK_CALLBACK_LOG)(void* context, LOG_LEVEL log_level, const char* message);
    int log_sink_callback_set_callback(LOG_SINK_CALLBACK_LOG log_callback, void* context);

    extern const LOG_SINK_IF log_sink_callback;
```

### log_sink_callback.init

The signature of `log_sink_callback.init` is:

```c
typedef int (*LOG_SINK_INIT_FUNC)(void);
```

Note: the only initialization needed is to set the callback function. However, this is a separate setup function because it requires arguments.

**SRS_LOG_SINK_CALLBACK_42_001: [** `log_sink_callback.init` shall return 0. **]**

### log_sink_callback.deinit

The signature of `log_sink_callback.deinit` is:

```c
typedef void (*LOG_SINK_DEINIT_FUNC)(void);
```

**SRS_LOG_SINK_CALLBACK_42_002: [** `log_sink_callback.deinit` shall return. **]**

### log_sink_callback_set_callback

```c
int log_sink_callback_set_callback(LOG_SINK_CALLBACK_LOG log_callback, void* context);
```

`log_sink_callback_set_callback` sets the `log_callback` and `context`. This function is not thread-safe. It should be called before any logging is done. Log messages before this is called will be dropped. This may be called multiple times but only the most recent `log_callback` and `context` will be used.

**SRS_LOG_SINK_CALLBACK_42_003: [** If `log_callback` is `NULL` then `log_sink_callback_set_callback` shall fail and return a non-zero value. **]**

**SRS_LOG_SINK_CALLBACK_42_004: [** `log_sink_callback_set_callback` shall store `log_callback` and `context` so that it is used by all future calls to `log_sink_callback.log`. **]**

**SRS_LOG_SINK_CALLBACK_42_005: [** `log_sink_callback_set_callback` shall return 0. **]**

### log_sink_callback.log

The signature of `log_sink_callback.log` is:

```c
typedef void (*LOG_SINK_LOG_FUNC)(LOG_LEVEL log_level, LOG_CONTEXT_HANDLE log_context, const char* file, const char* func, int line, const char* message_format, ...);
```

`log_sink_callback.log` implements logging to a string which is passed to a callback function. The setup function `log_sink_callback_set_callback` must be called or the callback will just be a no-op.

**SRS_LOG_SINK_CALLBACK_42_006: [** If `message_format` is `NULL`, `log_sink_callback.log` shall call the `log_callback` with an error message and return. **]**

**SRS_LOG_SINK_CALLBACK_42_007: [** `log_sink_callback.log` shall obtain the time by calling `time`. **]**

**SRS_LOG_SINK_CALLBACK_42_008: [** `log_sink_callback.log` shall write the time to string by calling `ctime`. **]**

**SRS_LOG_SINK_CALLBACK_42_009: [** `log_sink_callback.log` shall create a line in the format: `Time: {formatted time} File:{file}:{line} Func:{func} {optional context information} {formatted message}`. **]**

**SRS_LOG_SINK_CALLBACK_42_010: [** If the call to `time` fails then `log_sink_callback.log` shall format the time as `NULL`. **]**

**SRS_LOG_SINK_CALLBACK_42_011: [** If the call to `ctime` fails then `log_sink_callback.log` shall format the time as `NULL`. **]**

**SRS_LOG_SINK_CALLBACK_42_012: [** If `log_context` is non-`NULL`: **]**

 - **SRS_LOG_SINK_CALLBACK_42_013: [** `log_sink_callback.log` shall call `log_context_get_property_value_pair_count` to obtain the count of properties. **]**

 - **SRS_LOG_SINK_CALLBACK_42_014: [** `log_sink_callback.log` shall call `log_context_get_property_value_pairs` to obtain the properties. **]**

 - **SRS_LOG_SINK_CALLBACK_42_015: [** `log_sink_callback.log` shall call `log_context_property_to_string` to write the properties to the string buffer. **]**

**SRS_LOG_SINK_CALLBACK_42_016: [** `log_sink_callback.log` shall include at most `LOG_MAX_MESSAGE_LENGTH` characters including the null terminator in the callback argument (the rest of the context shall be truncated). **]**

**SRS_LOG_SINK_CALLBACK_42_017: [** If any encoding error occurs during formatting of the line (i.e. if any `printf` class functions fails), `log_sink_callback.log` shall call the `log_callback` with `Error formatting log line` and return. **]**

**SRS_LOG_SINK_CALLBACK_42_018: [** `log_sink_callback.log` shall call `log_callback` with its `context`, `log_level`, and the formatted message. **]**
