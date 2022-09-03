// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#define abort mock_abort
#define _set_abort_behavior mock__set_abort_behavior

#undef LOG_ABORT_ON_ERROR

#include "log_internal_error.c"
