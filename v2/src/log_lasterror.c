// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <inttypes.h>

#include "windows.h"

#include "macro_utils/macro_utils.h"

#include "c_logging/format_message_no_newline.h"

#include "c_logging/log_lasterror.h"

static const char FormatMessageA_failure_message[] = "failure in FormatMessageA";

#define MESSAGE_BUFFER_SIZE 512

// This array is not used for anything, but rather just to emit a compiler error
static const char test_message_to_trigger_a_compiler_error[1 * (MESSAGE_BUFFER_SIZE >= sizeof(FormatMessageA_failure_message))] = { 0 }; /*this construct will generate a compile time error (array of size 0) when MESSAGE_BUFFER_SIZE is not enough to hold even the failure message*/

int log_lasterror_fill_property(void* buffer, int dummy)
{
    (void)dummy;

    if (buffer == NULL)
    {
        /* Codes_SRS_LOG_LASTERROR_01_002: [ If buffer is NULL, log_lasterror_fill_property shall return 512 to indicate how many bytes shall be reserved for the last error string formatted version. ] */
    }
    else
    {
        /* Codes_SRS_LOG_LASTERROR_01_003: [ Otherwise, log_lasterror_fill_property shall call GetLastError to obtain the last error information. ] */
        DWORD last_error = GetLastError();

        /* Codes_SRS_LOG_LASTERROR_01_004: [ log_lasterror_fill_property shall call FormatMessageA_no_newline with FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, the last error value, LANG_NEUTRAL as language Id, buffer as buffer to place the output and 512 as buffer size. ] */
        DWORD chars_written = FormatMessageA_no_newline(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, last_error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buffer, MESSAGE_BUFFER_SIZE, NULL);
        if (chars_written == 0)
        {
            /* Codes_SRS_LOG_LASTERROR_01_005: [ If FormatMessageA returns 0, log_lasterror_fill_property shall copy in buffer the string failure in FormatMessageA and return 512. ] */
            (void)memcpy(buffer, FormatMessageA_failure_message, sizeof(FormatMessageA_failure_message));
        }
        else
        {
            // return
        }
    }

    /* Codes_SRS_LOG_LASTERROR_01_007: [ log_lasterror_fill_property shall return 512. ] */
    return MESSAGE_BUFFER_SIZE;
}
