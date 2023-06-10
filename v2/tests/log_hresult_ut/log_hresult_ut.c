// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "windows.h"
#include "psapi.h"

#include "macro_utils/macro_utils.h"

#include "c_logging/log_context.h"

#include "c_logging/log_hresult.h"

// defines how many mock calls we can have
#define MAX_MOCK_CALL_COUNT (128)

#define MOCK_CALL_TYPE_VALUES \
    MOCK_CALL_TYPE_GetLastError, \
    MOCK_CALL_TYPE_FormatMessageA_no_newline, \
    MOCK_CALL_TYPE_snprintf, \
    MOCK_CALL_TYPE_GetCurrentProcess, \
    MOCK_CALL_TYPE_EnumProcessModules

MU_DEFINE_ENUM(MOCK_CALL_TYPE, MOCK_CALL_TYPE_VALUES)

// very poor mans mocks :-(

typedef struct GetLastError_CALL_TAG
{
    bool override_result;
    int call_result;
} GetLastError_CALL;

typedef struct FormatMessageA_no_newline_CALL_TAG
{
    bool override_result;
    int call_result;
    DWORD captured_dwFlags;
    LPCVOID captured_lpSource;
    DWORD captured_dwMessageId;
    DWORD captured_dwLanguageId;
    LPSTR captured_lpBuffer;
    DWORD captured_nSize;
    va_list* captured_Arguments;
    const char* buffer_payload;
} FormatMessageA_no_newline_CALL;

typedef struct snprintf_CALL_TAG
{
    bool override_result;
    int call_result;
    const char* captured_format_arg;
} snprintf_CALL;

typedef struct GetCurrentProcess_CALL_TAG
{
    bool override_result;
    HANDLE call_result;
} GetCurrentProcess_CALL;

typedef struct EnumProcessModules_CALL_TAG
{
    bool override_result;
    BOOL call_result;
    DWORD returned_filled_bytes;
    const HMODULE* returned_lphModule;
    HANDLE captured_hProcess;
    DWORD captured_cb;
} EnumProcessModules_CALL;

typedef struct MOCK_CALL_TAG
{
    MOCK_CALL_TYPE mock_call_type;
    union
    {
        GetLastError_CALL GetLastError_call;
        FormatMessageA_no_newline_CALL FormatMessageA_no_newline_call;
        snprintf_CALL snprintf_call;
        GetCurrentProcess_CALL GetCurrentProcess_call;
        EnumProcessModules_CALL EnumProcessModules_call;
    };
} MOCK_CALL;

static MOCK_CALL expected_calls[MAX_MOCK_CALL_COUNT];
static size_t expected_call_count;
static size_t actual_call_count;
static bool actual_and_expected_match;

const char TEST_FORMATTED_HRESULT_S_OK[] = "A dummy S_OK";
const char TEST_FORMATTED_HRESULT_E_FAIL[] = "A dummy E_FAIL";
const char TEST_FORMATTED_HRESULT_E_FAIL_OTHER[] = "Another dummy E_FAIL";

#define POOR_MANS_ASSERT(cond) \
    if (!(cond)) \
    { \
        (void)printf("%s:%d test failed\r\n", __FUNCTION__, __LINE__); \
        abort(); \
    } \

static void setup_mocks(void)
{
    expected_call_count = 0;
    actual_call_count = 0;
    actual_and_expected_match = true;
}

DWORD mock_GetLastError(void)
{
    DWORD result;
    
    if ((actual_call_count == expected_call_count) ||
        (expected_calls[actual_call_count].mock_call_type != MOCK_CALL_TYPE_GetLastError))
    {
        actual_and_expected_match = false;
        result = MU_FAILURE;
    }
    else
    {
        if (expected_calls[actual_call_count].GetLastError_call.override_result)
        {
            result = expected_calls[actual_call_count].GetLastError_call.call_result;
        }
        else
        {
            result = 0;
        }
    
        actual_call_count++;
    }
    
    return result;
}

