// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef XLOGGING_H
#define XLOGGING_H

#ifdef __cplusplus
#include <cstdlib>
#else
#include <stdlib.h>
#endif

#include "azure_macro_utils/macro_utils.h"

#ifdef LOGERROR_CAPTURES_STACK_TRACES
#include "azure_c_logging/logging_stacktrace.h"
#endif

#define LOG_SIZE_REGULAR 4096 /*in bytes - a message is not expected to exceed this size in bytes, if it does, only LOG_SIZE_REGULAR characters are retained*/

#ifdef __cplusplus
#include <cstdio>
#else
#include <stdio.h>
#endif /* __cplusplus */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define LOG_CATEGORY_VALUES \
    AZ_LOG_CRITICAL, \
    AZ_LOG_ERROR, \
    AZ_LOG_WARNING, \
    AZ_LOG_INFO, \
    AZ_LOG_VERBOSE

MU_DEFINE_ENUM(LOG_CATEGORY, LOG_CATEGORY_VALUES)

#if defined _MSC_VER
#define FUNC_NAME __FUNCDNAME__
#else
#define FUNC_NAME __func__
#endif

typedef void(*LOGGER_LOG)(LOG_CATEGORY log_category, const char* file, const char* func, int line, unsigned int options, const char* format, ...);
typedef void(*LOGGER_LOG_GETLASTERROR)(const char* file, const char* func, int line, const char* format, ...);

#define MESSAGE_BUFFER_SIZE 260

#define LOG_LINE 0x01

/*no logging is useful when time and fprintf are mocked*/
#ifdef NO_LOGGING
#define LOG(...)
#define LogCritical(...)
#define LogError(...)
#define LogWarning(...)
#define LogInfo(...)
#define LogVerbose(...)
#define LogLastError(...)
#define xlogging_get_log_function() NULL
#define xlogging_set_log_function(...)
#define LogErrorWinHTTPWithGetLastErrorAsString(...)
#else /* NO_LOGGING */

// In order to make sure that the compiler evaluates the arguments and issues an error if they do not conform to printf
// specifications, we call printf with the format and __VA_ARGS__. Since C && operator is shortcircuiting no actual runtime call to printf is performed.
#if defined _MSC_VER
#ifndef LOGERROR_CAPTURES_STACK_TRACES
// ignore warning C4127 
#define LOG(log_category, log_options, format, ...) \
{ \
    (void)(0 && printf(format, __VA_ARGS__)); \
    { \
        LOGGER_LOG logger_function = xlogging_get_log_function(); \
        if (logger_function != NULL) \
        { \
            logger_function(log_category, __FILE__, FUNC_NAME, __LINE__, log_options, format, __VA_ARGS__); \
        } \
    } \
}
#else /*LOGERROR_CAPTURES_STACK_TRACES is defined*/ 
#define STACK_PRINT_FORMAT "\nStack:\n%s"
#define STACK_MAX_CHARACTERS 2048 /*whatever we get from stack cannot exceed this size*/
#define FORMAT_MAX_CHARACTERS 1024 /*user format + STACK_PRINT_FORMAT in a string cannot exceed this size*/

#define LOG(log_category, log_options, format, ...) MU_C2(LOG_,log_category)(log_category, log_options, format, __VA_ARGS__)

#define LOG_AZ_LOG_WITH_STACK(log_category, log_options, format, ...)                                                                                                   \
{                                                                                                                                                                       \
    (void)(0 && printf(format, __VA_ARGS__));                                                                                                                           \
    {                                                                                                                                                                   \
        LOGGER_LOG logger_function = xlogging_get_log_function();                                                                                                       \
        if (logger_function != NULL)                                                                                                                                    \
        {                                                                                                                                                               \
            char stackAsString[STACK_MAX_CHARACTERS];                                                                                                                   \
            getStackAsString(stackAsString, sizeof(STACK_MAX_CHARACTERS));                                                                                              \
            size_t formatSize = strlen(format);                                                                                                                         \
            if(formatSize+sizeof(STACK_PRINT_FORMAT)+1>FORMAT_MAX_CHARACTERS)                                                                                           \
            { /*skipping stack printing*/                                                                                                                               \
                logger_function(log_category, __FILE__, FUNC_NAME, __LINE__, log_options, format, __VA_ARGS__);                                                         \
            }                                                                                                                                                           \
            else                                                                                                                                                        \
            {                                                                                                                                                           \
                char formatWithStack[FORMAT_MAX_CHARACTERS];                                                                                                            \
                (void)memcpy(formatWithStack, format, formatSize);                                                                                                      \
                (void)memcpy(formatWithStack + formatSize, STACK_PRINT_FORMAT, sizeof(STACK_PRINT_FORMAT));                                                             \
                logger_function(log_category, __FILE__, FUNC_NAME, __LINE__, log_options, formatWithStack, __VA_ARGS__, stackAsString);                                 \
            }                                                                                                                                                           \
        }                                                                                                                                                               \
    }                                                                                                                                                                   \
}

#define LOG_AZ_LOG_WITHOUT_STACK(log_category, log_options, format, ...)                                                                                                \
{                                                                                                                                                                       \
    (void)(0 && printf(format, __VA_ARGS__));                                                                                                                           \
    {                                                                                                                                                                   \
        LOGGER_LOG logger_function = xlogging_get_log_function();                                                                                                                     \
        if (logger_function != NULL)                                                                                                                                                  \
        {                                                                                                                                                               \
            logger_function(log_category, __FILE__, FUNC_NAME, __LINE__, log_options, format, __VA_ARGS__);                                                                           \
        }                                                                                                                                                               \
    }                                                                                                                                                                   \
}

