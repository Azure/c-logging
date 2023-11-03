// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdbool.h>

#include "windows.h"
#include "winnt.h"
#include "dbghelp.h"

#include "c_logging/logging_stacktrace.h"

#define TRACE_MAX_STACK_FRAMES  1024
#define TRACE_MAX_SYMBOL_SIZE   1024
#define TRACE_MAX_STACK_LINE_AS_STRING_SIZE 1024

static volatile LONG doSymInit = 0; /*0 = not initialized, 1 = initializing, 2= initialized*/

typedef union SYMBOL_INFO_EXTENDED_TAG
{
    SYMBOL_INFO symbol;
    unsigned char extendsUnion[sizeof(SYMBOL_INFO) + TRACE_MAX_SYMBOL_SIZE - 1]; /*this field only exists to extend the size of the union to encompass "CHAR    Name[1];" of the SYMBOL_INFO to be as big as TRACE_MAX_SYMBOL_SIZE - 1 */ /*and the reason why it is not malloc'd is to exactly avoid a malloc that cannot be LogError'd (how does one log errors in a log function?!)*/
}SYMBOL_INFO_EXTENDED;

static SRWLOCK lockOverSymCalls = SRWLOCK_INIT;

/*returns number of characters copied into destination*/
static size_t memcat(char* destination, size_t destinationSize, const char* source, size_t sourceSize)
{
    size_t sizeToCpy = destinationSize<sourceSize?destinationSize:sourceSize;
    (void)memcpy(destination, source, sizeToCpy);
    return sizeToCpy;
}

static const char SymFromAddrFailed[] = "SymFromAddr failed\n";
static const char snprintfFailed[] = "snprintf failed\n";

#ifdef _MSC_VER
static XLOGGING_THREAD_LOCAL void* stack[TRACE_MAX_STACK_FRAMES];
#else
/*for C11 compilers*/
static _Thread_local void* stack[TRACE_MAX_STACK_FRAMES];
#endif

XLOGGING_THREAD_LOCAL char stackAsString[STACK_MAX_CHARACTERS];
XLOGGING_THREAD_LOCAL char formatWithStack[FORMAT_MAX_CHARACTERS];

/*tries to get as much as possible from the stack filling destination*/
void getStackAsString(char* destination, size_t destinationSize)
{
    /*lazily call once SymInitialize*/
    LONG state;
    while ((state = InterlockedCompareExchange(&doSymInit, 1, 0)) != 2)
    {
        if (state == 0)
        {
            (void)SymInitialize(GetCurrentProcess(), NULL, TRUE);
            (void)InterlockedExchange(&doSymInit, 2);
        }
    }

    size_t copied;

    /*all following function calls are protected by the same SRW*/
    AcquireSRWLockExclusive(&lockOverSymCalls);

    uint16_t numberOfFrames = CaptureStackBackTrace(1, TRACE_MAX_STACK_FRAMES, stack, NULL);
    if (numberOfFrames == 0)
    {
        (void)snprintf(destination, destinationSize, "!CaptureStackBackTrace returned 0 frames");
    }
    else
    {
        HANDLE process = GetCurrentProcess();

        SYMBOL_INFO_EXTENDED symbolExtended;
        SYMBOL_INFO* symbol = &symbolExtended.symbol;

        symbol->MaxNameLen = TRACE_MAX_SYMBOL_SIZE;
        symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

        for (uint16_t j = 0; j < numberOfFrames; j++)
        {
            DWORD64 address = (DWORD64)(stack[j]);
            DWORD displacement = 0;

            if (!SymFromAddr(process, address, NULL, symbol))
            {
                copied = memcat(destination, destinationSize, SymFromAddrFailed, sizeof(SymFromAddrFailed) - 1);
                destination += copied;
                destinationSize -= copied;
            }
            else
            {
                IMAGEHLP_LINE64 line;
                line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);

                char resultLine[TRACE_MAX_STACK_LINE_AS_STRING_SIZE];

                if (SymGetLineFromAddr64(process, address, &displacement, &line))
                {
                    int snprintfResult = snprintf(resultLine, sizeof(resultLine), "!%s %s:%" PRIu32 "%s", symbol->Name, line.FileName, (uint32_t)line.LineNumber, (j < numberOfFrames - 1) ? "\n" : "");
                    if (!(
                        (snprintfResult >= 0) && /*the returned value is nonnegative [...]*/
                        (snprintfResult < (int)sizeof(resultLine)) /*[...] and less than n.*/
                        ))
                    {
                        copied = memcat(destination, destinationSize, snprintfFailed, sizeof(snprintfFailed) - 1);
                        destination += copied;
                        destinationSize -= copied;
                    }
                    else
                    {
                        copied = memcat(destination, destinationSize, resultLine, snprintfResult);
                        destination += copied;
                        destinationSize -= copied;
                    }
                }
                else
                {
                    int snprintfResult = snprintf(resultLine, sizeof(resultLine), "!%s Address 0x%" PRIX64 "%s", symbol->Name, line.Address, (j < numberOfFrames - 1) ? "\n" : "");
                    if (!(
                        (snprintfResult >= 0) && /*the returned value is nonnegative [...]*/
                        (snprintfResult < (int)sizeof(resultLine)) /*[...] and less than n.*/
                        ))
                    {
                        copied = memcat(destination, destinationSize, snprintfFailed, sizeof(snprintfFailed) - 1);
                        destination += copied;
                        destinationSize -= copied;
                    }
                    else
                    {
                        copied = memcat(destination, destinationSize, resultLine, snprintfResult);
                        destination += copied;
                        destinationSize -= copied;
                    }
                }
            }
        }
        destination[-1] = '\0';
    }

    ReleaseSRWLockExclusive(&lockOverSymCalls);
}

