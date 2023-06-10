// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <inttypes.h>

#include "macro_utils/macro_utils.h"

#include "windows.h"
#include "psapi.h"

#include "c_logging/log_hresult.h"

#define MESSAGE_BUFFER_SIZE 512
#define N_MAX_MODULES 10

int log_hresult_fill_property(void* buffer, HRESULT hresult)
{
    if (buffer == NULL)
    {
        /* Codes_SRS_LOG_HRESULT_01_001: [ If buffer is NULL, log_hresult_fill_property shall return 512 to indicate how many bytes shall be reserved for the HRESULT string formatted version. ] */
    }
    else
    {
        /*see if the "system" can provide the code*/
        /* Codes_SRS_LOG_HRESULT_01_002: [ log_hresult_fill_property shall call FormatMessageA with FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, hresult, 0 as language Id, buffer as buffer to place the output and 512 as buffer size. ] */
        if (FormatMessageA(
            FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL,
            hresult,
            0, /*if you pass in zero, FormatMessage looks for a message for LANGIDs in the following order...*/
            buffer, MESSAGE_BUFFER_SIZE, NULL) != 0)
        {
            /*success, SYSTEM was able to find the message*/
            /*return as is*/
        }
        else
        {
            HANDLE currentProcess = GetCurrentProcess();
            /*apparently this cannot fail and returns somewhat of a "pseudo handle"*/

            HMODULE module_handles[N_MAX_MODULES];
            DWORD enumModulesUsedBytes;
            if (EnumProcessModules(currentProcess, module_handles, sizeof(module_handles), &enumModulesUsedBytes) == 0)
            {
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
                        module_handles[iModule],
                        hresult,
                        0,
                        buffer, MESSAGE_BUFFER_SIZE, NULL) != 0)
                    {
                        break;
                    }
                    else
                    {
                        /*this module does not have it...*/
                    }
                }

                if (iModule == (enumModulesUsedBytes / sizeof(HMODULE)))
                {
                    (void)snprintf(buffer, MESSAGE_BUFFER_SIZE, "unknown HRESULT 0x%x", hresult);
                }
                else
                {
                    // all good
                }
            }
        }
    }

    /* Codes_SRS_LOG_HRESULT_01_003: [ log_lasterror_fill_property shall return 512. ] */
    return MESSAGE_BUFFER_SIZE;
}
