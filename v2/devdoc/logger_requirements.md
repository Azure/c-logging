# `logger` requirements

`logger` implements the entry point of the logging library.

## Exposed API

```c
    extern const uint32_t log_sink_count;
    extern const LOG_SINK_IF* log_sinks[];

    int logger_init(void);
    void logger_deinit(void);
    void logger_log(LOG_LEVEL log_level, LOG_CONTEXT_HANDLE log_context, const char* file, const char* func, int line_no, const char* format, ...);

#define LOGGER_LOG(log_level, log_context, format, ...) \
  // ...

#define LOGGER_LOG_EX(log_level, ...) \
  // ...
```

### logger_init

```c
int logger_init(void);
```

`logger_init` initializes all logger sinks.

Note: No other `logger` API should be called while `logger_init` executes.

**SRS_LOGGER_01_002: [** If `logger` is already initialized, `logger_init` shall fail and return a non-zero value. **]**

**SRS_LOGGER_01_003: [** `logger_init` shall call the `init` function of every sink that is configured to be used. **]**

**SRS_LOGGER_01_004: [** If `init` fails, all sinks already initialized shall have their `deinit` function called and `logger_init` shall fail and return a non-zero value. **]**

**SRS_LOGGER_01_005: [** Otherwise, `logger_init` shall succeed and return 0. **]**

### logger_deinit

```c
void logger_deinit(void);
```

`logger_deinit` de-initializes all logger sinks.

Note: No other `logger` API should be called while `logger_deinit` executes.

**SRS_LOGGER_01_006: [** If `logger` is not initialized, `logger_deinit` shall return. **]**

**SRS_LOGGER_01_007: [** `logger_deinit` shall call the `deinit` function of every sink that is configured to be used. **]**

### LOGGER_LOG

```c
#define LOGGER_LOG(log_level, log_context, format, ...) \
  // ...
```

`LOGGER_LOG` allows the user to log one logging event.

**SRS_LOGGER_01_001: [** `LOGGER_LOG` shall call the `log` function of every sink that is configured to be used. **]**

### LOGGER_LOG

```c
#define LOGGER_LOG_EX(log_level, ...) \
  // ...
```

`LOGGER_LOG_EX` allows the user to log one logging event with a ist of logging context properties.

It is syntactic sugar for creating a context and calling `LOGGER_LOG`.

**SRS_LOGGER_01_008: [** `LOGGER_LOG_EX` shall call the `log` function of every sink that is configured to be used. **]**

If no properties are specified in `...`, `LOGGER_LOG_EX` shall call `log` with `log_context` being `NULL`.

Otherwise, `LOGGER_LOG_EX` shall construct a log context with all the properties specified in `...`.

Each `LOG_CONTEXT_STRING_PROPERTY` and `LOG_CONTEXT_PROPERTY` entry in `...` shall be added as properties in the context that is passed to `log`.

```c
LOG_CONTEXT_MESSAGE(message_format, ...)
```

If `LOG_CONTEXT_MESSAGE` is specified in `...`, `message_format` shall be passed to the `log` call together with a argument list made out of the `...` portion of the `LOG_CONTEXT_MESSAGE` macro.
