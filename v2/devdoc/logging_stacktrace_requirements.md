# logging_stacktrace requirements

## Overview

The logging_stacktrace module provides compatibility with v1 logging stack trace functionality, allowing applications to capture and format stack traces of the current thread.

## Exposed API

```c
#define STACK_PRINT_FORMAT "\nStack:\n%s"
#define STACK_MAX_CHARACTERS 2048
#define FORMAT_MAX_CHARACTERS 1024

XLOGGING_THREAD_LOCAL char stackAsString[STACK_MAX_CHARACTERS];
XLOGGING_THREAD_LOCAL char formatWithStack[FORMAT_MAX_CHARACTERS];

void getStackAsString(char* destination, size_t destinationSize);
```

## getStackAsString

```c
void getStackAsString(char* destination, size_t destinationSize);
```

`getStackAsString` captures the call stack of the current thread and formats it as a human-readable string. The function walks the stack frames starting from the current execution point and retrieves function names, file names, and line numbers for each frame. The resulting stack trace is stored in the provided destination buffer.

**SRS_LOGGING_STACKTRACE_01_001: [** `getStackAsString` shall get the stack trace of the current thread and store it in the destination buffer. **]**

**SRS_LOGGING_STACKTRACE_01_002: [** If `destination` is `NULL`, `getStackAsString` shall return without doing anything. **]**

**SRS_LOGGING_STACKTRACE_01_003: [** If `destinationSize` is 0, `getStackAsString` shall return without doing anything. **]**

**SRS_LOGGING_STACKTRACE_01_004: [** `getStackAsString` shall call `get_thread_stack` with the current thread ID to get the stack trace. **]**

**SRS_LOGGING_STACKTRACE_01_005: [** On Windows, `getStackAsString` shall use `GetCurrentThreadId()` to get the current thread ID. **]**

**SRS_LOGGING_STACKTRACE_01_006: [** On non-Windows platforms, `getStackAsString` shall set the destination to an empty string if `destinationSize` is greater than 0. **]**
