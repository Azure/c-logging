// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#ifdef _MSC_VER
#include "windows.h"
#endif

#include "macro_utils/macro_utils.h"

#include "c_logging/log_lasterror.h"
#include "c_logging/log_context.h"

// defines how many mock calls we can have
#define MAX_MOCK_CALL_COUNT (128)

#define MOCK_CALL_TYPE_VALUES \
    MOCK_CALL_TYPE_GetLastError, \
    MOCK_CALL_TYPE_FormatMessageA \

MU_DEFINE_ENUM(MOCK_CALL_TYPE, MOCK_CALL_TYPE_VALUES)

// very poor mans mocks :-(

typedef struct GetLastError_CALL_TAG
{
    bool override_result;
    int call_result;
} GetLastError_CALL;

typedef struct FormatMessageA_CALL_TAG
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
} FormatMessageA_CALL;

typedef struct MOCK_CALL_TAG
{
    MOCK_CALL_TYPE mock_call_type;
    union
    {
        GetLastError_CALL GetLastError_call;
        FormatMessageA_CALL FormatMessageA_call;
    };
} MOCK_CALL;

static MOCK_CALL expected_calls[MAX_MOCK_CALL_COUNT];
static size_t expected_call_count;
static size_t actual_call_count;
static bool actual_and_expected_match;

const char TEST_FORMATTED_LASTERROR_995_with_newlines[] = "A dummy 995\r\n\r\n";
const char TEST_FORMATTED_LASTERROR_995[] = "A dummy 995";

const char TEST_FORMATTED_LASTERROR_42_with_newlines[] = "A dummy 42\r\n\r\n";
const char TEST_FORMATTED_LASTERROR_42[] = "A dummy 42";

const char TEST_FAILURE_STRING[] = "failure in FormatMessageA";

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

