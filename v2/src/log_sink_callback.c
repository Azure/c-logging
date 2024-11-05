// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>

#include "macro_utils/macro_utils.h"

#include "c_logging/log_level.h"
#include "c_logging/log_context.h"
#include "c_logging/log_context_property_to_string.h"
#include "c_logging/log_context_property_value_pair.h"
#include "c_logging/log_sink_if.h"
#include "c_logging/logger.h"

#include "c_logging/log_sink_callback.h" // IWYU pragma: keep

#define MIN(a, b) ((a) < (b)) ? (a) : (b)

static const char error_string_invalid_args[] = "Error logging: invalid arguments";
static const char error_string_sink_callback[] = "Error formatting log line";

void log_sink_callback_noop_callback(void* context, LOG_LEVEL log_level, const char* message)
{
    (void)context;
    (void)log_level;
    (void)message;
}

static LOG_SINK_CALLBACK_LOG log_sink_callback_callback = log_sink_callback_noop_callback;
static void* log_sink_callback_context = NULL;
static LOG_LEVEL log_sink_callback_max_level = LOG_LEVEL_VERBOSE;

static int log_sink_callback_init(void)
{
    /* Codes_SRS_LOG_SINK_CALLBACK_42_001: [ log_sink_callback.init shall return 0. ]*/
    return 0;
}

static void log_sink_callback_deinit(void)
{
    /* Codes_SRS_LOG_SINK_CALLBACK_42_002: [ log_sink_callback.deinit shall return. ]*/
}

int log_sink_callback_set_callback(LOG_SINK_CALLBACK_LOG log_callback, void* context)
{
    int result;
    if (log_callback == NULL)
    {
        /*Codes_SRS_LOG_SINK_CALLBACK_42_003: [ If log_callback is NULL then log_sink_callback_set_callback shall fail and return a non-zero value. ]*/
        (void)printf("Invalid arguments: LOG_SINK_CALLBACK_LOG log_callback=%p, void* context=%p\r\n", log_callback, context);
        result = MU_FAILURE;
    }
    else
    {
        /*Codes_SRS_LOG_SINK_CALLBACK_42_004: [ log_sink_callback_set_callback shall store log_callback and context so that it is used by all future calls to log_sink_callback.log. ]*/
        log_sink_callback_callback = log_callback;
        log_sink_callback_context = context;
        /*Codes_SRS_LOG_SINK_CALLBACK_42_005: [ log_sink_callback_set_callback shall return 0. ]*/
        result = 0;
    }
    return result;
}

void log_sink_callback_set_max_level(LOG_LEVEL log_level)
{
    /*Codes_SRS_LOG_SINK_CALLBACK_42_019: [ log_sink_callback_set_max_level shall store log_level so that it is used by all future calls to log_sink_callback.log. ]*/
    log_sink_callback_max_level = log_level;
}

