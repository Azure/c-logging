// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef GET_THREAD_STACK_H
#define GET_THREAD_STACK_H

#ifdef __cplusplus
#include <cstddef>
#else
#include <stddef.h>
#endif

#if defined(_MSC_VER)
#include "windows.h" /*needed for DWORD*/
#elif defined(__linux__)
#include <pthread.h> /*needed for pthread_t*/
#else
#error no other platform supported
#endif

#ifdef __cplusplus
extern "C" {
#endif

    /*initializes statics needed to get a thread stack - not thread safe in itself*/
    int get_thread_stack_init(void);

#if defined(_MSC_VER)
    void get_thread_stack(DWORD threadId, char* destination, size_t destinationSize);
#elif defined(__linux__)
    void get_thread_stack(pthread_t threadId, char* destination, size_t destinationSize);
#endif

    /*refreshes the loaded module list so symbols from DLLs loaded after init are resolved*/
    void get_thread_stack_refresh_module_list(void);

    /*deinitializes statics - not thread safe*/
    void get_thread_stack_deinit(void);

#ifdef __cplusplus
}
#endif

#endif /*GET_THREAD_STACK_H*/
