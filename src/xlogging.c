// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef NO_LOGGING

#include "c_logging/xlogging.h"
#include "c_logging/consolelogger.h"

#ifdef WIN32
#include "windows.h"
#endif // WIN32


LOGGER_LOG global_log_function = consolelogger_log;

void xlogging_set_log_function(LOGGER_LOG log_function)
{
    global_log_function = log_function;
}

LOGGER_LOG xlogging_get_log_function(void)
{
    return global_log_function;
}

#if (defined(_MSC_VER))

LOGGER_LOG_GETLASTERROR global_log_function_GetLastError = consolelogger_log_with_GetLastError;

void xlogging_set_log_function_GetLastError(LOGGER_LOG_GETLASTERROR log_function_GetLastError)
{
    global_log_function_GetLastError = log_function_GetLastError;
}

LOGGER_LOG_GETLASTERROR xlogging_get_log_function_GetLastError(void)
{
    return global_log_function_GetLastError;
}
#else
LOGGER_LOG_GETERRORNO global_log_function_GetErrorNo = consolelogger_log_with_GetErrorNo;

void xlogging_set_log_function_GetLastError(LOGGER_LOG_GETLASTERROR log_function_GetErrorNo)
{
    global_log_function_GetErrorNo = log_function_GetErrorNo;
}

LOGGER_LOG_GETERRORNO xlogging_get_log_function_GetErrorNo(void)
{
    return global_log_function_GetErrorNo;
}
#endif

#ifdef WIN32

void xlogging_LogErrorWinHTTPWithGetLastErrorAsStringFormatter(int errorMessageID)
{
    char messageBuffer[MESSAGE_BUFFER_SIZE];
    if (errorMessageID == 0)
    {
        LogError("GetLastError() returned 0. Make sure you are calling this right after the code that failed. ");
    }
    else
    {
        int size = FormatMessage(FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS,
            GetModuleHandle("WinHttp"), errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), messageBuffer, MESSAGE_BUFFER_SIZE, NULL);
        if (size == 0)
        {
            size = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), messageBuffer, MESSAGE_BUFFER_SIZE, NULL);
            if (size == 0)
            {
                LogError("GetLastError Code: %d. ", errorMessageID);
            }
            else
            {
                LogError("GetLastError: %s.", messageBuffer);
            }
        }
        else
        {
            LogError("GetLastError: %s.", messageBuffer);
        }
    }
}
#endif // WIN32


#endif // NO_LOGGING
