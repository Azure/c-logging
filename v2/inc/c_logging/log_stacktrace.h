// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef LOG_STACKTRACE_H
#define LOG_STACKTRACE_H

#ifdef __cplusplus
#include <cstddef>
#else
#include <stddef.h>
#endif

#if defined(_MSC_VER) && defined(_WIN64)
#include "windows.h" /*needed for HANDLE*/
#endif

#ifdef _MSC_VER
#define XLOGGING_THREAD_LOCAL __declspec(thread)
#else
#define XLOGGING_THREAD_LOCAL _Thread_local
#endif


#define STACK_PRINT_FORMAT "\nStack:\n%s"
#define STACK_MAX_CHARACTERS 2048 /*whatever we get from stack cannot exceed this size*/
#define FORMAT_MAX_CHARACTERS 1024 /*user format + STACK_PRINT_FORMAT in a string cannot exceed this size*/

#ifdef __cplusplus
extern "C" {
#endif

extern  XLOGGING_THREAD_LOCAL char stackAsString[STACK_MAX_CHARACTERS];
extern  XLOGGING_THREAD_LOCAL char formatWithStack[FORMAT_MAX_CHARACTERS];

void getThreadStackAsString(HANDLE thread, char* destination, size_t destinationSize);

#ifdef __cplusplus
}
#endif

#endif /*LOG_STACKTRACE_H*/
