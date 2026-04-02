// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdarg.h>

#if defined(_MSC_VER)
#include "windows.h"
#include "winnt.h"
#include "dbghelp.h"
#endif

#include "macro_utils/macro_utils.h"

#include "c_logging/get_thread_stack.h"


/*note: the APIs in here cannot use LogError+friends because it would result in a stack overflow. Here's why.
Assume get_thread_stack fails the call to OpenThread. The next line would be LogLastError. If logLastError captures the stack it would end up back in get_thread_stack...
... which would fail the call to OpenThread... which would call again LogLastError... etc => STACK OVERFLOW*/

static const char snprintfFailed[] = "\nsnprintf failed";

#define snprintf_fallback(destination, destination_size, fallback_string, fallbackstring_size, format, ... )                                \
{                                                                                                                                           \
    snprintf_fallback_impl(destination, destination_size, fallback_string, fallbackstring_size, format __VA_OPT__(,) __VA_ARGS__);          \
    (void)(0 && printf(format __VA_OPT__(,)  __VA_ARGS__)); /*this is a no-op, but it will force the compiler to check the format string*/  \
}                                                                                                                                           \

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

#if defined(_M_ARM64)
#define INSTRUCTION_POINTER_REGISTER Pc
#define FRAME_POINTER_REGISTER Fp
#define STACK_POINTER_REGISTER Sp
#define STACK_WALK_IMAGE_TYPE IMAGE_FILE_MACHINE_ARM64
#define CAPTURE_TOP_OF_STACK true
#elif defined(_WIN64)
#define INSTRUCTION_POINTER_REGISTER Rip
#define FRAME_POINTER_REGISTER Rbp
#define STACK_POINTER_REGISTER Rsp
#define STACK_WALK_IMAGE_TYPE IMAGE_FILE_MACHINE_AMD64
#define CAPTURE_TOP_OF_STACK true
#elif defined(_WIN32)
#define INSTRUCTION_POINTER_REGISTER Eip
#define FRAME_POINTER_REGISTER Ebp
#define STACK_POINTER_REGISTER Esp
#define STACK_WALK_IMAGE_TYPE IMAGE_FILE_MACHINE_I386
#define CAPTURE_TOP_OF_STACK false
#else
#error unknown version of windows
#endif

#define SYM_INIT_VALUES  \
    SYM_INIT_INITIALIZED,   \
    SYM_INIT_NOT_INITIALIZED

MU_DEFINE_ENUM(SYM_INIT, SYM_INIT_VALUES);
MU_DEFINE_ENUM_STRINGS(SYM_INIT, SYM_INIT_VALUES);

typedef union SYMBOL_INFO_EXTENDED_TAG
{
    SYMBOL_INFO symbol;
    unsigned char extendsUnion[sizeof(SYMBOL_INFO) + MAX_SYM_NAME - 1]; /*this field only exists to extend the size of the union to encompass "CHAR    Name[1];" of the SYMBOL_INFO to be as big as TRACE_MAX_SYMBOL_SIZE - 1 */ /*and the reason why it is not malloc'd is to exactly avoid a malloc that cannot be LogError'd (how does one log errors in a log function?!)*/
}SYMBOL_INFO_EXTENDED;

static struct {
    SYM_INIT symbolsState; /*holds the state of the module.*/
    HANDLE processHandle; /*self-process handle*/
    SRWLOCK lockOverSymCalls;
#if defined(_M_ARM64)
    /*ARM64 Pointer Authentication Code (PAC) address mask - see detailed explanation below in init.*/
    DWORD64 addressMask;
#endif
} g = /*g comes from "global" because this is a singleton*/
{
    .symbolsState = SYM_INIT_NOT_INITIALIZED,
    .processHandle = NULL,
    .lockOverSymCalls = SRWLOCK_INIT,
#if defined(_M_ARM64)
    .addressMask = ~(DWORD64)0 /*default: no masking until init computes the real mask*/
#endif
};

