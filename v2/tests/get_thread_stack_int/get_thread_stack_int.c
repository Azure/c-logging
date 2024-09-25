// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "windows.h"

#ifdef USE_VLD
#include "vld.h"
#endif

#include "macro_utils/macro_utils.h"

#include "c_logging/get_thread_stack.h"

/* very "poor man's" way of testing, as no test harness and mocking framework are available */
#define POOR_MANS_ASSERT(cond) \
    if (!(cond)) \
    { \
        (void)printf("%s:%d test failed\r\n", __FUNCTION__, __LINE__); \
        abort(); \
    } \

static struct
{
    volatile LONG thread_running;
    volatile LONG  thread_should_exit;
}g;

/*the below function has been found to be optimized away with different x86/x64 flavors. #pragma optimize will prevent that from happening*/
#pragma optimize( "", off ) /*disable optimization*/
static void compute_stack(DWORD threadId, char* destination, size_t destination_size)
{
    get_thread_stack(threadId, destination, destination_size);
}
#pragma optimize( "", on ) /*restore optimization*/

/*the below function has been found to be optimized away with different x86/x64 flavors. #pragma optimize will prevent that from happening*/
#pragma optimize( "", off ) /*disable optimization*/
static void calls_end_frame(DWORD threadId, char* destination, size_t destination_size)
{
    /*works against optimization of this stack frame AND against having some predetermined characters in the destination.
    This code was experimentally determined and MIGHT fail with future compilers. To see the failure, put a breakpoint in get_thread_stack and examine the stack in Visual Studio debugger.
    If calls_end_frame does not appear... then modify this code to trick the compiler into actually emitting the code for this function.*/
    get_thread_stack(threadId, destination, destination_size);
}
#pragma optimize( "", on ) /*restore optimization*/


static DWORD WINAPI some_thread(
    LPVOID lpThreadParameter
)
{
    (void)lpThreadParameter;
    (void)InterlockedExchange(&g.thread_running, 1);

    /*about here the callstack of this thread is captured and it should contain "some_thread"*/

    while (InterlockedAdd(&g.thread_should_exit, 0) == 0)
    {
        Sleep(1);
    }

    return 0;
}

static void test_current_thread(void)
{
    char stack[4096];

    compute_stack(GetCurrentThreadId(), stack, sizeof(stack));
    POOR_MANS_ASSERT(strstr(stack, "!compute_stack") != NULL); /*assert that the stack contains "compute_stack"*/
    POOR_MANS_ASSERT(strstr(stack, "!get_thread_stack") == NULL); /*assert that the stack doesn't contain "get_thread_stack"*/

    calls_end_frame(GetCurrentThreadId(), stack, sizeof(stack));
    POOR_MANS_ASSERT(strstr(stack, "!calls_end_frame") != NULL); /*assert that the stack contains "calls_end_frame", even when the stack is not snapshotted in "calls_end_frame" */
    POOR_MANS_ASSERT(strstr(stack, "!get_thread_stack") == NULL); /*assert that the stack doesn't contain "get_thread_stack"*/
}

static void test_another_thread(void)
{
    char stack[4096];
    (void)InterlockedExchange(&g.thread_running, 0);
    (void)InterlockedExchange(&g.thread_should_exit, 0);

    /*this tries to capture the stack of another thread*/
    DWORD threadId = 0;
    HANDLE t = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)some_thread, NULL, 0, &threadId);
    POOR_MANS_ASSERT(t != NULL);

    while (InterlockedAdd(&g.thread_running, 0) != 1)
    {
        Sleep(1);
    }

    compute_stack(threadId, stack, sizeof(stack));

    POOR_MANS_ASSERT(strstr(stack, "!some_thread") != NULL); /*assert that the stack contains "calls_end_frame", even when the stack is not snapshotted in "calls_end_frame" */
    POOR_MANS_ASSERT(strstr(stack, "!get_thread_stack") == NULL); /*assert that the stack doesn't contain "get_thread_stack"*/

    (void)InterlockedExchange(&g.thread_should_exit, 1);

    POOR_MANS_ASSERT(WaitForSingleObject(t, INFINITE) == WAIT_OBJECT_0);
    POOR_MANS_ASSERT(CloseHandle(t));
}

/*this test will pass different sizes (starting with 0 and ending with 2048 to get*/
static void test_current_thread_insufficient_memory(void) /*doesn't really check anything except that it doesn't crash*/
{
    for (size_t i = 0; i < 2048; i++)
    {
        char* stack = malloc(i);
        POOR_MANS_ASSERT(stack != NULL);

        (void)memset(stack, '3', i); /*intentionally not passing a 0 initialized array*/

        compute_stack(GetCurrentThreadId(), stack, i);
        /*no assert, but must not crash*/

        free(stack);
    }
}

static void test_another_thread_insufficient_memory(void)
{
/*288 is some random observed stack size, such as in

!RtlDelayExecution failure in SymGetLineFromAddr64:0
!SleepEx failure in SymGetLineFromAddr64:0
!some_thread D:\r\c-logging\v2\tests\get_thread_stack_int\get_thread_stack_int.c:70
!BaseThreadInitThunk failure in SymGetLineFromAddr64:0
!RtlUserThreadStart failure in SymGetLineFromAddr64:0
*/

    for (size_t stack_string_size = 1; stack_string_size < 288 * 2; stack_string_size++)
    {
        char* stack = malloc(stack_string_size);
        POOR_MANS_ASSERT(stack != NULL);

        (void)memset(stack, '3', stack_string_size); /*intentionally not passing a 0 initialized array*/

        (void)InterlockedExchange(&g.thread_running, 0);
        (void)InterlockedExchange(&g.thread_should_exit, 0);

        /*this tries to capture the stack of another thread*/
        DWORD threadId=0;
        HANDLE t = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)some_thread, NULL, 0, &threadId); /*creating 288*2 threads can take a while...*/
        POOR_MANS_ASSERT(t != NULL);

        while (InterlockedAdd(&g.thread_running, 0) != 1)
        {
            Sleep(1);
        }

        compute_stack(threadId, stack, stack_string_size); /*must not crash*/

        (void)printf("stack i=%zu\n%s\n", stack_string_size, stack);

        (void)InterlockedExchange(&g.thread_should_exit, 1);

        POOR_MANS_ASSERT(WaitForSingleObject(t, INFINITE) == WAIT_OBJECT_0);
        POOR_MANS_ASSERT(CloseHandle(t));

        free(stack);
    }
}

int main(void)
{
    // make abort not popup
    _set_abort_behavior(_CALL_REPORTFAULT, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);

    test_current_thread();

    test_another_thread();

    test_current_thread_insufficient_memory();

    test_another_thread_insufficient_memory();

    return 0;
}
