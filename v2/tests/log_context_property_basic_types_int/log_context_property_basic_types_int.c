// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <inttypes.h>

#include "macro_utils/macro_utils.h"

#include "c_logging/log_context_property_type.h"
#include "c_logging/log_context_property_type_if.h"
#include "c_logging/log_context_property_basic_types.h"

static size_t asserts_failed = 0;

#define TEST_BUFFER_SIZE 1024

#define POOR_MANS_ASSERT(cond) \
    if (!(cond)) \
    { \
        (void)printf("%s:%d test failed\r\n", __FUNCTION__, __LINE__); \
        asserts_failed++; \
    } \

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int64_t).to_string */

static void int64_t_to_string_with_NULL_value_fails(void)
{
    // arrange
    char buffer[TEST_BUFFER_SIZE];

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int64_t).to_string(NULL, buffer, TEST_BUFFER_SIZE);

    // assert
    POOR_MANS_ASSERT(result < 0);
}

static void int64_t_to_string_succeeds(void)
{
    // arrange
    char buffer[TEST_BUFFER_SIZE];
    int64_t int64_t_value = 0;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int64_t).to_string(&int64_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 1);
    POOR_MANS_ASSERT(strcmp(buffer, "0") == 0);
}

static void int64_t_to_string_INT64_MAX_succeeds(void)
{
    // arrange
    char buffer[TEST_BUFFER_SIZE];
    int64_t int64_t_value = INT64_MAX;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int64_t).to_string(&int64_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 19);
    POOR_MANS_ASSERT(strcmp(buffer, "9223372036854775807") == 0);
}

static void int64_t_to_string_with_truncation_succeeds(void)
{
    // arrange
    char buffer[2];
    int64_t int64_t_value = 42;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int64_t).to_string(&int64_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 2);
    POOR_MANS_ASSERT(strcmp(buffer, "4") == 0);
}

static void int64_t_to_string_with_truncation_minus_1_succeeds(void)
{
    // arrange
    char buffer[2];
    int64_t int64_t_value = -1;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int64_t).to_string(&int64_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 2);
    POOR_MANS_ASSERT(strcmp(buffer, "-") == 0);
}

static void int64_t_to_string_with_negative_value_succeeds(void)
{
    // arrange
    char buffer[TEST_BUFFER_SIZE];
    int64_t int64_t_value = -1;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int64_t).to_string(&int64_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 2);
    POOR_MANS_ASSERT(strcmp(buffer, "-1") == 0);
}

static void int64_t_to_string_with_just_enough_big_buffer_succeeds(void)
{
    // arrange
    char buffer[2];
    int64_t int64_t_value = 1;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int64_t).to_string(&int64_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 1);
    POOR_MANS_ASSERT(strcmp(buffer, "1") == 0);
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int64_t).copy */

static void int64_t_copy_called_with_NULL_dst_value_fails(void)
{
    // arrange
    int64_t src = 42;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int64_t).copy(NULL, &src);

    // assert
    POOR_MANS_ASSERT(result != 0);
}

static void int64_t_copy_called_with_NULL_src_value_fails(void)
{
    // arrange
    int64_t dst;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int64_t).copy(&dst, NULL);

    // assert
    POOR_MANS_ASSERT(result != 0);
}

static void int64_t_copy_succeeds(void)
{
    // arrange
    int64_t src = 42;
    int64_t dst = 43;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int64_t).copy(&dst, &src);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(dst == 42);
}

static void int64_t_copy_succeeds_2(void)
{
    // arrange
    int64_t src = INT64_MAX;
    int64_t dst = 43;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int64_t).copy(&dst, &src);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(dst == INT64_MAX);
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int64_t).free */

static void int64_t_free_returns(void)
{
    // arrange
    int64_t value = INT64_MAX;

    // act
    LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int64_t).free(&value);

    // assert
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int64_t).get_type */

static void int64_t_get_type_returns_LOG_CONTEXT_PROPERTY_TYPE_int64_t(void)
{
    // arrange

    // act
    LOG_CONTEXT_PROPERTY_TYPE result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int64_t).get_type();

    // assert
    POOR_MANS_ASSERT(result == LOG_CONTEXT_PROPERTY_TYPE_int64_t);
}

/* LOG_CONTEXT_PROPERTY_TYPE_INIT(int64_t) */

static void MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(int64_t), _with_NULL_dst_value_fails)(void)
{
    // arrange

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_INIT(int64_t)(NULL, 42);

    // assert
    POOR_MANS_ASSERT(result != 0);
}

static void MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(int64_t),_with_int64_t_min_value_succeeds)(void)
{
    // arrange
    int64_t dst = 43;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_INIT(int64_t)(&dst, INT64_MIN);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(dst == INT64_MIN);
}

static void MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(int64_t),_with_int64_t_max_value_succeeds)(void)
{
    // arrange
    int64_t dst = 43;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_INIT(int64_t)(&dst, INT64_MAX);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(dst == INT64_MAX);
}

/* LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(int64_t) */

static void MU_C2(LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(int64_t), _succeeds)(void)
{
    // arrange

    // act
    size_t result = LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(int64_t)();

    // assert
    POOR_MANS_ASSERT(result == sizeof(int64_t));
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint64_t).to_string */

static void uint64_t_to_string_with_NULL_value_fails(void)
{
    // arrange
    char buffer[TEST_BUFFER_SIZE];

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint64_t).to_string(NULL, buffer, TEST_BUFFER_SIZE);

    // assert
    POOR_MANS_ASSERT(result < 0);
}

