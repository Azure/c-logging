// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

#include "c_logging/xlogging.h"
#include "c_logging/consolelogger.h"

#if (defined(_MSC_VER))
#include <string.h>
#include "windows.h"

/*this function will use 1x puts (in the happy case) .*/
/*more than 1x printf / function call can mean intermingled LogErrors in a multithreaded env*/
/*the function will also attempt to produce some human readable strings for GetLastError*/
void consolelogger_log_with_GetLastError(const char* file, const char* func, int line, const char* format, ...)
{
    DWORD lastError;

    char message[LOG_SIZE_REGULAR];

    int size = 0; /*size tracks number of character from "message" that are used so far, not counting the last null character. Uses int as data type because snprintf functions return int*/

    int snprintf_result;

    /*this function builds a string in message variable from 3 sources
    1) the system state (time, file, function, line)
    2) the user (format,...)
    3) whatever GetLastError can provide
    */
    va_list args;
    va_start(args, format);

    lastError = GetLastError();

    message[0] = '\0';

    {/*scope for 1) the system state (time, file, function, line)*/
        time_t t;
        t = time(NULL);
        snprintf_result = snprintf(message + size, sizeof(message) - size, "Error: Time:%.24s File:%s Func:%s Line:%d ", ctime(&t), file, func, line);
        if (snprintf_result < 0)
        {
            (void)puts("error in snprintf trying to output the system message");
        }
        else
        {
            if (snprintf_result >= (int)sizeof(message) - size)
            {
                (void)puts("not enough caracters in message to hold the system message");
            }
            else
            {
                size += snprintf_result;
            }
        }
    }

    {/*scope for 2) the user (format,...)*/
        snprintf_result = vsnprintf(message + size, sizeof(message) - size, format, args);
        if (snprintf_result < 0)
        {
            (void)puts("error in vsnprintf trying to output the user message");
        }
        else
        {
            if (snprintf_result >= (int)sizeof(message) - size)
            {
                (void)puts("not enough characters in message to hold the user message");
            }
            else
            {
                size += snprintf_result;
            }
        }
    }

    {/*scope for 3) whatever GetLastError can provide*/

        /*add the getlastError for good measure anyway*/
        snprintf_result = snprintf(message + size, sizeof(message) - size, " GetLastError()=%#lx ", lastError);
        if (snprintf_result < 0)
        {
            (void)puts("error in snprintf trying to output GetLastError's value");
        }
        else
        {
            if (snprintf_result >= (int)sizeof(message) - size)
            {
                (void)puts("not enough characters in message to hold  GetLastError's value");
            }
            else
            {
                size += snprintf_result;

                if (FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, lastError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), message + size, sizeof(message) - size, NULL) == 0)
                {
                    (void)puts("error in snprintf trying to output GetLastError's value as string");
                }
                else
                {
                    /*remove extraneous newlines from FormatMessageA's output*/
                    char* whereAreThey;
                    if ((whereAreThey = strchr(message + size, '\r')) != NULL)
                    {
                        *whereAreThey = '\0';
                    }
                    if ((whereAreThey = strchr(message + size, '\n')) != NULL)
                    {
                        *whereAreThey = '\0';
                    }

                    /*everything has been compiled in message...*/
                }
            }
        }
    }

    /*in any case, print the string as is*/
    (void)puts(message);

    va_end(args);
}
#else
#include <errno.h>

/*this function will use 1x puts (in the happy case) .*/
/*more than 1x printf / function call can mean intermingled LogErrors in a multithreaded env*/
/*the function will also attempt to produce some human readable strings for GetLastError*/
void consolelogger_log_with_GetErrorNo(const char* file, const char* func, int line, const char* format, ...)
{
    int error_no;

    char message[LOG_SIZE_REGULAR];

    int size = 0; /*size tracks number of character from "message" that are used so far, not counting the last null character. Uses int as data type because snprintf functions return int*/

    int snprintf_result;

    /*this function builds a string in message variable from 3 sources
    1) the system state (time, file, function, line)
    2) the user (format,...)
    3) whatever GetLastError can provide
    */
    va_list args;
    va_start(args, format);

    error_no = errno;

    message[0] = '\0';

    {/*scope for 1) the system state (time, file, function, line)*/
        time_t t;
        t = time(NULL);
        snprintf_result = snprintf(message + size, sizeof(message) - size, "Error: Time:%.24s File:%s Func:%s Line:%d ", ctime(&t), file, func, line);
        if (snprintf_result < 0)
        {
            (void)puts("error in snprintf trying to output the system message");
        }
        else
        {
            if (snprintf_result >= (int)sizeof(message) - size)
            {
                (void)puts("not enough caracters in message to hold the system message");
            }
            else
            {
                size += snprintf_result;
            }
        }
    }

    {/*scope for 2) the user (format,...)*/
        snprintf_result = vsnprintf(message + size, sizeof(message) - size, format, args);
        if (snprintf_result < 0)
        {
            (void)puts("error in vsnprintf trying to output the user message");
        }
        else
        {
            if (snprintf_result >= (int)sizeof(message) - size)
            {
                (void)puts("not enough characters in message to hold the user message");
            }
            else
            {
                size += snprintf_result;
            }
        }
    }

    {/*scope for 3) whatever GetLastError can provide*/

        /*add the getlastError for good measure anyway*/
        snprintf_result = snprintf(message + size, sizeof(message) - size, " ErrorNo()=%d ", error_no);
        if (snprintf_result < 0)
        {
            (void)puts("error in snprintf trying to output GetLastError's value");
        }
        else
        {
            if (snprintf_result >= (int)sizeof(message) - size)
            {
                (void)puts("not enough characters in message to hold  GetLastError's value");
            }
            else
            {
                size += snprintf_result;
                if (strerror_r(errno, message + size, sizeof(message) - size) != 0)
                {
                    (void)puts("error in snprintf trying to output strerror_r's value as string");
                }
                else
                {
                    /*remove extraneous newlines from FormatMessageA's output*/
                    char* whereAreThey;
                    if ((whereAreThey = strchr(message + size, '\r')) != NULL)
                    {
                        *whereAreThey = '\0';
                    }
                    if ((whereAreThey = strchr(message + size, '\n')) != NULL)
                    {
                        *whereAreThey = '\0';
                    }

                    /*everything has been compiled in message...*/
                }
            }
        }
    }

    /*in any case, print the string as is*/
    (void)puts(message);

    va_end(args);
}
#endif

#if defined(__GNUC__)
__attribute__ ((format (printf, 6, 7)))
#endif
void consolelogger_log(LOG_CATEGORY log_category, const char* file, const char* func, int line, unsigned int options, const char* format, ...)
{
    time_t t;
    va_list args;
    va_start(args, format);

    t = time(NULL);

    switch (log_category)
    {
    case AZ_LOG_CRITICAL:
        (void)printf("Critical: Time:%.24s File:%s Func:%s Line:%d ", ctime(&t), file, func, line);
        break;
    case AZ_LOG_ERROR:
        (void)printf("Error: Time:%.24s File:%s Func:%s Line:%d ", ctime(&t), file, func, line);
        break;
    case AZ_LOG_WARNING:
        (void)printf("Warning: Time:%.24s File:%s Func:%s Line:%d ", ctime(&t), file, func, line);
        break;
    case AZ_LOG_INFO:
        (void)printf("Info: ");
        break;
    case AZ_LOG_VERBOSE:
        (void)printf("Verbose: ");
        break;
    default:
        break;
    }

    (void)vprintf(format, args);
    va_end(args);

    if (options & LOG_LINE)
    {
        (void)printf("\r\n");
    }
}

