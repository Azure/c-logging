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

**SRS_LOG_HRESULT_01_001: [** If `buffer` is `NULL`, `log_hresult_fill_property` shall return 512 to indicate how many bytes shall be reserved for the `HRESULT` string formatted version. **]**

**SRS_LOG_HRESULT_01_002: [** `log_hresult_fill_property` shall call `FormatMessageA_no_newline` with `FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS`, `hresult`, 0 as language Id, `buffer` as buffer to place the output and 512 as buffer size. **]**

If `FormatMessageA` returns 0, `log_hresult_fill_property` attempt to look up the formatted string from the loaded modules:

- `log_hresult_fill_property` shall get the current process handle by calling `GetCurrentProcess`.

- `log_hresult_fill_property` shall call `EnumProcessModules` and obtain the information about 10 modules.

- For each module:

  - `log_hresult_fill_property` shall call `FormatMessageA` with `FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS`, the module handle, the `HRESULT` value, 0 as language Id, `buffer` as buffer to place the output and 512 as buffer size.

- If no module has the formatted message, `log_hresult_fill_property` shall place in buffer the string `unknown HRESULT 0x%x`, where %x is the `HRESULT value`.

If `EnumProcessModules` or other APIs fail, `log_hresult_fill_property` shall place in buffer a string indicating what failed, the last error and `unknown HRESULT 0x%x`, where %x is the `HRESULT value`.

**SRS_LOG_HRESULT_01_003: [** `log_lasterror_fill_property` shall return 512. **]**

## LOG_HRESULT

```c
#define LOG_HRESULT(hresult) \
    ...

```

`LOG_HRESULT` adds a string property with the string formatted value of `hresult` to the logging context. It is a wrapper for adding a property with the name `hresult` of  type `ascii_char_ptr`.

`LOG_HRESULT` shall expand to a `LOG_CONTEXT_PROPERTY_CUSTOM_FUNCTION` with name `hresult`, type `ascii_char_ptr` and value function call being `log_hresult_fill_property(hresult)`.
