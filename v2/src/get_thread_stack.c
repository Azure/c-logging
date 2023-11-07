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

#include "macro_utils/macro_utils.h"

#include "c_logging/get_thread_stack.h"


#define SYM_INIT_VALUES  \
    SYM_INIT_INITIALIZED,   \
    SYM_INIT_INITIALIZING,  \
    SYM_INIT_NOT_INITIALIZED

MU_DEFINE_ENUM(SYM_INIT, SYM_INIT_VALUES);
MU_DEFINE_ENUM_STRINGS(SYM_INIT, SYM_INIT_VALUES);

static char check_that_LONG_and_enum_have_the_same_size[sizeof(volatile LONG) == sizeof(SYM_INIT)];

typedef union SYM_INIT_STATE_TAG
{
    volatile LONG state;
    volatile SYM_INIT state_enum;
} SYM_INIT_STATE;

static SYM_INIT_STATE doSymInit = { .state_enum = SYM_INIT_NOT_INITIALIZED };

typedef union SYMBOL_INFO_EXTENDED_TAG
{
    SYMBOL_INFO symbol;
    unsigned char extendsUnion[sizeof(SYMBOL_INFO) + MAX_SYM_NAME - 1]; /*this field only exists to extend the size of the union to encompass "CHAR    Name[1];" of the SYMBOL_INFO to be as big as TRACE_MAX_SYMBOL_SIZE - 1 */ /*and the reason why it is not malloc'd is to exactly avoid a malloc that cannot be LogError'd (how does one log errors in a log function?!)*/
}SYMBOL_INFO_EXTENDED;

static SRWLOCK lockOverSymCalls = SRWLOCK_INIT;

static const char snprintfFailed[] = "\nsnprintf failed";

#define snprintf_fallback(destination,destination_size, fallback_string, fallbackstring_size, format, ... )                      \
{                                                                                                                                \
    snprintf_fallback_impl(destination,destination_size, fallback_string, fallbackstring_size, format, __VA_ARGS__);             \
    0 && printf(format, __VA_ARGS__); /*this is a no-op, but it will force the compiler to check the format string*/             \
}                                                                                                                                \

/*in all context where this is called, destination is non-NULL*/
/*function will try to snprintf into destination/destination size and if it fails, then tries to put there the fallback string*/
static void snprintf_fallback_impl(char** destination, size_t* destination_size, const char* fallback_string, size_t fallbackstring_size, const char* format, ...)
{
    va_list args;
    va_start(args, format);

    int snprintfResult;

    /*trying to complain*/
    snprintfResult = vsnprintf(*destination, *destination_size, format, args);
    if (!(
        (snprintfResult >= 0) && /*the returned value is nonnegative [...]*/
        (snprintfResult < (int)(*destination_size)) /*[...] and less than n.*/
        ))
    {
        /*complain about not being able to complain - only copy the part of the fallback string that fits*/

        size_t size_to_copy = *destination_size < fallbackstring_size ? *destination_size : fallbackstring_size - 1;
        (void)memcpy(*destination, fallback_string, size_to_copy);

        (*destination) += size_to_copy;
        (*destination_size) -= size_to_copy;
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
#define STACK_WALK_IMAGE_TYPE IMAGE_FILE_MACHINE_AMD64
#define CAPTURE_TOP_OF_STACK true
#else
#if defined(_WIN32)
#define INSTRUCTION_POINTER_REGISTER Eip
#define FRAME_POINTER_REGISTER Ebp
#define STACK_POINTER_REGISTER Esp
#define STACK_WALK_IMAGE_TYPE IMAGE_FILE_MACHINE_I386
#define CAPTURE_TOP_OF_STACK false
#else
#error unknown version of windows
#endif
#endif

void get_thread_stack(DWORD threadId, char* destination, size_t destinationSize)
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

        while ((state = InterlockedCompareExchange(&doSymInit.state, SYM_INIT_INITIALIZING, SYM_INIT_NOT_INITIALIZED)) != SYM_INIT_INITIALIZED)
        {
            if (state == SYM_INIT_NOT_INITIALIZED)
            {
                (void)SymInitialize(hProcess, NULL, TRUE); /*this is a process-wide initialization, and will leak because we don't know when to call SymCleanup. It is a good candidate for platform_init*/
                (void)InterlockedExchange(&doSymInit.state, SYM_INIT_INITIALIZED);
            }
        }

        /*3) get hThread's context.*/
        DWORD currentThreadId = GetCurrentThreadId();

bool wasThreadSuspended = false; /*only suspend threads that are not "current" thread to capture their stack frame*/
        bool wasContextAcquired = false;

        CONTEXT context = { 0 };
        context.ContextFlags = CONTEXT_CONTROL;

        HANDLE hThread = OpenThread(THREAD_GET_CONTEXT | THREAD_SUSPEND_RESUME, FALSE, threadId);
        if (hThread == NULL)
        {
            /*falling back to console*/
            (void)printf("failure (%" PRIu32 ") in OpenThread(THREAD_GET_CONTEXT, FALSE, threadId=%" PRIx32 ");", GetLastError(), threadId);
        }
        else
        {
            if (currentThreadId == threadId)
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
                        snprintf_fallback(&destination, &destinationSize, snprintfFailed, sizeof(snprintfFailed), "%sfailure (GetLastError=%" PRIu32 ") in GetThreadContext", firstLine ? (firstLine = false, "") : "\n", GetLastError());
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

                    bool thisIsFirstFrame = true; /*used to skip the first frame, which is "us", that is don't display information about "get_thread_stack"*/

                    /*4) once the context has been acquired, call StackWalk64 to get the stack frames. For every frame:*/
                    while (StackWalk64(
                        STACK_WALK_IMAGE_TYPE,
                        hProcess,
                        hThread,
                        &stackFrame,
                        &context,
                        NULL,
                        SymFunctionTableAccess64,
                        SymGetModuleBase64,
                        NULL))
                    {
                        if (thisIsFirstFrame && CAPTURE_TOP_OF_STACK) /*x64 does capture the current frame, x86 apparently does not... ?! really weird frankly. this CAPTURE_TOP_OF_STACK helps with not capturin the top of the stack*/
                        {
                            thisIsFirstFrame = false; /*no printing for the top of the stack, which is "us". us = get_thread_stack*/
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

            if (!CloseHandle(hThread))
            {
                /*falling back to console*/
                (void)printf("failure (%" PRIu32 ") in CloseHandle(currentThreadHandle=%p)", GetLastError(), hThread);
            }
        }

        destination[destinationSize - 1] = '\0';
    }
}
#else /*defined(_MSC_VER)*/
/*for all the others we don't provide a way to inspect another thread's call stack (yet).*/
void get_thread_stack(HANDLE thread, char* destination, size_t destinationSize)
{
    (void)thread;

    /*just return empty string to caller if possible*/

    if ((destination != NULL) && (destinationSize > 0))
    {
        destination[0] = '\0';
    }
}
#endif