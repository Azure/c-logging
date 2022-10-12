// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <string.h>
#include <stdbool.h>

#include "macro_utils/macro_utils.h"

#include "c_logging/xlogging.h"

/*not much in the way of testing, but "should not crash" */
int main(void)
{
    char really_big_string_fits[LOG_SIZE_REGULAR / 2]; /*expecting the string on the screen, even when it is big*/
    (void)memset(really_big_string_fits, '3', sizeof(really_big_string_fits));
    really_big_string_fits[sizeof(really_big_string_fits)-1] = '\0';

    char really_big_string_doesnt_fit[LOG_SIZE_REGULAR]; /*if the size of this array is greater than LOG_SIZE_REGULAR than it will not get printed as all memory is statically allocated and cannot grow*/
    (void)memset(really_big_string_doesnt_fit, 'M', sizeof(really_big_string_doesnt_fit));
    really_big_string_doesnt_fit[sizeof(really_big_string_doesnt_fit) - 1] = '\0';

    /*LogCritical*/
    LogCritical("hello world!");
    LogCritical("a more complicated hello with int=%d.", 42);
    LogCritical("the most complicated hello with int=%d and a really big string that fits =%s.", 42, really_big_string_fits);
    LogCritical("the most complicated hello with int=%d and a really big string that does not fit=%s.", 42, really_big_string_doesnt_fit);
    LogCritical("does PRI_BOOL work? %" PRI_BOOL "", MU_BOOL_VALUE(true) /*answer is yes, it works*/);
    LogCritical("does PRI_BOOL work with amazingly out of bounds value? %" PRI_BOOL "", MU_BOOL_VALUE(42));
    LogCritical("does PRI_BOOL works with false? %" PRI_BOOL "", MU_BOOL_VALUE(false));

    /*LogError*/
    LogError("hello world!");
    LogError("a more complicated hello with int=%d.", 42);
    LogError("the most complicated hello with int=%d and a really big string that fits =%s.", 42, really_big_string_fits);
    LogError("the most complicated hello with int=%d and a really big string that does not fit=%s.", 42, really_big_string_doesnt_fit);
    LogError("does PRI_BOOL work? %" PRI_BOOL "", MU_BOOL_VALUE(true) /*answer is yes, it works*/);
    LogError("does PRI_BOOL work with amazingly out of bounds value? %" PRI_BOOL "", MU_BOOL_VALUE(42));
    LogError("does PRI_BOOL works with false? %" PRI_BOOL "", MU_BOOL_VALUE(false));

#ifdef _MSC_VER /*linux doesn't have LogLastError*/
    /*LogLastError*/
    LogLastError("hello world!");
    LogLastError("a more complicated hello with int=%d.", 42);
    LogLastError("the most complicated hello with int=%d and a really big string that fits =%s.", 42, really_big_string_fits);
    LogLastError("the most complicated hello with int=%d and a really big string that does not fit=%s.", 42, really_big_string_doesnt_fit);
#endif
    /*LogWarning*/
    LogWarning("hello world!");
    LogWarning("a more complicated hello with int=%d.", 42);
    LogWarning("the most complicated hello with int=%d and a really big string that fits =%s.", 42, really_big_string_fits);
    LogWarning("the most complicated hello with int=%d and a really big string that does not fit=%s.", 42, really_big_string_doesnt_fit);
    LogWarning("does PRI_BOOL work? %" PRI_BOOL "", MU_BOOL_VALUE(true) /*answer is yes, it works*/);
    LogWarning("does PRI_BOOL work with amazingly out of bounds value? %" PRI_BOOL "", MU_BOOL_VALUE(42));
    LogWarning("does PRI_BOOL works with false? %" PRI_BOOL "", MU_BOOL_VALUE(false));

    /*LogInfo*/
    LogInfo("hello world!");
    LogInfo("a more complicated hello with int=%d.", 42);
    LogInfo("the most complicated hello with int=%d and a really big string that fits=%s.", 42, really_big_string_fits);
    LogInfo("the most complicated hello with int=%d and a really big string that does not fit=%s.", 42, really_big_string_doesnt_fit);
    LogInfo("does PRI_BOOL work? %" PRI_BOOL "", MU_BOOL_VALUE(true) /*answer is yes, it works*/);
    LogInfo("does PRI_BOOL work with amazingly out of bounds value? %" PRI_BOOL "", MU_BOOL_VALUE(42));
    LogInfo("does PRI_BOOL works with false? %" PRI_BOOL "", MU_BOOL_VALUE(false));

    /*LogVerbose*/
    LogVerbose("hello world!");
    LogVerbose("a more complicated hello with int=%d.", 42);
    LogVerbose("the most complicated hello with int=%d and a really big string that fits =%s.", 42, really_big_string_fits);
    LogVerbose("the most complicated hello with int=%d and a really big string that does not fit=%s.", 42, really_big_string_doesnt_fit);
    LogVerbose("does PRI_BOOL work? %" PRI_BOOL "", MU_BOOL_VALUE(true) /*answer is yes, it works*/);
    LogVerbose("does PRI_BOOL work with amazingly out of bounds value? %" PRI_BOOL "", MU_BOOL_VALUE(42));
    LogVerbose("does PRI_BOOL works with false? %" PRI_BOOL "", MU_BOOL_VALUE(false));
}