#define snprintf_fallback(destination,destination_size, fallback_string, fallbackstring_size, format, ... )                      \
{                                                                                                                                \
    0 && printf(format, __VA_ARGS__); /*this is a no-op, but it will force the compiler to check the format string*/             \
    snprintf_fallback_impl(destination,destination_size, fallback_string, fallbackstring_size, format, __VA_ARGS__);             \
}                                                                                                                                \

/*in all context where this is called, destination is non-NULL*/
/*function will try to snprintf into destination/destination size and if it fails, then tries to put there the fallback string*/
static void snprintf_fallback_impl(char** destination, size_t* destination_size, const char* fallback_string, size_t fallbackstring_size, const char* format, ...)
{
    va_list args;
    va_start(args, format);

    size_t copied;

    int snprintfResult;

    /*trying to complain*/
    snprintfResult = vsnprintf(*destination, *destination_size, format, args);
    if (!(
        (snprintfResult >= 0) && /*the returned value is nonnegative [...]*/
        (snprintfResult < (*destination_size)) /*[...] and less than n.*/
        ))
    {
        /*complain about not being able to complain*/
        copied = memcat(*destination, *destination_size, fallback_string, fallbackstring_size - 1);
        (*destination) += copied;
        (*destination_size) -= copied;
    }
    else
    {
        /*complain*/
        (*destination) += snprintfResult;
        (*destination_size) -= snprintfResult;
    }
    va_end(args);
}

