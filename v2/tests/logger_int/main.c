// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#include "macro_utils/macro_utils.h"

#include "c_logging/logger.h"

#define LOG_SIZE_REGULAR 1024

void log_from_a_function(LOG_CONTEXT_HANDLE log_context)
{
    LOGGER_LOG(LOG_LEVEL_ERROR, log_context, "log from a function!");
}

/*not much in the way of testing, but "should not crash" */
int main(void)
{
    char really_big_string_fits[LOG_SIZE_REGULAR / 2]; /*expecting the string on the screen, even when it is big*/
    (void)memset(really_big_string_fits, '3', sizeof(really_big_string_fits));
    really_big_string_fits[sizeof(really_big_string_fits) - 1] = '\0';

    char really_big_string_doesnt_fit[LOG_SIZE_REGULAR]; /*if the size of this array is greater than LOG_SIZE_REGULAR than it will not get printed as all memory is statically allocated and cannot grow*/
    (void)memset(really_big_string_doesnt_fit, 'M', sizeof(really_big_string_doesnt_fit));
    really_big_string_doesnt_fit[sizeof(really_big_string_doesnt_fit) - 1] = '\0';

    /*LogCritical*/
    LOGGER_LOG(LOG_LEVEL_CRITICAL, NULL, "hello world!");
    LOGGER_LOG(LOG_LEVEL_CRITICAL, NULL, "a more complicated hello with int=%d.", 42);
    LOGGER_LOG(LOG_LEVEL_CRITICAL, NULL, "the most complicated hello with int=%d and a really big string that fits =%s.", 42, really_big_string_fits);
    LOGGER_LOG(LOG_LEVEL_CRITICAL, NULL, "the most complicated hello with int=%d and a really big string that does not fit=%s.", 42, really_big_string_doesnt_fit);
    LOGGER_LOG(LOG_LEVEL_CRITICAL, NULL, "does PRI_BOOL work? %" PRI_BOOL "", MU_BOOL_VALUE(true) /*answer is yes, it works*/);
    LOGGER_LOG(LOG_LEVEL_CRITICAL, NULL, "does PRI_BOOL work with amazingly out of bounds value? %" PRI_BOOL "", MU_BOOL_VALUE(42));
    LOGGER_LOG(LOG_LEVEL_CRITICAL, NULL, "does PRI_BOOL works with false? %" PRI_BOOL "", MU_BOOL_VALUE(false));

    /*LogError*/
    LOGGER_LOG(LOG_LEVEL_ERROR, NULL, "LogError: hello world!");
    LOGGER_LOG(LOG_LEVEL_ERROR, NULL, "LogError: a more complicated hello with int=%d.", 42);
    LOGGER_LOG(LOG_LEVEL_ERROR, NULL, "LogError: the most complicated hello with int=%d and a really big string that fits =%s.", 42, really_big_string_fits);
    LOGGER_LOG(LOG_LEVEL_ERROR, NULL, "LogError: the most complicated hello with int=%d and a really big string that does not fit=%s.", 42, really_big_string_doesnt_fit);
    LOGGER_LOG(LOG_LEVEL_ERROR, NULL, "does PRI_BOOL work? %" PRI_BOOL "", MU_BOOL_VALUE(true) /*answer is yes, it works*/);
    LOGGER_LOG(LOG_LEVEL_ERROR, NULL, "does PRI_BOOL work with amazingly out of bounds value? %" PRI_BOOL "", MU_BOOL_VALUE(42));
    LOGGER_LOG(LOG_LEVEL_ERROR, NULL, "does PRI_BOOL works with false? %" PRI_BOOL "", MU_BOOL_VALUE(false));

//#ifdef _MSC_VER /*linux doesn't have LogLastError*/
//    /*LogLastError*/
//    LogLastError("hello world!");
//    LogLastError("a more complicated hello with int=%d.", 42);
//    LogLastError("the most complicated hello with int=%d and a really big string that fits =%s.", 42, really_big_string_fits);
//    LogLastError("the most complicated hello with int=%d and a really big string that does not fit=%s.", 42, really_big_string_doesnt_fit);
//#endif
    /*LogWarning*/
    LOGGER_LOG(LOG_LEVEL_WARNING, NULL, "hello world!");
    LOGGER_LOG(LOG_LEVEL_WARNING, NULL, "a more complicated hello with int=%d.", 42);
    LOGGER_LOG(LOG_LEVEL_WARNING, NULL, "the most complicated hello with int=%d and a really big string that fits =%s.", 42, really_big_string_fits);
    LOGGER_LOG(LOG_LEVEL_WARNING, NULL, "the most complicated hello with int=%d and a really big string that does not fit=%s.", 42, really_big_string_doesnt_fit);
    LOGGER_LOG(LOG_LEVEL_WARNING, NULL, "does PRI_BOOL work? %" PRI_BOOL "", MU_BOOL_VALUE(true) /*answer is yes, it works*/);
    LOGGER_LOG(LOG_LEVEL_WARNING, NULL, "does PRI_BOOL work with amazingly out of bounds value? %" PRI_BOOL "", MU_BOOL_VALUE(42));
    LOGGER_LOG(LOG_LEVEL_WARNING, NULL, "does PRI_BOOL works with false? %" PRI_BOOL "", MU_BOOL_VALUE(false));
    //
    ///*LogInfo*/
    LOGGER_LOG(LOG_LEVEL_INFO, NULL, "hello world!");
    LOGGER_LOG(LOG_LEVEL_INFO, NULL, "a more complicated hello with int=%d.", 42);
    LOGGER_LOG(LOG_LEVEL_INFO, NULL, "the most complicated hello with int=%d and a really big string that fits=%s.", 42, really_big_string_fits);
    LOGGER_LOG(LOG_LEVEL_INFO, NULL, "the most complicated hello with int=%d and a really big string that does not fit=%s.", 42, really_big_string_doesnt_fit);
    LOGGER_LOG(LOG_LEVEL_INFO, NULL, "does PRI_BOOL work? %" PRI_BOOL "", MU_BOOL_VALUE(true) /*answer is yes, it works*/);
    LOGGER_LOG(LOG_LEVEL_INFO, NULL, "does PRI_BOOL work with amazingly out of bounds value? %" PRI_BOOL "", MU_BOOL_VALUE(42));
    LOGGER_LOG(LOG_LEVEL_INFO, NULL, "does PRI_BOOL works with false? %" PRI_BOOL "", MU_BOOL_VALUE(false));
    //
    ///*LogVerbose*/
    LOGGER_LOG(LOG_LEVEL_VERBOSE, NULL, "hello world!");
    LOGGER_LOG(LOG_LEVEL_VERBOSE, NULL, "a more complicated hello with int=%d.", 42);
    LOGGER_LOG(LOG_LEVEL_VERBOSE, NULL, "the most complicated hello with int=%d and a really big string that fits =%s.", 42, really_big_string_fits);
    LOGGER_LOG(LOG_LEVEL_VERBOSE, NULL, "the most complicated hello with int=%d and a really big string that does not fit=%s.", 42, really_big_string_doesnt_fit);
    LOGGER_LOG(LOG_LEVEL_VERBOSE, NULL, "does PRI_BOOL work? %" PRI_BOOL "", MU_BOOL_VALUE(true) /*answer is yes, it works*/);
    LOGGER_LOG(LOG_LEVEL_VERBOSE, NULL, "does PRI_BOOL work with amazingly out of bounds value? %" PRI_BOOL "", MU_BOOL_VALUE(42));
    LOGGER_LOG(LOG_LEVEL_VERBOSE, NULL, "does PRI_BOOL works with false? %" PRI_BOOL "", MU_BOOL_VALUE(false));

    char* prop_value = malloc(2);
    (void)strcpy(prop_value, "a");

    // allocate a context dynamically
    LOG_CONTEXT_HANDLE log_context;
    LOG_CONTEXT_CREATE(log_context, NULL, LOG_CONTEXT_STRING_PROPERTY(property_name, "%s", MU_P_OR_NULL(prop_value)));

    LOGGER_LOG(LOG_LEVEL_CRITICAL, log_context, "some critical error with context");

    LOG_CONTEXT_DESTROY(log_context);

    // free the string, showing that a copy is made in the context
    free(prop_value);

    // chained allocated context
    LOG_CONTEXT_HANDLE context_1;
    LOG_CONTEXT_CREATE(context_1, NULL, LOG_CONTEXT_NAME(context_1), LOG_CONTEXT_STRING_PROPERTY(name, "%s", "haga"));
    LOG_CONTEXT_HANDLE context_2;
    LOG_CONTEXT_CREATE(context_2, context_1, LOG_CONTEXT_NAME(context_2), LOG_CONTEXT_STRING_PROPERTY(last name, "%s%s", "ua", "ga"), LOG_CONTEXT_PROPERTY(int32_t, age, 42));
    
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
        LOG_MESSAGE("some message here with and integer = %d", 42));
}

