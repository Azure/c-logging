// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <errno.h> // IWYU pragma: keep
#include <string.h>
#include <stddef.h>

#ifndef errno_t
typedef int errno_t;
#endif

#undef errno
#define errno (mock_errno())
#define strerror_r mock_strerror_r

errno_t mock_errno(void);
int mock_strerror_r(int errnum, char* buf, size_t buflen);

#include "log_errno_linux.c"
