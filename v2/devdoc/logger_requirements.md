# `logger` requirements

`logger` implements the entry point of the logging library.

## Exposed API

```c
    extern uint32_t log_sink_count;
    extern const LOG_SINK_IF** log_sinks;

    typedef struct LOGGER_CONFIG_TAG
    {
        uint32_t log_sink_count;
        const LOG_SINK_IF** log_sinks;
    } LOGGER_CONFIG;

/*a format specifier that can be used in printf function family to print the values behind a LOGGER_CONFIG, like printf("logger config is %" PRI_LOGGER_CONFIG "\n", LOGGER_CONFIG_VALUES(logger_config));*/
#define PRI_LOGGER_CONFIG "s(LOGGER_CONFIG){.log_sinks=%p, .log_sink_count=%" PRIu32 "}"

/*a macro expanding to the 2 fields in the LOGGER_CONFIG structure*/
#define LOGGER_CONFIG_VALUES(logger_config) \
    "",                                     \
    (logger_config).log_sinks,              \
    (logger_config).log_sink_count          \

    int logger_init(void);
    void logger_deinit(void);

    LOGGER_CONFIG logger_get_config(void);
    void logger_set_config(LOGGER_CONFIG new_config);

    void logger_log(LOG_LEVEL log_level, LOG_CONTEXT_HANDLE log_context, const char* file, const char* func, int line_no, const char* format, ...);
    void logger_log_with_config(LOGGER_CONFIG logger_config, LOG_LEVEL log_level, LOG_CONTEXT_HANDLE log_context, const char* file, const char* func, int line_no, const char* format, ...);

#define LOGGER_LOG(log_level, log_context, format, ...) \
    ...

#define LOGGER_LOG_WITH_CONFIG(logger_config, log_level, log_context, format, ...) \
    ...

#define LOGGER_LOG_EX(log_level, ...) \
    ...
```

### logger_init

```c
int logger_init(void);
```

`logger_init` initializes all logger sinks.

Note: No other `logger` API should be called while `logger_init` executes.

**SRS_LOGGER_01_019: [** If `logger` is not already initialized: **]**

- **SRS_LOGGER_01_020: [** `logger_init` shall set the logger initialization counter to 1. **]**

- **SRS_LOGGER_01_003: [** `logger_init` shall call the `init` function of every sink that is configured to be used. **]**

**SRS_LOGGER_01_004: [** If `init` fails, all sinks already initialized shall have their `deinit` function called and `logger_init` shall fail and return a non-zero value. **]**

**SRS_LOGGER_01_005: [** Otherwise, `logger_init` shall succeed and return 0. **]**

**SRS_LOGGER_01_002: [** If `logger` is already initialized, `logger_init` shall increment the logger initialization counter, succeed and return 0. **]**

### logger_deinit

```c
void logger_deinit(void);
```

`logger_deinit` de-initializes all logger sinks.

Note: No other `logger` API should be called while `logger_deinit` executes.

**SRS_LOGGER_01_006: [** If `logger` is not initialized, `logger_deinit` shall return. **]**

**SRS_LOGGER_01_021: [** Otherwise, `logger_deinit` shall decrement the initialization counter for the module. **]**

**SRS_LOGGER_01_022: [** If the initilization counter reaches 0: **]**

- **SRS_LOGGER_01_007: [** `logger_deinit` shall call the `deinit` function of every sink that is configured to be used. **]**

### logger_get_config

```c
LOGGER_CONFIG logger_get_config(void);
```

`logger_get_config` returns the current logging sink configuration.

Note: `logger_get_config` is not thread safe and should not be called when othe rlogger APIs are executing.

**SRS_LOGGER_01_013: [** `logger_get_config` shall return a `LOGGER_CONFIG` structure with `log_sink_count` set to the current log sink count and `log_sinks` set to the array of log sink interfaces currently used. **]**

### logger_set_config

```c
void logger_set_config(LOGGER_CONFIG new_config);
```

`logger_set_config` sets the logging sink configuration to the parameters indicated in `new_config`.

Note: `logger_get_config` is not thread safe and should not be called when othe rlogger APIs are executing.

**SRS_LOGGER_01_014: [** `logger_set_config` set the current log sink count to `new_config.log_sink_count` and the array of log sink interfaces currently used to `new_config.log_sinks`. **]**

### LOGGER_LOG

```c
#define LOGGER_LOG(log_level, log_context, format, ...) \
  // ...
```

`LOGGER_LOG` allows the user to log one logging event.

**SRS_LOGGER_01_017: [** If `logger` is not initialized, `LOGGER_LOG` shall abort the program. **]**

**SRS_LOGGER_01_023: [** `LOGGER_LOG` shall generate code that verifies at compile time that `format` and `...` are suitable to be passed as arguments to `printf`. **]**

**SRS_LOGGER_01_001: [** `LOGGER_LOG` shall call the `log` function of every sink that is configured to be used. **]**

### LOGGER_LOG_WITH_CONFIG

```c
#define LOGGER_LOG_WITH_CONFIG(logger_config, log_level, log_context, format, ...) \
  // ...
```

`LOGGER_LOG_WITH_CONFIG` allows the user to log one logging event for a specific logger sink configuration.

**SRS_LOGGER_01_015: [** If `logger_config.log_sinks` is `NULL` and `logger_config.log_sink_count` is greater than 0, `LOGGER_LOG_WITH_CONFIG` shall return. **]**

**SRS_LOGGER_01_018: [** If `logger` is not initialized, `LOGGER_LOG` shall abort the program. **]**

**SRS_LOGGER_01_024: [** `LOGGER_LOG_WITH_CONFIG` shall generate code that verifies at compile time that `format` and `...` are suitable to be passed as arguments to `printf`. **]**

**SRS_LOGGER_01_016: [** Otherwise, `LOGGER_LOG_WITH_CONFIG` shall call the `log` function of every sink specified in `logger_config`. **]**

### LOGGER_LOG_EX

```c
#define LOGGER_LOG_EX(log_level, ...) \
  // ...
```

`LOGGER_LOG_EX` allows the user to log one logging event with a list of logging context properties.

It is syntactic sugar for creating a context and calling `LOGGER_LOG`.

**SRS_LOGGER_01_008: [** `LOGGER_LOG_EX` shall call the `log` function of every sink that is configured to be used. **]**

**SRS_LOGGER_01_009: [** If no properties are specified in `...`, `LOGGER_LOG_EX` shall call `log` with `log_context` being `NULL`. **]**

**SRS_LOGGER_01_010: [** Otherwise, `LOGGER_LOG_EX` shall construct a log context with all the properties specified in `...`. **]**

**SRS_LOGGER_01_011: [** Each `LOG_CONTEXT_STRING_PROPERTY` and `LOG_CONTEXT_PROPERTY` entry in `...` shall be added as a property in the context that is passed to `log`. **]**

```c
LOG_CONTEXT_MESSAGE(message_format, ...)
```

**SRS_LOGGER_01_012: [** If `LOG_CONTEXT_MESSAGE` is specified in `...`, `message_format` shall be passed to the `log` call together with a argument list made out of the `...` portion of the `LOG_CONTEXT_MESSAGE` macro. **]**
