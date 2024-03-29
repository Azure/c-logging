// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifdef _MSC_VER
#include "windows.h"
#else
#include <errno.h>
#endif

#include "c_logging/xlogging.h"

int main(void)
{
    LogCritical("Hello World from LogCritical, here's a value: %d", 4546);

    LogError("Hello World from LogError, here's a value: %d", 42);

    LogWarning("Hello World from LogWarning, here's a value: %d", 4242);

    LogInfo("Hello World from LogInfo, here's a value: %d", 0x42);

    LogVerbose("Hello World from LogVerbose, here's a value: %d", 0x4242);

#ifdef _MSC_VER /*linux does not have LogLastError*/
    SetLastError(ERROR_ACCESS_DENIED);

    LogLastError("Hello World from LogLastError, some access was denied! here's a value: 0x%x", '3');

    SetLastError(ERROR_SUCCESS);

    LogLastError("Hello World from LogLastError, everything is fine now! here's a value: 0x%x", '3');

#else
    errno = EACCES;

    LogErrorNo("Hello World from LogLastError, some access was denied! here's a value: 0x%x", '3');

    errno = 0;

    LogErrorNo("Hello World from LogLastError, everything is fine now! here's a value: 0x%x", '3');

#endif

    return 0;
}
