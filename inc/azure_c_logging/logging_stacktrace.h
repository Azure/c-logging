// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef LOGGING_STACKTRACE_H
#define LOGGING_STACKTRACE_H

#define TRACE_MAX_STACK_AS_STRING_SIZE 10*1024
#define FORMAT_WITH_STACK_SIZE 1024

#ifdef __cplusplus
extern "C" {
#endif

extern __declspec(thread) char formatWithStack[FORMAT_WITH_STACK_SIZE];

char* getStackAsString(void);

#ifdef __cplusplus
}
#endif

#endif /*LOGGING_STACKTRACE_H*/