/*get_thread_stack_init is not thread safe. There should not be 2 threads that call this function - ever. Canon place to have it is: platform_init() -> logger_init() -> get_thread_stack_init() */
int get_thread_stack_init(void)
{
    int result;
    if (g.symbolsState == SYM_INIT_NOT_INITIALIZED)
    {
        /*duplicating the pseudo-handle to get a unique real handle for our own dbghelp symbol context*/
        /*mimics the implementation from https://learn.microsoft.com/en-us/windows/win32/debug/initializing-the-symbol-handler */
        HANDLE hCurrentProcess = GetCurrentProcess(); /*The pseudo handle need not be closed when it is no longer needed. Source: https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-getcurrentprocess*/
        if (!DuplicateHandle(hCurrentProcess, hCurrentProcess, hCurrentProcess, &g.processHandle, 0, FALSE, DUPLICATE_SAME_ACCESS))
        {
            (void)printf("failure (GetLastError()=0x%" PRIx32 ") in DuplicateHandle(hCurrentProcess=%p, hCurrentProcess=%p, hCurrentProcess=%p, &g.processHandle=%p, 0, FALSE, DUPLICATE_SAME_ACCESS), will use hCurrentProcess=%p",
                GetLastError(), hCurrentProcess, hCurrentProcess, hCurrentProcess, &g.processHandle, hCurrentProcess);
            result = MU_FAILURE;
        }
        else
        {
            // Per MSDN, the directory containing the executable is not automatically part of the symbol search path.
            // Use GetModuleFileName to obtain the exe path and extract its directory so that PDBs co-located with
            // the executable are found by SymInitialize.
            char exePath[MAX_PATH];
            char* userSearchPath = NULL;
            DWORD pathLen = GetModuleFileName(NULL, exePath, MAX_PATH);
            if (pathLen != 0 && pathLen < MAX_PATH)
            {
                char* lastBackslash = strrchr(exePath, '\\');
                if (lastBackslash != NULL)
                {
                    *lastBackslash = '\0';
                    userSearchPath = exePath;
                }
            }

            if (!SymInitialize(g.processHandle, userSearchPath, TRUE))
            {
                (void)printf("failure (GetLastError()=0x%" PRIx32 ") in SymInitialize(g.processHandle=%p, userSearchPath=%s, TRUE)",
                    GetLastError(), g.processHandle, MU_P_OR_NULL(userSearchPath));
                result = MU_FAILURE;
            }
            else
            {
                g.symbolsState = SYM_INIT_INITIALIZED;
#if defined(_M_ARM64)
                /*ARM64 Pointer Authentication Code (PAC) - why we need an address mask:

                ARM64 processors (ARMv8.3+) have a security feature called Pointer Authentication.
                When a function is called, the CPU signs the return address (LR register) using a
                cryptographic key before saving it on the stack. This places authentication bits in
                the upper unused bits of the 64-bit address. For example, a valid return address like
                0x00007FF64DB43D24 becomes 0xD327FFF64DB43D24 when PAC-signed on the stack.

                When StackWalk64 reads return addresses from the stack, it gets these PAC-signed values.
                Unfortunately StackWalk64 does not strip the PAC bits, so it tries to look up unwind info
                for address 0xD327FFF64DB43D24 which is not a valid address - causing it to fail to find
                the function table entry and stop walking prematurely (typically after only 2-3 frames
                for cross-thread captures).

                Microsoft documents this problem in the DIA SDK via
                IDiaStackWalkHelper2::GetPointerAuthenticationMask (see
                https://learn.microsoft.com/en-us/visualstudio/debugger/debug-interface-access/idiastackwalkhelper2-getpointerauthenticationmask)
                which provides a mask for stripping PAC bits. However that API is part of the DIA SDK
                (Visual Studio debugger infrastructure), not dbghelp.

                Our fix: derive the mask from GetSystemInfo().lpMaximumApplicationAddress. This gives
                the highest valid user-mode address (e.g. 0x00007FFFFFFEFFFF for 47-bit VA). We round
                it up to an all-ones mask (0x00007FFFFFFFFFFF) covering all valid address bits. Any bits
                above this mask are PAC authentication bits that must be stripped.

                For more background see Raymond Chen's blog post on ARM64 return address protection:
                https://devblogs.microsoft.com/oldnewthing/20220819-00/?p=107020

                Note: processors without PAC (e.g. Ampere Altra) do not sign return addresses, so the
                mask has no effect - all upper bits are already zero. This only matters on PAC-enabled
                processors like Microsoft Cobalt (Neoverse N2).*/
                {
                    SYSTEM_INFO si;
                    GetSystemInfo(&si);
                    DWORD64 maxAddr = (DWORD64)(ULONG_PTR)si.lpMaximumApplicationAddress;
                    /*round up to a mask covering all valid address bits (fill all bits below the MSB)*/
                    g.addressMask = maxAddr;
                    g.addressMask |= g.addressMask >> 1;
                    g.addressMask |= g.addressMask >> 2;
                    g.addressMask |= g.addressMask >> 4;
                    g.addressMask |= g.addressMask >> 8;
                    g.addressMask |= g.addressMask >> 16;
                    g.addressMask |= g.addressMask >> 32;
                }
#endif
                result = 0;
                goto allok;
            }

            if (!CloseHandle(g.processHandle))
            {
                (void)printf("failure (GetLastError()=0x%" PRIx32 ") in CloseHandle(g.processHandle=%p)",
                    GetLastError(), g.processHandle);
            }
            g.processHandle = NULL;
        }
    }
    else
    {
        /*already initialized, nothing to do*/
        result = 0;
    }
allok:
    return result;
}


