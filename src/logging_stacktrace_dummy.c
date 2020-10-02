// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stddef.h>
#include <stdio.h>

#include "azure_c_logging/logging_stacktrace.h"

static const char message[] = "stack capture disabled";

void getStackAsString(char* destination, size_t size)
{
    if (destination != NULL)
    {
        /*make a best effort at trying to provide some message*/
        (void)snprintf(destination, size, "%s", message);
    }
}