#if defined(_MSC_VER) && defined(_WIN64)
/*this function only exists for Windows machines*/
/*the function does the following
1) parameter validation
2) call SymInitialize for the current process
3) get hThread's context.
    3.1) if the current thread is hThread, call RtlCaptureContext
    3.2) if the current thread is not hThread, call SuspendThread and GetThreadContext
4) once the context has been acquired, call StackWalk64 to get the stack frames. For every frame:
    4.1) determine the function name
    4.2) determine the file name and line number
    4.3) append the function name, file name and line number to the destination
*/
void getThreadStackAsString(HANDLE hThread, char* destination, size_t destinationSize)
{

    /*1) parameter validation*/
    if ((destination == NULL) || (destinationSize == 0))
    {
        /*cannot compute if the output space is not sufficient (invalid args)*/
        return;
    }
    else
    {
        destination[0] = '\0';

        bool firstLine = true; /*only used to insert a \n between stack frames*/

        HANDLE hProcess = GetCurrentProcess();

        /*2) call SymInitialize for the current process*/
        /*lazily call once SymInitialize*/
        LONG state;
        while ((state = InterlockedCompareExchange(&doSymInit, 1, 0)) != 2)
        {
            if (state == 0)
            {
                (void)SymInitialize(hProcess, NULL, TRUE); /*this is a process-wide initialization, and will leak because we don't know when to call SymCleanup. It is a good candidate for platform_init*/
                (void)InterlockedExchange(&doSymInit, 2);
            }
        }

        /*3) get hThread's context.*/
        HANDLE currentThread = GetCurrentThread();

        bool wasThreadSuspended = false; /*only suspend threads that are not "current" thread to capture their stack frame*/

        bool wasContextAcquired = false;

        CONTEXT context = { 0 };
        context.ContextFlags = CONTEXT_FULL;

        if (hThread == currentThread)
        {
            /*3.1) if the current thread is hThread, call RtlCaptureContext*/
            RtlCaptureContext(&context); /*this is GetThreadContext for current thread*/
            wasContextAcquired = true;
        }
        else
        {
            /*3.2) if the current thread is not hThread, call SuspendThread and GetThreadContext*/
            if (SuspendThread(hThread) == (DWORD)-1)
            {
                snprintf_fallback(&destination, &destinationSize, snprintfFailed, sizeof(snprintfFailed), "%sfailure (GetLastError=%" PRIu32 ") in SuspendThread", firstLine ? (firstLine = false, "") : "\n", GetLastError());
            }
            else
            {
                if (!GetThreadContext(hThread, &context))
                {
                    snprintf_fallback(&destination, &destinationSize, snprintfFailed, sizeof(snprintfFailed), "%sfailure (GetLastError=%" PRIu32 ") in GetThreadContext\n", firstLine ? (firstLine = false, "") : "\n", GetLastError());
                }
                else
                {
                    wasContextAcquired = true;
                }
                wasThreadSuspended = true;
            }
        }

        if (!wasContextAcquired)
        {
            /*not much to do without the thread context*/
        }
        else
        {
            /*all following function calls are protected by the same SRW*/
            AcquireSRWLockExclusive(&lockOverSymCalls);
            {
                STACKFRAME64 stackFrame = { 0 };
                stackFrame.AddrPC.Offset = context.Rip;
                stackFrame.AddrPC.Mode = AddrModeFlat;
                stackFrame.AddrFrame.Offset = context.Rbp;
                stackFrame.AddrFrame.Mode = AddrModeFlat;
                stackFrame.AddrStack.Offset = context.Rsp;
                stackFrame.AddrStack.Mode = AddrModeFlat;

                bool thisIsFirstFrame = true; /*used to skip the first frame, which is "us", that is don't display information about "getThreadStackAsString"*/

                /*4) once the context has been acquired, call StackWalk64 to get the stack frames. For every frame:*/
                while (StackWalk64(
                    IMAGE_FILE_MACHINE_AMD64,
                    hProcess,
                    hThread,
                    &stackFrame,
                    &context,
                    NULL,
                    SymFunctionTableAccess64,
                    SymGetModuleBase64,
                    NULL))
                {

                    if (thisIsFirstFrame)
                    {
                        thisIsFirstFrame = false; /*no printing for the top of the stack, which is "us"*/
                        continue;
                    }

                    DWORD64 displacement = 0;

                    SYMBOL_INFO_EXTENDED buffer;

                    PSYMBOL_INFO pSymbol = &buffer.symbol;

                    pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
                    pSymbol->MaxNameLen = MAX_SYM_NAME;

                    const char* function_name;

                    /*4.1) determine the function name*/
                    if (!SymFromAddr(hProcess, stackFrame.AddrPC.Offset, &displacement, pSymbol))
                    {
                        function_name = "failure in SymFromAddr";
                    }
                    else
                    {
                        function_name = pSymbol->Name;
                    }

                    IMAGEHLP_LINE64 line;
                    line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
                    DWORD lineDisplacement;
                    const char* file_name;
                    uint32_t line_number;

                    /*4.2) determine the file name and line number*/
                    if (!SymGetLineFromAddr64(hProcess, stackFrame.AddrPC.Offset, &lineDisplacement, &line))
                    {
                        file_name = "failure in SymGetLineFromAddr64";
                        line_number = 0;
                    }
                    else
                    {
                        file_name = line.FileName;
                        line_number = line.LineNumber;
                    }

                    /*4.3) append the function name, file name and line number to the destination*/
                    snprintf_fallback(&destination, &destinationSize, snprintfFailed, sizeof(snprintfFailed), "%s!%s %s:%" PRIu32 "", firstLine ? (firstLine = false, "") : "\n", function_name, file_name, line_number);
                }

                ReleaseSRWLockExclusive(&lockOverSymCalls);
            }
        }


        if (wasThreadSuspended)
        {
            if (ResumeThread(hThread) == (DWORD)-1)
            {
                /*falling back to console*/
                (void)printf("failure (%" PRIu32 ") in ResumeThread", GetLastError());
                return;
            }
        }
        destination[destinationSize - 1] = '\0';
    }
}
#else /*defined(_MSC_VER) && defined(_WIN64)*/
/*for all the other platforms we don't provide a way to inspect another thread's call stack (yet).*/
void getThreadStackAsString(HANDLE thread, char* destination, size_t destinationSize)
{
    (void)thread;

    /*just return empty string to caller if possible*/

    if ((destination != NULL) && (destinationSize > 0))
    {
        destination[0] = '\0';
    }
}
#endif