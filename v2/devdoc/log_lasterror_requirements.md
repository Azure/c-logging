# `log_lasterror` requirements

`log_lasterror` implements a custom context property that adds to a logging context the Windows last error value at the time of creation of the context.

## Exposed API

```
#define LOG_LASTERROR() \
    ...
```

## LOG_LASTERROR

```c
#define LOG_LASTERROR() \
    ...
```

`LOG_LASTERROR` adds the last error value on Windows to the logging context. It is a wrapper for adding a property with the name `LastError` of  type `int32_t`.

**SRS_LOG_LASTERROR_01_001: [** `LOG_LASTERROR` shall expand to a `LOG_CONTEXT_PROPERTY` with name `LastError`, type `int32_t` and value the result of calling `GetLastError`. **]**