DWORD mock_FormatMessageA_no_newline(DWORD dwFlags, LPCVOID lpSource, DWORD dwMessageId, DWORD dwLanguageId, LPSTR lpBuffer, DWORD nSize, va_list* Arguments)
{
    DWORD result;

    if ((actual_call_count == expected_call_count) ||
        (expected_calls[actual_call_count].mock_call_type != MOCK_CALL_TYPE_FormatMessageA_no_newline))
    {
        actual_and_expected_match = false;
        result = MU_FAILURE;
    }
    else
    {
        expected_calls[actual_call_count].FormatMessageA_no_newline_call.captured_dwFlags = dwFlags;
        expected_calls[actual_call_count].FormatMessageA_no_newline_call.captured_lpSource = lpSource;
        expected_calls[actual_call_count].FormatMessageA_no_newline_call.captured_dwLanguageId = dwLanguageId;
        expected_calls[actual_call_count].FormatMessageA_no_newline_call.captured_lpBuffer = lpBuffer;
        expected_calls[actual_call_count].FormatMessageA_no_newline_call.captured_nSize = nSize;
        expected_calls[actual_call_count].FormatMessageA_no_newline_call.captured_Arguments = Arguments;
        expected_calls[actual_call_count].FormatMessageA_no_newline_call.captured_dwMessageId = dwMessageId;

        if (expected_calls[actual_call_count].FormatMessageA_no_newline_call.override_result)
        {
            if (expected_calls[actual_call_count].FormatMessageA_no_newline_call.buffer_payload != NULL)
            {
                (void)memcpy(lpBuffer, expected_calls[actual_call_count].FormatMessageA_no_newline_call.buffer_payload, expected_calls[actual_call_count].FormatMessageA_no_newline_call.call_result + 1);
            }
            result = expected_calls[actual_call_count].FormatMessageA_no_newline_call.call_result;
        }
        else
        {
            result = FormatMessageA(dwFlags, lpSource, dwMessageId, dwLanguageId, lpBuffer, nSize, Arguments);
        }

        actual_call_count++;
    }

    return result;
}

int mock_snprintf(char* s, size_t n, const char* format, ...)
{
    int result;

    if ((actual_call_count == expected_call_count) ||
        (expected_calls[actual_call_count].mock_call_type != MOCK_CALL_TYPE_snprintf))
    {
        actual_and_expected_match = false;
        return -1;
    }
    else
    {
        if (expected_calls[actual_call_count].snprintf_call.override_result)
        {
            result = expected_calls[actual_call_count].snprintf_call.call_result;
        }
        else
        {
            va_list args;
            va_start(args, format);

            expected_calls[actual_call_count].snprintf_call.captured_format_arg = format;

            result = vsnprintf(s, n, format, args);

            va_end(args);
        }

        actual_call_count++;
    }

    return result;
}

HANDLE mock_GetCurrentProcess(void)
{
    HANDLE result;

    if ((actual_call_count == expected_call_count) ||
        (expected_calls[actual_call_count].mock_call_type != MOCK_CALL_TYPE_GetCurrentProcess))
    {
        actual_and_expected_match = false;
        result = NULL;
    }
    else
    {
        if (expected_calls[actual_call_count].GetCurrentProcess_call.override_result)
        {
            result = expected_calls[actual_call_count].GetCurrentProcess_call.call_result;
        }
        else
        {
            result = GetCurrentProcess();
        }

        actual_call_count++;
    }

    return result;
}

BOOL mock_EnumProcessModules(HANDLE hProcess, HMODULE* lphModule, DWORD cb, LPDWORD lpcbNeeded)
{
    BOOL result;

    if ((actual_call_count == expected_call_count) ||
        (expected_calls[actual_call_count].mock_call_type != MOCK_CALL_TYPE_EnumProcessModules))
    {
        actual_and_expected_match = false;
        result = FALSE;
    }
    else
    {
        expected_calls[actual_call_count].EnumProcessModules_call.captured_hProcess = hProcess;
        expected_calls[actual_call_count].EnumProcessModules_call.captured_cb = cb;

        if (expected_calls[actual_call_count].EnumProcessModules_call.override_result)
        {
            *lpcbNeeded = expected_calls[actual_call_count].EnumProcessModules_call.returned_filled_bytes;
            (void)memcpy(lphModule, expected_calls[actual_call_count].EnumProcessModules_call.returned_lphModule, expected_calls[actual_call_count].EnumProcessModules_call.returned_filled_bytes);
            result = expected_calls[actual_call_count].EnumProcessModules_call.call_result;
        }
        else
        {
            result = EnumProcessModules(hProcess, lphModule, cb, lpcbNeeded);
        }

        actual_call_count++;
    }

    return result;
}

static void setup_GetLastError_call(void)
{
    expected_calls[expected_call_count].mock_call_type = MOCK_CALL_TYPE_GetLastError;
    expected_calls[expected_call_count].GetLastError_call.override_result = false;
    expected_call_count++;
}

static void setup_FormatMessageA_no_newline_call(void)
{
    expected_calls[expected_call_count].mock_call_type = MOCK_CALL_TYPE_FormatMessageA_no_newline;
    expected_calls[expected_call_count].FormatMessageA_no_newline_call.override_result = false;
    expected_calls[expected_call_count].FormatMessageA_no_newline_call.buffer_payload = NULL;
    expected_call_count++;
}

static void setup_snprintf_call(void)
{
    expected_calls[expected_call_count].mock_call_type = MOCK_CALL_TYPE_snprintf;
    expected_calls[expected_call_count].snprintf_call.override_result = false;
    expected_call_count++;
}

static void setup_GetCurrentProcess_call(void)
{
    expected_calls[expected_call_count].mock_call_type = MOCK_CALL_TYPE_GetCurrentProcess;
    expected_calls[expected_call_count].GetCurrentProcess_call.override_result = false;
    expected_call_count++;
}

