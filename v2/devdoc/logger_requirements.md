# `logger` requirements

`logger` implements the entry point of the logging library.

## Exposed API

```c
int logger_init(void);
void logger_deinit(void);

void logger_log(LOG_LEVEL log_level, LOG_CONTEXT_HANDLE log_context, const char* file, const char* func, int line_no, const char* format, ...);

#define LOGGER_LOG(log_level, log_context, format, ...) \
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
int logger_deinit(void);
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
