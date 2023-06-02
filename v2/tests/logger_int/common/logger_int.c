// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifdef _MSC_VER
#include "windows.h"
#endif

#include "macro_utils/macro_utils.h"

#include "c_logging/log_level.h"
#include "c_logging/log_context.h"

#include "c_logging/logger.h"

/* a simple test executable that aims at verifying that at least we do not crash when going through various ways of logging */
int main(void)
{
#ifdef _MSC_VER
    // make abort not popup
    _set_abort_behavior(_CALL_REPORTFAULT, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
#endif

    logger_init();

    LOGGER_LOG(LOG_LEVEL_CRITICAL, NULL, "Hello World from LogCritical, here's a value: %d", 4546);

    LOGGER_LOG(LOG_LEVEL_ERROR, NULL, "Hello World from LogError, here's a value: %d", 42);

    LOGGER_LOG(LOG_LEVEL_WARNING, NULL, "Hello World from LogWarning, here's a value: %d", 4242);

    LOGGER_LOG(LOG_LEVEL_INFO, NULL, "Hello World from LogInfo, here's a value: %d", 0x42);

    LOGGER_LOG(LOG_LEVEL_VERBOSE, NULL, "Hello World from LogVerbose, here's a value: %d", 0x4242);

    logger_deinit();

    return 0;
}