static void setup_EnumProcessModules_call(void)
{
    expected_calls[expected_call_count].mock_call_type = MOCK_CALL_TYPE_EnumProcessModules;
    expected_calls[expected_call_count].EnumProcessModules_call.override_result = false;
    expected_call_count++;
}

/* log_hresult_fill_property */

/* Tests_SRS_LOG_HRESULT_01_001: [ If buffer is NULL, log_hresult_fill_property shall return 512 to indicate how many bytes shall be reserved for the HRESULT string formatted version. ] */
static void log_hresult_fill_property_with_NULL_returns_needed_buffer_size(void)
{
    // arrange
    setup_mocks();

    // act
    int result = log_hresult_fill_property(NULL, S_OK);

    // assert
    POOR_MANS_ASSERT(result == 512);
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_HRESULT_01_002: [ log_hresult_fill_property shall call FormatMessageA_no_newline with FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, hresult, 0 as language Id, buffer as buffer to place the output and 512 as buffer size. ] */
/* Tests_SRS_LOG_HRESULT_01_003: [ log_lasterror_fill_property shall return 512. ] */
static void log_hresult_fill_property_with_non_NULL_formats_S_OK(void)
{
    // arrange
    char buffer[512];

    setup_mocks();
    setup_FormatMessageA_no_newline_call();
    expected_calls[0].FormatMessageA_no_newline_call.override_result = true;
    expected_calls[0].FormatMessageA_no_newline_call.call_result = sizeof(TEST_FORMATTED_HRESULT_S_OK) - 1;
    expected_calls[0].FormatMessageA_no_newline_call.buffer_payload = TEST_FORMATTED_HRESULT_S_OK;

    // act
    int result = log_hresult_fill_property(buffer, S_OK);

    // assert
    POOR_MANS_ASSERT(result == 512);
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_no_newline_call.captured_dwFlags == (FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS));
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_no_newline_call.captured_lpSource == NULL);
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_no_newline_call.captured_dwMessageId == S_OK);
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_no_newline_call.captured_dwLanguageId == 0);
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_no_newline_call.captured_lpBuffer == buffer);
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_no_newline_call.captured_nSize == 512);
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_no_newline_call.captured_Arguments == NULL);
    POOR_MANS_ASSERT(strcmp(buffer, TEST_FORMATTED_HRESULT_S_OK) == 0);
}

/* Tests_SRS_LOG_HRESULT_01_002: [ log_hresult_fill_property shall call FormatMessageA_no_newline with FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, hresult, 0 as language Id, buffer as buffer to place the output and 512 as buffer size. ] */
/* Tests_SRS_LOG_HRESULT_01_003: [ log_lasterror_fill_property shall return 512. ] */
static void log_hresult_fill_property_with_non_NULL_formats_E_FAIL(void)
{
    // arrange
    char buffer[512];

    setup_mocks();
    setup_FormatMessageA_no_newline_call();
    expected_calls[0].FormatMessageA_no_newline_call.override_result = true;
    expected_calls[0].FormatMessageA_no_newline_call.call_result = sizeof(TEST_FORMATTED_HRESULT_E_FAIL) - 1;
    expected_calls[0].FormatMessageA_no_newline_call.buffer_payload = TEST_FORMATTED_HRESULT_E_FAIL;

    // act
    int result = log_hresult_fill_property(buffer, E_FAIL);

    // assert
    POOR_MANS_ASSERT(result == 512);
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_no_newline_call.captured_dwFlags == (FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS));
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_no_newline_call.captured_lpSource == NULL);
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_no_newline_call.captured_dwMessageId == E_FAIL);
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_no_newline_call.captured_dwLanguageId == 0);
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_no_newline_call.captured_lpBuffer == buffer);
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_no_newline_call.captured_nSize == 512);
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_no_newline_call.captured_Arguments == NULL);
    POOR_MANS_ASSERT(strcmp(buffer, TEST_FORMATTED_HRESULT_E_FAIL) == 0);
}