static void uint64_t_to_string_succeeds(void)
{
    // arrange
    char buffer[TEST_BUFFER_SIZE];
    uint64_t uint64_t_value = 0;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint64_t).to_string(&uint64_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 1);
    POOR_MANS_ASSERT(strcmp(buffer, "0") == 0);
}

static void uint64_t_to_string_UINT64_MAX_succeeds(void)
{
    // arrange
    char buffer[TEST_BUFFER_SIZE];
    uint64_t uint64_t_value = UINT64_MAX;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint64_t).to_string(&uint64_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 20);
    POOR_MANS_ASSERT(strcmp(buffer, "18446744073709551615") == 0);
}

static void uint64_t_to_string_with_truncation_succeeds(void)
{
    // arrange
    char buffer[2];
    uint64_t uint64_t_value = 42;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint64_t).to_string(&uint64_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 2);
    POOR_MANS_ASSERT(strcmp(buffer, "4") == 0);
}

static void uint64_t_to_string_with_just_enough_big_buffer_succeeds(void)
{
    // arrange
    char buffer[2];
    uint64_t uint64_t_value = 1;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint64_t).to_string(&uint64_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 1);
    POOR_MANS_ASSERT(strcmp(buffer, "1") == 0);
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint64_t).copy */

static void uint64_t_copy_called_with_NULL_dst_value_fails(void)
{
    // arrange
    uint64_t src = 42;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint64_t).copy(NULL, &src);

    // assert
    POOR_MANS_ASSERT(result != 0);
}

static void uint64_t_copy_called_with_NULL_src_value_fails(void)
{
    // arrange
    uint64_t dst;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint64_t).copy(&dst, NULL);

    // assert
    POOR_MANS_ASSERT(result != 0);
}

static void uint64_t_copy_succeeds(void)
{
    // arrange
    uint64_t src = 42;
    uint64_t dst = 43;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint64_t).copy(&dst, &src);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(dst == 42);
}

static void uint64_t_copy_succeeds_2(void)
{
    // arrange
    uint64_t src = UINT64_MAX;
    uint64_t dst = 43;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint64_t).copy(&dst, &src);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(dst == UINT64_MAX);
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint64_t).free */

static void uint64_t_free_returns(void)
{
    // arrange
    uint64_t value = UINT64_MAX;

    // act
    LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint64_t).free(&value);

    // assert
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint64_t).get_type */

static void uint64_t_get_type_returns_LOG_CONTEXT_PROPERTY_TYPE_uint64_t(void)
{
    // arrange

    // act
    LOG_CONTEXT_PROPERTY_TYPE result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint64_t).get_type();

    // assert
    POOR_MANS_ASSERT(result == LOG_CONTEXT_PROPERTY_TYPE_uint64_t);
}

/* LOG_CONTEXT_PROPERTY_TYPE_INIT(uint64_t) */

static void MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(uint64_t), _with_NULL_dst_value_fails)(void)
{
    // arrange

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_INIT(uint64_t)(NULL, 42);

    // assert
    POOR_MANS_ASSERT(result != 0);
}

static void MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(uint64_t), _with_uint64_t_min_value_succeeds)(void)
{
    // arrange
    uint64_t dst = 43;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_INIT(uint64_t)(&dst, 0);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(dst == 0);
}

static void MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(uint64_t), _with_uint64_t_max_value_succeeds)(void)
{
    // arrange
    uint64_t dst = 43;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_INIT(uint64_t)(&dst, UINT64_MAX);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(dst == UINT64_MAX);
}

/* LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(uint64_t) */

static void MU_C2(LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(uint64_t), _succeeds)(void)
{
    // arrange

    // act
    size_t result = LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(uint64_t)();

    // assert
    POOR_MANS_ASSERT(result == sizeof(uint64_t));
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int32_t).to_string */

static void int32_t_to_string_with_NULL_value_fails(void)
{
    // arrange
    char buffer[TEST_BUFFER_SIZE];

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int32_t).to_string(NULL, buffer, TEST_BUFFER_SIZE);

    // assert
    POOR_MANS_ASSERT(result < 0);
}

static void int32_t_to_string_succeeds(void)
{
    // arrange
    char buffer[TEST_BUFFER_SIZE];
    int32_t int32_t_value = 0;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int32_t).to_string(&int32_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 1);
    POOR_MANS_ASSERT(strcmp(buffer, "0") == 0);
}

static void int32_t_to_string_INT32_MAX_succeeds(void)
{
    // arrange
    char buffer[TEST_BUFFER_SIZE];
    int32_t int32_t_value = INT32_MAX;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int32_t).to_string(&int32_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 10);
    POOR_MANS_ASSERT(strcmp(buffer, "2147483647") == 0);
}

static void int32_t_to_string_with_truncation_succeeds(void)
{
    // arrange
    char buffer[2];
    int32_t int32_t_value = 42;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int32_t).to_string(&int32_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 2);
    POOR_MANS_ASSERT(strcmp(buffer, "4") == 0);
}

static void int32_t_to_string_with_truncation_minus_1_succeeds(void)
{
    // arrange
    char buffer[2];
    int32_t int32_t_value = -1;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int32_t).to_string(&int32_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 2);
    POOR_MANS_ASSERT(strcmp(buffer, "-") == 0);
}

