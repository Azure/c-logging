// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <wchar.h>

#include "c_logging/log_context_property_type.h"
#include "c_logging/log_context_property_type_if.h"

#include "c_logging/log_context_property_type_wchar_t_ptr.h"

#define POOR_MANS_ASSERT(cond) \
    if (!(cond)) \
    { \
        (void)printf("%s:%d test failed\r\n", __FUNCTION__, __LINE__); \
        abort(); \
    } \

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(wchar_t_ptr).to_string */

static void wchar_t_ptr_to_string_with_NULL_property_value_fails(void)
{
    // arrange
    char buffer[128];

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(wchar_t_ptr).to_string(NULL, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result < 0);
}

static void wchar_t_ptr_to_string_with_NULL_buffer_and_non_zero_buffer_length_fails(void)
{
    // arrange

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(wchar_t_ptr).to_string(L"cucu", NULL, 1);

    // assert
    POOR_MANS_ASSERT(result < 0);
}

static void wchar_t_ptr_to_string_with_NULL_buffer_and_zero_buffer_length_returns_string_length(void)
{
    // arrange

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(wchar_t_ptr).to_string(L"cucu", NULL, 0);

    // assert
    POOR_MANS_ASSERT(result == (int)wcslen(L"cucu"));
}

static void wchar_t_ptr_to_string_copies_the_string(void)
{
    // arrange
    char buffer[4];

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(wchar_t_ptr).to_string(L"bau", buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 3);
    POOR_MANS_ASSERT(strcmp(buffer, "bau") == 0);
}

