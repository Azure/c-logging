// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "macro_utils/macro_utils.h"

#include "c_logging/log_context_property_type.h"
#include "c_logging/log_context_property_type_if.h"
#include "c_logging/log_context_property_bool_type.h"

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

static void bool_false_to_string_succeeds(void)
{
    // arrange
    char buffer[TEST_BUFFER_SIZE];
    bool bool_value = false;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).to_string(&bool_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 9);
    char bool_value_buffer[10];
    (void)snprintf(bool_value_buffer, 10, "%" PRI_BOOL, MU_BOOL_VALUE(bool_value));
    POOR_MANS_ASSERT(strcmp(buffer, bool_value_buffer) == 0);
}

static void bool_true_to_string_succeeds(void)
{
    // arrange
    char buffer[TEST_BUFFER_SIZE];
    bool bool_value = true;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).to_string(&bool_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 8);
    char bool_value_buffer[9];
    (void)snprintf(bool_value_buffer, 9, "%" PRI_BOOL, MU_BOOL_VALUE(bool_value));
    POOR_MANS_ASSERT(strcmp(buffer, bool_value_buffer) == 0);
}

static void bool_true_to_string_with_just_enough_big_buffer_succeeds(void)
{
    // arrange
    char buffer[2];
    bool bool_value = true;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).to_string(&bool_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 8);
    char bool_value_buffer[2];
    (void)snprintf(bool_value_buffer, 2, "%" PRI_BOOL, MU_BOOL_VALUE(bool_value));
    POOR_MANS_ASSERT(strcmp(buffer, bool_value_buffer) == 0);
}

static void bool_false_to_string_with_just_enough_big_buffer_succeeds(void)
{
    // arrange
    char buffer[2];
    bool bool_value = false;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).to_string(&bool_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 9);
    char bool_value_buffer[2];
    (void)snprintf(bool_value_buffer, 2, "%" PRI_BOOL, MU_BOOL_VALUE(bool_value));
    POOR_MANS_ASSERT(strcmp(buffer, bool_value_buffer) == 0);
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).copy */

static void bool_copy_called_with_NULL_dst_value_fails(void)
{
    // arrange
    bool src = true;

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

static void bool_copy_true_succeeds(void)
{
    // arrange
    bool src = true;
    bool dst = false;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).copy(&dst, &src);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(dst == true);
}

static void bool_copy_false_succeeds(void)
{
    // arrange
    bool src = false;
    bool dst = true;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).copy(&dst, &src);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(dst == false);
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).free */

static void bool_free_true_returns(void)
{
    // arrange
    bool value = true;

    // act
    LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool).free(&value);

    // assert
    // no explicit assert, no crash expected
}

static void bool_free_false_returns(void)
{
    // arrange
    bool value = false;

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
    int result = LOG_CONTEXT_PROPERTY_TYPE_INIT(bool)(NULL, true);

    // assert
    POOR_MANS_ASSERT(result != 0);
}

static void MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(bool), _with_bool_true_succeeds)(void)
{
    // arrange
    bool dst = false;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_INIT(bool)(&dst, true);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(dst == true);
}

static void MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(bool), _with_bool_false_succeeds)(void)
{
    // arrange
    bool dst = true;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_INIT(bool)(&dst, false);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(dst == false);
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
    bool_true_to_string_succeeds();
    bool_false_to_string_succeeds();
    bool_true_to_string_with_just_enough_big_buffer_succeeds();
    bool_false_to_string_with_just_enough_big_buffer_succeeds();

    bool_copy_called_with_NULL_dst_value_fails();
    bool_copy_called_with_NULL_src_value_fails();
    bool_copy_true_succeeds();
    bool_copy_false_succeeds();

    bool_free_true_returns();
    bool_free_false_returns();

    bool_get_type_returns_LOG_CONTEXT_PROPERTY_TYPE_bool();

    MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(bool), _with_NULL_dst_value_fails)();
    MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(bool), _with_bool_true_succeeds)();
    MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(bool), _with_bool_false_succeeds)();

    MU_C2(LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(bool), _succeeds)();
}