#define LOG_AZ_LOG_CRITICAL(log_category, log_options, format, ...) \
    LOG_AZ_LOG_WITH_STACK(log_category, log_options, format, __VA_ARGS__)

#define LOG_AZ_LOG_ERROR(log_category, log_options, format, ...) \
    LOG_AZ_LOG_WITH_STACK(log_category, log_options, format, __VA_ARGS__)

#define LOG_AZ_LOG_WARNING(log_category, log_options, format, ...) \
    LOG_AZ_LOG_WITH_STACK(log_category, log_options, format, __VA_ARGS__)

#define LOG_AZ_LOG_INFO(log_category, log_options, format, ...) \
    LOG_AZ_LOG_WITHOUT_STACK(log_category, log_options, format, __VA_ARGS__)

#define LOG_AZ_LOG_VERBOSE(log_category, log_options, format, ...) \
    LOG_AZ_LOG_WITHOUT_STACK(log_category, log_options, format, __VA_ARGS__)

#endif /*LOGERROR_CAPTURES_STACK_TRACES */
#else /* _MSC_VER */
#define LOG(log_category, log_options, format, ...) { (void)(0 && printf(format, ##__VA_ARGS__)); { LOGGER_LOG logger_function = xlogging_get_log_function(); if (logger_function != NULL) logger_function(log_category, __FILE__, FUNC_NAME, __LINE__, log_options, format, ##__VA_ARGS__); } }
#endif /* _MSC_VER */

#ifdef WIN32
void xlogging_LogErrorWinHTTPWithGetLastErrorAsStringFormatter(int errorMessageID);
#endif /* WIN32 */

#if defined _MSC_VER

void xlogging_set_log_function_GetLastError(LOGGER_LOG_GETLASTERROR log_function);
LOGGER_LOG_GETLASTERROR xlogging_get_log_function_GetLastError(void);
#define LogLastError(FORMAT, ...) do{ LOGGER_LOG_GETLASTERROR logger_function = xlogging_get_log_function_GetLastError(); if(logger_function != NULL) logger_function(__FILE__, FUNC_NAME, __LINE__, FORMAT, __VA_ARGS__); }while((void)0,0)

#define LogCritical(FORMAT, ...) do{ LOG(AZ_LOG_CRITICAL, LOG_LINE, FORMAT, __VA_ARGS__); }while((void)0,0)
#define LogError(FORMAT, ...) do{ LOG(AZ_LOG_ERROR, LOG_LINE, FORMAT, __VA_ARGS__); }while((void)0,0)
#define LogWarning(FORMAT, ...) do{ LOG(AZ_LOG_WARNING, LOG_LINE, FORMAT, __VA_ARGS__); }while((void)0,0)
#define LogInfo(FORMAT, ...) do{LOG(AZ_LOG_INFO, LOG_LINE, FORMAT, __VA_ARGS__); }while((void)0,0)
#define LogVerbose(FORMAT, ...) do{LOG(AZ_LOG_VERBOSE, LOG_LINE, FORMAT, __VA_ARGS__); }while((void)0,0)

#define LogErrorWinHTTPWithGetLastErrorAsString(FORMAT, ...) do { \
                int errorMessageID = GetLastError(); \
                LogError(FORMAT, __VA_ARGS__); \
                xlogging_LogErrorWinHTTPWithGetLastErrorAsStringFormatter(errorMessageID); \
            } while((void)0,0)
#else // _MSC_VER

#define LogCritical(FORMAT, ...) do{ LOG(AZ_LOG_CRITICAL, LOG_LINE, FORMAT, ##__VA_ARGS__); }while((void)0,0)
#define LogError(FORMAT, ...) do{ LOG(AZ_LOG_ERROR, LOG_LINE, FORMAT, ##__VA_ARGS__); }while((void)0,0)
#define LogWarning(FORMAT, ...) do{ LOG(AZ_LOG_WARNING, LOG_LINE, FORMAT, ##__VA_ARGS__); }while((void)0,0)
#define LogInfo(FORMAT, ...) do{LOG(AZ_LOG_INFO, LOG_LINE, FORMAT, ##__VA_ARGS__); }while((void)0,0)
#define LogVerbose(FORMAT, ...) do{LOG(AZ_LOG_VERBOSE, LOG_LINE, FORMAT, ##__VA_ARGS__); }while((void)0,0)

#ifdef WIN32
// Included when compiling on Windows but not with MSVC, e.g. with MinGW.
#define LogErrorWinHTTPWithGetLastErrorAsString(FORMAT, ...) do { \
                int errorMessageID = GetLastError(); \
                LogError(FORMAT, ##__VA_ARGS__); \
                xlogging_LogErrorWinHTTPWithGetLastErrorAsStringFormatter(errorMessageID); \
            } while((void)0,0)
#endif // WIN32

#endif // _MSC_VER

void xlogging_set_log_function(LOGGER_LOG log_function);
LOGGER_LOG xlogging_get_log_function(void);

#endif /* NO_LOGGING */

#ifdef __cplusplus
}   // extern "C"
#endif /* __cplusplus */

#endif /* XLOGGING_H */
