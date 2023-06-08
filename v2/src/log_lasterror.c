// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <inttypes.h>

#include "macro_utils/macro_utils.h"

#include "windows.h"

#include "c_logging/log_lasterror.h"

static const char FormatMessageA_failure_message[] = "failure in FormatMessageA";

#define MESSAGE_BUFFER_SIZE 512

// This array is not used for anything, but rather just to emit a compiler error
static const char test_message_to_trigger_a_compiler_error[1 * (MESSAGE_BUFFER_SIZE >= sizeof(FormatMessageA_failure_message))] = { 0 }; /*this construct will generate a compile time error (array of size 0) when LOG_SIZE_REGULAR is not enough to hold even the failure message*/

int log_lasterror_fill_property(void* buffer)
{
    if (buffer == NULL)
    {
        /* Codes_SRS_LOG_LASTERROR_01_002: [ If buffer is NULL, log_lasterror_fill_property shall return 512 to indicate how many bytes shall be reserved for the last error string formatted version. ] */
    }
    else
    {
        /* Codes_SRS_LOG_LASTERROR_01_003: [ Otherwise, log_lasterror_fill_property shall call GetLastError to obtain the last error information. ] */
        DWORD last_error = GetLastError();

        /* Codes_SRS_LOG_LASTERROR_01_004: [ log_lasterror_fill_property shall call FormatMessageA with FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, the last error value, LANG_NEUTRAL as language Id, buffer as buffer to place the output and 512 as buffer size. ] */
        DWORD chars_written = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, last_error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buffer, MESSAGE_BUFFER_SIZE, NULL);
        if (chars_written == 0)
        {
            /* Codes_SRS_LOG_LASTERROR_01_005: [ If FormatMessageA returns 0, log_lasterror_fill_property shall copy in buffer the string failure in FormatMessageA and return 512. ] */
            (void)memcpy(buffer, FormatMessageA_failure_message, sizeof(FormatMessageA_failure_message));
        }
        else
        {
            /* Codes_SRS_LOG_LASTERROR_01_006: [ Otherwise, log_lasterror_fill_property shall remove any \r or \n characters that have been placed at the end of the formatted output by FormatMessageA. ] */
            char* where_is_last_char = (char*)buffer + chars_written - 1;
            while (where_is_last_char >= (char*)buffer)
            {
                if (
                    (*where_is_last_char != '\r') &&
                    (*where_is_last_char != '\n')
                    )
                {
                    break;
                }

                where_is_last_char--;
            }

            // get the next character, as we go backwards by design of the loop one extra character
            where_is_last_char++;
            *where_is_last_char = '\0';

            /* Codes_SRS_LOG_LASTERROR_01_007: [ log_lasterror_fill_property shall return 512. ] */
        }
    }

    return MESSAGE_BUFFER_SIZE;
}