static void int32_t_to_string_with_negative_value_succeeds(void)
{
    // arrange
    char buffer[TEST_BUFFER_SIZE];
    int32_t int32_t_value = -1;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int32_t).to_string(&int32_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result > 0);
    POOR_MANS_ASSERT(strcmp(buffer, "-1") == 0);
}

static void int32_t_to_string_with_just_enough_big_buffer_succeeds(void)
{
    // arrange
    char buffer[2];
    int32_t int32_t_value = 1;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int32_t).to_string(&int32_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 1);
    POOR_MANS_ASSERT(strcmp(buffer, "1") == 0);
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int32_t).copy */

static void int32_t_copy_called_with_NULL_dst_value_fails(void)
{
    // arrange
    int32_t src = 42;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int32_t).copy(NULL, &src);

    // assert
    POOR_MANS_ASSERT(result != 0);
}

static void int32_t_copy_called_with_NULL_src_value_fails(void)
{
    // arrange
    int32_t dst;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int32_t).copy(&dst, NULL);

    // assert
    POOR_MANS_ASSERT(result != 0);
}

static void int32_t_copy_succeeds(void)
{
    // arrange
    int32_t src = 42;
    int32_t dst = 43;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int32_t).copy(&dst, &src);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(dst == 42);
}

static void int32_t_copy_succeeds_2(void)
{
    // arrange
    int32_t src = INT32_MAX;
    int32_t dst = 43;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int32_t).copy(&dst, &src);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(dst == INT32_MAX);
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int32_t).free */

static void int32_t_free_returns(void)
{
    // arrange
    int32_t value = INT32_MAX;

    // act
    LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int32_t).free(&value);

    // assert
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int32_t).get_type */

static void int32_t_get_type_returns_LOG_CONTEXT_PROPERTY_TYPE_int32_t(void)
{
    // arrange

    // act
    LOG_CONTEXT_PROPERTY_TYPE result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int32_t).get_type();

    // assert
    POOR_MANS_ASSERT(result == LOG_CONTEXT_PROPERTY_TYPE_int32_t);
}

/* LOG_CONTEXT_PROPERTY_TYPE_INIT(int32_t) */

static void MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(int32_t), _with_NULL_dst_value_fails)(void)
{
    // arrange

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_INIT(int32_t)(NULL, 42);

    // assert
    POOR_MANS_ASSERT(result != 0);
}

static void MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(int32_t), _with_int32_t_min_value_succeeds)(void)
{
    // arrange
    int32_t dst = 43;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_INIT(int32_t)(&dst, INT32_MIN);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(dst == INT32_MIN);
}

static void MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(int32_t), _with_int32_t_max_value_succeeds)(void)
{
    // arrange
    int32_t dst = 43;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_INIT(int32_t)(&dst, INT32_MAX);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(dst == INT32_MAX);
}

/* LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(int32_t) */

static void MU_C2(LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(int32_t), _succeeds)(void)
{
    // arrange

    // act
    size_t result = LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(int32_t)();

    // assert
    POOR_MANS_ASSERT(result == sizeof(int32_t));
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint32_t).to_string */

static void uint32_t_to_string_with_NULL_value_fails(void)
{
    // arrange
    char buffer[TEST_BUFFER_SIZE];

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint32_t).to_string(NULL, buffer, TEST_BUFFER_SIZE);

    // assert
    POOR_MANS_ASSERT(result < 0);
}

static void uint32_t_to_string_succeeds(void)
{
    // arrange
    char buffer[TEST_BUFFER_SIZE];
    uint32_t uint32_t_value = 0;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint32_t).to_string(&uint32_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 1);
    POOR_MANS_ASSERT(strcmp(buffer, "0") == 0);
}

static void uint32_t_to_string_UINT32_MAX_succeeds(void)
{
    // arrange
    char buffer[TEST_BUFFER_SIZE];
    uint32_t uint32_t_value = UINT32_MAX;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint32_t).to_string(&uint32_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 10);
    POOR_MANS_ASSERT(strcmp(buffer, "4294967295") == 0);
}

static void uint32_t_to_string_with_truncation_succeeds(void)
{
    // arrange
    char buffer[2];
    uint32_t uint32_t_value = 42;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint32_t).to_string(&uint32_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 2);
    POOR_MANS_ASSERT(strcmp(buffer, "4") == 0);
}

static void uint32_t_to_string_with_just_enough_big_buffer_succeeds(void)
{
    // arrange
    char buffer[2];
    uint32_t uint32_t_value = 1;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint32_t).to_string(&uint32_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 1);
    POOR_MANS_ASSERT(strcmp(buffer, "1") == 0);
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint32_t).copy */

static void uint32_t_copy_called_with_NULL_dst_value_fails(void)
{
    // arrange
    uint32_t src = 42;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint32_t).copy(NULL, &src);

    // assert
    POOR_MANS_ASSERT(result != 0);
}

static void uint32_t_copy_called_with_NULL_src_value_fails(void)
{
    // arrange
    uint32_t dst;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint32_t).copy(&dst, NULL);

    // assert
    POOR_MANS_ASSERT(result != 0);
}

static void uint32_t_copy_succeeds(void)
{
    // arrange
    uint32_t src = 42;
    uint32_t dst = 43;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint32_t).copy(&dst, &src);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(dst == 42);
}

