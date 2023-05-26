# `logger` requirements

`logger` implements the entry point of the logging library.

## Exposed API

```c
void logger_log(LOG_LEVEL log_level, LOG_CONTEXT_HANDLE log_context, const char* file, const char* func, int line_no, const char* format, ...);

#define LOGGER_LOG(log_level, log_context, format, ...) \
  // ...
```

### LOGGER_LOG

```c
#define LOGGER_LOG(log_level, log_context, format, ...) \
  // ...
```

`LOGGER_LOG` allows the user to log one logging event.

**SRS_LOGGER_01_001: [** `LOGGER_LOG` shall call the `log` function of every sink that is configured to be used. **]**
