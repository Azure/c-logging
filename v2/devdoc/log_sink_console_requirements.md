# `log_sink_console` requirements

`log_sink_console` implements a log sink interface that logs the events to the console.

## Exposed API

```
    extern const LOG_SINK_IF console_log_sink;
```

### log_sink_console.log_sink_log

The signature of `log_sink_console.log_sink_log` is:

```c
typedef void (*LOG_SINK_LOG_FUNC)(LOG_LEVEL log_level, LOG_CONTEXT_HANDLE log_context, const char* file, const char* func, int line, const char* message_format, ...);
```


`log_sink_console.log_sink_log` implements logging to console.

If `message_format` is `NULL`, `log_sink_console.log_sink_log` shall return.

`log_sink_console.log_sink_log` shall obtain the time by calling `time`.

`log_sink_console.log_sink_log` shall convert the time to string by calling `ctime`.

`log_sink_console.log_sink_log` shall print a line in the format:

`{log_level} Time: {formatted time} File:{file}:{line} Func:{func} {optional context information} {formatted message}`

`log_sink_console.log_sink_log` shall print the line in such a way that only one `printf` call is made in order to not break the line in multiple parts when displayed on the console.

`log_sink_console.log_sink_log` shall color the lines using ANSI color codes (https://en.wikipedia.org/wiki/ANSI_escape_code#Colors), as follows:

  `LOG_LEVEL_CRITICAL` shall be displayed with bright red `\x1b[31;1m`.

  `LOG_LEVEL_ERROR` shall be displayed with red `\x1b[31m`.

  `LOG_LEVEL_WARNING` shall be displayed with bright yellow `\x1b[33;1m`.

  `LOG_LEVEL_INFO` shall be displayed with yellow `\x1b[33m`.

  `LOG_LEVEL_VERBOSE` shall be displayed with white `\x1b[37m`.

At the end of each line that is printed, the color shall be reset by using the `\x1b[0m` code.

If `log_context` is non-`NULL`:

  `log_sink_console.log_sink_log` shall call `log_context_get_property_value_pair_count` to obtain the count of properties to print.

  `log_sink_console.log_sink_log` shall call `log_context_get_property_value_pairs` to obtain the properties to print.

  For each property:

    If the property type is `struct`:

      `log_sink_console.log_sink_log` shall obtain the number of fields in the `struct`.

      `log_sink_console.log_sink_log` shall print the name of the `struct` property and print the next `n` properties as being the fields that are part of the `struct`.

    Otherwise `log_sink_console.log_sink_log` shall call `to_string` for the property and print its name and value.

`log_sink_console.log_sink_log` shall print at most `LOG_MAX_MESSAGE_LENGTH` characters including the null terminator (the rest of the context shall be truncated).
