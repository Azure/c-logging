// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <inttypes.h>

#include "macro_utils/macro_utils.h"

#include "windows.h"

#include "c_logging/log_hresult.h"

#define MESSAGE_BUFFER_SIZE 512
#define N_MAX_MODULES 10

int log_hresult_fill_property(void* buffer, ...)
{
    int result;
    va_list args;
    va_start(args, buffer);

    HRESULT hr = va_arg(args, HRESULT);
    va_end(args);

    /*Codes_SRS_HRESULT_TO_STRING_02_002: [ log_hresult_fill_property shall call FormatMessageA with dwFlags set to FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS. ]*/
    /*see if the "system" can provide the code*/
    if (FormatMessageA(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        hr,
        0, /*if you pass in zero, FormatMessage looks for a message for LANGIDs in the following order...*/
        buffer, MESSAGE_BUFFER_SIZE, NULL) != 0)
    {
        /*Codes_SRS_HRESULT_TO_STRING_02_003: [ If FormatMessageA succeeds then hresult_to_string shall return the value as given by FormatMessageA. ]*/
        /*success, SYSTEM was able to find the message*/
        /*return as is*/
    }
    else
    {
        HANDLE currentProcess = GetCurrentProcess();
        /*apparently this cannot fail and returns somewhat of a "pseudo handle"*/

        HMODULE hModules[N_MAX_MODULES];
        DWORD enumModulesUsedBytes;
        if (EnumProcessModules(currentProcess, hModules, sizeof(hModules), &enumModulesUsedBytes) == 0)
        {
            /*Codes_SRS_HRESULT_TO_STRING_02_008: [ If there are any failures then hresult_to_string shall return NULL. ]*/
            (void)snprintf(buffer, MESSAGE_BUFFER_SIZE, "failure in EnumProcessModules, LE=%lu", GetLastError());
        }
        else
        {
            size_t iModule;
            for (iModule = 0; iModule < (enumModulesUsedBytes / sizeof(HMODULE)); iModule++)
            {
                /*see if this module */
                if (FormatMessageA(
                    FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS,
                    hModules[iModule],
                    hr,
                    0,
                    (LPVOID)result, MESSAGE_BUFFER_SIZE, NULL) != 0)
                {
                    /*Codes_SRS_HRESULT_TO_STRING_02_006: [ If a module can decode hresult then that value shall be returned. */
                    break;
                }
                else
                {
                    /*this module does not have it...*/
                }
            }

            /*Codes_SRS_HRESULT_TO_STRING_02_007: [ Otherwise NULL shall be returned. ]*/
            if (iModule == (enumModulesUsedBytes / sizeof(HMODULE)))
            {
                (void)snprintf(buffer, MESSAGE_BUFFER_SIZE, "unknown HRESULT 0x%x", hr);
            }
            else
            {
                // all good
            }
        }
    }
    return result;
}
