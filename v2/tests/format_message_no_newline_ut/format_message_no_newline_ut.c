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

#include "c_logging/format_message_no_newline.h"

// defines how many mock calls we can have
#define MAX_MOCK_CALL_COUNT (128)

#define MOCK_CALL_TYPE_VALUES \
    MOCK_CALL_TYPE_FormatMessageA \

MU_DEFINE_ENUM(MOCK_CALL_TYPE, MOCK_CALL_TYPE_VALUES)

// very poor mans mocks :-(

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
            if (expected_calls[actual_call_count].FormatMessageA_call.buffer_payload != NULL)
            {
                (void)memcpy(lpBuffer, expected_calls[actual_call_count].FormatMessageA_call.buffer_payload, expected_calls[actual_call_count].FormatMessageA_call.call_result + 1);
            }
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

static void setup_FormatMessageA_call(void)
{
    expected_calls[expected_call_count].mock_call_type = MOCK_CALL_TYPE_FormatMessageA;
    expected_calls[expected_call_count].FormatMessageA_call.override_result = false;
    expected_calls[expected_call_count].FormatMessageA_call.buffer_payload = NULL;
    expected_call_count++;
}

/* FormatMessageA_no_newline */

#define TEST_FLAGS          (FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS)
#define TEST_SOURCE         ((void*)0x4242)
#define TEST_MESSAGE_ID     (995)
#define TEST_LANGUAGE_ID    (MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT))
#define TEST_VA_LIST        ((va_list*)0x4243)

#define TEST_FLAGS_2        (0)
#define TEST_SOURCE_2       ((void*)0x42)
#define TEST_MESSAGE_ID_2   (42)
#define TEST_LANGUAGE_ID_2  (0)
#define TEST_VA_LIST_2      ((va_list*)0x43)

/* Tests_SRS_FORMATMESSAGE_NO_NEWLINE_01_001: [ FormatMessageA_no_newline shall call FormatMessageA with the same arguments. ] */
/* Tests_SRS_FORMATMESSAGE_NO_NEWLINE_01_003: [ Otherwise, FormatMessageA_no_newline shall remove any r or n characters that have been placed at the end of the formatted output by FormatMessageA and return the number of CHARs left in lpBuffer. ] */
static void FormatMessageA_no_newline_calls_FormatMessageA_and_succeeds(void)
{
    // arrange
    char buffer[512];

    setup_mocks();
    setup_FormatMessageA_call();
    expected_calls[0].FormatMessageA_call.override_result = true;
    expected_calls[0].FormatMessageA_call.call_result = sizeof(TEST_FORMATTED_LASTERROR_995_with_newlines) - 1;
    expected_calls[0].FormatMessageA_call.buffer_payload = TEST_FORMATTED_LASTERROR_995_with_newlines;

    // act
    int result = FormatMessageA_no_newline(TEST_FLAGS, TEST_SOURCE, TEST_MESSAGE_ID, TEST_LANGUAGE_ID, buffer, sizeof(buffer), TEST_VA_LIST);

    // assert
    POOR_MANS_ASSERT(result == (sizeof(TEST_FORMATTED_LASTERROR_995) - 1));
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_call.captured_dwFlags == TEST_FLAGS);
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_call.captured_lpSource == TEST_SOURCE);
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_call.captured_dwMessageId == TEST_MESSAGE_ID);
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_call.captured_dwLanguageId == TEST_LANGUAGE_ID);
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_call.captured_lpBuffer == buffer);
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_call.captured_nSize == 512);
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_call.captured_Arguments == TEST_VA_LIST);
    POOR_MANS_ASSERT(strcmp(buffer, TEST_FORMATTED_LASTERROR_995) == 0);
}

