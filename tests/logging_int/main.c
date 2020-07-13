// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "azure_c_logging/xlogging.h"

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
    LogCritical("LogCritical: hello world!");
    LogCritical("LogCritical: a more complicated hello with int=%d.", 42);
    LogCritical("LogCritical: the most complicated hello with int=%d and a really big string that fits =%s.", 42, really_big_string_fits);
    LogCritical("LogCritical: the most complicated hello with int=%d and a really big string that does not fit=%s.", 42, really_big_string_doesnt_fit);

    /*LogError*/
    LogError("LogError: hello world!");
    LogError("LogError: a more complicated hello with int=%d.", 42);
    LogError("LogError: the most complicated hello with int=%d and a really big string that fits =%s.", 42, really_big_string_fits);
    LogError("LogError: the most complicated hello with int=%d and a really big string that does not fit=%s.", 42, really_big_string_doesnt_fit);

    /*LogWarning*/
    LogWarning("LogWarning: hello world!");
    LogWarning("LogWarning: a more complicated hello with int=%d.", 42);
    LogWarning("LogWarning: the most complicated hello with int=%d and a really big string that fits =%s.", 42, really_big_string_fits);
    LogWarning("LogWarning: the most complicated hello with int=%d and a really big string that does not fit=%s.", 42, really_big_string_doesnt_fit);

    /*LogInfo*/
    LogInfo("LogInfo: hello world!");
    LogInfo("LogInfo: a more complicated hello with int=%d.", 42);
    LogInfo("LogInfo: the most complicated hello with int=%d and a really big string that fits=%s.", 42, really_big_string_fits);
    LogInfo("LogInfo: the most complicated hello with int=%d and a really big string that does not fit=%s.", 42, really_big_string_doesnt_fit);

    /*LogVerbose*/
    LogVerbose("LogVerbose: hello world!");
    LogVerbose("LogVerbose: a more complicated hello with int=%d.", 42);
    LogVerbose("LogVerbose: the most complicated hello with int=%d and a really big string that fits =%s.", 42, really_big_string_fits);
    LogVerbose("LogVerbose: the most complicated hello with int=%d and a really big string that does not fit=%s.", 42, really_big_string_doesnt_fit);
}

