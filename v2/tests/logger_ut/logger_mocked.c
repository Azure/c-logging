// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h> // IWYU pragma: keep

#define abort mock_abort
#define get_thread_stack_init mock_get_thread_stack_init
#define get_thread_stack_deinit mock_get_thread_stack_deinit

void mock_abort(void);
int mock_get_thread_stack_init(void);
void mock_get_thread_stack_deinit(void);

#include "logger.c"