static void uint32_t_copy_succeeds_2(void)
{
    // arrange
    uint32_t src = UINT32_MAX;
    uint32_t dst = 43;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint32_t).copy(&dst, &src);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(dst == UINT32_MAX);
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint32_t).free */

static void uint32_t_free_returns(void)
{
    // arrange
    uint32_t value = UINT32_MAX;

    // act
    LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint32_t).free(&value);

    // assert
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint32_t).get_type */

static void uint32_t_get_type_returns_LOG_CONTEXT_PROPERTY_TYPE_uint32_t(void)
{
    // arrange
    // act
    LOG_CONTEXT_PROPERTY_TYPE result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint32_t).get_type();

    // assert
    POOR_MANS_ASSERT(result == LOG_CONTEXT_PROPERTY_TYPE_uint32_t);
}

/* LOG_CONTEXT_PROPERTY_TYPE_INIT(uint32_t) */

static void MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(uint32_t), _with_NULL_dst_value_fails)(void)
{
    // arrange

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_INIT(uint32_t)(NULL, 42);

    // assert
    POOR_MANS_ASSERT(result != 0);
}

static void MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(uint32_t), _with_uint32_t_min_value_succeeds)(void)
{
    // arrange
    uint32_t dst = 43;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_INIT(uint32_t)(&dst, 0);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(dst == 0);
}

static void MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(uint32_t), _with_uint32_t_max_value_succeeds)(void)
{
    // arrange
    uint32_t dst = 43;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_INIT(uint32_t)(&dst, UINT32_MAX);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(dst == UINT32_MAX);
}

/* LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(uint32_t) */

static void MU_C2(LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(uint32_t), _succeeds)(void)
{
    // arrange

    // act
    size_t result = LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(uint32_t)();

    // assert
    POOR_MANS_ASSERT(result == sizeof(uint32_t));
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int16_t).to_string */

static void int16_t_to_string_with_NULL_value_fails(void)
{
    // arrange
    char buffer[TEST_BUFFER_SIZE];

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int16_t).to_string(NULL, buffer, TEST_BUFFER_SIZE);

    // assert
    POOR_MANS_ASSERT(result < 0);
}

static void int16_t_to_string_succeeds(void)
{
    // arrange
    char buffer[TEST_BUFFER_SIZE];
    int16_t int16_t_value = 0;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int16_t).to_string(&int16_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 1);
    POOR_MANS_ASSERT(strcmp(buffer, "0") == 0);
}

static void int16_t_to_string_INT16_MAX_succeeds(void)
{
    // arrange
    char buffer[TEST_BUFFER_SIZE];
    int16_t int16_t_value = INT16_MAX;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int16_t).to_string(&int16_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 5);
    POOR_MANS_ASSERT(strcmp(buffer, "32767") == 0);
}

static void int16_t_to_string_with_truncation_succeeds(void)
{
    // arrange
    char buffer[2];
    int16_t int16_t_value = 42;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int16_t).to_string(&int16_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 2);
    POOR_MANS_ASSERT(strcmp(buffer, "4") == 0);
}

static void int16_t_to_string_with_truncation_minus_1_succeeds(void)
{
    // arrange
    char buffer[2];
    int16_t int16_t_value = -1;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int16_t).to_string(&int16_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 2);
    POOR_MANS_ASSERT(strcmp(buffer, "-") == 0);
}

static void int16_t_to_string_with_negative_value_succeeds(void)
{
    // arrange
    char buffer[TEST_BUFFER_SIZE];
    int16_t int16_t_value = -1;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int16_t).to_string(&int16_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result > 0);
    POOR_MANS_ASSERT(strcmp(buffer, "-1") == 0);
}

static void int16_t_to_string_with_just_enough_big_buffer_succeeds(void)
{
    // arrange
    char buffer[2];
    int16_t int16_t_value = 1;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int16_t).to_string(&int16_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 1);
    POOR_MANS_ASSERT(strcmp(buffer, "1") == 0);
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int16_t).copy */

static void int16_t_copy_called_with_NULL_dst_value_fails(void)
{
    // arrange
    int16_t src = 42;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int16_t).copy(NULL, &src);

    // assert
    POOR_MANS_ASSERT(result != 0);
}

static void int16_t_copy_called_with_NULL_src_value_fails(void)
{
    // arrange
    int16_t dst;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int16_t).copy(&dst, NULL);

    // assert
    POOR_MANS_ASSERT(result != 0);
}

static void int16_t_copy_succeeds(void)
{
    // arrange
    int16_t src = 42;
    int16_t dst = 43;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int16_t).copy(&dst, &src);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(dst == 42);
}

static void int16_t_copy_succeeds_2(void)
{
    // arrange
    int16_t src = INT16_MAX;
    int16_t dst = 43;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int16_t).copy(&dst, &src);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(dst == INT16_MAX);
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int16_t).free */

static void int16_t_free_returns(void)
{
    // arrange
    int16_t value = INT16_MAX;

    // act
    LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int16_t).free(&value);

    // assert
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int16_t).get_type */

static void int16_t_get_type_returns_LOG_CONTEXT_PROPERTY_TYPE_int16_t(void)
{
    // arrange

    // act
    LOG_CONTEXT_PROPERTY_TYPE result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int16_t).get_type();

    // assert
    POOR_MANS_ASSERT(result == LOG_CONTEXT_PROPERTY_TYPE_int16_t);
}

/* LOG_CONTEXT_PROPERTY_TYPE_INIT(int16_t) */

