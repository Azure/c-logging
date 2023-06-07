# `log_lasterror` requirements

`log_lasterror` implements a custom context property that adds to a logging context the Windows last error value at the time of creation of the context.

## Exposed API

```c
int log_lasterror_fill_property(void* buffer)

#define LOG_LASTERROR() \
    ...
```

## log_lasterror_fill_property

```c
int log_lasterror_fill_property(void* buffer);
```

`log_lasterror_fill_property` fills a property buffer with the last error formatter string. There is no check on the size of the buffer (it is by convention that the buffer has 512 bytes in size).

**SRS_LOG_LASTERROR_01_002: [** If `buffer` is `NULL`, `log_lasterror_fill_property` shall return 512 to indicate how many bytes shall be reserved for the last error string formatted version. **]**

**SRS_LOG_LASTERROR_01_003: [** Otherwise, `log_lasterror_fill_property` shall call `GetLastError` to obtain the last error ionformation. **]**

**SRS_LOG_LASTERROR_01_004: [** `log_lasterror_fill_property` shall call `FormatMessageA` with `FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS`, the last error value, `LANG_NEUTRAL` as language Id, `buffer` as buffer to place the output and 512 as buffer size. **]**

**SRS_LOG_LASTERROR_01_005: [** If `FormatMessageA` fails, `log_lasterror_fill_property` shall copy in `buffer` the string `failure in FormatMessageA` and return the size of the `failure in FormatMessageA` string. **]**

**SRS_LOG_LASTERROR_01_006: [** Otherwise, `log_lasterror_fill_property` shall remove any `\r` or `\n` characters that have been placed at the end of the formatted output by `FormatMessageA`. **]**

**SRS_LOG_LASTERROR_01_007: [** `log_lasterror_fill_property` shall return 512. **]**

Note: the implementation declares that it uses always 512 bytes as we do not want to have extra calls to `FormatMessageA`.

## LOG_LASTERROR

```c
#define LOG_LASTERROR() \
    ...

```

`LOG_LASTERROR` adds the last error value on Windows to the logging context. It is a wrapper for adding a property with the name `LastError` of  type `int32_t`.

**SRS_LOG_LASTERROR_01_001: [** `LOG_LASTERROR` shall expand to a `LOG_CONTEXT_PROPERTY` with name `LastError`, type `int32_t` and value the result of calling `GetLastError`. **]**
