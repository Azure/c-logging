// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <stddef.h>

#include "windows.h"
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