static void MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(int16_t), _with_NULL_dst_value_fails)(void)
{
    // arrange

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_INIT(int16_t)(NULL, 42);

    // assert
    POOR_MANS_ASSERT(result != 0);
}

static void MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(int16_t), _with_int16_t_min_value_succeeds)(void)
{
    // arrange
    int16_t dst = 43;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_INIT(int16_t)(&dst, INT16_MIN);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(dst == INT16_MIN);
}

static void MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(int16_t), _with_int16_t_max_value_succeeds)(void)
{
    // arrange
    int16_t dst = 43;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_INIT(int16_t)(&dst, INT16_MAX);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(dst == INT16_MAX);
}

/* LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(int16_t) */

static void MU_C2(LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(int16_t), _succeeds)(void)
{
    // arrange

    // act
    size_t result = LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(int16_t)();

    // assert
    POOR_MANS_ASSERT(result == sizeof(int16_t));
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint16_t).to_string */

static void uint16_t_to_string_with_NULL_value_fails(void)
{
    // arrange
    char buffer[TEST_BUFFER_SIZE];

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint16_t).to_string(NULL, buffer, TEST_BUFFER_SIZE);

    // assert
    POOR_MANS_ASSERT(result < 0);
}

static void uint16_t_to_string_succeeds(void)
{
    // arrange
    char buffer[TEST_BUFFER_SIZE];
    uint16_t uint16_t_value = 0;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint16_t).to_string(&uint16_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 1);
    POOR_MANS_ASSERT(strcmp(buffer, "0") == 0);
}

static void uint16_t_to_string_UINT16_MAX_succeeds(void)
{
    // arrange
    char buffer[TEST_BUFFER_SIZE];
    uint16_t uint16_t_value = UINT16_MAX;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint16_t).to_string(&uint16_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 5);
    POOR_MANS_ASSERT(strcmp(buffer, "65535") == 0);
}

static void uint16_t_to_string_with_truncation_succeeds(void)
{
    // arrange
    char buffer[2];
    uint16_t uint16_t_value = 42;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint16_t).to_string(&uint16_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 2);
    POOR_MANS_ASSERT(strcmp(buffer, "4") == 0);
}

static void uint16_t_to_string_with_just_enough_big_buffer_succeeds(void)
{
    // arrange
    char buffer[2];
    uint16_t uint16_t_value = 1;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint16_t).to_string(&uint16_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 1);
    POOR_MANS_ASSERT(strcmp(buffer, "1") == 0);
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint16_t).copy */

static void uint16_t_copy_called_with_NULL_dst_value_fails(void)
{
    // arrange
    uint16_t src = 42;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint16_t).copy(NULL, &src);

    // assert
    POOR_MANS_ASSERT(result != 0);
}

static void uint16_t_copy_called_with_NULL_src_value_fails(void)
{
    // arrange
    uint16_t dst;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint16_t).copy(&dst, NULL);

    // assert
    POOR_MANS_ASSERT(result != 0);
}

static void uint16_t_copy_succeeds(void)
{
    // arrange
    uint16_t src = 42;
    uint16_t dst = 43;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint16_t).copy(&dst, &src);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(dst == 42);
}

static void uint16_t_copy_succeeds_2(void)
{
    // arrange
    uint16_t src = UINT16_MAX;
    uint16_t dst = 43;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint16_t).copy(&dst, &src);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(dst == UINT16_MAX);
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint16_t).free */

static void uint16_t_free_returns(void)
{
    // arrange
    uint16_t value = UINT16_MAX;

    // act
    LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint16_t).free(&value);

    // assert
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint16_t).get_type */

static void uint16_t_get_type_returns_LOG_CONTEXT_PROPERTY_TYPE_uint16_t(void)
{
    // arrange

    // act
    LOG_CONTEXT_PROPERTY_TYPE result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint16_t).get_type();

    // assert
    POOR_MANS_ASSERT(result == LOG_CONTEXT_PROPERTY_TYPE_uint16_t);
}

/* LOG_CONTEXT_PROPERTY_TYPE_INIT(uint16_t) */

static void MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(uint16_t), _with_NULL_dst_value_fails)(void)
{
    // arrange

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_INIT(uint16_t)(NULL, 42);

    // assert
    POOR_MANS_ASSERT(result != 0);
}

static void MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(uint16_t), _with_uint16_t_min_value_succeeds)(void)
{
    // arrange
    uint16_t dst = 43;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_INIT(uint16_t)(&dst, 0);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(dst == 0);
}

static void MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(uint16_t), _with_uint16_t_max_value_succeeds)(void)
{
    // arrange
    uint16_t dst = 43;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_INIT(uint16_t)(&dst, UINT16_MAX);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(dst == UINT16_MAX);
}

/* LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(uint16_t) */

static void MU_C2(LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(uint16_t), _succeeds)(void)
{
    // arrange

    // act
    size_t result = LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(uint16_t)();

    // assert
    POOR_MANS_ASSERT(result == sizeof(uint16_t));
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int8_t).to_string */

static void int8_t_to_string_with_NULL_value_fails(void)
{
    // arrange
    char buffer[TEST_BUFFER_SIZE];

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int8_t).to_string(NULL, buffer, TEST_BUFFER_SIZE);

    // assert
    POOR_MANS_ASSERT(result < 0);
}

