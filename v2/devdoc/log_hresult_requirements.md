# `log_hresult` requirements

`log_hresult` implements a custom context property that adds to a logging context a formatted HRESULT value.

## Exposed API

```c
int log_hresult_fill_property(void* buffer, ...)

#define LOG_HRESULT(hresult) \
    ...
```

## log_hresult_fill_property

```c
int log_hresult_fill_property(void* buffer, ...);
```

`log_hresult_fill_property` fills a property buffer with a formatted string for an `HRESULT` value. There is no check on the size of the buffer (it is by convention that the buffer has 512 bytes in size).

If `buffer` is `NULL`, `log_hresult_fill_property` shall return 512 to indicate how many bytes shall be reserved for the last error string formatted version.

Otherwise, `log_hresult_fill_property` shall call `GetLastError` to obtain the last error information.

`log_hresult_fill_property` shall call `FormatMessageA` with `FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS`, the last error value, `LANG_NEUTRAL` as language Id, `buffer` as buffer to place the output and 512 as buffer size.

If `FormatMessageA` returns 0, `log_lasterror_fill_property` shall copy in `buffer` the string `failure in FormatMessageA` and return 512.

Otherwise, `log_lasterror_fill_property` shall remove any `\r` or `\n` characters that have been placed at the end of the formatted output by `FormatMessageA`.

`log_lasterror_fill_property` shall return 512.

Note: the implementation declares that it uses always 512 bytes as we do not want to have extra calls to `FormatMessageA`.

## LOG_HRESULT

```c
#define LOG_HRESULT(hresult) \
    ...

```

`LOG_HRESULT` adds a string property with the string formatted value of hresult to the logging context. It is a wrapper for adding a property with the name `hresult` of  type `ascii_char_ptr`.

`LOG_HRESULT` shall expand to a `LOG_CONTEXT_PROPERTY_CUSTOM_FUNCTION` with name `hresult`, type `ascii_char_ptr` and value function call being `log_lasterror_fill_property(hresult)`.
