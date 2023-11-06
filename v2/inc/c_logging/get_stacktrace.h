// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef GET_STACKTRACE_H
#define GET_STACKTRACE_H

#ifdef __cplusplus
#include <cstddef>
#else
#include <stddef.h>
#endif

#if defined(_MSC_VER)
#include "windows.h" /*needed for HANDLE*/
#else
#error no other platform supported
#endif

#ifdef __cplusplus
extern "C" {
#endif

    void getThreadStackAsString(HANDLE thread, char* destination, size_t destinationSize);

#ifdef __cplusplus
}
#endif

#endif /*GET_STACKTRACE_H*/