/* Tests_SRS_LOG_HRESULT_01_004: [ If FormatMessageA_no_newline returns 0, log_hresult_fill_property attempt to look up the formatted string from the loaded modules: ] */
/* Tests_SRS_LOG_HRESULT_01_005: [ log_hresult_fill_property shall get the current process handle by calling GetCurrentProcess. ] */
/* Tests_SRS_LOG_HRESULT_01_006: [ log_hresult_fill_property shall call EnumProcessModules and obtain the information about 10 modules. ] */
/* Tests_SRS_LOG_HRESULT_01_007: [ If no module has the formatted message, log_hresult_fill_property shall place in buffer the string unknown HRESULT 0x%x, where %x is the hresult value. ] */
static void when_FormatMessageA_fails_and_no_modules_are_returned_log_hresult_fill_property_places_unknown_hresult_in_buffer(void)
{
    // arrange
    char buffer[512];

    setup_mocks();
    setup_FormatMessageA_no_newline_call();
    setup_GetCurrentProcess_call();
    setup_EnumProcessModules_call();
    setup_snprintf_call();
    
    expected_calls[0].FormatMessageA_no_newline_call.override_result = true;
    expected_calls[0].FormatMessageA_no_newline_call.call_result = 0;
    expected_calls[1].GetCurrentProcess_call.override_result = true;
    expected_calls[1].GetCurrentProcess_call.call_result = (HANDLE)0x4242;
    expected_calls[2].EnumProcessModules_call.override_result = true;
    expected_calls[2].EnumProcessModules_call.returned_filled_bytes = 0;
    expected_calls[2].EnumProcessModules_call.call_result = TRUE;

    // act
    int result = log_hresult_fill_property(buffer, E_FAIL);

    // assert
    POOR_MANS_ASSERT(result == 512);
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_no_newline_call.captured_dwFlags == (FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS));
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_no_newline_call.captured_lpSource == NULL);
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_no_newline_call.captured_dwMessageId == E_FAIL);
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_no_newline_call.captured_dwLanguageId == 0);
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_no_newline_call.captured_lpBuffer == buffer);
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_no_newline_call.captured_nSize == 512);
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_no_newline_call.captured_Arguments == NULL);
    POOR_MANS_ASSERT(expected_calls[2].EnumProcessModules_call.captured_hProcess == (HANDLE)0x4242);
    POOR_MANS_ASSERT(expected_calls[2].EnumProcessModules_call.captured_cb == (sizeof(HMODULE) * 10));
    POOR_MANS_ASSERT(strcmp(buffer, "unknown HRESULT 0x80004005") == 0);
}

/* Tests_SRS_LOG_HRESULT_01_008: [ If printing the unknown HRESULT 0x%x string fails, log_hresult_fill_property shall place in buffer the string snprintf failed. ] */
static void when_snprintf_fails_log_hresult_fill_property_places_snprintf_failed_in_the_buffer(void)
{
    // arrange
    char buffer[512];

    setup_mocks();
    setup_FormatMessageA_no_newline_call();
    setup_GetCurrentProcess_call();
    setup_EnumProcessModules_call();
    setup_snprintf_call();

    expected_calls[0].FormatMessageA_no_newline_call.override_result = true;
    expected_calls[0].FormatMessageA_no_newline_call.call_result = 0;
    expected_calls[1].GetCurrentProcess_call.override_result = true;
    expected_calls[1].GetCurrentProcess_call.call_result = (HANDLE)0x4242;
    expected_calls[2].EnumProcessModules_call.override_result = true;
    expected_calls[2].EnumProcessModules_call.returned_filled_bytes = 0;
    expected_calls[2].EnumProcessModules_call.call_result = TRUE;
    expected_calls[3].snprintf_call.override_result = true;
    expected_calls[3].snprintf_call.call_result = -1;

    // act
    int result = log_hresult_fill_property(buffer, E_FAIL);

    // assert
    POOR_MANS_ASSERT(result == 512);
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_no_newline_call.captured_dwFlags == (FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS));
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_no_newline_call.captured_lpSource == NULL);
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_no_newline_call.captured_dwMessageId == E_FAIL);
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_no_newline_call.captured_dwLanguageId == 0);
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_no_newline_call.captured_lpBuffer == buffer);
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_no_newline_call.captured_nSize == 512);
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_no_newline_call.captured_Arguments == NULL);
    POOR_MANS_ASSERT(expected_calls[2].EnumProcessModules_call.captured_hProcess == (HANDLE)0x4242);
    POOR_MANS_ASSERT(expected_calls[2].EnumProcessModules_call.captured_cb == (sizeof(HMODULE) * 10));
    POOR_MANS_ASSERT(strcmp(buffer, "snprintf failed") == 0);
}

/* Tests_SRS_LOG_HRESULT_01_009: [ If EnumProcessModules fails, log_hresult_fill_property shall place in buffer a string indicating what failed, the last error and unknown HRESULT 0x%x, where %x is the hresult value. ] */
static void when_EnumProcessModules_fails_log_hresult_fill_property_places_unknown_hresult_with_lasterror_in_buffer(void)
{
    // arrange
    char buffer[512];

    setup_mocks();
    setup_FormatMessageA_no_newline_call();
    setup_GetCurrentProcess_call();
    setup_EnumProcessModules_call();
    setup_GetLastError_call();
    setup_snprintf_call();

    expected_calls[0].FormatMessageA_no_newline_call.override_result = true;
    expected_calls[0].FormatMessageA_no_newline_call.call_result = 0;
    expected_calls[1].GetCurrentProcess_call.override_result = true;
    expected_calls[1].GetCurrentProcess_call.call_result = (HANDLE)0x4242;
    expected_calls[2].EnumProcessModules_call.override_result = true;
    expected_calls[2].EnumProcessModules_call.call_result = FALSE;
    expected_calls[3].GetLastError_call.override_result = true;
    expected_calls[3].GetLastError_call.call_result = 42;

    // act
    int result = log_hresult_fill_property(buffer, E_FAIL);

    // assert
    POOR_MANS_ASSERT(result == 512);
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_no_newline_call.captured_dwFlags == (FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS));
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_no_newline_call.captured_lpSource == NULL);
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_no_newline_call.captured_dwMessageId == E_FAIL);
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_no_newline_call.captured_dwLanguageId == 0);
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_no_newline_call.captured_lpBuffer == buffer);
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_no_newline_call.captured_nSize == 512);
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_no_newline_call.captured_Arguments == NULL);
    POOR_MANS_ASSERT(expected_calls[2].EnumProcessModules_call.captured_hProcess == (HANDLE)0x4242);
    POOR_MANS_ASSERT(expected_calls[2].EnumProcessModules_call.captured_cb == (sizeof(HMODULE) * 10));
    POOR_MANS_ASSERT(strcmp(buffer, "failure in EnumProcessModules, LE=42, unknown HRESULT 0x80004005") == 0);
}