static void test_with_crlf(const char* returned_formatted_string, const char* expected_string)
{
    // arrange
    char buffer[512];

    setup_mocks();
    setup_FormatMessageA_call();
    expected_calls[0].FormatMessageA_call.override_result = true;
    expected_calls[0].FormatMessageA_call.call_result = (int)strlen(returned_formatted_string);
    expected_calls[0].FormatMessageA_call.buffer_payload = returned_formatted_string;

    // act
    int result = FormatMessageA_no_newline(TEST_FLAGS_2, TEST_SOURCE_2, TEST_MESSAGE_ID_2, TEST_LANGUAGE_ID_2, buffer, sizeof(buffer), TEST_VA_LIST_2);

    // assert
    POOR_MANS_ASSERT(result == (int)strlen(expected_string));
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_call.captured_dwFlags == TEST_FLAGS_2);
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_call.captured_lpSource == TEST_SOURCE_2);
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_call.captured_dwMessageId == TEST_MESSAGE_ID_2);
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_call.captured_dwLanguageId == TEST_LANGUAGE_ID_2);
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_call.captured_lpBuffer == buffer);
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_call.captured_nSize == 512);
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_call.captured_Arguments == TEST_VA_LIST_2);
    POOR_MANS_ASSERT(strcmp(buffer, expected_string) == 0);
}

/* Tests_SRS_FORMATMESSAGE_NO_NEWLINE_01_003: [ Otherwise, FormatMessageA_no_newline shall remove any r or n characters that have been placed at the end of the formatted output by FormatMessageA and return the number of CHARs left in lpBuffer. ] */
static void FormatMessageA_no_newline_removes_one_newline(void)
{
    test_with_crlf("A dummy LE\n", "A dummy LE");
}

/* Tests_SRS_FORMATMESSAGE_NO_NEWLINE_01_003: [ Otherwise, FormatMessageA_no_newline shall remove any r or n characters that have been placed at the end of the formatted output by FormatMessageA and return the number of CHARs left in lpBuffer. ] */
static void FormatMessageA_no_newline_removes_2_newlines(void)
{
    test_with_crlf("A dummy LE\n\n", "A dummy LE");
}

/* Tests_SRS_FORMATMESSAGE_NO_NEWLINE_01_003: [ Otherwise, FormatMessageA_no_newline shall remove any r or n characters that have been placed at the end of the formatted output by FormatMessageA and return the number of CHARs left in lpBuffer. ] */
static void FormatMessageA_no_newline_removes_1_CR(void)
{
    test_with_crlf("A dummy LE\r", "A dummy LE");
}

/* Tests_SRS_FORMATMESSAGE_NO_NEWLINE_01_003: [ Otherwise, FormatMessageA_no_newline shall remove any r or n characters that have been placed at the end of the formatted output by FormatMessageA and return the number of CHARs left in lpBuffer. ] */
static void FormatMessageA_no_newline_removes_2_CR(void)
{
    test_with_crlf("A dummy LE\r\r", "A dummy LE");
}

/* Tests_SRS_FORMATMESSAGE_NO_NEWLINE_01_003: [ Otherwise, FormatMessageA_no_newline shall remove any r or n characters that have been placed at the end of the formatted output by FormatMessageA and return the number of CHARs left in lpBuffer. ] */
static void FormatMessageA_no_newline_removes_2_pairs(void)
{
    test_with_crlf("A dummy LE\r\n\r\n", "A dummy LE");
}

/* Tests_SRS_FORMATMESSAGE_NO_NEWLINE_01_003: [ Otherwise, FormatMessageA_no_newline shall remove any r or n characters that have been placed at the end of the formatted output by FormatMessageA and return the number of CHARs left in lpBuffer. ] */
static void FormatMessageA_no_newline_removes_2_pairs_reverse(void)
{
    test_with_crlf("A dummy LE\n\r\n\r", "A dummy LE");
}

/* Tests_SRS_FORMATMESSAGE_NO_NEWLINE_01_003: [ Otherwise, FormatMessageA_no_newline shall remove any r or n characters that have been placed at the end of the formatted output by FormatMessageA and return the number of CHARs left in lpBuffer. ] */
static void FormatMessageA_no_newline_removes_only_end_crlf(void)
{
    test_with_crlf("A dummy LE\n\r\n\rx", "A dummy LE\n\r\n\rx");
}

