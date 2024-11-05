// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stddef.h>
#include <stdio.h>

#include "c_logging/logging_stacktrace.h"

void getStackAsString(char* destination, size_t size)
{
    static const char message[] = "stack capture disabled";

    if (destination != NULL)
    {
        /*make a best effort at trying to provide some message*/
        (void)snprintf(destination, size, "%s", message);
    }
}
