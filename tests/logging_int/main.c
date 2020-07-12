// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "azure_c_logging/xlogging.h"

/*not much in the way of testing, but "should not crash" */
int main(void)
{
    char really_big_string[LOG_SIZE_REGULAR/2]; /*if the size of this array is greater than LOG_SIZE_REGULAR than it will not get printed as all memory is statically allocated and cannot grow*/
    (void)memset(really_big_string, '3', sizeof(really_big_string));
    really_big_string[sizeof(really_big_string)-1] = '\0';

    /*LogInfo*/
    LogInfo("LogInfo: hello world!");
    LogInfo("LogInfo: a more complicated hello with int=%d.", 42);
    LogInfo("LogInfo: the most complicated hello with int=%d and a really big string=%s.", 42, really_big_string);


    /*LogError*/
    LogError("LogError: hello world!");
    LogError("LogError: a more complicated hello with int=%d.", 42);
    LogError("LogError: the most complicated hello with int=%d and a really big string=%s.", 42, really_big_string);
}