static void int8_t_to_string_succeeds(void)
{
    // arrange
    char buffer[TEST_BUFFER_SIZE];
    int8_t int8_t_value = 0;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int8_t).to_string(&int8_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 1);
    POOR_MANS_ASSERT(strcmp(buffer, "0") == 0);
}

static void int8_t_to_string_INT8_MAX_succeeds(void)
{
    // arrange
    char buffer[TEST_BUFFER_SIZE];
    int8_t int8_t_value = INT8_MAX;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int8_t).to_string(&int8_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 3);
    POOR_MANS_ASSERT(strcmp(buffer, "127") == 0);
}

static void int8_t_to_string_with_truncation_succeeds(void)
{
    // arrange
    char buffer[2];
    int8_t int8_t_value = 42;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int8_t).to_string(&int8_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 2);
    POOR_MANS_ASSERT(strcmp(buffer, "4") == 0);
}

static void int8_t_to_string_with_truncation_minus_1_succeeds(void)
{
    // arrange
    char buffer[2];
    int8_t int8_t_value = -1;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int8_t).to_string(&int8_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 2);
    POOR_MANS_ASSERT(strcmp(buffer, "-") == 0);
}

static void int8_t_to_string_with_negative_value_succeeds(void)
{
    // arrange
    char buffer[TEST_BUFFER_SIZE];
    int8_t int8_t_value = -1;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int8_t).to_string(&int8_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result > 0);
    POOR_MANS_ASSERT(strcmp(buffer, "-1") == 0);
}

static void int8_t_to_string_with_just_enough_big_buffer_succeeds(void)
{
    // arrange
    char buffer[2];
    int8_t int8_t_value = 1;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int8_t).to_string(&int8_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 1);
    POOR_MANS_ASSERT(strcmp(buffer, "1") == 0);
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int8_t).copy */

static void int8_t_copy_called_with_NULL_dst_value_fails(void)
{
    // arrange
    int8_t src = 42;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int8_t).copy(NULL, &src);

    // assert
    POOR_MANS_ASSERT(result != 0);
}

static void int8_t_copy_called_with_NULL_src_value_fails(void)
{
    // arrange
    int8_t dst;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int8_t).copy(&dst, NULL);

    // assert
    POOR_MANS_ASSERT(result != 0);
}

static void int8_t_copy_succeeds(void)
{
    // arrange
    int8_t src = 42;
    int8_t dst = 43;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int8_t).copy(&dst, &src);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(dst == 42);
}

static void int8_t_copy_succeeds_2(void)
{
    // arrange
    int8_t src = INT8_MAX;
    int8_t dst = 43;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int8_t).copy(&dst, &src);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(dst == INT8_MAX);
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int8_t).free */

static void int8_t_free_returns(void)
{
    // arrange
    int8_t value = INT8_MAX;

    // act
    LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int8_t).free(&value);

    // assert
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int8_t).get_type */

static void int8_t_get_type_returns_LOG_CONTEXT_PROPERTY_TYPE_int8_t(void)
{
    // arrange

    // act
    LOG_CONTEXT_PROPERTY_TYPE result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(int8_t).get_type();

    // assert
    POOR_MANS_ASSERT(result == LOG_CONTEXT_PROPERTY_TYPE_int8_t);
}

/* LOG_CONTEXT_PROPERTY_TYPE_INIT(int8_t) */

static void MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(int8_t), _with_NULL_dst_value_fails)(void)
{
    // arrange

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_INIT(int8_t)(NULL, 42);

    // assert
    POOR_MANS_ASSERT(result != 0);
}

static void MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(int8_t), _with_int8_t_min_value_succeeds)(void)
{
    // arrange
    int8_t dst = 43;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_INIT(int8_t)(&dst, INT8_MIN);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(dst == INT8_MIN);
}

static void MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(int8_t), _with_int8_t_max_value_succeeds)(void)
{
    // arrange
    int8_t dst = 43;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_INIT(int8_t)(&dst, INT8_MAX);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(dst == INT8_MAX);
}

/* LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(int8_t) */

static void MU_C2(LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(int8_t), _succeeds)(void)
{
    // arrange

    // act
    size_t result = LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(int8_t)();

    // assert
    POOR_MANS_ASSERT(result == sizeof(int8_t));
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint8_t).to_string */

static void uint8_t_to_string_with_NULL_value_fails(void)
{
    // arrange
    char buffer[TEST_BUFFER_SIZE];

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint8_t).to_string(NULL, buffer, TEST_BUFFER_SIZE);

    // assert
    POOR_MANS_ASSERT(result < 0);
}

static void uint8_t_to_string_succeeds(void)
{
    // arrange
    char buffer[TEST_BUFFER_SIZE];
    uint8_t uint8_t_value = 0;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint8_t).to_string(&uint8_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 1);
    POOR_MANS_ASSERT(strcmp(buffer, "0") == 0);
}

static void uint8_t_to_string_UINT8_MAX_succeeds(void)
{
    // arrange
    char buffer[TEST_BUFFER_SIZE];
    uint8_t uint8_t_value = UINT8_MAX;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint8_t).to_string(&uint8_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 3);
    POOR_MANS_ASSERT(strcmp(buffer, "255") == 0);
}

static void uint8_t_to_string_with_truncation_succeeds(void)
{
    // arrange
    char buffer[2];
    uint8_t uint8_t_value = 42;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint8_t).to_string(&uint8_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 2);
    POOR_MANS_ASSERT(strcmp(buffer, "4") == 0);
}

