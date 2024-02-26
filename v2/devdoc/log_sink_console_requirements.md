# `log_sink_console` requirements

`log_sink_console` implements a log sink interface that logs the events to the console.

## Exposed API

```
    extern const LOG_SINK_IF log_sink_console;
```

### log_sink_console.init

The signature of `log_sink_console.init` is:

```c
typedef int (*LOG_SINK_INIT_FUNC)(void);
```

**SRS_LOG_SINK_CONSOLE_01_027: [** `log_sink_console.init` shall return 0. **]** Note: No initialization is needed for the console sink.

### log_sink_console.deinit

The signature of `log_sink_console.deinit` is:

```c
typedef void (*LOG_SINK_DEINIT_FUNC)(void);
```

**SRS_LOG_SINK_CONSOLE_01_028: [** `log_sink_console.deinit` shall return. **]**

### log_sink_console.log

The signature of `log_sink_console.log` is:

```c
typedef void (*LOG_SINK_LOG_FUNC)(LOG_LEVEL log_level, LOG_CONTEXT_HANDLE log_context, const char* file, const char* func, int line, const char* message_format, ...);
```

`log_sink_console.log` implements logging to console.

**SRS_LOG_SINK_CONSOLE_01_001: [** If `message_format` is `NULL`, `log_sink_console.log` shall print an error and return. **]**

**SRS_LOG_SINK_CONSOLE_01_002: [** `log_sink_console.log` shall obtain the time by calling `time`. **]**

**SRS_LOG_SINK_CONSOLE_01_003: [** `log_sink_console.log` shall convert the time to string by calling `ctime`. **]**

**SRS_LOG_SINK_CONSOLE_01_004: [** `log_sink_console.log` shall print a line in the format: `{log_level} Time: {formatted time} File:{file}:{line} Func:{func} {optional context information} {formatted message}` **]**

**SRS_LOG_SINK_CONSOLE_01_023: [** If the call to `time` fails then `log_sink_console.log` shall print the time as `NULL`. **]**

**SRS_LOG_SINK_CONSOLE_01_024: [** If the call to `ctime` fails then `log_sink_console.log` shall print the time as `NULL`. **]**

**SRS_LOG_SINK_CONSOLE_01_005: [** In order to not break the line in multiple parts when displayed on the console, `log_sink_console.log` shall print the line in such a way that only one `printf` call is made. **]**

**SRS_LOG_SINK_CONSOLE_01_006: [** `log_sink_console.log` shall color the lines using ANSI color codes (https://en.wikipedia.org/wiki/ANSI_escape_code#Colors), as follows: **]**

  **SRS_LOG_SINK_CONSOLE_01_007: [** `LOG_LEVEL_CRITICAL` shall be displayed with bright red `\x1b[31;1m`. **]**

  **SRS_LOG_SINK_CONSOLE_01_008: [** `LOG_LEVEL_ERROR` shall be displayed with red `\x1b[31m`. **]**

  **SRS_LOG_SINK_CONSOLE_01_009: [** `LOG_LEVEL_WARNING` shall be displayed with yellow `\x1b[33m`. **]**

  **SRS_LOG_SINK_CONSOLE_01_010: [** `LOG_LEVEL_INFO` shall be displayed with white `\x1b[37m`. **]**

  **SRS_LOG_SINK_CONSOLE_01_011: [** `LOG_LEVEL_VERBOSE` shall be displayed with gray `\x1b[90m`. **]**

**SRS_LOG_SINK_CONSOLE_01_012: [** At the end of each line that is printed, the color shall be reset by using the `\x1b[0m` code. **]**

**SRS_LOG_SINK_CONSOLE_01_013: [** If `log_context` is non-`NULL`: **]**

 - **SRS_LOG_SINK_CONSOLE_01_014: [** `log_sink_console.log` shall call `log_context_get_property_value_pair_count` to obtain the count of properties to print. **]**

 - **SRS_LOG_SINK_CONSOLE_01_015: [** `log_sink_console.log` shall call `log_context_get_property_value_pairs` to obtain the properties to print. **]**

 - **SRS_LOG_SINK_CONSOLE_42_001: [** `log_sink_console.log` shall call `log_context_property_to_string` to print the properties to the string buffer. **]**

**SRS_LOG_SINK_CONSOLE_01_021: [** `log_sink_console.log` shall print at most `LOG_MAX_MESSAGE_LENGTH` characters including the null terminator (the rest of the context shall be truncated). **]**

**SRS_LOG_SINK_CONSOLE_01_022: [** If any encoding error occurs during formatting of the line (i.e. if any `printf` class functions fails), `log_sink_console.log` shall print `Error formatting log line` and return. **]**

