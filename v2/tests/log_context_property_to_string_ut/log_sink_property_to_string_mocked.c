// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdint.h>
#include <stdio.h>

#include "c_logging/log_context_property_value_pair.h"

#define snprintf mock_snprintf

int mock_snprintf(char* s, size_t n, const char* format, ...);

#include "log_context_property_to_string.c"