void get_thread_stack(DWORD threadId, char* destination, size_t destinationSize)
{
    /*1) parameter validation*/
    if ((destination == NULL) || (destinationSize == 0))
    {
        /*cannot compute if the output space is not sufficient (invalid args)*/
    }
    else if (g.symbolsState != SYM_INIT_INITIALIZED)
    {
        /* cannot compute if we are not initialized*/
        snprintf_fallback(&destination, &destinationSize, snprintfFailed, sizeof(snprintfFailed), "failure: get_thread_stack is not initialized, g.doSymInit=%" PRI_MU_ENUM "", MU_ENUM_VALUE(SYM_INIT, g.symbolsState));
    }
    else
    {
        destination[0] = '\0';

        bool firstLine = true; /*only used to insert a \n between stack frames*/

        /*2) has been replaced by init/deinit */

        /*3) get hThread's context.*/
        DWORD currentThreadId = GetCurrentThreadId();

        bool wasThreadSuspended = false; /*only suspend threads that are not "current" thread to capture their stack frame*/
        bool wasContextAcquired = false;

        CONTEXT context = { 0 };
        context.ContextFlags = CONTEXT_FULL; /*ARM64 unwinding needs integer registers (x19-x28) to restore non-volatile register values during stack walk*/

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
#if defined(_M_ARM64)
                        /*TEMP DIAG: print ARM64 context registers to verify GetThreadContext filled them correctly*/
                        (void)printf("[DIAG] GetThreadContext ARM64: Pc=0x%016" PRIX64 " Sp=0x%016" PRIX64 " Fp=0x%016" PRIX64 " Lr=0x%016" PRIX64 "\n",
                            (uint64_t)context.Pc, (uint64_t)context.Sp, (uint64_t)context.Fp, (uint64_t)context.Lr);
                        (void)fflush(stdout);
#endif
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
                AcquireSRWLockExclusive(&g.lockOverSymCalls);
                {
                    STACKFRAME64 stackFrame = { 0 };
                    stackFrame.AddrPC.Offset = context.INSTRUCTION_POINTER_REGISTER;
                    stackFrame.AddrPC.Mode = AddrModeFlat;
                    stackFrame.AddrFrame.Offset = context.FRAME_POINTER_REGISTER;
                    stackFrame.AddrFrame.Mode = AddrModeFlat;
                    stackFrame.AddrStack.Offset = context.STACK_POINTER_REGISTER;
                    stackFrame.AddrStack.Mode = AddrModeFlat;

                    /*skip the first frame only when capturing the current thread's stack,
                    because in that case the top frame is get_thread_stack itself.
                    For another thread the top frame is whatever that thread was executing,
                    so skipping it would lose real call-stack information.*/
                    bool skipFirstFrame = (currentThreadId == threadId) && CAPTURE_TOP_OF_STACK;

                    /*4) once the context has been acquired, call StackWalk64 to get the stack frames. For every frame:*/
                    int frameIndex = 0;
                    while (StackWalk64(
                        STACK_WALK_IMAGE_TYPE,
                        g.processHandle,
                        hThread,
                        &stackFrame,
                        &context,
                        NULL,
                        SymFunctionTableAccess64,
                        SymGetModuleBase64,
                        NULL))
                    {
#if defined(_M_ARM64)
                        /*strip PAC authentication bits from addresses (see detailed explanation in init).
                        Without this, StackWalk64 returns PAC-signed return addresses that cannot be
                        resolved by SymFromAddr and prevent the walk from continuing to the next frame.
                        Stripping AddrPC fixes symbol lookup for the current frame.
                        Stripping AddrReturn fixes the next iteration of StackWalk64 which uses it as
                        the next frame's PC to find unwind info.*/
                        stackFrame.AddrPC.Offset &= g.addressMask;
                        stackFrame.AddrReturn.Offset &= g.addressMask;
#endif
                        /*TEMP DIAG: print raw hex address for each frame*/
                        (void)printf("[DIAG] frame[%d]: AddrPC=0x%016" PRIX64 " AddrFrame=0x%016" PRIX64 " AddrReturn=0x%016" PRIX64 "\n",
                            frameIndex, (uint64_t)stackFrame.AddrPC.Offset, (uint64_t)stackFrame.AddrFrame.Offset, (uint64_t)stackFrame.AddrReturn.Offset);
                        (void)fflush(stdout);
                        frameIndex++;
                        if (skipFirstFrame)
                        {
                            skipFirstFrame = false; /*no printing for the top of the stack, which is "us". us = get_thread_stack*/
                            continue;
                        }

                        DWORD64 displacement = 0;

                        SYMBOL_INFO_EXTENDED buffer;

                        PSYMBOL_INFO pSymbol = &buffer.symbol;

                        pSymbol->SizeOfStruct = sizeof(SYMBOL_INFO);
                        pSymbol->MaxNameLen = MAX_SYM_NAME;

                        const char* function_name;

                        /*4.1) determine the function name*/
                        if (!SymFromAddr(g.processHandle, stackFrame.AddrPC.Offset, &displacement, pSymbol))
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
                        if (!SymGetLineFromAddr64(g.processHandle, stackFrame.AddrPC.Offset, &lineDisplacement, &line))
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

                    /*TEMP DIAG: print why StackWalk64 stopped*/
                    (void)printf("[DIAG] StackWalk64 stopped after %d frames, GetLastError=%" PRIu32 "\n", frameIndex, GetLastError());
                    (void)fflush(stdout);

                    ReleaseSRWLockExclusive(&g.lockOverSymCalls);
                }
            }

            if (wasThreadSuspended)
            {
                if (ResumeThread(hThread) == (DWORD)-1)
                {
                    /*falling back to console*/
                    (void)printf("failure (%" PRIu32 ") in ResumeThread(hThread=%p)", GetLastError(), hThread);
                    return;
                }
            }

            if (!CloseHandle(hThread))
            {
                /*falling back to console*/
                (void)printf("failure (%" PRIu32 ") in CloseHandle(hThread=%p)", GetLastError(), hThread);
            }
        }

        destination[destinationSize - 1] = '\0';
    }
}

