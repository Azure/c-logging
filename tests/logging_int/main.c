// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <string.h>
#include <stdbool.h>

#include "macro_utils/macro_utils.h"

#include "c_logging/logger.h"

#define LOG_SIZE_REGULAR 4096 /*in bytes - a message is not expected to exceed this size in bytes, if it does, only LOG_SIZE_REGULAR characters are retained*/

/*not much in the way of testing, but "should not crash" */
int main(void)
{

    if (logger_init()!=0)
    {
        return MU_FAILURE;
    }

    char really_big_string_fits[LOG_SIZE_REGULAR / 2]; /*expecting the string on the screen, even when it is big*/
    (void)memset(really_big_string_fits, '3', sizeof(really_big_string_fits));
    really_big_string_fits[sizeof(really_big_string_fits)-1] = '\0';

    char really_big_string_doesnt_fit[LOG_SIZE_REGULAR]; /*if the size of this array is greater than LOG_SIZE_REGULAR than it will not get printed as all memory is statically allocated and cannot grow*/
    (void)memset(really_big_string_doesnt_fit, 'M', sizeof(really_big_string_doesnt_fit));
    really_big_string_doesnt_fit[sizeof(really_big_string_doesnt_fit) - 1] = '\0';

    /*LogCritical*/
    LogCritical("LogCritical: hello world!");
    LogCritical("LogCritical: a more complicated hello with int=%d.", 42);
    LogCritical("LogCritical: the most complicated hello with int=%d and a really big string that fits =%s.", 42, really_big_string_fits);
    LogCritical("LogCritical: the most complicated hello with int=%d and a really big string that does not fit=%s.", 42, really_big_string_doesnt_fit);
    LogCritical("LogCritical: does PRI_BOOL work? %" PRI_BOOL "", MU_BOOL_VALUE(true) /*answer is yes, it works*/);
    LogCritical("LogCritical: does PRI_BOOL work with amazingly out of bounds value? %" PRI_BOOL "", MU_BOOL_VALUE(42));
    LogCritical("LogCritical: does PRI_BOOL works with false? %" PRI_BOOL "", MU_BOOL_VALUE(false));

    /*LogError*/
    LogError("LogError: hello world!");
    LogError("LogError: a more complicated hello with int=%d.", 42);
    LogError("LogError: the most complicated hello with int=%d and a really big string that fits =%s.", 42, really_big_string_fits);
    LogError("LogError: the most complicated hello with int=%d and a really big string that does not fit=%s.", 42, really_big_string_doesnt_fit);
    LogError("LogCritical: does PRI_BOOL work? %" PRI_BOOL "", MU_BOOL_VALUE(true) /*answer is yes, it works*/);
    LogError("LogCritical: does PRI_BOOL work with amazingly out of bounds value? %" PRI_BOOL "", MU_BOOL_VALUE(42));
    LogError("LogCritical: does PRI_BOOL works with false? %" PRI_BOOL "", MU_BOOL_VALUE(false));

#ifdef _MSC_VER /*linux doesn't have LogLastError*/
    /*LogLastError*/
    LogLastError("LogLastError: hello world!");
    LogLastError("LogLastError: a more complicated hello with int=%d.", 42);
    LogLastError("LogLastError: the most complicated hello with int=%d and a really big string that fits =%s.", 42, really_big_string_fits);
    LogLastError("LogLastError: the most complicated hello with int=%d and a really big string that does not fit=%s.", 42, really_big_string_doesnt_fit);
#else   /*Windows doesn't have LogErrorNo for now*/
    /*LogErrorNo*/
    LogErrorNo("LogErrorNo: hello world!");
    LogErrorNo("LogLastError: a more complicated hello with int=%d.", 42);
    LogErrorNo("LogLastError: the most complicated hello with int=%d and a really big string that fits =%s.", 42, really_big_string_fits);
    LogErrorNo("LogLastError: the most complicated hello with int=%d and a really big string that does not fit=%s.", 42, really_big_string_doesnt_fit);
#endif

    /*LogWarning*/
    LogWarning("LogWarning: hello world!");
    LogWarning("LogWarning: a more complicated hello with int=%d.", 42);
    LogWarning("LogWarning: the most complicated hello with int=%d and a really big string that fits =%s.", 42, really_big_string_fits);
    LogWarning("LogWarning: the most complicated hello with int=%d and a really big string that does not fit=%s.", 42, really_big_string_doesnt_fit);
    LogWarning("LogCritical: does PRI_BOOL work? %" PRI_BOOL "", MU_BOOL_VALUE(true) /*answer is yes, it works*/);
    LogWarning("LogCritical: does PRI_BOOL work with amazingly out of bounds value? %" PRI_BOOL "", MU_BOOL_VALUE(42));
    LogWarning("LogCritical: does PRI_BOOL works with false? %" PRI_BOOL "", MU_BOOL_VALUE(false));

    /*LogInfo*/
    LogInfo("LogInfo: hello world!");
    LogInfo("LogInfo: a more complicated hello with int=%d.", 42);
    LogInfo("LogInfo: the most complicated hello with int=%d and a really big string that fits=%s.", 42, really_big_string_fits);
    LogInfo("LogInfo: the most complicated hello with int=%d and a really big string that does not fit=%s.", 42, really_big_string_doesnt_fit);
    LogInfo("LogCritical: does PRI_BOOL work? %" PRI_BOOL "", MU_BOOL_VALUE(true) /*answer is yes, it works*/);
    LogInfo("LogCritical: does PRI_BOOL work with amazingly out of bounds value? %" PRI_BOOL "", MU_BOOL_VALUE(42));
    LogInfo("LogCritical: does PRI_BOOL works with false? %" PRI_BOOL "", MU_BOOL_VALUE(false));

    /*LogVerbose*/
    LogVerbose("LogVerbose: hello world!");
    LogVerbose("LogVerbose: a more complicated hello with int=%d.", 42);
    LogVerbose("LogVerbose: the most complicated hello with int=%d and a really big string that fits =%s.", 42, really_big_string_fits);
    LogVerbose("LogVerbose: the most complicated hello with int=%d and a really big string that does not fit=%s.", 42, really_big_string_doesnt_fit);
    LogVerbose("LogCritical: does PRI_BOOL work? %" PRI_BOOL "", MU_BOOL_VALUE(true) /*answer is yes, it works*/);
    LogVerbose("LogCritical: does PRI_BOOL work with amazingly out of bounds value? %" PRI_BOOL "", MU_BOOL_VALUE(42));
    LogVerbose("LogCritical: does PRI_BOOL works with false? %" PRI_BOOL "", MU_BOOL_VALUE(false));

    logger_deinit();
}

