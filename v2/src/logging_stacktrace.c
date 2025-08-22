// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <stddef.h>

#ifdef WIN32
#include "windows.h"
#include "c_logging/get_thread_stack.h"
#endif

#include "c_logging/logging_stacktrace.h"

// Thread-local storage for stack strings (compatibility with v1)
XLOGGING_THREAD_LOCAL char stackAsString[STACK_MAX_CHARACTERS];
XLOGGING_THREAD_LOCAL char formatWithStack[FORMAT_MAX_CHARACTERS];

/*tries to get as much as possible from the stack filling destination*/
void getStackAsString(char* destination, size_t destinationSize)
{
    /*Codes_SRS_LOGGING_STACKTRACE_01_002: [ If destination is NULL, getStackAsString shall return without doing anything. ]*/
    /*Codes_SRS_LOGGING_STACKTRACE_01_003: [ If destinationSize is 0, getStackAsString shall return without doing anything. ]*/
    if (destination == NULL || destinationSize == 0)
    {
        return;
    }

#ifdef WIN32
    /*Codes_SRS_LOGGING_STACKTRACE_01_001: [ getStackAsString shall get the stack trace of the current thread and store it in the destination buffer. ]*/
    /*Codes_SRS_LOGGING_STACKTRACE_01_004: [ getStackAsString shall call get_thread_stack with the current thread ID to get the stack trace. ]*/
    /*Codes_SRS_LOGGING_STACKTRACE_01_005: [ On Windows, getStackAsString shall use GetCurrentThreadId() to get the current thread ID. ]*/
    // Use the v2 get_thread_stack functionality to get current thread's stack
    DWORD currentThreadId = GetCurrentThreadId();
    get_thread_stack(currentThreadId, destination, destinationSize);
#else
    /*Codes_SRS_LOGGING_STACKTRACE_01_006: [ On non-Windows platforms, getStackAsString shall set the destination to an empty string if destinationSize is greater than 0. ]*/
    // For non-Windows platforms, just return empty string
    if (destinationSize > 0)
    {
        destination[0] = '\0';
    }
#endif
}
