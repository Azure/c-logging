// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include "c_logging/log_context_property_type.h"
#include "c_logging/log_context_property_type_if.h"
#include "c_logging/log_context_property_type_struct.h"

static size_t asserts_failed = 0;

#define POOR_MANS_ASSERT(cond) \
    if (!(cond)) \
    { \
        (void)printf("%s:%d test failed\r\n", __FUNCTION__, __LINE__); \
        asserts_failed++; \
    } \

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(struct).to_string */

/* Tests_SRS_LOG_CONTEXT_PROPERTY_TYPE_STRUCT_01_001: [ If property_value is NULL, LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(struct).to_string shall fail and return a negative value. ]*/
static void struct_to_string_with_NULL_property_value_fails(void)
{
    // arrange
    char buffer[128];

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(struct).to_string(NULL, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result < 0);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_TYPE_STRUCT_01_003: [ If buffer is NULL, LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(struct).to_string shall return 0 (the length of an empty string). ]*/
static void struct_to_string_with_NULL_buffer_and_0_size_returns_0(void)
{
    // arrange
    uint8_t data[1];

    POOR_MANS_ASSERT(LOG_CONTEXT_PROPERTY_TYPE_INIT(struct)(data, 1) == 0);

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(struct).to_string(data, NULL, 0);

    // assert
    POOR_MANS_ASSERT(result == 0);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_TYPE_STRUCT_01_003: [ If buffer is NULL, LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(struct).to_string shall return 0 (the length of an empty string). ]*/
static void struct_to_string_with_NULL_buffer_and_positive_size_returns_0(void)
{
    // arrange
    uint8_t data[1];

    POOR_MANS_ASSERT(LOG_CONTEXT_PROPERTY_TYPE_INIT(struct)(data, 1) == 0);

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(struct).to_string(data, NULL, 1);

    // assert
    POOR_MANS_ASSERT(result == 0);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_TYPE_STRUCT_01_015: [ If buffer_length is 0, LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(struct).to_string shall return 0 (the length of an empty string). ]*/
static void struct_to_string_with_non_NULL_buffer_and_zero_buffer_length_returns_0(void)
{
    // arrange
    char buffer[1] = { 0x42 };
    uint8_t data[1];

    POOR_MANS_ASSERT(LOG_CONTEXT_PROPERTY_TYPE_INIT(struct)(data, 1) == 0);

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(struct).to_string(data, buffer, 0);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(buffer[0] == 0x42);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_TYPE_STRUCT_01_004: [ Otherwise, LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(struct).to_string shall write 0 as the first element in buffer. ]*/
static void struct_to_string_succeeds(void)
{
    // arrange
    char buffer[1];
    uint8_t data[1];

    POOR_MANS_ASSERT(LOG_CONTEXT_PROPERTY_TYPE_INIT(struct)(data, 1) == 0);

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(struct).to_string(data, buffer, sizeof(buffer));

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(strcmp(buffer, "") == 0);
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(struct).copy */

/* Tests_SRS_LOG_CONTEXT_PROPERTY_TYPE_STRUCT_01_005: [ If src_value is NULL, LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(struct).copy shall fail and return a non-zero value. ]*/
static void struct_copy_with_NULL_src_value_fails(void)
{
    // arrange
    uint8_t dst_value[1];

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(struct).copy(dst_value, NULL);

    // assert
    POOR_MANS_ASSERT(result != 0);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_TYPE_STRUCT_01_006: [ If dst_value is NULL, LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(struct).copy shall fail and return a non-zero value. ]*/
static void struct_copy_with_NULL_dst_value_fails(void)
{
    // arrange
    uint8_t src_value[1];

    POOR_MANS_ASSERT(LOG_CONTEXT_PROPERTY_TYPE_INIT(struct)(src_value, 1) == 0);

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(struct).copy(NULL, src_value);

    // assert
    POOR_MANS_ASSERT(result != 0);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_TYPE_STRUCT_01_007: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(struct).copy shall copy the number of fields associated with the structure from src_value to dst_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_TYPE_STRUCT_01_008: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(struct).copy shall succeed and return 0. ]*/
static void struct_copy_copies_one_byte_with_the_field_count(void)
{
    // arrange
    uint8_t src_value[1];
    uint8_t dst_value[1];

    POOR_MANS_ASSERT(LOG_CONTEXT_PROPERTY_TYPE_INIT(struct)(src_value, 42) == 0);

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(struct).copy(dst_value, src_value);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(dst_value[0] == 42);
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(struct).free */

/* Tests_SRS_LOG_CONTEXT_PROPERTY_TYPE_STRUCT_01_009: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(struct).free shall return. ]*/
static void struct_free_returns(void)
{
    // arrange
    uint8_t value[1];

    POOR_MANS_ASSERT(LOG_CONTEXT_PROPERTY_TYPE_INIT(struct)(value, 42) == 0);

    // act
    LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(struct).free(value);

    // assert
}

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(struct).get_type */

/* Tests_SRS_LOG_CONTEXT_PROPERTY_TYPE_STRUCT_01_010: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(struct).get_type shall return the property type LOG_CONTEXT_PROPERTY_TYPE_struct. ]*/
static void struct_get_type_returns_LOG_CONTEXT_PROPERTY_TYPE_struct(void)
{
    // arrange

    // act
    LOG_CONTEXT_PROPERTY_TYPE result = LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(struct).get_type();

    // assert
    POOR_MANS_ASSERT(result == LOG_CONTEXT_PROPERTY_TYPE_struct);
}

/* LOG_CONTEXT_PROPERTY_TYPE_INIT(struct) */

/* Tests_SRS_LOG_CONTEXT_PROPERTY_TYPE_STRUCT_01_011: [ If dst_value is NULL, LOG_CONTEXT_PROPERTY_TYPE_INIT(struct) shall fail and return a non-zero value. ]*/
static void struct_init_with_NULL_dst_value_fails(void)
{
    // arrange

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_INIT(struct)(NULL, 42);

    // assert
    POOR_MANS_ASSERT(result != 0);
}

/* Tests_SRS_LOG_CONTEXT_PROPERTY_TYPE_STRUCT_01_012: [ LOG_CONTEXT_PROPERTY_TYPE_INIT(struct) shall copy the field_count byte to dst_value. ]*/
/* Tests_SRS_LOG_CONTEXT_PROPERTY_TYPE_STRUCT_01_013: [ LOG_CONTEXT_PROPERTY_TYPE_INIT(struct) shall succeed and return 0. ]*/
static void struct_init_succeeds(void)
{
    // arrange
    uint8_t value[1];

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_INIT(struct)(value, 43);

    // assert
    POOR_MANS_ASSERT(result == 0);
    POOR_MANS_ASSERT(value[0] == 43);
}

/* LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(struct) */

/* Tests_SRS_LOG_CONTEXT_PROPERTY_TYPE_STRUCT_01_014: [ LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(struct) shall return sizeof(uint8_t). ]*/
static void struct_get_init_data_size_returns_1(void)
{
    // arrange

    // act
    int result = LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(struct)();

    // assert
    POOR_MANS_ASSERT(result == sizeof(uint8_t));
}

/* very "poor man's" way of testing, as no test harness and mocking framework are available */
int main(void)
{
    struct_to_string_with_NULL_property_value_fails();
    struct_to_string_with_NULL_buffer_and_0_size_returns_0();
    struct_to_string_with_NULL_buffer_and_positive_size_returns_0();
    struct_to_string_with_non_NULL_buffer_and_zero_buffer_length_returns_0();
    struct_to_string_succeeds();

    struct_copy_with_NULL_src_value_fails();
    struct_copy_with_NULL_dst_value_fails();
    struct_copy_copies_one_byte_with_the_field_count();

    struct_free_returns();

    struct_get_type_returns_LOG_CONTEXT_PROPERTY_TYPE_struct();

    struct_init_with_NULL_dst_value_fails();
    struct_init_succeeds();

    struct_get_init_data_size_returns_1();

    return asserts_failed;
}
