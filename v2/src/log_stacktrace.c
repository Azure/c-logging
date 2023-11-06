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

#include "c_logging/log_stacktrace.h"

static volatile LONG doSymInit = 0; /*0 = not initialized, 1 = initializing, 2= initialized*/

typedef union SYMBOL_INFO_EXTENDED_TAG
{
    SYMBOL_INFO symbol;
    unsigned char extendsUnion[sizeof(SYMBOL_INFO) + MAX_SYM_NAME - 1]; /*this field only exists to extend the size of the union to encompass "CHAR    Name[1];" of the SYMBOL_INFO to be as big as TRACE_MAX_SYMBOL_SIZE - 1 */ /*and the reason why it is not malloc'd is to exactly avoid a malloc that cannot be LogError'd (how does one log errors in a log function?!)*/
}SYMBOL_INFO_EXTENDED;

static SRWLOCK lockOverSymCalls = SRWLOCK_INIT;

/*returns number of characters copied into destination*/
static size_t memcat(char* destination, size_t destinationSize, const char* source, size_t sourceSize)
{
    size_t sizeToCpy = destinationSize<sourceSize?destinationSize:sourceSize;
    (void)memcpy(destination, source, sizeToCpy);
    return sizeToCpy;
}

static const char snprintfFailed[] = "snprintf failed\n";

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
        (snprintfResult < (int)(*destination_size)) /*[...] and less than n.*/
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

#if defined(_MSC_VER)
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

#if defined(_WIN64)
    #define INSTRUCTION_POINTER_REGISTER Rip
    #define FRAME_POINTER_REGISTER Rbp
    #define STACK_POINTER_REGISTER Rsp
#else
    #if defined(_WIN32)
        #define INSTRUCTION_POINTER_REGISTER Eip
        #define FRAME_POINTER_REGISTER Ebp
        #define STACK_POINTER_REGISTER Esp
    #else
        #error unknown version of windows
    #endif
#endif

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
                stackFrame.AddrPC.Offset = context.INSTRUCTION_POINTER_REGISTER;
                stackFrame.AddrPC.Mode = AddrModeFlat;
                stackFrame.AddrFrame.Offset = context.FRAME_POINTER_REGISTER;
                stackFrame.AddrFrame.Mode = AddrModeFlat;
                stackFrame.AddrStack.Offset = context.STACK_POINTER_REGISTER;
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
                        thisIsFirstFrame = false; /*no printing for the top of the stack, which is "us". us = getThreadStackAsString*/
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