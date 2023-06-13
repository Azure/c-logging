// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <inttypes.h>

#include "macro_utils/macro_utils.h"

#include "c_logging/log_errno.h"

#define MESSAGE_BUFFER_SIZE 512

static const char strerror_s_failure_message[] = "strerror_s failed";

// This array is not used for anything, but rather just to emit a compiler error
static const char test_message_to_trigger_a_compiler_error[1 * (MESSAGE_BUFFER_SIZE >= sizeof(strerror_s_failure_message))] = { 0 }; /*this construct will generate a compile time error (array of size 0) when LOG_SIZE_REGULAR is not enough to hold even the failure message*/

int log_errno_fill_property(void* buffer, int dummy)
{
    (void)dummy;

    if (buffer == NULL)
    {
    }
    else
    {
        if (strerror_s(buffer, MESSAGE_BUFFER_SIZE, errno) != 0)
        {
            (void)memcpy(buffer, strerror_s_failure_message, sizeof(strerror_s_failure_message));
        }
        else
        {
            // return
        }
    }

    return MESSAGE_BUFFER_SIZE;
}