static const char TEST_NEWLINES_ONLY[] = "\r\n";

/* Tests_SRS_FORMATMESSAGE_NO_NEWLINE_01_003: [ Otherwise, FormatMessageA_no_newline shall remove any r or n characters that have been placed at the end of the formatted output by FormatMessageA and return the number of CHARs left in lpBuffer. ] */
static void when_string_only_contains_newlines_FormatMessageA_no_newline_returns_0(void)
{
    // arrange
    char buffer[512];

    setup_mocks();
    setup_FormatMessageA_call();
    expected_calls[0].FormatMessageA_call.override_result = true;
    expected_calls[0].FormatMessageA_call.call_result = (int)strlen(TEST_NEWLINES_ONLY);
    expected_calls[0].FormatMessageA_call.buffer_payload = TEST_NEWLINES_ONLY;

    // act
    int result = FormatMessageA_no_newline(TEST_FLAGS_2, TEST_SOURCE_2, TEST_MESSAGE_ID_2, TEST_LANGUAGE_ID_2, buffer, sizeof(buffer), TEST_VA_LIST_2);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_call.captured_dwFlags == TEST_FLAGS_2);
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_call.captured_lpSource == TEST_SOURCE_2);
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_call.captured_dwMessageId == TEST_MESSAGE_ID_2);
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_call.captured_dwLanguageId == TEST_LANGUAGE_ID_2);
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_call.captured_lpBuffer == buffer);
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_call.captured_nSize == 512);
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_call.captured_Arguments == TEST_VA_LIST_2);
}

/* Tests_SRS_FORMATMESSAGE_NO_NEWLINE_01_002: [ If FormatMessageA returns 0, FormatMessageA_no_newline shall return 0. ] */
static void when_FormatMessageA_fails_FormatMessageA_no_newline_returns_0(void)
{
    // arrange
    char buffer[512];

    setup_mocks();
    setup_FormatMessageA_call();
    expected_calls[0].FormatMessageA_call.override_result = true;
    expected_calls[0].FormatMessageA_call.call_result = 0;
    expected_calls[0].FormatMessageA_call.buffer_payload = TEST_FORMATTED_LASTERROR_995_with_newlines;

    // act
    int result = FormatMessageA_no_newline(TEST_FLAGS_2, TEST_SOURCE_2, TEST_MESSAGE_ID_2, TEST_LANGUAGE_ID_2, buffer, sizeof(buffer), TEST_VA_LIST_2);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_call.captured_dwFlags == TEST_FLAGS_2);
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_call.captured_lpSource == TEST_SOURCE_2);
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_call.captured_dwMessageId == TEST_MESSAGE_ID_2);
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_call.captured_dwLanguageId == TEST_LANGUAGE_ID_2);
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_call.captured_lpBuffer == buffer);
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_call.captured_nSize == 512);
    POOR_MANS_ASSERT(expected_calls[0].FormatMessageA_call.captured_Arguments == TEST_VA_LIST_2);
}

/* very "poor man's" way of testing, as no test harness and mocking framework are available */
int main(void)
{
    // make abort not popup
    _set_abort_behavior(_CALL_REPORTFAULT, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);

    FormatMessageA_no_newline_calls_FormatMessageA_and_succeeds();

    FormatMessageA_no_newline_removes_one_newline();
    FormatMessageA_no_newline_removes_2_newlines();
    FormatMessageA_no_newline_removes_1_CR();
    FormatMessageA_no_newline_removes_2_CR();
    FormatMessageA_no_newline_removes_2_pairs();
    FormatMessageA_no_newline_removes_2_pairs_reverse();
    FormatMessageA_no_newline_removes_only_end_crlf();

    when_string_only_contains_newlines_FormatMessageA_no_newline_returns_0();
    
    when_FormatMessageA_fails_FormatMessageA_no_newline_returns_0();

    return 0;
}