static void to_string_with_truncation_succeeds(void)
{
    // arrange
    char buffer[2];

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(wchar_t_ptr).to_string(L"bau", buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 2);
    POOR_MANS_ASSERT(strncmp(buffer, "ba", 2) == 0);
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(wchar_t_ptr).copy */

static void wchar_t_ptr_copy_with_NULL_src_value_fails(void)
{
    // arrange
    wchar_t buffer[128];

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(wchar_t_ptr).copy(buffer, NULL);

    // assert
    POOR_MANS_ASSERT(result != 0);
}

static void wchar_t_ptr_copy_with_NULL_dst_value_fails(void)
{
    // arrange
    wchar_t buffer[sizeof(L"cucu")];
    LOG_CONTEXT_PROPERTY_TYPE_INIT(wchar_t_ptr)(buffer, sizeof(buffer)/sizeof(buffer[0]), L"cucu");

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(wchar_t_ptr).copy(NULL, buffer);

    // assert
    POOR_MANS_ASSERT(result != 0);
}

static void wchar_t_ptr_copy_succeeds(void)
{
    // arrange
    wchar_t src_buffer[5];
    wchar_t dst_buffer[5];
    LOG_CONTEXT_PROPERTY_TYPE_INIT(wchar_t_ptr)(src_buffer, wcslen(L"cucu") + 1, L"cucu");

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(wchar_t_ptr).copy(dst_buffer, src_buffer);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(wcscmp(dst_buffer, L"cucu") == 0);
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(wchar_t_ptr).free */

/* Tests_SRS_LOG_CONTEXT_PROPERTY_TYPE_WCHAR_T_PTR_07_011: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(wchar_t_ptr).free shall return. ]*/
static void wchar_t_ptr_free_returns(void)
{
    // arrange
    wchar_t buffer[sizeof(L"cucu")];
    LOG_CONTEXT_PROPERTY_TYPE_INIT(wchar_t_ptr)(buffer, wcslen(L"cucu") + 1, L"cucu");

    // act
    LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(wchar_t_ptr).free(buffer);

    // assert
    // no explicit assert, no crash expected
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(wchar_t_ptr).get_type */

static void wchar_t_ptr_get_type_returns_LOG_CONTEXT_PROPERTY_TYPE_wchar_t_ptr(void)
{
    // arrange

    // act
    LOG_CONTEXT_PROPERTY_TYPE result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(wchar_t_ptr).get_type();

    // assert
    POOR_MANS_ASSERT(result == LOG_CONTEXT_PROPERTY_TYPE_wchar_t_ptr);
}

/* LOG_CONTEXT_PROPERTY_TYPE_INIT(wchar_t_ptr) */

static void wchar_t_ptr_LOG_CONTEXT_PROPERTY_TYPE_INIT_with_NULL_dst_value_fails(void)
{
    // arrange

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_INIT(wchar_t_ptr)(NULL, wcslen(L"cucu") + 1, L"cucu");

    // assert
    POOR_MANS_ASSERT(result != 0);
}

static void wchar_t_ptr_LOG_CONTEXT_PROPERTY_TYPE_INIT_with_NULL_format_fails(void)
{
    // arrange
    wchar_t buffer[5];

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_INIT(wchar_t_ptr)(buffer, 5, NULL);

    // assert
    POOR_MANS_ASSERT(result != 0);
}

static void wchar_t_ptr_LOG_CONTEXT_PROPERTY_TYPE_INIT_succeeds(void)
{
    // arrange
    wchar_t buffer[5];

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_INIT(wchar_t_ptr)(buffer, wcslen(L"cucu") + 1, L"cucu");

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(wcscmp(buffer, L"cucu") == 0);
}

static void wchar_t_ptr_LOG_CONTEXT_PROPERTY_TYPE_INIT_with_multiple_args_succeeds(void)
{
    // arrange
    wchar_t buffer[43];

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_INIT(wchar_t_ptr)(buffer, sizeof(buffer)/sizeof(buffer[0]), L"The answer is %d and let's say hello %ls", 42, L"world");

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(wcscmp(buffer, L"The answer is 42 and let's say hello world") == 0);
}

/* LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(wchar_t_ptr)*/

static void wchar_t_ptr_LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE_succeeds(void)
{
    // arrange

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(wchar_t_ptr)(L"cucu");

    // assert
    POOR_MANS_ASSERT(result == 5*sizeof(wchar_t));
}

static void wchar_t_ptr_LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE_with_multiple_args_succeeds(void)
{
    // arrange

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(wchar_t_ptr)(L"The answer is %d and let's say hello %ls", 43, L"world");

    // assert
    POOR_MANS_ASSERT(result == 43*sizeof(wchar_t));
}

/* very "poor man's" way of testing, as no test harness and mocking framework are available */
int main(void)
{
    wchar_t_ptr_to_string_with_NULL_property_value_fails();
    wchar_t_ptr_to_string_with_NULL_buffer_and_non_zero_buffer_length_fails();
    wchar_t_ptr_to_string_with_NULL_buffer_and_zero_buffer_length_returns_string_length();
    wchar_t_ptr_to_string_copies_the_string();
    to_string_with_truncation_succeeds();

    wchar_t_ptr_copy_with_NULL_src_value_fails();
    wchar_t_ptr_copy_with_NULL_dst_value_fails();
    wchar_t_ptr_copy_succeeds();

    wchar_t_ptr_free_returns();

    wchar_t_ptr_get_type_returns_LOG_CONTEXT_PROPERTY_TYPE_wchar_t_ptr();

    wchar_t_ptr_LOG_CONTEXT_PROPERTY_TYPE_INIT_with_NULL_dst_value_fails();
    wchar_t_ptr_LOG_CONTEXT_PROPERTY_TYPE_INIT_with_NULL_format_fails();
    wchar_t_ptr_LOG_CONTEXT_PROPERTY_TYPE_INIT_succeeds();
    wchar_t_ptr_LOG_CONTEXT_PROPERTY_TYPE_INIT_with_multiple_args_succeeds();

    wchar_t_ptr_LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE_succeeds();
    wchar_t_ptr_LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE_with_multiple_args_succeeds();

    return 0;
}
