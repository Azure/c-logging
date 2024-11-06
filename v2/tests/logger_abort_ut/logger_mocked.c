// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h> // IWYU pragma: keep

#define abort mock_abort

void mock_abort(void);

#include "logger.c"

#undef abort