/* Tests_SRS_LOG_HRESULT_01_008: [ If printing the unknown HRESULT 0x%x string fails, log_hresult_fill_property shall place in buffer the string snprintf failed. ] */
static void when_snprintf_fails_after_EnumProcessModules_fails_log_hresult_fill_property_places_snprintf_failed_in_the_buffer(void)
{
    // arrange
    char buffer[512];

    setup_mocks();
    setup_FormatMessageA_no_newline_call();
    setup_GetCurrentProcess_call();
    setup_EnumProcessModules_call();
    setup_GetLastError_call();
    setup_snprintf_call();

    expected_calls[0].FormatMessageA_no_newline_call.override_result = true;
    expected_calls[0].FormatMessageA_no_newline_call.call_result = 0;
    expected_calls[1].GetCurrentProcess_call.override_result = true;
    expected_calls[1].GetCurrentProcess_call.call_result = (HANDLE)0x4242;
    expected_calls[2].EnumProcessModules_call.override_result = true;
    expected_calls[2].EnumProcessModules_call.call_result = FALSE;
    expected_calls[4].snprintf_call.override_result = true;
    expected_calls[4].snprintf_call.call_result = -1;

    // act
    int result = log_hresult_fill_property(buffer, E_FAIL);

    // assert
    POOR_MANS_ASSERT(result == 512);
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_no_newline_call.captured_dwFlags == (FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS));
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_no_newline_call.captured_lpSource == NULL);
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_no_newline_call.captured_dwMessageId == E_FAIL);
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_no_newline_call.captured_dwLanguageId == 0);
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_no_newline_call.captured_lpBuffer == buffer);
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_no_newline_call.captured_nSize == 512);
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_no_newline_call.captured_Arguments == NULL);
    POOR_MANS_ASSERT(expected_calls[2].EnumProcessModules_call.captured_hProcess == (HANDLE)0x4242);
    POOR_MANS_ASSERT(expected_calls[2].EnumProcessModules_call.captured_cb == (sizeof(HMODULE) * 10));
    POOR_MANS_ASSERT(strcmp(buffer, "snprintf failed") == 0);
}

#define TEST_MODULE_1 ((HANDLE)0x45)
#define TEST_MODULE_2 ((HANDLE)0x46)

