// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "macro_utils/macro_utils.h"

#include "c_logging/log_context_property_type.h"
#include "c_logging/log_context_property_type_if.h"
#include "c_logging/log_context_property_boolean_type.h"

#define TEST_BUFFER_SIZE 1024

#define POOR_MANS_ASSERT(cond) \
    if (!(cond)) \
    { \
        (void)printf("%s:%d test failed\r\n", __FUNCTION__, __LINE__); \
        abort(); \
    } \

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).to_string */

static void bool_to_string_with_NULL_value_fails(void)
{
    // arrange
    char buffer[TEST_BUFFER_SIZE];

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).to_string(NULL, buffer, TEST_BUFFER_SIZE);

    // assert
    POOR_MANS_ASSERT(result < 0);
}

static void bool_to_string_succeeds(void)
{
    // arrange
    char buffer[TEST_BUFFER_SIZE];
    bool bool_value = 0;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).to_string(&bool_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 9);
    POOR_MANS_ASSERT(strcmp(buffer, "false (0)") == 0);
}

static void bool_to_string_INT32_MAX_succeeds(void)
{
    // arrange
    char buffer[TEST_BUFFER_SIZE];
    bool bool_value = INT32_MAX;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).to_string(&bool_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 8);
    POOR_MANS_ASSERT(strcmp(buffer, "true (1)") == 0);
}

static void bool_to_string_with_truncation_succeeds(void)
{
    // arrange
    char buffer[2];
    bool bool_value = 42;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).to_string(&bool_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 8);
    POOR_MANS_ASSERT(strcmp(buffer, "t") == 0);
}

static void bool_to_string_with_truncation_minus_1_succeeds(void)
{
    // arrange
    char buffer[2];
    bool bool_value = -1;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).to_string(&bool_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 8);
    POOR_MANS_ASSERT(strcmp(buffer, "t") == 0);
}

static void bool_to_string_with_negative_value_succeeds(void)
{
    // arrange
    char buffer[TEST_BUFFER_SIZE];
    bool bool_value = -1;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).to_string(&bool_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 8);
    POOR_MANS_ASSERT(strcmp(buffer, "true (1)") == 0);
}

static void bool_to_string_with_just_enough_big_buffer_succeeds(void)
{
    // arrange
    char buffer[2];
    bool bool_value = 1;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).to_string(&bool_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 8);
    POOR_MANS_ASSERT(strcmp(buffer, "t") == 0);
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).copy */

static void bool_copy_called_with_NULL_dst_value_fails(void)
{
    // arrange
    bool src = 42;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).copy(NULL, &src);

    // assert
    POOR_MANS_ASSERT(result != 0);
}

static void bool_copy_called_with_NULL_src_value_fails(void)
{
    // arrange
    bool dst;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).copy(&dst, NULL);

    // assert
    POOR_MANS_ASSERT(result != 0);
}

static void bool_copy_succeeds(void)
{
    // arrange
    bool src = 42;
    bool dst = 43;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).copy(&dst, &src);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(dst == 1);
}

static void bool_copy_succeeds_2(void)
{
    // arrange
    bool src = INT32_MAX;
    bool dst = 43;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).copy(&dst, &src);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(dst == 1);
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).free */

static void bool_free_returns(void)
{
    // arrange
    bool value = INT32_MAX;

    // act
    LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).free(&value);

    // assert
    // no explicit assert, no crash expected
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).get_type */

static void bool_get_type_returns_LOG_CONTEXT_PROPERTY_TYPE_bool(void)
{
    // arrange

    // act
    LOG_CONTEXT_PROPERTY_TYPE result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).get_type();

    // assert
    POOR_MANS_ASSERT(result == LOG_CONTEXT_PROPERTY_TYPE_bool);
}

/* LOG_CONTEXT_PROPERTY_TYPE_INIT(bool) */

static void MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(bool), _with_NULL_dst_value_fails)(void)
{
    // arrange

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_INIT(bool)(NULL, 42);

    // assert
    POOR_MANS_ASSERT(result != 0);
}

static void MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(bool), _with_bool_min_value_succeeds)(void)
{
    // arrange
    bool dst = 43;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_INIT(bool)(&dst, INT32_MIN);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(dst == 1);
}

static void MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(bool), _with_bool_max_value_succeeds)(void)
{
    // arrange
    bool dst = 43;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_INIT(bool)(&dst, INT32_MAX);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(dst == 1);
}

/* LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(bool) */

static void MU_C2(LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(bool), _succeeds)(void)
{
    // arrange

    // act
    size_t result = LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(bool)();

    // assert
    POOR_MANS_ASSERT(result == sizeof(bool));
}

/* very "poor man's" way of testing, as no test harness and mocking framework are available */
int main(void)
{
    bool_to_string_with_NULL_value_fails();
    bool_to_string_succeeds();
    bool_to_string_INT32_MAX_succeeds();
    bool_to_string_with_truncation_succeeds();
    bool_to_string_with_truncation_minus_1_succeeds();
    bool_to_string_with_negative_value_succeeds();
    bool_to_string_with_just_enough_big_buffer_succeeds();

    bool_copy_called_with_NULL_dst_value_fails();
    bool_copy_called_with_NULL_src_value_fails();
    bool_copy_succeeds();
    bool_copy_succeeds_2();

    bool_free_returns();

    bool_get_type_returns_LOG_CONTEXT_PROPERTY_TYPE_bool();

    MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(bool), _with_NULL_dst_value_fails)();
    MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(bool), _with_bool_min_value_succeeds)();
    MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(bool), _with_bool_max_value_succeeds)();

    MU_C2(LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(bool), _succeeds)();
}
