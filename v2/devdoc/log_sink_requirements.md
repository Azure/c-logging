# `log_sink` requirements

`log_sink` defines an interface for a log sink (as a structure that contains one function pointer).

A log sink has the responsibility to output a logging event (be that to the console, producting an ETW event, writing it to a file, etc.)

## Exposed API

```
typedef void (*LOG_SINK_LOG_FUNC)(LOG_LEVEL log_level, LOG_CONTEXT_HANDLE log_context, char* message, const char* file, const char* func, int line);

typedef struct LOG_SINK_TAG
{
    LOG_SINK_LOG_FUNC log_sink_log;
} LOG_SINK;
```
