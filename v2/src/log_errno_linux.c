// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <errno.h>
#include <string.h>

#include "c_logging/log_errno.h"

#define MESSAGE_BUFFER_SIZE 512

static const char strerror_r_failure_message[] = "failure in strerror_r";

// This array is not used for anything, but rather just to emit a compiler error
static const char test_message_to_trigger_a_compiler_error[1 * (MESSAGE_BUFFER_SIZE >= sizeof(strerror_r_failure_message))] = { 0 }; /*this construct will generate a compile time error (array of size 0) when MESSAGE_BUFFER_SIZE is not enough to hold even the failure message*/

// Function is to avoid Warning -Werror=unused-variable in gcc
static void unused_var_function(void)
{
    (void)test_message_to_trigger_a_compiler_error;
}

int log_errno_fill_property(void* buffer, int dummy)
{
    (void)dummy;

    if (buffer == NULL)
    {
        /* Codes_SRS_LOG_ERRNO_LINUX_01_002: [ If buffer is NULL, log_errno_fill_property shall return 512 to indicate how many bytes shall be reserved for the string formatted errno. ] */
        // do nothing
    }
    else
    {
        /* Codes_SRS_LOG_ERRNO_LINUX_01_003: [ Otherwise, log_errno_fill_property shall obtain the errno value. ] */
        /* Codes_SRS_LOG_ERRNO_LINUX_01_004: [ log_errno_fill_property shall call strerror_s with buffer, 512 and the errno value. ] */
        if (strerror_r(errno, buffer, MESSAGE_BUFFER_SIZE) != 0)
        {
            /* Codes_SRS_LOG_ERRNO_LINUX_01_005: [ If strerror_s fails, log_errno_fill_property shall copy in buffer the string failure in strerror_s and return 512. ] */
            (void)memcpy(buffer, strerror_r_failure_message, sizeof(strerror_r_failure_message));
        }
        else
        {
            // return
        }
    }

    /* Codes_SRS_LOG_ERRNO_LINUX_01_007: [ log_errno_fill_property shall return 512. ] */
    return MESSAGE_BUFFER_SIZE;
}
