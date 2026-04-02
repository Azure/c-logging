// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "macro_utils/macro_utils.h"

#include "c_logging/get_thread_stack.h"
#include "c_logging/logging_stacktrace.h"

#define POOR_MANS_ASSERT(cond) \
    if (!(cond)) \
    { \
        (void)printf("ASSERT failure in file %s line %d\r\n", __FILE__, __LINE__); \
        exit(__LINE__); \
    }

static void some_function_that_calls_getStackAsString(void)
{
    char stack[STACK_MAX_CHARACTERS];

    // Test the basic functionality
    getStackAsString(stack, sizeof(stack));

    // Verify we got some stack content (on Windows)
#ifdef WIN32
    POOR_MANS_ASSERT(strlen(stack) > 0);
    POOR_MANS_ASSERT(strstr(stack, "some_function_that_calls_getStackAsString") != NULL);
#else
    // On non-Windows platforms, we expect an empty string
    POOR_MANS_ASSERT(strlen(stack) == 0);
#endif

    printf("Stack trace:\n%s\n", stack);
}

static void test_null_parameters(void)
{
    char stack[100];

    // These should not crash
    getStackAsString(NULL, 100);
    getStackAsString(stack, 0);
    getStackAsString(NULL, 0);

    printf("Null parameter tests passed\n");
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

    printf("Thread-local variable tests passed\n");
}

int main(void)
{
    (void)printf("Starting logging_stacktrace integration tests\n");

    POOR_MANS_ASSERT(get_thread_stack_init() == 0);

    (void)printf("failure in get_thread_stack_init\n");

    test_null_parameters();
    test_thread_local_variables();
    some_function_that_calls_getStackAsString();

    get_thread_stack_deinit();

    (void)printf("All tests passed\n");

    return 0;
}