static void log_sink_callback_log(LOG_LEVEL log_level, LOG_CONTEXT_HANDLE log_context, const char* file, const char* func, int line, const char* message_format, va_list args)
{

    if (message_format == NULL)
    {
        /* Codes_SRS_LOG_SINK_CALLBACK_42_006: [ If message_format is NULL, log_sink_callback.log shall call the log_callback with an error message and return. ]*/
        log_sink_callback_callback(log_sink_callback_context, LOG_LEVEL_CRITICAL, error_string_invalid_args);
    }
    else
    {
        if (log_level > log_sink_callback_max_level)
        {
            /* Codes_SRS_LOG_SINK_CALLBACK_42_020: [ If log_level is greater than the maximum level set by log_sink_callback_set_max_level, then log_sink_callback.log shall return without calling the log_callback. ]*/
        }
        else
        {
            /* Codes_SRS_LOG_SINK_CALLBACK_42_016: [ log_sink_callback.log shall include at most LOG_MAX_MESSAGE_LENGTH characters including the null terminator in the callback argument (the rest of the context shall be truncated). ]*/
            char temp[LOG_MAX_MESSAGE_LENGTH];
            char* buffer = temp;
            size_t buffer_size = sizeof(temp);

            /* Codes_SRS_LOG_SINK_CALLBACK_42_007: [ log_sink_callback.log shall obtain the time by calling time. ]*/
            time_t t = time(NULL);
            /* Codes_SRS_LOG_SINK_CALLBACK_42_008: [ log_sink_callback.log shall write the time to string by calling ctime. ]*/
            /* Codes_SRS_LOG_SINK_CALLBACK_42_010: [ If the call to time fails then log_sink_callback.log shall format the time as NULL. ]*/
            char* ctime_result = (t == (time_t)-1) ? NULL : ctime(&t);

            /* Codes_SRS_LOG_SINK_CALLBACK_42_009: [ log_sink_callback.log shall create a line in the format: Time: {formatted time} File:{file}:{line} Func:{func} {optional context information} {formatted message}. ]*/
            int snprintf_result = snprintf(buffer, buffer_size, "Time:%.24s File:%s:%d Func:%s",
                /* Codes_SRS_LOG_SINK_CALLBACK_42_011: [ If the call to ctime fails then log_sink_callback.log shall format the time as NULL. ]*/
                MU_P_OR_NULL(ctime_result),
                MU_P_OR_NULL(file),
                line,
                MU_P_OR_NULL(func));
            if (snprintf_result < 0)
            {
                /* Codes_SRS_LOG_SINK_CALLBACK_42_017: [ If any encoding error occurs during formatting of the line (i.e. if any printf class functions fails), log_sink_callback.log shall call the log_callback with Error formatting log line and return. ]*/
                log_sink_callback_callback(log_sink_callback_context, LOG_LEVEL_CRITICAL, error_string_sink_callback);
            }
            else
            {
                bool error = false;

                snprintf_result = MIN(snprintf_result, (int)buffer_size);
                buffer += snprintf_result;
                buffer_size -= snprintf_result;

                /* Codes_SRS_LOG_SINK_CALLBACK_42_012: [ If log_context is non-NULL: ]*/
                if (log_context != NULL)
                {
                    /* Codes_SRS_LOG_SINK_CALLBACK_42_013: [ log_sink_callback.log shall call log_context_get_property_value_pair_count to obtain the count of properties. ]*/
                    size_t property_value_pair_count = log_context_get_property_value_pair_count(log_context);
                    /* Codes_SRS_LOG_SINK_CALLBACK_42_014: [ log_sink_callback.log shall call log_context_get_property_value_pairs to obtain the properties. ]*/
                    const LOG_CONTEXT_PROPERTY_VALUE_PAIR* property_value_pairs = log_context_get_property_value_pairs(log_context);

                    /* Codes_SRS_LOG_SINK_CALLBACK_42_015: [ log_sink_callback.log shall call log_context_property_to_string to write the properties to the string buffer. ]*/
                    int log_n_properties_result = log_context_property_to_string(buffer, buffer_size, property_value_pairs, property_value_pair_count); // lgtm[cpp/unguardednullreturndereference] Tests and code review ensure that NULL access cannot happen
                    if (log_n_properties_result < 0)
                    {
                        /* Codes_SRS_LOG_SINK_CALLBACK_42_017: [ If any encoding error occurs during formatting of the line (i.e. if any printf class functions fails), log_sink_callback.log shall call the log_callback with Error formatting log line and return. ]*/
                        error = true;
                    }
                    else
                    {
                        log_n_properties_result = MIN(log_n_properties_result, (int)buffer_size);
                        buffer += log_n_properties_result;
                        buffer_size -= log_n_properties_result;
                    }
                }

                if (!error)
                {
                    if (buffer_size > 1)
                    {
                        *buffer = ' ';
                        buffer++;
                        buffer_size--;

                        int vsnprintf_result = vsnprintf(buffer, buffer_size, message_format, args);
                        if (vsnprintf_result < 0)
                        {
                            /* Codes_SRS_LOG_SINK_CALLBACK_42_017: [ If any encoding error occurs during formatting of the line (i.e. if any printf class functions fails), log_sink_callback.log shall call the log_callback with Error formatting log line and return. ]*/
                            error = true;
                        }
                        else
                        {
                            // all ok
                        }
                    }
                }

                if (error)
                {
                    log_sink_callback_callback(log_sink_callback_context, LOG_LEVEL_CRITICAL, error_string_sink_callback);
                }
                else
                {
                    /* Codes_SRS_LOG_SINK_CALLBACK_42_018: [ log_sink_callback.log shall call log_callback with its context, log_level, and the formatted message. ]*/
                    log_sink_callback_callback(log_sink_callback_context, log_level, temp);
                }
            }
        }
    }
}

const LOG_SINK_IF log_sink_callback =
{
    .init = log_sink_callback_init,
    .deinit = log_sink_callback_deinit,
    .log = log_sink_callback_log
};
