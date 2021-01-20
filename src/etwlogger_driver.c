// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifdef USE_TRACELOGGING

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "windows.h"

#include "c_logging/xlogging.h"

#include "TraceLoggingProvider.h"
#include "evntrace.h"

#if CALL_CONSOLE_LOGGER
#include "c_logging/consolelogger.h"
#endif

TRACELOGGING_DEFINE_PROVIDER(
    g_hMyComponentProvider,
    "block_storage_2",
    (0xDAD29F36, 0x0A48, 0x4DEF, 0x9D, 0x50, 0x8E, 0xF9, 0x03, 0x6B, 0x92, 0xB4));
/*DAD29F36-0A48-4DEF-9D50-8EF9036B92B4*/

static volatile LONG isETWLoggerRegistered = 0;

static void lazyRegisterEventProvider(void)
{
    /*lazily init the logger*/
    LONG state;
    while ((state=InterlockedCompareExchange(&isETWLoggerRegistered, 1, 0)) != 2) /*0 - not init, 1 - initializing, 2 - initialized*/
    {
        if (state == 0)
        {
            /* register the provider*/
            TLG_STATUS t = TraceLoggingRegister(g_hMyComponentProvider);
            if (SUCCEEDED(t))
            {
                (void)InterlockedExchange(&isETWLoggerRegistered, 2);
                LogInfo("block_storage_2 ETW provider was registered succesfully (self test). Executable file full path name = %s", _pgmptr); /*_pgmptr comes from https://docs.microsoft.com/en-us/windows/win32/api/libloaderapi/nf-libloaderapi-getmodulefilenamea */
            }
            else
            {
                (void)printf("block_storage_2 ETW provider was NOT registered.");
                (void)InterlockedExchange(&isETWLoggerRegistered, 0);
            }
        }
    }
}

static void perform_EventWriteLogCriticalEvent(const char* content, const char* file, const char* func, int line)
{
    TraceLoggingWrite(g_hMyComponentProvider,
        "LogCritical",
        TraceLoggingLevel(TRACE_LEVEL_CRITICAL),
        TraceLoggingString(content, "content"),
        TraceLoggingString(file, "file"),
        TraceLoggingString(func, "func"),
        TraceLoggingInt32(line, "line")
    );

#if CALL_CONSOLE_LOGGER
    consolelogger_log(AZ_LOG_CRITICAL, file, func, line, LOG_LINE, "%s", content);
#endif
}

static void perform_EventWriteLogErrorEvent(const char* content, const char* file, const char* func, int line)
{
    TraceLoggingWrite(g_hMyComponentProvider,
        "LogError",
        TraceLoggingLevel(TRACE_LEVEL_ERROR),
        TraceLoggingString(content, "content"),
        TraceLoggingString(file, "file"),
        TraceLoggingString(func, "func"),
        TraceLoggingInt32(line, "line")
    );

#if CALL_CONSOLE_LOGGER
    consolelogger_log(AZ_LOG_ERROR, file, func, line, LOG_LINE, "%s", content);
#endif
}

static void perform_EventWriteLogWarningEvent(const char* content, const char* file, const char* func, int line)
{
    TraceLoggingWrite(g_hMyComponentProvider,
        "LogWarning",
        TraceLoggingLevel(TRACE_LEVEL_WARNING),
        TraceLoggingString(content, "content"),
        TraceLoggingString(file, "file"),
        TraceLoggingString(func, "func"),
        TraceLoggingInt32(line, "line")
    );

#if CALL_CONSOLE_LOGGER
    consolelogger_log(AZ_LOG_WARNING, file, func, line, LOG_LINE, "%s", content);
#endif
}

static void perform_EventWriteLogLastError(const char* userMessage, const char* file, const char* func, int line, const char* lastErrorAsString)
{
    TraceLoggingWrite(g_hMyComponentProvider,
        "LogLastError",
        TraceLoggingLevel(TRACE_LEVEL_ERROR),
        TraceLoggingString(userMessage, "content"),
        TraceLoggingString(file, "file"),
        TraceLoggingString(func, "func"),
        TraceLoggingInt32(line, "line"),
        TraceLoggingString(lastErrorAsString, "GetLastError")
    );

#if CALL_CONSOLE_LOGGER
    consolelogger_log(AZ_LOG_ERROR, file, func, line, LOG_LINE, "%s %s", userMessage, lastErrorAsString);
#endif
}

static void perform_EventWriteLogInfoEvent(const char* message, const char* file, const char* func, int line)
{
    TraceLoggingWrite(g_hMyComponentProvider,
        "LogInfo",
        TraceLoggingLevel(TRACE_LEVEL_INFORMATION),
        TraceLoggingString(message, "content"),
        TraceLoggingString(file, "file"),
        TraceLoggingString(func, "func"),
        TraceLoggingInt32(line, "line")
    );

#if CALL_CONSOLE_LOGGER
    consolelogger_log(AZ_LOG_INFO, file, func, line, LOG_LINE, "%s", message);
#endif
}