DWORD mock_FormatMessageA(DWORD dwFlags, LPCVOID lpSource, DWORD dwMessageId, DWORD dwLanguageId, LPSTR lpBuffer, DWORD nSize, va_list* Arguments)
{
    DWORD result;

    if ((actual_call_count == expected_call_count) ||
        (expected_calls[actual_call_count].mock_call_type != MOCK_CALL_TYPE_FormatMessageA))
    {
        actual_and_expected_match = false;
        result = MU_FAILURE;
    }
    else
    {
        expected_calls[actual_call_count].FormatMessageA_call.captured_dwFlags = dwFlags;
        expected_calls[actual_call_count].FormatMessageA_call.captured_lpSource = lpSource;
        expected_calls[actual_call_count].FormatMessageA_call.captured_dwLanguageId = dwLanguageId;
        expected_calls[actual_call_count].FormatMessageA_call.captured_lpBuffer = lpBuffer;
        expected_calls[actual_call_count].FormatMessageA_call.captured_nSize = nSize;
        expected_calls[actual_call_count].FormatMessageA_call.captured_Arguments = Arguments;
        expected_calls[actual_call_count].FormatMessageA_call.captured_dwMessageId = dwMessageId;

        if (expected_calls[actual_call_count].FormatMessageA_call.override_result)
        {
            (void)memcpy(lpBuffer, expected_calls[actual_call_count].FormatMessageA_call.buffer_payload, expected_calls[actual_call_count].FormatMessageA_call.call_result);
            result = expected_calls[actual_call_count].FormatMessageA_call.call_result;
        }
        else
        {
            result = FormatMessageA(dwFlags, lpSource, dwMessageId, dwLanguageId, lpBuffer, nSize, Arguments);
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

static void setup_FormatMessageA_call(void)
{
    expected_calls[expected_call_count].mock_call_type = MOCK_CALL_TYPE_FormatMessageA;
    expected_calls[expected_call_count].FormatMessageA_call.override_result = false;
    expected_call_count++;
}

/* log_lasterror_fill_property */

/* Tests_SRS_LOG_LASTERROR_01_002: [ If buffer is NULL, log_lasterror_fill_property shall return 512 to indicate how many bytes shall be reserved for the last error string formatted version. ] */
static void log_lasterror_fill_property_with_NULL_returns_needed_buffer_size(void)
{
    // arrange
    setup_mocks();

    // act
    int result = log_lasterror_fill_property(NULL);

    // assert
    POOR_MANS_ASSERT(result == 512);
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_LASTERROR_01_003: [ Otherwise, log_lasterror_fill_property shall call GetLastError to obtain the last error information. ] */
/* Tests_SRS_LOG_LASTERROR_01_004: [ log_lasterror_fill_property shall call FormatMessageA with FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, the last error value, LANG_NEUTRAL as language Id, buffer as buffer to place the output and 512 as buffer size. ] */
/* Tests_SRS_LOG_LASTERROR_01_006: [ Otherwise, log_lasterror_fill_property shall remove any \r or \n characters that have been placed at the end of the formatted output by FormatMessageA. ] */
/* Tests_SRS_LOG_LASTERROR_01_007: [ log_lasterror_fill_property shall return 512. ] */
static void log_lasterror_fill_property_with_non_NULL_formats_last_error_995(void)
{
    // arrange
    char buffer[512];

    setup_mocks();
    setup_GetLastError_call();
    setup_FormatMessageA_call();
    expected_calls[0].GetLastError_call.override_result = true;
    expected_calls[0].GetLastError_call.call_result = 995;
    expected_calls[1].FormatMessageA_call.override_result = true;
    expected_calls[1].FormatMessageA_call.call_result = sizeof(TEST_FORMATTED_LASTERROR_995_with_newlines) - 1;
    expected_calls[1].FormatMessageA_call.buffer_payload = TEST_FORMATTED_LASTERROR_995_with_newlines;

    // act
    int result = log_lasterror_fill_property(buffer);

    // assert
    POOR_MANS_ASSERT(result == 512);
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(expected_calls[1].FormatMessageA_call.captured_dwFlags == (FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS));
    POOR_MANS_ASSERT(expected_calls[1].FormatMessageA_call.captured_lpSource == NULL);
    POOR_MANS_ASSERT(expected_calls[1].FormatMessageA_call.captured_dwMessageId == 995);
    POOR_MANS_ASSERT(expected_calls[1].FormatMessageA_call.captured_dwLanguageId == MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT));
    POOR_MANS_ASSERT(expected_calls[1].FormatMessageA_call.captured_lpBuffer == buffer);
    POOR_MANS_ASSERT(expected_calls[1].FormatMessageA_call.captured_nSize == 512);
    POOR_MANS_ASSERT(expected_calls[1].FormatMessageA_call.captured_Arguments == NULL);
    POOR_MANS_ASSERT(strcmp(buffer, TEST_FORMATTED_LASTERROR_995) == 0);
}

/* Tests_SRS_LOG_LASTERROR_01_003: [ Otherwise, log_lasterror_fill_property shall call GetLastError to obtain the last error information. ] */
/* Tests_SRS_LOG_LASTERROR_01_004: [ log_lasterror_fill_property shall call FormatMessageA with FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, the last error value, LANG_NEUTRAL as language Id, buffer as buffer to place the output and 512 as buffer size. ] */
/* Tests_SRS_LOG_LASTERROR_01_006: [ Otherwise, log_lasterror_fill_property shall remove any \r or \n characters that have been placed at the end of the formatted output by FormatMessageA. ] */
/* Tests_SRS_LOG_LASTERROR_01_007: [ log_lasterror_fill_property shall return 512. ] */
static void log_lasterror_fill_property_with_non_NULL_formats_last_error_42(void)
{
    // arrange
    char buffer[512];

    setup_mocks();
    setup_GetLastError_call();
    setup_FormatMessageA_call();
    expected_calls[0].GetLastError_call.override_result = true;
    expected_calls[0].GetLastError_call.call_result = 995;
    expected_calls[1].FormatMessageA_call.override_result = true;
    expected_calls[1].FormatMessageA_call.call_result = sizeof(TEST_FORMATTED_LASTERROR_42_with_newlines) - 1;
    expected_calls[1].FormatMessageA_call.buffer_payload = TEST_FORMATTED_LASTERROR_42_with_newlines;

    // act
    int result = log_lasterror_fill_property(buffer);

    // assert
    POOR_MANS_ASSERT(result == 512);
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(expected_calls[1].FormatMessageA_call.captured_dwFlags == (FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS));
    POOR_MANS_ASSERT(expected_calls[1].FormatMessageA_call.captured_lpSource == NULL);
    POOR_MANS_ASSERT(expected_calls[1].FormatMessageA_call.captured_dwMessageId == 995);
    POOR_MANS_ASSERT(expected_calls[1].FormatMessageA_call.captured_dwLanguageId == MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT));
    POOR_MANS_ASSERT(expected_calls[1].FormatMessageA_call.captured_lpBuffer == buffer);
    POOR_MANS_ASSERT(expected_calls[1].FormatMessageA_call.captured_nSize == 512);
    POOR_MANS_ASSERT(expected_calls[1].FormatMessageA_call.captured_Arguments == NULL);
    POOR_MANS_ASSERT(strcmp(buffer, TEST_FORMATTED_LASTERROR_42) == 0);
}

static void test_with_crlf(const char* returned_formatted_string, const char* expected_string)
{
    // arrange
    char buffer[512];

    setup_mocks();
    setup_GetLastError_call();
    setup_FormatMessageA_call();
    expected_calls[0].GetLastError_call.override_result = true;
    expected_calls[0].GetLastError_call.call_result = 995;
    expected_calls[1].FormatMessageA_call.override_result = true;
    expected_calls[1].FormatMessageA_call.call_result = (int)strlen(returned_formatted_string);
    expected_calls[1].FormatMessageA_call.buffer_payload = returned_formatted_string;

    // act
    int result = log_lasterror_fill_property(buffer);

    // assert
    POOR_MANS_ASSERT(result == 512);
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(expected_calls[1].FormatMessageA_call.captured_dwFlags == (FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS));
    POOR_MANS_ASSERT(expected_calls[1].FormatMessageA_call.captured_lpSource == NULL);
    POOR_MANS_ASSERT(expected_calls[1].FormatMessageA_call.captured_dwMessageId == 995);
    POOR_MANS_ASSERT(expected_calls[1].FormatMessageA_call.captured_dwLanguageId == MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT));
    POOR_MANS_ASSERT(expected_calls[1].FormatMessageA_call.captured_lpBuffer == buffer);
    POOR_MANS_ASSERT(expected_calls[1].FormatMessageA_call.captured_nSize == 512);
    POOR_MANS_ASSERT(expected_calls[1].FormatMessageA_call.captured_Arguments == NULL);
    POOR_MANS_ASSERT(strcmp(buffer, expected_string) == 0);
}

/* Tests_SRS_LOG_LASTERROR_01_006: [ Otherwise, log_lasterror_fill_property shall remove any \r or \n characters that have been placed at the end of the formatted output by FormatMessageA. ] */
static void log_lasterror_fill_property_with_non_NULL_formats_last_error_42_one_newline(void)
{
    test_with_crlf("A dummy LE\n", "A dummy LE");
}

/* Tests_SRS_LOG_LASTERROR_01_006: [ Otherwise, log_lasterror_fill_property shall remove any \r or \n characters that have been placed at the end of the formatted output by FormatMessageA. ] */
static void log_lasterror_fill_property_with_non_NULL_formats_last_error_42_2_newlines(void)
{
    test_with_crlf("A dummy LE\n\n", "A dummy LE");
}

/* Tests_SRS_LOG_LASTERROR_01_006: [ Otherwise, log_lasterror_fill_property shall remove any \r or \n characters that have been placed at the end of the formatted output by FormatMessageA. ] */
static void log_lasterror_fill_property_with_non_NULL_formats_last_error_42_1_CR(void)
{
    test_with_crlf("A dummy LE\r", "A dummy LE");
}

/* Tests_SRS_LOG_LASTERROR_01_006: [ Otherwise, log_lasterror_fill_property shall remove any \r or \n characters that have been placed at the end of the formatted output by FormatMessageA. ] */
static void log_lasterror_fill_property_with_non_NULL_formats_last_error_42_2_CR(void)
{
    test_with_crlf("A dummy LE\r\r", "A dummy LE");
}

/* Tests_SRS_LOG_LASTERROR_01_006: [ Otherwise, log_lasterror_fill_property shall remove any \r or \n characters that have been placed at the end of the formatted output by FormatMessageA. ] */
static void log_lasterror_fill_property_with_non_NULL_formats_last_error_42_2_pairs(void)
{
    test_with_crlf("A dummy LE\r\n\r\n", "A dummy LE");
}

/* Tests_SRS_LOG_LASTERROR_01_006: [ Otherwise, log_lasterror_fill_property shall remove any \r or \n characters that have been placed at the end of the formatted output by FormatMessageA. ] */
static void log_lasterror_fill_property_with_non_NULL_formats_last_error_42_2_pairs_reverse(void)
{
    test_with_crlf("A dummy LE\n\r\n\r", "A dummy LE");
}

/* Tests_SRS_LOG_LASTERROR_01_006: [ Otherwise, log_lasterror_fill_property shall remove any \r or \n characters that have been placed at the end of the formatted output by FormatMessageA. ] */
static void log_lasterror_fill_property_with_non_NULL_formats_last_error_42_only_end_crlf_removed(void)
{
    test_with_crlf("A dummy LE\n\r\n\rx", "A dummy LE\n\r\n\rx");
}

/* Tests_SRS_LOG_LASTERROR_01_005: [ If FormatMessageA returns 0, log_lasterror_fill_property shall copy in buffer the string failure in FormatMessageA and return 512. ] */
static void when_FormatMessageA_fails_log_lasterror_fill_property_places_a_default_string_in_the_buffer(void)
{
    // arrange
    char buffer[512];

    setup_mocks();
    setup_GetLastError_call();
    setup_FormatMessageA_call();
    expected_calls[0].GetLastError_call.override_result = true;
    expected_calls[0].GetLastError_call.call_result = 995;
    expected_calls[1].FormatMessageA_call.override_result = true;
    expected_calls[1].FormatMessageA_call.call_result = 0;
    expected_calls[1].FormatMessageA_call.buffer_payload = TEST_FORMATTED_LASTERROR_995_with_newlines;

    // act
    int result = log_lasterror_fill_property(buffer);

    // assert
    POOR_MANS_ASSERT(result == 512);
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(expected_calls[1].FormatMessageA_call.captured_dwFlags == (FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS));
    POOR_MANS_ASSERT(expected_calls[1].FormatMessageA_call.captured_lpSource == NULL);
    POOR_MANS_ASSERT(expected_calls[1].FormatMessageA_call.captured_dwMessageId == 995);
    POOR_MANS_ASSERT(expected_calls[1].FormatMessageA_call.captured_dwLanguageId == MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT));
    POOR_MANS_ASSERT(expected_calls[1].FormatMessageA_call.captured_lpBuffer == buffer);
    POOR_MANS_ASSERT(expected_calls[1].FormatMessageA_call.captured_nSize == 512);
    POOR_MANS_ASSERT(expected_calls[1].FormatMessageA_call.captured_Arguments == NULL);
    POOR_MANS_ASSERT(strcmp(buffer, TEST_FAILURE_STRING) == 0);
}

