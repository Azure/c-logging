// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <errno.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>

#undef errno
#define errno (mock_errno())
#define strerror_s mock_strerror_s

errno_t mock_errno(void);
errno_t mock_strerror_s(char* s, rsize_t maxsize, errno_t errnum);

#include "log_errno.c"
