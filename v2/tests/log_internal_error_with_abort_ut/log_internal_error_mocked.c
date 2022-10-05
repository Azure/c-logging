// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h> // IWYU pragma: keep

#define abort mock_abort
#define _set_abort_behavior mock__set_abort_behavior

void mock_abort(void);
unsigned int mock__set_abort_behavior(unsigned int _Flags, unsigned int _Mask);

#undef LOG_ABORT_ON_ERROR
#define LOG_ABORT_ON_ERROR

#include "log_internal_error.c"
