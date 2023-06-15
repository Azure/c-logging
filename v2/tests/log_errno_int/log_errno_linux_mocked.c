// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <errno.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>

#ifndef errno_t
typedef int errno_t;
#endif

#undef errno
#define errno (mock_errno())

errno_t mock_errno(void);

#include "log_errno_linux.c"
