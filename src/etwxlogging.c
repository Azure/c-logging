// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "c_logging/xlogging.h"

#include "c_logging/etwlogger_driver.h"

#ifndef _MSC_VER
#error only supported on Windows
#endif

static LOGGER_LOG global_log_function = etwlogger_log;

void xlogging_set_log_function(LOGGER_LOG log_function)
{
    global_log_function = log_function;
}

LOGGER_LOG xlogging_get_log_function(void)
{
    return global_log_function;
}

LOGGER_LOG_GETLASTERROR global_log_function_GetLastError = etwlogger_log_with_GetLastError;

void xlogging_set_log_function_GetLastError(LOGGER_LOG_GETLASTERROR log_function_GetLastError)
{
    global_log_function_GetLastError = log_function_GetLastError;
}

LOGGER_LOG_GETLASTERROR xlogging_get_log_function_GetLastError(void)
{
    return global_log_function_GetLastError;
}

void xlogging_LogErrorWinHTTPWithGetLastErrorAsStringFormatter(int errorMessageID)
{
    (void)errorMessageID;
}
