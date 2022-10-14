// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdio.h>

#define printf mock_printf

extern int mock_printf(const char* format, ...);

#include "log_sink_console.c"
