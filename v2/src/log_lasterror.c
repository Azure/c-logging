// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <inttypes.h>

#include "macro_utils/macro_utils.h"

#include "windows.h"

#include "c_logging/log_lasterror.h"

static const char FormatMessageA_failure_message[] = "failure in FormatMessageA";

#define MESSAGE_BUFFER_SIZE 512

static const char message[MESSAGE_BUFFER_SIZE * (MESSAGE_BUFFER_SIZE >= sizeof(FormatMessageA_failure_message))] = { 0 }; /*this construct will generate a compile time error (array of size 0) when LOG_SIZE_REGULAR is not enough to hold even the failure message*/

int log_lasterror_fill_property(void* buffer)
{
    int result;
    if (buffer == NULL)
    {
        result = MESSAGE_BUFFER_SIZE;
    }
    else
    {
        DWORD last_error = GetLastError();

        DWORD chars_written = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, last_error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buffer, MESSAGE_BUFFER_SIZE, NULL);
        if (chars_written == 0)
        {
            (void)memcpy(buffer, FormatMessageA_failure_message, sizeof(FormatMessageA_failure_message));
            result = sizeof(FormatMessageA_failure_message);
        }
        else
        {
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

            result = MESSAGE_BUFFER_SIZE;
        }
    }

    return result;
}
