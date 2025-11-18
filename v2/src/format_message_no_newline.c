// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdarg.h>

#include "windows.h"

#include "c_logging/format_message_no_newline.h"

DWORD FormatMessageA_no_newline(DWORD dwFlags, LPCVOID lpSource, DWORD dwMessageId, DWORD dwLanguageId, LPSTR lpBuffer, DWORD nSize, va_list* Arguments)
{
    /* Codes_SRS_FORMATMESSAGE_NO_NEWLINE_01_001: [ FormatMessageA_no_newline shall call FormatMessageA with the same arguments. ] */
    DWORD result = FormatMessageA(dwFlags, lpSource, dwMessageId, dwLanguageId, lpBuffer, nSize, Arguments);

    if (result == 0)
    {
        /* Codes_SRS_FORMATMESSAGE_NO_NEWLINE_01_002: [ If FormatMessageA returns 0, FormatMessageA_no_newline shall return 0. ] */
        // return as is
    }
    else
    {
        /* Codes_SRS_FORMATMESSAGE_NO_NEWLINE_01_003: [ Otherwise, FormatMessageA_no_newline shall remove any r or n characters that have been placed at the end of the formatted output by FormatMessageA and return the number of CHARs left in lpBuffer. ] */
        char* where_is_last_char = (char*)lpBuffer + result - 1;
        while (where_is_last_char >= (char*)lpBuffer)
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

        result = (DWORD)(where_is_last_char - lpBuffer);
    }

    return result;
}