/*void get_thread_stack_deinit(void)
 is not thread safe. There should not be 2 threads that call this function - ever. Canon place to have it is: platform_deinit() -> logger_deinit() -> get_thread_stack_deinit() */
void get_thread_stack_deinit(void)
{
    if (g.symbolsState == SYM_INIT_INITIALIZED)
    {
        if (!SymCleanup(g.processHandle))
        {
            (void)printf("failure (GetLastError()=0x%" PRIx32 ") in SymCleanup(g.processHandle=%p)",
                GetLastError(), g.processHandle);
        }

        if (!CloseHandle(g.processHandle))
        {
            (void)printf("failure (GetLastError()=0x%" PRIx32 ") in CloseHandle(g.processHandle=%p)",
                GetLastError(), g.processHandle);
        }
        g.processHandle = NULL;

        g.symbolsState = SYM_INIT_NOT_INITIALIZED;
    }
    else
    {
        /*do nothing*/
    }
}

#elif defined(__linux__)
/*for all the others we don't provide a way to inspect another thread's call stack (yet).*/

int get_thread_stack_init(void)
{
    return 0;
}

void get_thread_stack(pthread_t thread, char* destination, size_t destinationSize)
{
    (void)thread;

    /*just inform this is not supported*/

    if ((destination != NULL) && (destinationSize > 0))
    {
        snprintf_fallback(&destination, &destinationSize, snprintfFailed, sizeof(snprintfFailed), "currently running platform not supported.");
    }
}

void get_thread_stack_deinit(void)
{
    /*do nothing*/
}
#else
#error "get_thread_stack is not implemented for this platform yet."
#endif
