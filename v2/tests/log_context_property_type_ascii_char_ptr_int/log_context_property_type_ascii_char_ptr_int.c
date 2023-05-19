// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "c_logging/log_context_property_type.h"
#include "c_logging/log_context_property_type_if.h"

#include "c_logging/log_context_property_type_ascii_char_ptr.h"

#define POOR_MANS_ASSERT(cond) \
    if (!(cond)) \
    { \
        (void)printf("%s:%d test failed\r\n", __FUNCTION__, __LINE__); \
        abort(); \
    } \

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(ascii_char_ptr).to_string */

static void ascii_char_ptr_to_string_with_NULL_property_value_fails(void)
{
    // arrange
    char buffer[128];

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(ascii_char_ptr).to_string(NULL, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result < 0);
}

static void ascii_char_ptr_to_string_with_NULL_buffer_and_non_zero_buffer_length_fails(void)
{
    // arrange

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(ascii_char_ptr).to_string("cucu", NULL, 1);

    // assert
    POOR_MANS_ASSERT(result < 0);
}

static void ascii_char_ptr_to_string_with_NULL_buffer_and_zero_buffer_length_returns_string_length(void)
{
    // arrange

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(ascii_char_ptr).to_string("cucu", NULL, 0);

    // assert
    POOR_MANS_ASSERT(result == (int)strlen("cucu"));
}

static void ascii_char_ptr_to_string_copies_the_string(void)
{
    // arrange
    char buffer[4];

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(ascii_char_ptr).to_string("bau", buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 3);
    POOR_MANS_ASSERT(strcmp(buffer, "bau") == 0);
}

static void to_string_with_truncation_succeeds(void)
{
    // arrange
    char buffer[2];

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(ascii_char_ptr).to_string("bau", buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 3);
    POOR_MANS_ASSERT(strcmp(buffer, "b") == 0);
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(ascii_char_ptr).copy */

static void ascii_char_ptr_copy_with_NULL_src_value_fails(void)
{
    // arrange
    char buffer[128];

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(ascii_char_ptr).copy(buffer, NULL);

    // assert
    POOR_MANS_ASSERT(result != 0);
}

static void ascii_char_ptr_copy_with_NULL_dst_value_fails(void)
{
    // arrange
    char buffer[128];
    LOG_CONTEXT_PROPERTY_TYPE_INIT(ascii_char_ptr)(buffer, "cucu");

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(ascii_char_ptr).copy(NULL, buffer);

    // assert
    POOR_MANS_ASSERT(result != 0);
}

static void ascii_char_ptr_copy_succeeds(void)
{
    // arrange
    char src_buffer[5];
    char dst_buffer[5];
    LOG_CONTEXT_PROPERTY_TYPE_INIT(ascii_char_ptr)(src_buffer, "cucu");

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(ascii_char_ptr).copy(dst_buffer, src_buffer);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(strcmp(dst_buffer, "cucu") == 0);
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(ascii_char_ptr).free */

/* Tests_SRS_LOG_CONTEXT_PROPERTY_TYPE_ASCII_CHAR_PTR_01_018: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(ascii_char_ptr).free shall return. ]*/
static void ascii_char_ptr_free_returns(void)
{
    // arrange
    char buffer[128];
    LOG_CONTEXT_PROPERTY_TYPE_INIT(ascii_char_ptr)(buffer, "cucu");

    // act
    LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(ascii_char_ptr).free(buffer);

    // assert
    // no explicit assert, no crash expected
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(ascii_char_ptr).get_type */

static void ascii_char_ptr_get_type_returns_LOG_CONTEXT_PROPERTY_TYPE_ascii_char_ptr(void)
{
    // arrange

    // act
    LOG_CONTEXT_PROPERTY_TYPE result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(ascii_char_ptr).get_type();

    // assert
    POOR_MANS_ASSERT(result == LOG_CONTEXT_PROPERTY_TYPE_ascii_char_ptr);
}

/* LOG_CONTEXT_PROPERTY_TYPE_INIT(ascii_char_ptr) */

static void ascii_char_ptr_LOG_CONTEXT_PROPERTY_TYPE_INIT_with_NULL_dst_value_fails(void)
{
    // arrange

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_INIT(ascii_char_ptr)(NULL, "cucu");

    // assert
    POOR_MANS_ASSERT(result != 0);
}

static void ascii_char_ptr_LOG_CONTEXT_PROPERTY_TYPE_INIT_with_NULL_format_fails(void)
{
    // arrange
    char buffer[5];

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_INIT(ascii_char_ptr)(buffer, NULL);

    // assert
    POOR_MANS_ASSERT(result != 0);
}

static void ascii_char_ptr_LOG_CONTEXT_PROPERTY_TYPE_INIT_succeeds(void)
{
    // arrange
    char buffer[5];

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_INIT(ascii_char_ptr)(buffer, "cucu");

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(strcmp(buffer, "cucu") == 0);
}

static void ascii_char_ptr_LOG_CONTEXT_PROPERTY_TYPE_INIT_with_multiple_args_succeeds(void)
{
    // arrange
    char buffer[43];

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_INIT(ascii_char_ptr)(buffer, "The answer is %d and let's say hello %s", 42, "world");

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(strcmp(buffer, "The answer is 42 and let's say hello world") == 0);
}

/* LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(ascii_char_ptr)*/

static void ascii_char_ptr_LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE_succeeds(void)
{
    // arrange

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(ascii_char_ptr)("cucu");

    // assert
    POOR_MANS_ASSERT(result == 5);
}

static void ascii_char_ptr_LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE_with_multiple_args_succeeds(void)
{
    // arrange

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(ascii_char_ptr)("The answer is %d and let's say hello %s", 42, "world");

    // assert
    POOR_MANS_ASSERT(result == 43);
}

/* very "poor man's" way of testing, as no test harness and mocking framework are available */
int main(void)
{
    ascii_char_ptr_to_string_with_NULL_property_value_fails();
    ascii_char_ptr_to_string_with_NULL_buffer_and_non_zero_buffer_length_fails();
    ascii_char_ptr_to_string_with_NULL_buffer_and_zero_buffer_length_returns_string_length();
    ascii_char_ptr_to_string_copies_the_string();
    to_string_with_truncation_succeeds();

    ascii_char_ptr_copy_with_NULL_src_value_fails();
    ascii_char_ptr_copy_with_NULL_dst_value_fails();
    ascii_char_ptr_copy_succeeds();

    ascii_char_ptr_free_returns();

    ascii_char_ptr_get_type_returns_LOG_CONTEXT_PROPERTY_TYPE_ascii_char_ptr();

    ascii_char_ptr_LOG_CONTEXT_PROPERTY_TYPE_INIT_with_NULL_dst_value_fails();
    ascii_char_ptr_LOG_CONTEXT_PROPERTY_TYPE_INIT_with_NULL_format_fails();
    ascii_char_ptr_LOG_CONTEXT_PROPERTY_TYPE_INIT_succeeds();
    ascii_char_ptr_LOG_CONTEXT_PROPERTY_TYPE_INIT_with_multiple_args_succeeds();

    ascii_char_ptr_LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE_succeeds();
    ascii_char_ptr_LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE_with_multiple_args_succeeds();

    return 0;
}