static void perform_EventWriteLogVerboseEvent(const char* message, const char* file, const char* func, int line)
{
    TraceLoggingWrite(g_hMyComponentProvider,
        "LogVerbose",
        TraceLoggingLevel(TRACE_LEVEL_VERBOSE),
        TraceLoggingString(message, "content"),
        TraceLoggingString(file, "file"),
        TraceLoggingString(func, "func"),
        TraceLoggingInt32(line, "line")
    );

#if CALL_CONSOLE_LOGGER
    consolelogger_log(AZ_LOG_VERBOSE, file, func, line, LOG_LINE, "%s", message);
#endif
}


static const char vsnprintf_failure_message[] = "failure in vsnprintf";
static const char FormatMessageA_failure_message[] = "failure in FormatMessageA";
void etwlogger_log_with_GetLastError(const char* file, const char* func, int line, const char* format, ...)
{
    DWORD lastError;

    lastError = GetLastError();
    lazyRegisterEventProvider();

    va_list args;
    va_start(args, format);

    char message[LOG_SIZE_REGULAR * (LOG_SIZE_REGULAR>=sizeof(vsnprintf_failure_message))]; /*this construct will generate a compile time error (array of size 0) when LOG_SIZE_REGULAR is not enough to hold even the failure message*/
    {/*scope for constructing the user (format,...)*/
        int vsnprintf_result;
        vsnprintf_result = vsnprintf(message, sizeof(message), format, args);
        if (
            (vsnprintf_result < 0)||
            (vsnprintf_result >= (int)sizeof(message))
            )
        {
            (void)memcpy(message, vsnprintf_failure_message, sizeof(vsnprintf_failure_message));
        }
        else
        {
            /*all fine, message now contains user message*/
        }
    }

    char lastErrorAsString[MESSAGE_BUFFER_SIZE *(MESSAGE_BUFFER_SIZE>sizeof(FormatMessageA_failure_message))];
    if (FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, lastError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), lastErrorAsString, sizeof(lastErrorAsString), NULL) == 0)
    {
        (void)memcpy(lastErrorAsString, FormatMessageA_failure_message, sizeof(FormatMessageA_failure_message));
    }
    else
    {
        /*remove extraneous newlines from FormatMessageA's output*/
        char* whereAreThey;
        if ((whereAreThey = strchr(lastErrorAsString, '\r')) != NULL)
        {
            *whereAreThey = '\0';
        }
        if ((whereAreThey = strchr(lastErrorAsString, '\n')) != NULL)
        {
            *whereAreThey = '\0';
        }
    }

    perform_EventWriteLogLastError(message, file, func, line, lastErrorAsString);

    va_end(args);
}

void etwlogger_log(LOG_CATEGORY log_category, const char* file, const char* func, int line, unsigned int options, const char* format, ...)
{
    (void)options;

    lazyRegisterEventProvider();

    va_list args;
    va_start(args, format);

    char message[LOG_SIZE_REGULAR * (LOG_SIZE_REGULAR >= sizeof(vsnprintf_failure_message))]; /*this construct will generate a compile time error (array of size 0) when LOG_SIZE_REGULAR is not enough to hold even the failure message*/
    {/*scope for constructing the user (format,...)*/
        int vsnprintf_result;
        vsnprintf_result = vsnprintf(message, sizeof(message), format, args);
        if (
            (vsnprintf_result < 0) ||
            (vsnprintf_result >= (int)sizeof(message))
            )
        {
            (void)memcpy(message, vsnprintf_failure_message, sizeof(vsnprintf_failure_message));
        }
        else
        {
            /*all fine, message now contains user message*/
        }
    }

    switch (log_category)
    {
        case AZ_LOG_CRITICAL:
        {
            perform_EventWriteLogCriticalEvent(message, file, func, line);
            break;
        }
        case AZ_LOG_ERROR:
        {
            perform_EventWriteLogErrorEvent(message, file, func, line);
            break;
        }
        case AZ_LOG_WARNING:
        {
            perform_EventWriteLogWarningEvent(message, file, func, line);
            break;
        }
        case AZ_LOG_INFO:
        {
            perform_EventWriteLogInfoEvent(message, file, func, line);
            break;
        }
        case AZ_LOG_VERBOSE:
        {
            perform_EventWriteLogVerboseEvent(message, file, func, line);
            break;
        }
        default:
            break;
    }
    va_end(args);
}

#endif /* USE_TRACELOGGING */
