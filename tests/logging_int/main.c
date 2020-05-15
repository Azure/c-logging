// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "azure_c_logging/xlogging.h"

/*not much in the way of testing, but "should not crash" */
int main(void)
{

    char really_big_string[4096]; /*4096 (the size of this array) needs top be bigger than LOG_SIZE_REGULAR from etwlogger_driver.c*/
    (void)memset(really_big_string, '3', sizeof(really_big_string));
    really_big_string[4095] = '\0';

    /*LogInfo*/
    LogInfo("LogInfo: hello world!");
    LogInfo("LogInfo: a more complicated hello with int=%d.", 42);
    LogInfo("LogInfo: the most complicated hello with int=%d and a really big string=%s.", 42, really_big_string);


    /*LogError*/
    LogError("LogError: hello world!");
    LogError("LogError: a more complicated hello with int=%d.", 42);
    LogError("LogError: the most complicated hello with int=%d and a really big string=%s.", 42, really_big_string);
}