static void uint8_t_to_string_with_just_enough_big_buffer_succeeds(void)
{
    // arrange
    char buffer[2];
    uint8_t uint8_t_value = 1;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint8_t).to_string(&uint8_t_value, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 1);
    POOR_MANS_ASSERT(strcmp(buffer, "1") == 0);
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint8_t).copy */

static void uint8_t_copy_called_with_NULL_dst_value_fails(void)
{
    // arrange
    uint8_t src = 42;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint8_t).copy(NULL, &src);

    // assert
    POOR_MANS_ASSERT(result != 0);
}

static void uint8_t_copy_called_with_NULL_src_value_fails(void)
{
    // arrange
    uint8_t dst;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint8_t).copy(&dst, NULL);

    // assert
    POOR_MANS_ASSERT(result != 0);
}

static void uint8_t_copy_succeeds(void)
{
    // arrange
    uint8_t src = 42;
    uint8_t dst = 43;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint8_t).copy(&dst, &src);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(dst == 42);
}

static void uint8_t_copy_succeeds_2(void)
{
    // arrange
    uint8_t src = UINT8_MAX;
    uint8_t dst = 43;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint8_t).copy(&dst, &src);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(dst == UINT8_MAX);
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint8_t).free */

static void uint8_t_free_returns(void)
{
    // arrange
    uint8_t value = UINT8_MAX;

    // act
    LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint8_t).free(&value);

    // assert
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint8_t).get_type */

static void uint8_t_get_type_returns_LOG_CONTEXT_PROPERTY_TYPE_uint8_t(void)
{
    // arrange

    // act
    LOG_CONTEXT_PROPERTY_TYPE result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(uint8_t).get_type();

    // assert
    POOR_MANS_ASSERT(result == LOG_CONTEXT_PROPERTY_TYPE_uint8_t);
}

/* LOG_CONTEXT_PROPERTY_TYPE_INIT(uint8_t) */

static void MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(uint8_t), _with_NULL_dst_value_fails)(void)
{
    // arrange

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_INIT(uint8_t)(NULL, 42);

    // assert
    POOR_MANS_ASSERT(result != 0);
}

static void MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(uint8_t), _with_uint8_t_min_value_succeeds)(void)
{
    // arrange
    uint8_t dst = 43;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_INIT(uint8_t)(&dst, 0);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(dst == 0);
}

static void MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(uint8_t), _with_uint8_t_max_value_succeeds)(void)
{
    // arrange
    uint8_t dst = 43;

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_INIT(uint8_t)(&dst, UINT8_MAX);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(dst == UINT8_MAX);
}

/* LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(uint8_t) */

static void MU_C2(LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(uint8_t), _succeeds)(void)
{
    // arrange

    // act
    size_t result = LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(uint8_t)();

    // assert
    POOR_MANS_ASSERT(result == sizeof(uint8_t));
}