/* Tests_SRS_LOG_HRESULT_01_010: [ For each module: ] */
/* Tests_SRS_LOG_HRESULT_01_011: [ log_hresult_fill_property shall call FormatMessageA with FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS, the module handle, the hresult value, 0 as language Id, buffer as buffer to place the output and 512 as buffer size. ] */
static void when_first_module_out_of_2_FormatMessageA_no_newline_formats_log_hresult_fill_property_formats_success(void)
{
    // arrange
    char buffer[512];
    HMODULE test_modules[] = { TEST_MODULE_1, TEST_MODULE_2 };

    setup_mocks();
    setup_FormatMessageA_no_newline_call();
    setup_GetCurrentProcess_call();
    setup_EnumProcessModules_call();
    setup_FormatMessageA_no_newline_call();

    expected_calls[0].FormatMessageA_no_newline_call.override_result = true;
    expected_calls[0].FormatMessageA_no_newline_call.call_result = 0;
    expected_calls[1].GetCurrentProcess_call.override_result = true;
    expected_calls[1].GetCurrentProcess_call.call_result = (HANDLE)0x4242;
    expected_calls[2].EnumProcessModules_call.override_result = true;
    expected_calls[2].EnumProcessModules_call.returned_filled_bytes = sizeof(HMODULE) * 2;
    expected_calls[2].EnumProcessModules_call.returned_lphModule = test_modules;
    expected_calls[2].EnumProcessModules_call.call_result = TRUE;
    expected_calls[3].FormatMessageA_no_newline_call.override_result = true;
    expected_calls[3].FormatMessageA_no_newline_call.call_result = sizeof(TEST_FORMATTED_HRESULT_E_FAIL);
    expected_calls[3].FormatMessageA_no_newline_call.buffer_payload = TEST_FORMATTED_HRESULT_E_FAIL;

    // act
    int result = log_hresult_fill_property(buffer, E_FAIL);

    // assert
    POOR_MANS_ASSERT(result == 512);
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_no_newline_call.captured_dwFlags == (FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS));
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_no_newline_call.captured_lpSource == NULL);
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_no_newline_call.captured_dwMessageId == E_FAIL);
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_no_newline_call.captured_dwLanguageId == 0);
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_no_newline_call.captured_lpBuffer == buffer);
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_no_newline_call.captured_nSize == 512);
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_no_newline_call.captured_Arguments == NULL);
    POOR_MANS_ASSERT(expected_calls[2].EnumProcessModules_call.captured_hProcess == (HANDLE)0x4242);
    POOR_MANS_ASSERT(expected_calls[2].EnumProcessModules_call.captured_cb == (sizeof(HMODULE) * 10));
    POOR_MANS_ASSERT(expected_calls[3].FormatMessageA_no_newline_call.captured_dwFlags == (FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS));
    POOR_MANS_ASSERT(expected_calls[3].FormatMessageA_no_newline_call.captured_lpSource == TEST_MODULE_1);
    POOR_MANS_ASSERT(expected_calls[3].FormatMessageA_no_newline_call.captured_dwMessageId == E_FAIL);
    POOR_MANS_ASSERT(expected_calls[3].FormatMessageA_no_newline_call.captured_dwLanguageId == 0);
    POOR_MANS_ASSERT(expected_calls[3].FormatMessageA_no_newline_call.captured_lpBuffer == buffer);
    POOR_MANS_ASSERT(expected_calls[3].FormatMessageA_no_newline_call.captured_nSize == 512);
    POOR_MANS_ASSERT(expected_calls[3].FormatMessageA_no_newline_call.captured_Arguments == NULL);
    POOR_MANS_ASSERT(strcmp(buffer, TEST_FORMATTED_HRESULT_E_FAIL) == 0);
}

/* Tests_SRS_LOG_HRESULT_01_010: [ For each module: ] */
/* Tests_SRS_LOG_HRESULT_01_011: [ log_hresult_fill_property shall call FormatMessageA with FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS, the module handle, the hresult value, 0 as language Id, buffer as buffer to place the output and 512 as buffer size. ] */
static void when_2nd_module_out_of_2_FormatMessageA_no_newline_formats_log_hresult_fill_property_formats_success(void)
{
    // arrange
    char buffer[512];
    HMODULE test_modules[] = { TEST_MODULE_1, TEST_MODULE_2 };

    setup_mocks();
    setup_FormatMessageA_no_newline_call();
    setup_GetCurrentProcess_call();
    setup_EnumProcessModules_call();
    setup_FormatMessageA_no_newline_call();
    setup_FormatMessageA_no_newline_call();

    expected_calls[0].FormatMessageA_no_newline_call.override_result = true;
    expected_calls[0].FormatMessageA_no_newline_call.call_result = 0;
    expected_calls[1].GetCurrentProcess_call.override_result = true;
    expected_calls[1].GetCurrentProcess_call.call_result = (HANDLE)0x4242;
    expected_calls[2].EnumProcessModules_call.override_result = true;
    expected_calls[2].EnumProcessModules_call.returned_filled_bytes = sizeof(HMODULE) * 2;
    expected_calls[2].EnumProcessModules_call.returned_lphModule = test_modules;
    expected_calls[2].EnumProcessModules_call.call_result = TRUE;
    expected_calls[3].FormatMessageA_no_newline_call.override_result = true;
    expected_calls[3].FormatMessageA_no_newline_call.call_result = 0;
    expected_calls[4].FormatMessageA_no_newline_call.override_result = true;
    expected_calls[4].FormatMessageA_no_newline_call.call_result = sizeof(TEST_FORMATTED_HRESULT_E_FAIL_OTHER);
    expected_calls[4].FormatMessageA_no_newline_call.buffer_payload = TEST_FORMATTED_HRESULT_E_FAIL_OTHER;

    // act
    int result = log_hresult_fill_property(buffer, E_FAIL);

    // assert
    POOR_MANS_ASSERT(result == 512);
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_no_newline_call.captured_dwFlags == (FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS));
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_no_newline_call.captured_lpSource == NULL);
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_no_newline_call.captured_dwMessageId == E_FAIL);
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_no_newline_call.captured_dwLanguageId == 0);
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_no_newline_call.captured_lpBuffer == buffer);
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_no_newline_call.captured_nSize == 512);
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_no_newline_call.captured_Arguments == NULL);
    POOR_MANS_ASSERT(expected_calls[2].EnumProcessModules_call.captured_hProcess == (HANDLE)0x4242);
    POOR_MANS_ASSERT(expected_calls[2].EnumProcessModules_call.captured_cb == (sizeof(HMODULE) * 10));
    POOR_MANS_ASSERT(expected_calls[3].FormatMessageA_no_newline_call.captured_dwFlags == (FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS));
    POOR_MANS_ASSERT(expected_calls[3].FormatMessageA_no_newline_call.captured_lpSource == TEST_MODULE_1);
    POOR_MANS_ASSERT(expected_calls[3].FormatMessageA_no_newline_call.captured_dwMessageId == E_FAIL);
    POOR_MANS_ASSERT(expected_calls[3].FormatMessageA_no_newline_call.captured_dwLanguageId == 0);
    POOR_MANS_ASSERT(expected_calls[3].FormatMessageA_no_newline_call.captured_lpBuffer == buffer);
    POOR_MANS_ASSERT(expected_calls[3].FormatMessageA_no_newline_call.captured_nSize == 512);
    POOR_MANS_ASSERT(expected_calls[3].FormatMessageA_no_newline_call.captured_Arguments == NULL);
    POOR_MANS_ASSERT(expected_calls[4].FormatMessageA_no_newline_call.captured_dwFlags == (FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS));
    POOR_MANS_ASSERT(expected_calls[4].FormatMessageA_no_newline_call.captured_lpSource == TEST_MODULE_2);
    POOR_MANS_ASSERT(expected_calls[4].FormatMessageA_no_newline_call.captured_dwMessageId == E_FAIL);
    POOR_MANS_ASSERT(expected_calls[4].FormatMessageA_no_newline_call.captured_dwLanguageId == 0);
    POOR_MANS_ASSERT(expected_calls[4].FormatMessageA_no_newline_call.captured_lpBuffer == buffer);
    POOR_MANS_ASSERT(expected_calls[4].FormatMessageA_no_newline_call.captured_nSize == 512);
    POOR_MANS_ASSERT(expected_calls[4].FormatMessageA_no_newline_call.captured_Arguments == NULL);
    POOR_MANS_ASSERT(strcmp(buffer, TEST_FORMATTED_HRESULT_E_FAIL_OTHER) == 0);
}

