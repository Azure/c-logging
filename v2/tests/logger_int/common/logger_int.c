// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef _MSC_VER
#include "windows.h"
#endif

#ifdef USE_VLD
#include "vld.h"
#endif

#include "macro_utils/macro_utils.h"

#include "c_logging/log_level.h"
#include "c_logging/log_context.h"
#include "c_logging/log_context_property_basic_types.h"
#include "c_logging/log_context_property_type_ascii_char_ptr.h"

#include "c_logging/logger.h"

void log_from_a_function(LOG_CONTEXT_HANDLE log_context)
{
    LOGGER_LOG(LOG_LEVEL_ERROR, log_context, "log from a function!");
}

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

    char* prop_value = malloc(2);
    (void)strcpy(prop_value, "a");

    // allocate a context dynamically
    LOG_CONTEXT_HANDLE log_context;
    LOG_CONTEXT_CREATE(log_context, NULL, LOG_CONTEXT_STRING_PROPERTY(property_name, "%s", MU_P_OR_NULL(prop_value)));

    // free the string, showing that a copy is made in the context
    free(prop_value);

    LOGGER_LOG(LOG_LEVEL_CRITICAL, log_context, "some critical error with context");

    LOG_CONTEXT_DESTROY(log_context);

    // chained allocated context
    LOG_CONTEXT_HANDLE context_1;
    LOG_CONTEXT_CREATE(context_1, NULL, LOG_CONTEXT_STRING_PROPERTY(name, "%s", "haga"));
    LOG_CONTEXT_HANDLE context_2;
    LOG_CONTEXT_CREATE(context_2, context_1, LOG_CONTEXT_STRING_PROPERTY(last_name, "%s%s", "ua", "ga"), LOG_CONTEXT_PROPERTY(int32_t, age, 42));

    LOGGER_LOG(LOG_LEVEL_ERROR, context_1, "value of some_var = %d", 42);
    LOGGER_LOG(LOG_LEVEL_WARNING, context_2, "some other string value is %s", "mumu");

    LOG_CONTEXT_DESTROY(context_1);

    log_from_a_function(context_2);

    // stack allocated context
    LOG_CONTEXT_LOCAL_DEFINE(local_context, context_2, LOG_CONTEXT_STRING_PROPERTY(the_knights_that_say, "%s", "Nee!"));

    LOGGER_LOG(LOG_LEVEL_INFO, &local_context, "log info with local context");

    // local context chaining another local context
    LOG_CONTEXT_LOCAL_DEFINE(local_context_2, &local_context, LOG_CONTEXT_STRING_PROPERTY(other_knights_that_say, "%s", "Moo!"));

    LOGGER_LOG(LOG_LEVEL_VERBOSE, &local_context_2, "log with nee and moo");

    log_from_a_function(&local_context_2);

    LOG_CONTEXT_DESTROY(context_2);

    LOGGER_LOG_EX(LOG_LEVEL_ERROR,
        LOG_CONTEXT_STRING_PROPERTY(name, "%s%s", "go", "gu"),
        LOG_CONTEXT_PROPERTY(int32_t, age, 42));

    LOGGER_LOG_EX(LOG_LEVEL_ERROR,
        LOG_CONTEXT_STRING_PROPERTY(name, "%s%s", "go", "gu"),
        LOG_MESSAGE("some message here with an integer = %d", 42));

#ifdef WIN32
    LOGGER_LOG_EX(LOG_LEVEL_ERROR,
        LOG_LASTERROR(),
        LOG_MESSAGE("some message here with an integer = %d", 42));
#endif

    logger_deinit();

    return 0;
}
