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

#ifdef __cplusplus
extern "C" {
#endif

    void getThreadStackAsString(HANDLE thread, char* destination, size_t destinationSize);

#ifdef __cplusplus
}
#endif

#endif /*LOG_STACKTRACE_H*/