/* Tests_SRS_LOG_HRESULT_01_007: [ If no module has the formatted message, log_hresult_fill_property shall place in buffer the string unknown HRESULT 0x%x, where %x is the hresult value. ] */
static void when_none_of_2_FormatMessageA_no_newline_formats_it_log_hresult_fill_property_formats_yields_unknown(void)
{
    // arrange
    char buffer[512];
    HMODULE test_modules[] = { TEST_MODULE_1, TEST_MODULE_2 };

    setup_mocks();
    setup_FormatMessageA_no_newline_call();
    setup_GetCurrentProcess_call();
    setup_EnumProcessModules_call();
    setup_FormatMessageA_no_newline_call();
    setup_FormatMessageA_no_newline_call();
    setup_snprintf_call(); // for unknown

    expected_calls[0].FormatMessageA_no_newline_call.override_result = true;
    expected_calls[0].FormatMessageA_no_newline_call.call_result = 0;
    expected_calls[1].GetCurrentProcess_call.override_result = true;
    expected_calls[1].GetCurrentProcess_call.call_result = (HANDLE)0x4242;
    expected_calls[2].EnumProcessModules_call.override_result = true;
    expected_calls[2].EnumProcessModules_call.returned_filled_bytes = sizeof(HMODULE) * 2;
    expected_calls[2].EnumProcessModules_call.returned_lphModule = test_modules;
    expected_calls[2].EnumProcessModules_call.call_result = TRUE;
    expected_calls[3].FormatMessageA_no_newline_call.override_result = true;
    expected_calls[3].FormatMessageA_no_newline_call.call_result = 0;
    expected_calls[4].FormatMessageA_no_newline_call.override_result = true;
    expected_calls[4].FormatMessageA_no_newline_call.call_result = 0;

    // act
    int result = log_hresult_fill_property(buffer, E_FAIL);

    // assert
    POOR_MANS_ASSERT(result == 512);
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_no_newline_call.captured_dwFlags == (FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS));
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_no_newline_call.captured_lpSource == NULL);
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_no_newline_call.captured_dwMessageId == E_FAIL);
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_no_newline_call.captured_dwLanguageId == 0);
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_no_newline_call.captured_lpBuffer == buffer);
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_no_newline_call.captured_nSize == 512);
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_no_newline_call.captured_Arguments == NULL);
    POOR_MANS_ASSERT(expected_calls[2].EnumProcessModules_call.captured_hProcess == (HANDLE)0x4242);
    POOR_MANS_ASSERT(expected_calls[2].EnumProcessModules_call.captured_cb == (sizeof(HMODULE) * 10));
    POOR_MANS_ASSERT(expected_calls[3].FormatMessageA_no_newline_call.captured_dwFlags == (FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS));
    POOR_MANS_ASSERT(expected_calls[3].FormatMessageA_no_newline_call.captured_lpSource == TEST_MODULE_1);
    POOR_MANS_ASSERT(expected_calls[3].FormatMessageA_no_newline_call.captured_dwMessageId == E_FAIL);
    POOR_MANS_ASSERT(expected_calls[3].FormatMessageA_no_newline_call.captured_dwLanguageId == 0);
    POOR_MANS_ASSERT(expected_calls[3].FormatMessageA_no_newline_call.captured_lpBuffer == buffer);
    POOR_MANS_ASSERT(expected_calls[3].FormatMessageA_no_newline_call.captured_nSize == 512);
    POOR_MANS_ASSERT(expected_calls[3].FormatMessageA_no_newline_call.captured_Arguments == NULL);
    POOR_MANS_ASSERT(expected_calls[4].FormatMessageA_no_newline_call.captured_dwFlags == (FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS));
    POOR_MANS_ASSERT(expected_calls[4].FormatMessageA_no_newline_call.captured_lpSource == TEST_MODULE_2);
    POOR_MANS_ASSERT(expected_calls[4].FormatMessageA_no_newline_call.captured_dwMessageId == E_FAIL);
    POOR_MANS_ASSERT(expected_calls[4].FormatMessageA_no_newline_call.captured_dwLanguageId == 0);
    POOR_MANS_ASSERT(expected_calls[4].FormatMessageA_no_newline_call.captured_lpBuffer == buffer);
    POOR_MANS_ASSERT(expected_calls[4].FormatMessageA_no_newline_call.captured_nSize == 512);
    POOR_MANS_ASSERT(expected_calls[4].FormatMessageA_no_newline_call.captured_Arguments == NULL);
    POOR_MANS_ASSERT(strcmp(buffer, "unknown HRESULT 0x80004005") == 0);
}

