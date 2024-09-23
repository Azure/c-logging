// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h> // IWYU pragma: keep

extern void* mock_malloc(size_t size);
extern void mock_free(void* ptr);

#define malloc mock_malloc
#define free mock_free
#define log_internal_error_report mock_log_internal_error_report

#include "log_context.c"
