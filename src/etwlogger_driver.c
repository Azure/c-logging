// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdarg.h>
#include <stdio.h>

#include <inttypes.h>

#include "windows.h"

#include "azure_c_logging/xlogging.h"

#include "azure_c_logging/consolelogger.h"

#ifdef USE_TRACELOGGING
#include "TraceLoggingProvider.h"
#include "evntrace.h"
#endif

/*a define to a size that is not expected to be exceeded by most messages*/
#define LOG_SIZE_REGULAR 2048 /*in bytes*/

/*returns a string as if printed by vprintf*/
static char* vprintf_alloc(const char* format, va_list va)
{
    char* result;
    
    /*allocate first, ask questions about the right size after*/
    result = (char*)malloc(LOG_SIZE_REGULAR);
    if (result == NULL)
    {
        /*return as is*/
    }
    else
    {
        int vsnprintf_result = vsnprintf(result, LOG_SIZE_REGULAR, format, va);
        if (vsnprintf_result < 0) /*C11 chapter 7.21.6.12: The vsnprintf function returns [...] a negative value if an encoding error occurred.*/
        {
            (void)printf("a negative value has been returned from vsnprintf, this means an encoding error occurred.");
        }
        else
        {
            if (vsnprintf_result < LOG_SIZE_REGULAR) /*C11: the null-terminated output has been completely written if and only if the returned value is nonnegative and less than n*/
            {
                /*return as is, we good*/
                goto allOk;
            }
            else
            {
                /*here we go again*/
                char* temp = realloc(result, vsnprintf_result + 1);
                if (temp == NULL)
                {
                    (void)printf("failed to realloc(result=%p, vsnprintf_result=%d + 1)", result, vsnprintf_result);
                }
                else
                {
                    int vsnprintf_result_2 = vsnprintf(result, (size_t)vsnprintf_result + 1, format, va);
                    if ((vsnprintf_result_2 < 0) || (vsnprintf_result_2 != vsnprintf_result))
                    {
                        (void)printf("an unexpected return vsnprintf_result_2=%d = vsnprintf(result=%p, vsnprintf_result=%d + 1, format, va)",
                            vsnprintf_result_2, result, vsnprintf_result);
                    }
                    else
                    {
                        goto allOk;
                    }
                }
            }
        }
        free(result);
        result = NULL;
allOk:;
    }
    
    return result;
}

/*returns a string as if printed by printf*/
static char* printf_alloc(const char* format, ...)
{
    char* result;
    va_list va;
    va_start(va, format);
    result = vprintf_alloc(format, va);
    va_end(va);
    return result;
}

/*returns NULL if it fails*/
static char* lastErrorToString(DWORD lastError)
{
    char* result;
    if (lastError == 0)
    {
        result = printf_alloc(""); /*no error should appear*/
        if (result == NULL)
        {
            (void)printf("failure in printf_alloc\r\n");
        }
        else
        {
            /*return as is*/
        }
    }
    else
    {
        char temp[MESSAGE_BUFFER_SIZE];
        if (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, lastError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), temp, MESSAGE_BUFFER_SIZE, NULL) == 0)
        {
            result = printf_alloc("GetLastError()=0X%x", lastError);
            if (result == NULL)
            {
                (void)printf("failure in printf_alloc\r\n");
                /*return as is*/
            }
            else
            {
                /*return as is*/
            }
        }
        else
        {
            /*eliminate the \r or \n from the string*/
            /*one replace of each is enough*/
            char* whereAreThey;
            if ((whereAreThey = strchr(temp, '\r')) != NULL)
            {
                *whereAreThey = '\0';
            }
            if ((whereAreThey = strchr(temp, '\n')) != NULL)
            {
                *whereAreThey = '\0';
            }

            result = printf_alloc("GetLastError()==0X%x (%s)", lastError, temp);

            if (result == NULL)
            {
                (void)printf("failure in printf_alloc\r\n");
                /*return as is*/
            }
            else
            {
                /*return as is*/
            }
        }
    }
    return result;
}

#ifdef USE_TRACELOGGING

TRACELOGGING_DEFINE_PROVIDER(
    g_hMyComponentProvider,
    "block_storage_2",
    (0xDAD29F36, 0x0A48, 0x4DEF, 0x9D, 0x50, 0x8E, 0xF9, 0x03, 0x6B, 0x92, 0xB4));
/*DAD29F36-0A48-4DEF-9D50-8EF9036B92B4*/

    
static volatile LONG isETWLoggerInit = 0;