/* LOG_HRESULT */

/* Tests_SRS_LOG_HRESULT_01_012: [ LOG_HRESULT shall expand to a LOG_CONTEXT_PROPERTY_CUSTOM_FUNCTION with name hresult, type ascii_char_ptr and value function call being log_hresult_fill_property(hresult). ] */
static void LOG_HRESULT_emits_the_underlying_property(void)
{
    // arrange
    setup_mocks();
    setup_FormatMessageA_no_newline_call();

    expected_calls[0].GetLastError_call.override_result = true;
    expected_calls[0].GetLastError_call.call_result = E_FAIL;
    expected_calls[1].FormatMessageA_no_newline_call.override_result = true;
    expected_calls[1].FormatMessageA_no_newline_call.call_result = sizeof(TEST_FORMATTED_HRESULT_E_FAIL_OTHER) - 1;
    expected_calls[1].FormatMessageA_no_newline_call.buffer_payload = TEST_FORMATTED_HRESULT_E_FAIL_OTHER;

    // act
    LOG_CONTEXT_LOCAL_DEFINE(log_context, NULL, LOG_HRESULT(E_FAIL));

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);

    uint32_t property_count = log_context_get_property_value_pair_count(&log_context);
    const LOG_CONTEXT_PROPERTY_VALUE_PAIR* properties = log_context_get_property_value_pairs(&log_context);
    POOR_MANS_ASSERT(property_count == 2);
    POOR_MANS_ASSERT(strcmp(properties[0].name, "") == 0);
    POOR_MANS_ASSERT(properties[0].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_struct);
    POOR_MANS_ASSERT(*(uint8_t*)properties[0].value == 1);
    POOR_MANS_ASSERT(strcmp(properties[1].name, "LastError") == 0);
    POOR_MANS_ASSERT(properties[1].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_ascii_char_ptr);
    POOR_MANS_ASSERT(strcmp(properties[1].value, TEST_FORMATTED_HRESULT_E_FAIL_OTHER) == 0);
}

/* very "poor man's" way of testing, as no test harness and mocking framework are available */
int main(void)
{
#ifdef _MSC_VER
    // make abort not popup
    _set_abort_behavior(_CALL_REPORTFAULT, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
#endif

    log_hresult_fill_property_with_NULL_returns_needed_buffer_size();
    log_hresult_fill_property_with_non_NULL_formats_S_OK();
    log_hresult_fill_property_with_non_NULL_formats_E_FAIL();
    
    when_FormatMessageA_fails_and_no_modules_are_returned_log_hresult_fill_property_places_unknown_hresult_in_buffer();
    when_snprintf_fails_log_hresult_fill_property_places_snprintf_failed_in_the_buffer();
    when_EnumProcessModules_fails_log_hresult_fill_property_places_unknown_hresult_with_lasterror_in_buffer();
    when_snprintf_fails_after_EnumProcessModules_fails_log_hresult_fill_property_places_snprintf_failed_in_the_buffer();

    when_first_module_out_of_2_FormatMessageA_no_newline_formats_log_hresult_fill_property_formats_success();
    when_2nd_module_out_of_2_FormatMessageA_no_newline_formats_log_hresult_fill_property_formats_success();
    when_none_of_2_FormatMessageA_no_newline_formats_it_log_hresult_fill_property_formats_yields_unknown();

    //LOG_LASTERROR_emits_the_underlying_property();

    return 0;
}