/* very "poor man's" way of testing, as no test harness and mocking framework are available */
int main(void)
{
    int64_t_to_string_with_NULL_value_fails();
    int64_t_to_string_succeeds();
    int64_t_to_string_INT64_MAX_succeeds();
    int64_t_to_string_with_truncation_succeeds();
    int64_t_to_string_with_truncation_minus_1_succeeds();
    int64_t_to_string_with_negative_value_succeeds();
    int64_t_to_string_with_just_enough_big_buffer_succeeds();

    int64_t_copy_called_with_NULL_dst_value_fails();
    int64_t_copy_called_with_NULL_src_value_fails();
    int64_t_copy_succeeds();
    int64_t_copy_succeeds_2();

    int64_t_free_returns();

    int64_t_get_type_returns_LOG_CONTEXT_PROPERTY_TYPE_int64_t();

    MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(int64_t), _with_NULL_dst_value_fails)();
    MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(int64_t), _with_int64_t_min_value_succeeds)();
    MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(int64_t), _with_int64_t_max_value_succeeds)();

    MU_C2(LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(int64_t), _succeeds)();

    uint64_t_to_string_with_NULL_value_fails();
    uint64_t_to_string_succeeds();
    uint64_t_to_string_UINT64_MAX_succeeds();
    uint64_t_to_string_with_truncation_succeeds();
    uint64_t_to_string_with_just_enough_big_buffer_succeeds();

    uint64_t_copy_called_with_NULL_dst_value_fails();
    uint64_t_copy_called_with_NULL_src_value_fails();
    uint64_t_copy_succeeds();
    uint64_t_copy_succeeds_2();

    uint64_t_free_returns();

    uint64_t_get_type_returns_LOG_CONTEXT_PROPERTY_TYPE_uint64_t();

    MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(uint64_t), _with_NULL_dst_value_fails)();
    MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(uint64_t), _with_uint64_t_min_value_succeeds)();
    MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(uint64_t), _with_uint64_t_max_value_succeeds)();

    MU_C2(LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(uint64_t), _succeeds)();

    int32_t_to_string_with_NULL_value_fails();
    int32_t_to_string_succeeds();
    int32_t_to_string_INT32_MAX_succeeds();
    int32_t_to_string_with_truncation_succeeds();
    int32_t_to_string_with_truncation_minus_1_succeeds();
    int32_t_to_string_with_negative_value_succeeds();
    int32_t_to_string_with_just_enough_big_buffer_succeeds();

    int32_t_copy_called_with_NULL_dst_value_fails();
    int32_t_copy_called_with_NULL_src_value_fails();
    int32_t_copy_succeeds();
    int32_t_copy_succeeds_2();

    int32_t_free_returns();

    int32_t_get_type_returns_LOG_CONTEXT_PROPERTY_TYPE_int32_t();

    MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(int32_t), _with_NULL_dst_value_fails)();
    MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(int32_t), _with_int32_t_min_value_succeeds)();
    MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(int32_t), _with_int32_t_max_value_succeeds)();

    MU_C2(LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(int32_t), _succeeds)();

    uint32_t_to_string_with_NULL_value_fails();
    uint32_t_to_string_succeeds();
    uint32_t_to_string_UINT32_MAX_succeeds();
    uint32_t_to_string_with_truncation_succeeds();
    uint32_t_to_string_with_just_enough_big_buffer_succeeds();

    uint32_t_copy_called_with_NULL_dst_value_fails();
    uint32_t_copy_called_with_NULL_src_value_fails();
    uint32_t_copy_succeeds();
    uint32_t_copy_succeeds_2();

    uint32_t_free_returns();

    uint32_t_get_type_returns_LOG_CONTEXT_PROPERTY_TYPE_uint32_t();

    MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(uint32_t), _with_NULL_dst_value_fails)();
    MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(uint32_t), _with_uint32_t_min_value_succeeds)();
    MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(uint32_t), _with_uint32_t_max_value_succeeds)();

    MU_C2(LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(uint32_t), _succeeds)();

    int16_t_to_string_with_NULL_value_fails();
    int16_t_to_string_succeeds();
    int16_t_to_string_INT16_MAX_succeeds();
    int16_t_to_string_with_truncation_succeeds();
    int16_t_to_string_with_truncation_minus_1_succeeds();
    int16_t_to_string_with_negative_value_succeeds();
    int16_t_to_string_with_just_enough_big_buffer_succeeds();

    int16_t_copy_called_with_NULL_dst_value_fails();
    int16_t_copy_called_with_NULL_src_value_fails();
    int16_t_copy_succeeds();
    int16_t_copy_succeeds_2();

    int16_t_free_returns();

    int16_t_get_type_returns_LOG_CONTEXT_PROPERTY_TYPE_int16_t();

    MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(int16_t), _with_NULL_dst_value_fails)();
    MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(int16_t), _with_int16_t_min_value_succeeds)();
    MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(int16_t), _with_int16_t_max_value_succeeds)();

    MU_C2(LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(int16_t), _succeeds)();

    uint16_t_to_string_with_NULL_value_fails();
    uint16_t_to_string_succeeds();
    uint16_t_to_string_UINT16_MAX_succeeds();
    uint16_t_to_string_with_truncation_succeeds();
    uint16_t_to_string_with_just_enough_big_buffer_succeeds();

    uint16_t_copy_called_with_NULL_dst_value_fails();
    uint16_t_copy_called_with_NULL_src_value_fails();
    uint16_t_copy_succeeds();
    uint16_t_copy_succeeds_2();

    uint16_t_free_returns();

    uint16_t_get_type_returns_LOG_CONTEXT_PROPERTY_TYPE_uint16_t();

    MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(uint16_t), _with_NULL_dst_value_fails)();
    MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(uint16_t), _with_uint16_t_min_value_succeeds)();
    MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(uint16_t), _with_uint16_t_max_value_succeeds)();

    MU_C2(LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(uint16_t), _succeeds)();

    int8_t_to_string_with_NULL_value_fails();
    int8_t_to_string_succeeds();
    int8_t_to_string_INT8_MAX_succeeds();
    int8_t_to_string_with_truncation_succeeds();
    int8_t_to_string_with_truncation_minus_1_succeeds();
    int8_t_to_string_with_negative_value_succeeds();
    int8_t_to_string_with_just_enough_big_buffer_succeeds();

    int8_t_copy_called_with_NULL_dst_value_fails();
    int8_t_copy_called_with_NULL_src_value_fails();
    int8_t_copy_succeeds();
    int8_t_copy_succeeds_2();

    int8_t_free_returns();

    int8_t_get_type_returns_LOG_CONTEXT_PROPERTY_TYPE_int8_t();

    MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(int8_t), _with_NULL_dst_value_fails)();
    MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(int8_t), _with_int8_t_min_value_succeeds)();
    MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(int8_t), _with_int8_t_max_value_succeeds)();

    MU_C2(LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(int8_t), _succeeds)();

    uint8_t_to_string_with_NULL_value_fails();
    uint8_t_to_string_succeeds();
    uint8_t_to_string_UINT8_MAX_succeeds();
    uint8_t_to_string_with_truncation_succeeds();
    uint8_t_to_string_with_just_enough_big_buffer_succeeds();

    uint8_t_copy_called_with_NULL_dst_value_fails();
    uint8_t_copy_called_with_NULL_src_value_fails();
    uint8_t_copy_succeeds();
    uint8_t_copy_succeeds_2();

    uint8_t_free_returns();

    uint8_t_get_type_returns_LOG_CONTEXT_PROPERTY_TYPE_uint8_t();

    MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(uint8_t), _with_NULL_dst_value_fails)();
    MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(uint8_t), _with_uint8_t_min_value_succeeds)();
    MU_C2(LOG_CONTEXT_PROPERTY_TYPE_INIT(uint8_t), _with_uint8_t_max_value_succeeds)();

    MU_C2(LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(uint8_t), _succeeds)();

    return asserts_failed;
}