static void lazyRegisterEventProvider(void)
{
    /*lazily init the logger*/
    if (InterlockedCompareExchange(&isETWLoggerInit, 1, 0) == 0)
    {
        // Register the provider
        TLG_STATUS t = TraceLoggingRegister(g_hMyComponentProvider);
        if (SUCCEEDED(t))
        {
            LogInfo("block_storage_2 ETW provider was registered succesfully (self test). Executable file full path name = %s", _pgmptr); /*_pgmptr comes from https://docs.microsoft.com/en-us/windows/win32/api/libloaderapi/nf-libloaderapi-getmodulefilenamea */
        }
        else
        {
            (void)printf("block_storage_2 ETW provider was NOT registered.");
            (void)InterlockedExchange(&isETWLoggerInit, 0);
        }
    }
    else
    {
        /*do nothing, already registered/attempted*/
    }
}

static void perform_EventWriteLogErrorEvent(const char* content, const char* file, const SYSTEMTIME* t, const char* func, int line)
{
    (void)t;
    TraceLoggingWrite(g_hMyComponentProvider,
        "LogError",
        TraceLoggingLevel(TRACE_LEVEL_ERROR),
        TraceLoggingString(file, "file"),
        TraceLoggingString(func, "func"),
        TraceLoggingInt32(line, "line"),
        TraceLoggingString(content, "content")
    );

#if CALL_CONSOLE_LOGGER
    consolelogger_log(AZ_LOG_ERROR, file, func, line, LOG_LINE, "%s", content);
#endif
}

static void perform_EventWriteLogLastError(const char* userMessage, const char* file, const SYSTEMTIME* t, const char* func, int line, const char* lastErrorAsString)
{
    (void)t;
    TraceLoggingWrite(g_hMyComponentProvider,
        "LogLastError",
        TraceLoggingLevel(TRACE_LEVEL_ERROR),
        TraceLoggingString(file, "file"),
        TraceLoggingString(func, "func"),
        TraceLoggingInt32(line, "line"),
        TraceLoggingString(userMessage, "content"),
        TraceLoggingString(lastErrorAsString, "GetLastError")
    );

#if CALL_CONSOLE_LOGGER
    consolelogger_log(AZ_LOG_ERROR, file, func, line, LOG_LINE, "%s %s", userMessage, lastErrorAsString);
#endif
}

static void perform_EventWriteLogInfoEvent(const char* message)
{
    TraceLoggingWrite(g_hMyComponentProvider,
        "LogInfo",
        TraceLoggingLevel(TRACE_LEVEL_INFORMATION),
        TraceLoggingString(message, "content")
    );

#if CALL_CONSOLE_LOGGER
    consolelogger_log(AZ_LOG_INFO, NULL, NULL, 0, LOG_LINE, "%s", message);
#endif

}

void etwlogger_log_with_GetLastError(const char* file, const char* func, int line, const char* format, ...)
{
    DWORD lastError;
    char* lastErrorAsString;

    lastError = GetLastError();
    lazyRegisterEventProvider();

    va_list args;
    va_start(args, format);

    SYSTEMTIME t;
    GetSystemTime(&t);

    lastErrorAsString = lastErrorToString(lastError);
    if (lastErrorAsString == NULL)
    {
        char* userMessage = vprintf_alloc(format, args);
        if (userMessage == NULL)
        {
            perform_EventWriteLogLastError("unable to print user error", file, &t, func, line, "last error was erroneously NULL");
        }
        else
        {
            perform_EventWriteLogLastError(userMessage, file, &t, func, line, "last error was erroneously NULL");
            free(userMessage);
        }
    }
    else
    {
        char* userMessage = vprintf_alloc(format, args);
        if (userMessage == NULL)
        {
            perform_EventWriteLogLastError("unable to print user error", file, &t, func, line, lastErrorAsString);
        }
        else
        {
            perform_EventWriteLogLastError(userMessage, file, &t, func, line, lastErrorAsString);
            free(userMessage);
        }
        free(lastErrorAsString);
    }

    va_end(args);
}

void etwlogger_log(LOG_CATEGORY log_category, const char* file, const char* func, int line, unsigned int options, const char* format, ...)
{
    (void)options;

    lazyRegisterEventProvider();

    va_list args;
    va_start(args, format);
    char* text = vprintf_alloc(format, args);
    if (text == NULL)
    {
        switch (log_category)
        {
            case AZ_LOG_INFO:
            {
                perform_EventWriteLogInfoEvent("INTERNAL LOGGING ERROR: failed in vprintf_alloc");
                break;
            }
            case AZ_LOG_ERROR:
            {
                SYSTEMTIME t;
                GetSystemTime(&t);
                perform_EventWriteLogErrorEvent("INTERNAL LOGGING ERROR: failed in vprintf_alloc", file, &t, func, line);
                break;
            }
            default:
                break;
        }
    }
    else
    {
        switch (log_category)
        {
            case AZ_LOG_INFO:
            {
                perform_EventWriteLogInfoEvent(text);
                break;
            }
            case AZ_LOG_ERROR:
            {
                SYSTEMTIME t;
                GetSystemTime(&t);
                perform_EventWriteLogErrorEvent(text, file, &t, func, line);
                break;
            }
            default:
                break;
        }
        free(text);
    }
    va_end(args);
}

#endif

