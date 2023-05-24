# `log_sink_if` requirements

`log_sink_if` defines an interface for a log sink (as a structure that contains one function pointer).

A log sink has the responsibility to output a logging event (be that to the console, producing an ETW event, writing it to a file etc.)

## Exposed API

```
typedef void (*LOG_SINK_INIT_FUNC)(void);
typedef void (*LOG_SINK_LOG_FUNC)(LOG_LEVEL log_level, LOG_CONTEXT_HANDLE log_context, const char* file, const char* func, int line, const char* message_format, ...);
typedef void (*LOG_SINK_DEINIT_FUNC)(void);

typedef struct LOG_SINK_IF_TAG
{
    LOG_SINK_INIT_FUNC init;
    LOG_SINK_LOG_FUNC log;
    LOG_SINK_DEINIT_FUNC deinit;
} LOG_SINK_IF;
```

### init

`init` initializes a given sink. `init` is called once - upon logging system initialization - in order to make ure that `log` can perform its duties.

Note: No other APIs (`deinit`, `log`) should be called while `init` executes.

### deinit

`deinit` de-initializes a given sink.

Note: No other APIs (`init`, `log`) should be called while `deinit` executes.

### log

`log` logs one logging event (each sink can have different mechanisms to log the event).