/* LOGGER_LOG */

/* Tests_SRS_LOG_LASTERROR_01_001: [ LOG_LASTERROR shall expand to a LOG_CONTEXT_PROPERTY with name LastError, type int32_t and value the result of calling GetLastError. ] */
static void LOG_LASTERROR_emits_the_underlying_property(void)
{
    // arrange
    setup_mocks();
    setup_GetLastError_call();
    setup_FormatMessageA_call();
    expected_calls[0].GetLastError_call.override_result = true;
    expected_calls[0].GetLastError_call.call_result = 995;
    expected_calls[1].FormatMessageA_call.override_result = true;
    expected_calls[1].FormatMessageA_call.call_result = sizeof(TEST_FORMATTED_LASTERROR_995_with_newlines) - 1;
    expected_calls[1].FormatMessageA_call.buffer_payload = TEST_FORMATTED_LASTERROR_995_with_newlines;

    // act
    LOG_CONTEXT_LOCAL_DEFINE(log_context, NULL, LOG_LASTERROR());

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
    POOR_MANS_ASSERT(strcmp(properties[1].value, TEST_FORMATTED_LASTERROR_995) == 0);
}

/* very "poor man's" way of testing, as no test harness and mocking framework are available */
int main(void)
{
#ifdef _MSC_VER
    // make abort not popup
    _set_abort_behavior(_CALL_REPORTFAULT, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
#endif

    log_lasterror_fill_property_with_NULL_returns_needed_buffer_size();
    log_lasterror_fill_property_with_non_NULL_formats_last_error_995();
    log_lasterror_fill_property_with_non_NULL_formats_last_error_42();

    log_lasterror_fill_property_with_non_NULL_formats_last_error_42_one_newline();
    log_lasterror_fill_property_with_non_NULL_formats_last_error_42_2_newlines();
    log_lasterror_fill_property_with_non_NULL_formats_last_error_42_1_CR();
    log_lasterror_fill_property_with_non_NULL_formats_last_error_42_2_CR();
    log_lasterror_fill_property_with_non_NULL_formats_last_error_42_2_pairs();
    log_lasterror_fill_property_with_non_NULL_formats_last_error_42_2_pairs_reverse();
    log_lasterror_fill_property_with_non_NULL_formats_last_error_42_only_end_crlf_removed();

    when_FormatMessageA_fails_log_lasterror_fill_property_places_a_default_string_in_the_buffer();

    LOG_LASTERROR_emits_the_underlying_property();

    return 0;
}
