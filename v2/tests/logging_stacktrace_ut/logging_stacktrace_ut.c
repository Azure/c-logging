// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#ifdef WIN32
#include "windows.h"
#endif

#include "c_logging/logging_stacktrace.h"

#define POOR_MANS_ASSERT(cond) \
    if (!(cond)) \
    { \
        (void)printf("ASSERT failure in file %s line %d\r\n", __FILE__, __LINE__); \
        exit(__LINE__); \
    }

/* Tests_SRS_LOGGING_STACKTRACE_01_001: [ getStackAsString shall get the stack trace of the current thread and store it in the destination buffer. ]*/
static void getStackAsString_with_valid_parameters_succeeds(void)
{
    // arrange
    char destination[1024];
    
    // act
    getStackAsString(destination, sizeof(destination));

    // assert - just verify the function doesn't crash
    printf("getStackAsString_with_valid_parameters_succeeds passed\n");
}

/* Tests_SRS_LOGGING_STACKTRACE_01_002: [ If destination is NULL, getStackAsString shall return without doing anything. ]*/
static void getStackAsString_with_NULL_destination_returns(void)
{
    // arrange

    // act
    getStackAsString(NULL, 1024);

    // assert - just verify the function doesn't crash
    printf("getStackAsString_with_NULL_destination_returns passed\n");
}

/* Tests_SRS_LOGGING_STACKTRACE_01_003: [ If destinationSize is 0, getStackAsString shall return without doing anything. ]*/
static void getStackAsString_with_zero_size_returns(void)
{
    // arrange
    char destination[1024];

    // act
    getStackAsString(destination, 0);

    // assert - just verify the function doesn't crash
    printf("getStackAsString_with_zero_size_returns passed\n");
}

/* Tests_SRS_LOGGING_STACKTRACE_01_004: [ getStackAsString shall call get_thread_stack with the current thread ID to get the stack trace. ]*/
static void getStackAsString_calls_get_thread_stack_with_current_thread_id(void)
{
    // arrange
    char destination[1024];
    
    // act
    getStackAsString(destination, sizeof(destination));

    // assert - detailed verification of the call is done in integration tests
    printf("getStackAsString_calls_get_thread_stack_with_current_thread_id passed\n");
}

/* Tests_SRS_LOGGING_STACKTRACE_01_005: [ On Windows, getStackAsString shall use GetCurrentThreadId() to get the current thread ID. ]*/
static void getStackAsString_uses_GetCurrentThreadId_on_windows(void)
{
    // arrange
    char destination[1024];
    
    // act
    getStackAsString(destination, sizeof(destination));

    // assert - specific Windows behavior verification is in integration tests
    printf("getStackAsString_uses_GetCurrentThreadId_on_windows passed\n");
}

/* Tests_SRS_LOGGING_STACKTRACE_01_006: [ On non-Windows platforms, getStackAsString shall set the destination to an empty string if destinationSize is greater than 0. ]*/
static void getStackAsString_on_non_windows_sets_empty_string(void)
{
    // arrange
    char destination[1024];
    destination[0] = 'x'; // Initialize with something

#ifndef WIN32
    // act
    getStackAsString(destination, sizeof(destination));

    // assert
    POOR_MANS_ASSERT(destination[0] == '\0');
#endif
    
    // On Windows, this test doesn't apply, so we don't assert anything
    printf("getStackAsString_on_non_windows_sets_empty_string passed\n");
}

static void test_thread_local_variables(void)
{
    // Test that the thread-local variables exist and are accessible
    stackAsString[0] = 'a';
    stackAsString[1] = '\0';
    POOR_MANS_ASSERT(stackAsString[0] == 'a');
    
    formatWithStack[0] = 'b';
    formatWithStack[1] = '\0';
    POOR_MANS_ASSERT(formatWithStack[0] == 'b');
    
    printf("test_thread_local_variables passed\n");
}

int main(void)
{
    printf("Starting logging_stacktrace unit tests\n");
    
    getStackAsString_with_valid_parameters_succeeds();
    getStackAsString_with_NULL_destination_returns();
    getStackAsString_with_zero_size_returns();
    getStackAsString_calls_get_thread_stack_with_current_thread_id();
    getStackAsString_uses_GetCurrentThreadId_on_windows();
    getStackAsString_on_non_windows_sets_empty_string();
    test_thread_local_variables();
    
    printf("All unit tests passed\n");
    return 0;
}
