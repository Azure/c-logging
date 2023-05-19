// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "macro_utils/macro_utils.h"

#define log_internal_error_report mock_log_internal_error_report

#include "c_logging/log_context_property_basic_types.h"
#include "c_logging/log_context_property_type_ascii_char_ptr.h"
#include "c_logging/log_context_property_value_pair.h"
#include "c_logging/log_context_property_type.h"
#include "c_logging/log_context_property_type_if.h"
#include "c_logging/log_context.h"

// defines how many mock calls we can have
#define MAX_MOCK_CALL_COUNT (128)

#define MOCK_CALL_TYPE_VALUES \
    MOCK_CALL_TYPE_malloc, \
    MOCK_CALL_TYPE_free, \
    MOCK_CALL_TYPE_log_internal_error_report \

MU_DEFINE_ENUM(MOCK_CALL_TYPE, MOCK_CALL_TYPE_VALUES)

// very poor mans mocks :-(
typedef struct malloc_CALL_TAG
{
    bool override_result;
    void* call_result;
    size_t size;
} malloc_CALL;

typedef struct free_CALL_TAG
{
    void* ptr;
} free_CALL;

typedef struct MOCK_CALL_TAG
{
    MOCK_CALL_TYPE mock_call_type;
    union
    {
        malloc_CALL malloc_call;
        free_CALL free_call;
    } u;
} MOCK_CALL;

static MOCK_CALL expected_calls[MAX_MOCK_CALL_COUNT];
static size_t expected_call_count;
static size_t actual_call_count;
static bool actual_and_expected_match;

static void setup_mocks(void)
{
    expected_call_count = 0;
    actual_call_count = 0;
    actual_and_expected_match = true;
}

void* mock_malloc(size_t size)
{
    void* result;

    if ((actual_call_count == expected_call_count) ||
        (expected_calls[actual_call_count].mock_call_type != MOCK_CALL_TYPE_malloc))
    {
        actual_and_expected_match = false;
        result = NULL;
    }
    else
    {
        if (expected_calls[actual_call_count].u.malloc_call.override_result)
        {
            result = expected_calls[actual_call_count].u.malloc_call.call_result;
        }
        else
        {
            expected_calls[actual_call_count].u.malloc_call.size = size;

            result = malloc(size);
        }

        actual_call_count++;
    }

    return result;
}

void mock_free(void* ptr)
{
    if ((actual_call_count == expected_call_count) ||
        (expected_calls[actual_call_count].mock_call_type != MOCK_CALL_TYPE_free))
    {
        actual_and_expected_match = false;
    }
    else
    {
        expected_calls[actual_call_count].u.free_call.ptr = ptr;
        free(ptr);

        actual_call_count++;
    }
}

void mock_log_internal_error_report(void)
{
    if ((actual_call_count == expected_call_count) ||
        (expected_calls[actual_call_count].mock_call_type != MOCK_CALL_TYPE_log_internal_error_report))
    {
        actual_and_expected_match = false;
    }
    else
    {
        actual_call_count++;
    }
}

#define POOR_MANS_ASSERT(cond) \
    if (!(cond)) \
    { \
        (void)printf("%s:%d test failed\r\n", __FUNCTION__, __LINE__); \
        abort(); \
    } \

static void setup_malloc_call(void)
{
    expected_calls[expected_call_count].mock_call_type = MOCK_CALL_TYPE_malloc;
    expected_calls[expected_call_count].u.malloc_call.override_result = false;
    expected_call_count++;
}

static void setup_free_call(void)
{
    expected_calls[expected_call_count].mock_call_type = MOCK_CALL_TYPE_free;
    expected_call_count++;
}

static void setup_log_internal_error_report(void)
{
    expected_calls[expected_call_count].mock_call_type = MOCK_CALL_TYPE_log_internal_error_report;
    expected_call_count++;
}

/* LOG_CONTEXT_CREATE */

/* Tests_SRS_LOG_CONTEXT_01_001: [ LOG_CONTEXT_CREATE shall allocate memory for the log context. ]*/
/* Tests_SRS_LOG_CONTEXT_01_013: [ LOG_CONTEXT_CREATE shall store one property/value pair that with a property type of struct with as many fields as the total number of properties passed to LOG_CONTEXT_CREATE. ]*/
/* Tests_SRS_LOG_CONTEXT_01_009: [ LOG_CONTEXT_NAME shall be optional. ]*/
/* Tests_SRS_LOG_CONTEXT_01_010: [ If LOG_CONTEXT_NAME is not used the name for the context shall be empty string. ]*/
static void LOG_CONTEXT_CREATE_with_no_properties_succeeds(void)
{
    // arrange
    setup_mocks();
    setup_malloc_call();

    // act
    LOG_CONTEXT_HANDLE result;
    LOG_CONTEXT_CREATE(result, NULL);

    // assert
    POOR_MANS_ASSERT(result != NULL);
    POOR_MANS_ASSERT(log_context_get_property_value_pair_count(result) == 1);
    const LOG_CONTEXT_PROPERTY_VALUE_PAIR* pairs = log_context_get_property_value_pairs(result);
    // context struct
    POOR_MANS_ASSERT(*(uint8_t*)pairs[0].value == 0);
    POOR_MANS_ASSERT(strcmp(pairs[0].name, "") == 0);
    POOR_MANS_ASSERT(pairs[0].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_struct);
    POOR_MANS_ASSERT(actual_and_expected_match);

    // cleanup
    setup_mocks();
    setup_free_call();
    LOG_CONTEXT_DESTROY(result);
}

/* Tests_SRS_LOG_CONTEXT_01_002: [ If any error occurs, LOG_CONTEXT_CREATE shall fail and return NULL. ]*/
static void when_malloc_fails_LOG_CONTEXT_CREATE_with_no_properties_also_fails(void)
{
    // arrange
    setup_mocks();

    expected_calls[expected_call_count].mock_call_type = MOCK_CALL_TYPE_malloc;
    expected_calls[expected_call_count].u.malloc_call.override_result = true;
    expected_calls[expected_call_count].u.malloc_call.call_result = NULL;
    expected_call_count++;

    // act
    LOG_CONTEXT_HANDLE result;
    LOG_CONTEXT_CREATE(result, NULL);

    // assert
    POOR_MANS_ASSERT(result == NULL);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* LOG_CONTEXT_DESTROY */

/* Tests_SRS_LOG_CONTEXT_01_006: [ LOG_CONTEXT_DESTROY shall free the memory and resources associated with log_context that were allocated by LOG_CONTEXT_CREATE. ]*/
static void LOG_CONTEXT_DESTROY_frees_memory(void)
{
    // arrange
    setup_mocks();
    setup_malloc_call();
    LOG_CONTEXT_HANDLE result;
    LOG_CONTEXT_CREATE(result, NULL, LOG_CONTEXT_PROPERTY(int32_t, test_int32_t, 42));
    POOR_MANS_ASSERT(result != NULL);

    setup_mocks();
    setup_free_call();

    // act
    LOG_CONTEXT_DESTROY(result);

    // assert
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* LOG_CONTEXT_PROPERTY */

/* Tests_SRS_LOG_CONTEXT_01_001: [ LOG_CONTEXT_CREATE shall allocate memory for the log context. ]*/
/* Tests_SRS_LOG_CONTEXT_01_003: [ LOG_CONTEXT_CREATE shall store the property types and values specified by using LOG_CONTEXT_PROPERTY in the context. ]*/
/* Tests_SRS_LOG_CONTEXT_01_004: [ LOG_CONTEXT_PROPERTY shall expand to code allocating a property/value pair entry with the type property_type and the name property_name. ]*/
/* Tests_SRS_LOG_CONTEXT_01_005: [ LOG_CONTEXT_PROPERTY shall expand to code copying the value property_value to be the value of the property/value pair. ]*/
static void LOG_CONTEXT_CREATE_with_one_int32_t_succeeds(void)
{
    // arrange
    setup_mocks();
    setup_malloc_call();

    // act
    LOG_CONTEXT_HANDLE result;
    LOG_CONTEXT_CREATE(result, NULL, LOG_CONTEXT_PROPERTY(int32_t, test_int32_t, 42));
    
    // assert
    POOR_MANS_ASSERT(result != NULL);
    POOR_MANS_ASSERT(log_context_get_property_value_pair_count(result) == 2);
    const LOG_CONTEXT_PROPERTY_VALUE_PAIR* pairs = log_context_get_property_value_pairs(result);
    // context struct
    POOR_MANS_ASSERT(*(uint8_t*)pairs[0].value == 1);
    POOR_MANS_ASSERT(strcmp(pairs[0].name, "") == 0);
    POOR_MANS_ASSERT(pairs[0].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_struct);
    // int32_t property
    POOR_MANS_ASSERT(*(int32_t*)pairs[1].value == 42);
    POOR_MANS_ASSERT(strcmp(pairs[1].name, "test_int32_t") == 0);
    POOR_MANS_ASSERT(pairs[1].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_int32_t);
    POOR_MANS_ASSERT(actual_and_expected_match);

    // clean
    setup_mocks();
    setup_free_call();
    LOG_CONTEXT_DESTROY(result);
}

/* Tests_SRS_LOG_CONTEXT_01_001: [ LOG_CONTEXT_CREATE shall allocate memory for the log context. ]*/
/* Tests_SRS_LOG_CONTEXT_01_003: [ LOG_CONTEXT_CREATE shall store the property types and values specified by using LOG_CONTEXT_PROPERTY in the context. ]*/
/* Tests_SRS_LOG_CONTEXT_01_004: [ LOG_CONTEXT_PROPERTY shall expand to code allocating a property/value pair entry with the type property_type and the name property_name. ]*/
/* Tests_SRS_LOG_CONTEXT_01_005: [ LOG_CONTEXT_PROPERTY shall expand to code copying the value property_value to be the value of the property/value pair. ]*/
static void LOG_CONTEXT_CREATE_with_two_properties_succeeds(void)
{
    // arrange
    setup_mocks();
    setup_malloc_call();

    // act
    LOG_CONTEXT_HANDLE result;
    LOG_CONTEXT_CREATE(result, NULL,
        LOG_CONTEXT_PROPERTY(int32_t, test_int32_t, 42),
        LOG_CONTEXT_PROPERTY(int64_t, test_int64_t, 1));

    // assert
    POOR_MANS_ASSERT(result != NULL);
    POOR_MANS_ASSERT(log_context_get_property_value_pair_count(result) == 3);
    const LOG_CONTEXT_PROPERTY_VALUE_PAIR* pairs = log_context_get_property_value_pairs(result);
    // context struct
    POOR_MANS_ASSERT(*(uint8_t*)pairs[0].value == 2);
    POOR_MANS_ASSERT(strcmp(pairs[0].name, "") == 0);
    POOR_MANS_ASSERT(pairs[0].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_struct);
    // int32_t property
    POOR_MANS_ASSERT(*(int32_t*)pairs[1].value == 42);
    POOR_MANS_ASSERT(strcmp(pairs[1].name, "test_int32_t") == 0);
    POOR_MANS_ASSERT(pairs[1].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_int32_t);
    // int64_t property
    POOR_MANS_ASSERT(*(int64_t*)pairs[2].value == 1);
    POOR_MANS_ASSERT(strcmp(pairs[2].name, "test_int64_t") == 0);
    POOR_MANS_ASSERT(pairs[2].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_int64_t);
    POOR_MANS_ASSERT(actual_and_expected_match);

    // clean
    setup_mocks();
    setup_free_call();
    LOG_CONTEXT_DESTROY(result);
}

/* Tests_SRS_LOG_CONTEXT_01_001: [ LOG_CONTEXT_CREATE shall allocate memory for the log context. ]*/
/* Tests_SRS_LOG_CONTEXT_01_003: [ LOG_CONTEXT_CREATE shall store the property types and values specified by using LOG_CONTEXT_PROPERTY in the context. ]*/
/* Tests_SRS_LOG_CONTEXT_01_004: [ LOG_CONTEXT_PROPERTY shall expand to code allocating a property/value pair entry with the type property_type and the name property_name. ]*/
/* Tests_SRS_LOG_CONTEXT_01_005: [ LOG_CONTEXT_PROPERTY shall expand to code copying the value property_value to be the value of the property/value pair. ]*/
static void LOG_CONTEXT_CREATE_with_all_int_property_types_succeeds(void)
{
    // arrange
    setup_mocks();
    setup_malloc_call();

    // act
    LOG_CONTEXT_HANDLE result;
    LOG_CONTEXT_CREATE(result, NULL,
        LOG_CONTEXT_PROPERTY(int8_t, test_int8_t, INT8_MIN),
        LOG_CONTEXT_PROPERTY(uint8_t, test_uint8_t, UINT8_MAX),
        LOG_CONTEXT_PROPERTY(int16_t, test_int16_t, INT16_MIN),
        LOG_CONTEXT_PROPERTY(uint16_t, test_uint16_t, UINT16_MAX),
        LOG_CONTEXT_PROPERTY(int32_t, test_int32_t, INT32_MIN),
        LOG_CONTEXT_PROPERTY(uint32_t, test_uint32_t, UINT32_MAX),
        LOG_CONTEXT_PROPERTY(int64_t, test_int64_t, INT64_MIN),
        LOG_CONTEXT_PROPERTY(uint64_t, test_uint64_t, UINT64_MAX)
        );

    // assert
    POOR_MANS_ASSERT(result != NULL);
    POOR_MANS_ASSERT(log_context_get_property_value_pair_count(result) == 9);
    const LOG_CONTEXT_PROPERTY_VALUE_PAIR* pairs = log_context_get_property_value_pairs(result);
    // context struct
    POOR_MANS_ASSERT(*(uint8_t*)pairs[0].value == 8);
    POOR_MANS_ASSERT(strcmp(pairs[0].name, "") == 0);
    POOR_MANS_ASSERT(pairs[0].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_struct);
    // int8_t property
    POOR_MANS_ASSERT(*(int8_t*)pairs[1].value == INT8_MIN);
    POOR_MANS_ASSERT(strcmp(pairs[1].name, "test_int8_t") == 0);
    POOR_MANS_ASSERT(pairs[1].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_int8_t);
    // uint8_t property
    POOR_MANS_ASSERT(*(uint8_t*)pairs[2].value == UINT8_MAX);
    POOR_MANS_ASSERT(strcmp(pairs[2].name, "test_uint8_t") == 0);
    POOR_MANS_ASSERT(pairs[2].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_uint8_t);
    // int16_t property
    POOR_MANS_ASSERT(*(int16_t*)pairs[3].value == INT16_MIN);
    POOR_MANS_ASSERT(strcmp(pairs[3].name, "test_int16_t") == 0);
    POOR_MANS_ASSERT(pairs[3].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_int16_t);
    // uint16_t property
    POOR_MANS_ASSERT(*(uint16_t*)pairs[4].value == UINT16_MAX);
    POOR_MANS_ASSERT(strcmp(pairs[4].name, "test_uint16_t") == 0);
    POOR_MANS_ASSERT(pairs[4].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_uint16_t);
    // int32_t property
    POOR_MANS_ASSERT(*(int32_t*)pairs[5].value == INT32_MIN);
    POOR_MANS_ASSERT(strcmp(pairs[5].name, "test_int32_t") == 0);
    POOR_MANS_ASSERT(pairs[5].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_int32_t);
    // uint32_t property
    POOR_MANS_ASSERT(*(uint32_t*)pairs[6].value == UINT32_MAX);
    POOR_MANS_ASSERT(strcmp(pairs[6].name, "test_uint32_t") == 0);
    POOR_MANS_ASSERT(pairs[6].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_uint32_t);
    // int64_t property
    POOR_MANS_ASSERT(*(int64_t*)pairs[7].value == INT64_MIN);
    POOR_MANS_ASSERT(strcmp(pairs[7].name, "test_int64_t") == 0);
    POOR_MANS_ASSERT(pairs[7].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_int64_t);
    // uint64_t property
    POOR_MANS_ASSERT(*(uint64_t*)pairs[8].value == UINT64_MAX);
    POOR_MANS_ASSERT(strcmp(pairs[8].name, "test_uint64_t") == 0);
    POOR_MANS_ASSERT(pairs[8].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_uint64_t);
    POOR_MANS_ASSERT(actual_and_expected_match);

    // clean
    setup_mocks();
    setup_free_call();
    LOG_CONTEXT_DESTROY(result);
}

/* LOG_CONTEXT_STRING_PROPERTY */

/* Tests_SRS_LOG_CONTEXT_01_007: [ LOG_CONTEXT_STRING_PROPERTY shall expand to code allocating a property/value pair of type ascii_char_ptr and the name property_name. ]*/
/* Tests_SRS_LOG_CONTEXT_01_008: [ LOG_CONTEXT_STRING_PROPERTY shall expand to code that stores as value a string that is constructed using printf-like formatting based on format and all the arguments in .... ]*/
static void LOG_CONTEXT_CREATE_with_one_string_property_with_only_format_passed_to_it_succeeds(void)
{
    // arrange
    setup_mocks();
    setup_malloc_call();

    // act
    LOG_CONTEXT_HANDLE result;
    LOG_CONTEXT_CREATE(result, NULL, LOG_CONTEXT_STRING_PROPERTY(test_string, "gogu"));

    // assert
    POOR_MANS_ASSERT(result != NULL);
    POOR_MANS_ASSERT(log_context_get_property_value_pair_count(result) == 2);
    const LOG_CONTEXT_PROPERTY_VALUE_PAIR* pairs = log_context_get_property_value_pairs(result);
    // context struct
    POOR_MANS_ASSERT(*(uint8_t*)pairs[0].value == 1);
    POOR_MANS_ASSERT(strcmp(pairs[0].name, "") == 0);
    POOR_MANS_ASSERT(pairs[0].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_struct);
    // string property
    POOR_MANS_ASSERT(strcmp(pairs[1].value, "gogu") == 0);
    POOR_MANS_ASSERT(strcmp(pairs[1].name, "test_string") == 0);
    POOR_MANS_ASSERT(pairs[1].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_ascii_char_ptr);
    POOR_MANS_ASSERT(actual_and_expected_match);

    // clean
    setup_mocks();
    setup_free_call();
    LOG_CONTEXT_DESTROY(result);
}

/* Tests_SRS_LOG_CONTEXT_01_007: [ LOG_CONTEXT_STRING_PROPERTY shall expand to code allocating a property/value pair of type ascii_char_ptr and the name property_name. ]*/
/* Tests_SRS_LOG_CONTEXT_01_008: [ LOG_CONTEXT_STRING_PROPERTY shall expand to code that stores as value a string that is constructed using printf-like formatting based on format and all the arguments in .... ]*/
static void LOG_CONTEXT_CREATE_with_2_string_properties_with_only_format_passed_to_it_succeeds(void)
{
    // arrange
    setup_mocks();
    setup_malloc_call();

    // act
    LOG_CONTEXT_HANDLE result;
    LOG_CONTEXT_CREATE(result, NULL,
        LOG_CONTEXT_STRING_PROPERTY(test_string_1, "haga"),
        LOG_CONTEXT_STRING_PROPERTY(test_string_2, "uaga"));

    // assert
    POOR_MANS_ASSERT(result != NULL);
    POOR_MANS_ASSERT(log_context_get_property_value_pair_count(result) == 3);
    const LOG_CONTEXT_PROPERTY_VALUE_PAIR* pairs = log_context_get_property_value_pairs(result);
    // context struct
    POOR_MANS_ASSERT(*(uint8_t*)pairs[0].value == 2);
    POOR_MANS_ASSERT(strcmp(pairs[0].name, "") == 0);
    POOR_MANS_ASSERT(pairs[0].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_struct);
    // 1st string property
    POOR_MANS_ASSERT(strcmp(pairs[1].value, "haga") == 0);
    POOR_MANS_ASSERT(strcmp(pairs[1].name, "test_string_1") == 0);
    POOR_MANS_ASSERT(pairs[1].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_ascii_char_ptr);
    // 2nd string property
    POOR_MANS_ASSERT(strcmp(pairs[2].value, "uaga") == 0);
    POOR_MANS_ASSERT(strcmp(pairs[2].name, "test_string_2") == 0);
    POOR_MANS_ASSERT(pairs[2].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_ascii_char_ptr);
    POOR_MANS_ASSERT(actual_and_expected_match);

    // clean
    setup_mocks();
    setup_free_call();
    LOG_CONTEXT_DESTROY(result);
}

/* Tests_SRS_LOG_CONTEXT_01_007: [ LOG_CONTEXT_STRING_PROPERTY shall expand to code allocating a property/value pair of type ascii_char_ptr and the name property_name. ]*/
/* Tests_SRS_LOG_CONTEXT_01_008: [ LOG_CONTEXT_STRING_PROPERTY shall expand to code that stores as value a string that is constructed using printf-like formatting based on format and all the arguments in .... ]*/
static void LOG_CONTEXT_CREATE_with_a_string_property_using_printf_formatting_succeeds(void)
{
    // arrange
    setup_mocks();
    setup_malloc_call();

    // act
    LOG_CONTEXT_HANDLE result;
    LOG_CONTEXT_CREATE(result, NULL,
        LOG_CONTEXT_STRING_PROPERTY(test_string, "here we have a person named %s %s of age %d",
            "Baba", "Cloantza", 4200));

    // assert
    POOR_MANS_ASSERT(result != NULL);
    POOR_MANS_ASSERT(log_context_get_property_value_pair_count(result) == 2);
    const LOG_CONTEXT_PROPERTY_VALUE_PAIR* pairs = log_context_get_property_value_pairs(result);
    // context struct
    POOR_MANS_ASSERT(*(uint8_t*)pairs[0].value == 1);
    POOR_MANS_ASSERT(strcmp(pairs[0].name, "") == 0);
    POOR_MANS_ASSERT(pairs[0].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_struct);
    // string property
    POOR_MANS_ASSERT(strcmp(pairs[1].value, "here we have a person named Baba Cloantza of age 4200") == 0);
    POOR_MANS_ASSERT(strcmp(pairs[1].name, "test_string") == 0);
    POOR_MANS_ASSERT(pairs[1].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_ascii_char_ptr);
    POOR_MANS_ASSERT(actual_and_expected_match);

    // clean
    setup_mocks();
    setup_free_call();
    LOG_CONTEXT_DESTROY(result);
}

/* Tests_SRS_LOG_CONTEXT_01_007: [ LOG_CONTEXT_STRING_PROPERTY shall expand to code allocating a property/value pair of type ascii_char_ptr and the name property_name. ]*/
/* Tests_SRS_LOG_CONTEXT_01_008: [ LOG_CONTEXT_STRING_PROPERTY shall expand to code that stores as value a string that is constructed using printf-like formatting based on format and all the arguments in .... ]*/
static void LOG_CONTEXT_CREATE_with_a_string_property_followed_by_another_int_property_succeeds(void)
{
    // arrange
    setup_mocks();
    setup_malloc_call();

    // act
    LOG_CONTEXT_HANDLE result;
    LOG_CONTEXT_CREATE(result, NULL,
        LOG_CONTEXT_STRING_PROPERTY(test_string, "The answer is %d", 42),
        LOG_CONTEXT_PROPERTY(int32_t, answer, 42));

    // assert
    POOR_MANS_ASSERT(result != NULL);
    POOR_MANS_ASSERT(log_context_get_property_value_pair_count(result) == 3);
    const LOG_CONTEXT_PROPERTY_VALUE_PAIR* pairs = log_context_get_property_value_pairs(result);
    // context struct
    POOR_MANS_ASSERT(*(uint8_t*)pairs[0].value == 2);
    POOR_MANS_ASSERT(strcmp(pairs[0].name, "") == 0);
    POOR_MANS_ASSERT(pairs[0].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_struct);
    // string property
    POOR_MANS_ASSERT(strcmp(pairs[1].value, "The answer is 42") == 0);
    POOR_MANS_ASSERT(strcmp(pairs[1].name, "test_string") == 0);
    POOR_MANS_ASSERT(pairs[1].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_ascii_char_ptr);
    // int32_t property
    POOR_MANS_ASSERT(*(int32_t*)pairs[2].value == 42);
    POOR_MANS_ASSERT(strcmp(pairs[2].name, "answer") == 0);
    POOR_MANS_ASSERT(pairs[2].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_int32_t);
    POOR_MANS_ASSERT(actual_and_expected_match);

    // clean
    setup_mocks();
    setup_free_call();
    LOG_CONTEXT_DESTROY(result);
}

/* Tests_SRS_LOG_CONTEXT_01_007: [ LOG_CONTEXT_STRING_PROPERTY shall expand to code allocating a property/value pair of type ascii_char_ptr and the name property_name. ]*/
/* Tests_SRS_LOG_CONTEXT_01_008: [ LOG_CONTEXT_STRING_PROPERTY shall expand to code that stores as value a string that is constructed using printf-like formatting based on format and all the arguments in .... ]*/
static void LOG_CONTEXT_CREATE_with_a_string_property_preceded_by_another_int_property_succeeds(void)
{
    // arrange
    setup_mocks();
    setup_malloc_call();

    // act
    LOG_CONTEXT_HANDLE result;
    LOG_CONTEXT_CREATE(result, NULL,
        LOG_CONTEXT_PROPERTY(int32_t, answer, 42),
        LOG_CONTEXT_STRING_PROPERTY(test_string, "The answer is %d", 42)
        );

    // assert
    POOR_MANS_ASSERT(result != NULL);
    POOR_MANS_ASSERT(log_context_get_property_value_pair_count(result) == 3);
    const LOG_CONTEXT_PROPERTY_VALUE_PAIR* pairs = log_context_get_property_value_pairs(result);
    // context struct
    POOR_MANS_ASSERT(*(uint8_t*)pairs[0].value == 2);
    POOR_MANS_ASSERT(strcmp(pairs[0].name, "") == 0);
    POOR_MANS_ASSERT(pairs[0].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_struct);
    // int32_t property
    POOR_MANS_ASSERT(*(int32_t*)pairs[1].value == 42);
    POOR_MANS_ASSERT(strcmp(pairs[1].name, "answer") == 0);
    POOR_MANS_ASSERT(pairs[1].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_int32_t);
    // string property
    POOR_MANS_ASSERT(strcmp(pairs[2].value, "The answer is 42") == 0);
    POOR_MANS_ASSERT(strcmp(pairs[2].name, "test_string") == 0);
    POOR_MANS_ASSERT(pairs[2].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_ascii_char_ptr);
    POOR_MANS_ASSERT(actual_and_expected_match);

    // clean
    setup_mocks();
    setup_free_call();
    LOG_CONTEXT_DESTROY(result);
}

/* LOG_CONTEXT_NAME */

/* Tests_SRS_LOG_CONTEXT_01_013: [ LOG_CONTEXT_CREATE shall store one property/value pair that with a property type of struct with as many fields as the total number of properties passed to LOG_CONTEXT_CREATE. ]*/
/* Tests_SRS_LOG_CONTEXT_01_012: [ The name of the struct property shall be the context name specified by using LOG_CONTEXT_NAME (if specified). ]*/ \
static void LOG_CONTEXT_CREATE_with_LOG_CONTEXT_NAME_uses_the_context_name(void)
{
    // arrange
    setup_mocks();
    setup_malloc_call();

    // act
    LOG_CONTEXT_HANDLE result;
    LOG_CONTEXT_CREATE(result, NULL,
        LOG_CONTEXT_NAME(grrrrr)
    );

    // assert
    POOR_MANS_ASSERT(result != NULL);
    POOR_MANS_ASSERT(log_context_get_property_value_pair_count(result) == 1);
    const LOG_CONTEXT_PROPERTY_VALUE_PAIR* pairs = log_context_get_property_value_pairs(result);
    // context struct
    POOR_MANS_ASSERT(*(uint8_t*)pairs[0].value == 0);
    POOR_MANS_ASSERT(strcmp(pairs[0].name, "grrrrr") == 0);
    POOR_MANS_ASSERT(pairs[0].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_struct);
    POOR_MANS_ASSERT(actual_and_expected_match);

    // clean
    setup_mocks();
    setup_free_call();
    LOG_CONTEXT_DESTROY(result);
}

/* Tests_SRS_LOG_CONTEXT_01_011: [ If LOG_CONTEXT_NAME is specified multiple times a compiler error shall be emitted. ]*/
#if 0
// If this code compiles we are not passing the test
static void LOG_CONTEXT_CREATE_with_LOG_CONTEXT_NAME_twice_does_not_compile(void)
{
    // arrange

    // act
    LOG_CONTEXT_HANDLE result;
    LOG_CONTEXT_CREATE(result, NULL,
        LOG_CONTEXT_NAME(grrrrr),
        LOG_CONTEXT_NAME(grrrr_again)
    );

    // assert

    // clean
}
#endif

/* Tests_SRS_LOG_CONTEXT_01_019: [ If 2 properties have the same property_name for a context a compiler error shall be emitted. ]*/
#if 0
// If this code compiles we are not passing the test
static void LOG_CONTEXT_CREATE_with_2_properties_with_the_same_name_does_not_compile(void)
{
    // arrange

    // act
    LOG_CONTEXT_HANDLE result;
    LOG_CONTEXT_CREATE(result, NULL,
        LOG_CONTEXT_NAME(grrrrr),
        LOG_CONTEXT_PROPERTY(int32_t, x, 42),
        LOG_CONTEXT_PROPERTY(int16_t, x, 43)
    );

    // assert

    // clean
}
#endif

/* Tests_SRS_LOG_CONTEXT_01_026: [ If 2 properties have the same property_name for a context a compiler error shall be emitted. ]*/
#if 0
// If this code compiles we are not passing the test
static void LOG_CONTEXT_CREATE_with_2_properties_with_the_same_name_when_one_is_a_string_property_does_not_compile(void)
{
    // arrange

    // act
    LOG_CONTEXT_HANDLE result;
    LOG_CONTEXT_CREATE(result, NULL,
        LOG_CONTEXT_NAME(grrrrr),
        LOG_CONTEXT_PROPERTY(int32_t, x, 42),
        LOG_CONTEXT_STRING_PROPERTY(x, "x")
    );

    // assert

    // clean
}
#endif

static LOG_CONTEXT_HANDLE setup_parent_context_with_no_properties(void)
{
    LOG_CONTEXT_HANDLE parent_context;

    setup_mocks();
    setup_malloc_call();

    LOG_CONTEXT_CREATE(parent_context, NULL);

    return parent_context;
}

static LOG_CONTEXT_HANDLE setup_parent_context_with_int_property(void)
{
    LOG_CONTEXT_HANDLE parent_context;

    setup_mocks();
    setup_malloc_call();

    LOG_CONTEXT_CREATE(parent_context, NULL, LOG_CONTEXT_PROPERTY(int32_t, parent_int32_t_property, 42));

    return parent_context;
}

static LOG_CONTEXT_HANDLE setup_parent_context_with_2_int_properties(void)
{
    LOG_CONTEXT_HANDLE parent_context;

    setup_mocks();
    setup_malloc_call();

    LOG_CONTEXT_CREATE(parent_context, NULL,
        LOG_CONTEXT_PROPERTY(int32_t, parent_int32_t_property, 42),
        LOG_CONTEXT_PROPERTY(int64_t, parent_int64_t_property, 43)
    );

    return parent_context;
}

static LOG_CONTEXT_HANDLE setup_parent_context_with_a_string_property(void)
{
    LOG_CONTEXT_HANDLE parent_context;

    setup_mocks();
    setup_malloc_call();

    LOG_CONTEXT_CREATE(parent_context, NULL,
        LOG_CONTEXT_STRING_PROPERTY(parent_string_property, "Baba cloantza")
    );

    return parent_context;
}

static LOG_CONTEXT_HANDLE setup_parent_context_with_2_string_properties(void)
{
    LOG_CONTEXT_HANDLE parent_context;

    setup_mocks();
    setup_malloc_call();

    LOG_CONTEXT_CREATE(parent_context, NULL,
        LOG_CONTEXT_STRING_PROPERTY(parent_string_property, "Baba cloantza"),
        LOG_CONTEXT_STRING_PROPERTY(parent_string_property_2, "muuu")
    );

    return parent_context;
}

static LOG_CONTEXT_HANDLE setup_parent_context_shawarma_with_everything(void)
{
    LOG_CONTEXT_HANDLE parent_context;

    setup_mocks();
    setup_malloc_call();

    LOG_CONTEXT_CREATE(parent_context, NULL,
        LOG_CONTEXT_STRING_PROPERTY(parent_string_property, "Baba cloantza"),
        LOG_CONTEXT_PROPERTY(int32_t, the_answer, 42),
        LOG_CONTEXT_NAME(context_name),
        LOG_CONTEXT_STRING_PROPERTY(another_string, "%d", 43),
        LOG_CONTEXT_PROPERTY(uint64_t, max_uint64_t, UINT64_MAX)
    );

    return parent_context;
}

/* Tests_SRS_LOG_CONTEXT_01_014: [ If parent_context is non-NULL, the created context shall copy all the property/value pairs of parent_context. ]*/
static void LOG_CONTEXT_CREATE_with_a_parent_that_has_no_properties_succeeds(void)
{
    // arrange
    LOG_CONTEXT_HANDLE parent_context = setup_parent_context_with_no_properties();
    setup_mocks();
    setup_malloc_call();

    // act
    LOG_CONTEXT_HANDLE result;
    LOG_CONTEXT_CREATE(result, parent_context);

    // assert
    POOR_MANS_ASSERT(result != NULL);
    POOR_MANS_ASSERT(log_context_get_property_value_pair_count(result) == 2);
    const LOG_CONTEXT_PROPERTY_VALUE_PAIR* pairs = log_context_get_property_value_pairs(result);
    // context struct of parent
    POOR_MANS_ASSERT(*(uint8_t*)pairs[0].value == 1);
    POOR_MANS_ASSERT(strcmp(pairs[0].name, "") == 0);
    POOR_MANS_ASSERT(pairs[0].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_struct);
    // context struct
    POOR_MANS_ASSERT(*(uint8_t*)pairs[1].value == 0);
    POOR_MANS_ASSERT(strcmp(pairs[1].name, "") == 0);
    POOR_MANS_ASSERT(pairs[1].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_struct);
    POOR_MANS_ASSERT(actual_and_expected_match);

    // cleanup
    setup_mocks();
    setup_free_call();
    setup_free_call();
    LOG_CONTEXT_DESTROY(result);
    LOG_CONTEXT_DESTROY(parent_context);
}

/* Tests_SRS_LOG_CONTEXT_01_014: [ If parent_context is non-NULL, the created context shall copy all the property/value pairs of parent_context. ]*/
static void LOG_CONTEXT_CREATE_with_a_parent_that_has_one_int_property_succeeds(void)
{
    // arrange
    LOG_CONTEXT_HANDLE parent_context = setup_parent_context_with_int_property();
    setup_mocks();
    setup_malloc_call();

    // act
    LOG_CONTEXT_HANDLE result;
    LOG_CONTEXT_CREATE(result, parent_context);

    // assert
    POOR_MANS_ASSERT(result != NULL);
    POOR_MANS_ASSERT(log_context_get_property_value_pair_count(result) == 3);
    const LOG_CONTEXT_PROPERTY_VALUE_PAIR* pairs = log_context_get_property_value_pairs(result);
    // context struct
    POOR_MANS_ASSERT(*(uint8_t*)pairs[0].value == 2);
    POOR_MANS_ASSERT(strcmp(pairs[0].name, "") == 0);
    POOR_MANS_ASSERT(pairs[0].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_struct);
    // parent context struct
    POOR_MANS_ASSERT(*(uint8_t*)pairs[1].value == 1);
    POOR_MANS_ASSERT(strcmp(pairs[1].name, "") == 0);
    POOR_MANS_ASSERT(pairs[1].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_struct);
    // parent int32_t property
    POOR_MANS_ASSERT(*(int32_t*)pairs[2].value == 42);
    POOR_MANS_ASSERT(strcmp(pairs[2].name, "parent_int32_t_property") == 0);
    POOR_MANS_ASSERT(pairs[2].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_int32_t);
    POOR_MANS_ASSERT(actual_and_expected_match);

    // cleanup
    setup_mocks();
    setup_free_call();
    setup_free_call();
    LOG_CONTEXT_DESTROY(result);
    LOG_CONTEXT_DESTROY(parent_context);
}

/* Tests_SRS_LOG_CONTEXT_01_014: [ If parent_context is non-NULL, the created context shall copy all the property/value pairs of parent_context. ]*/
static void LOG_CONTEXT_CREATE_with_a_parent_that_has_2_int_properties_succeeds(void)
{
    // arrange
    LOG_CONTEXT_HANDLE parent_context = setup_parent_context_with_2_int_properties();
    setup_mocks();
    setup_malloc_call();

    // act
    LOG_CONTEXT_HANDLE result;
    LOG_CONTEXT_CREATE(result, parent_context);

    // assert
    POOR_MANS_ASSERT(result != NULL);
    POOR_MANS_ASSERT(log_context_get_property_value_pair_count(result) == 4);
    const LOG_CONTEXT_PROPERTY_VALUE_PAIR* pairs = log_context_get_property_value_pairs(result);
    // context struct
    POOR_MANS_ASSERT(*(uint8_t*)pairs[0].value == 3);
    POOR_MANS_ASSERT(strcmp(pairs[0].name, "") == 0);
    POOR_MANS_ASSERT(pairs[0].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_struct);
    // parent context struct
    POOR_MANS_ASSERT(*(uint8_t*)pairs[1].value == 2);
    POOR_MANS_ASSERT(strcmp(pairs[1].name, "") == 0);
    POOR_MANS_ASSERT(pairs[1].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_struct);
    // parent 1st int32_t property
    POOR_MANS_ASSERT(*(int32_t*)pairs[2].value == 42);
    POOR_MANS_ASSERT(strcmp(pairs[2].name, "parent_int32_t_property") == 0);
    POOR_MANS_ASSERT(pairs[2].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_int32_t);
    // parent 2nd int64_t property
    POOR_MANS_ASSERT(*(int64_t*)pairs[3].value == 43);
    POOR_MANS_ASSERT(strcmp(pairs[3].name, "parent_int64_t_property") == 0);
    POOR_MANS_ASSERT(pairs[3].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_int64_t);
    POOR_MANS_ASSERT(actual_and_expected_match);

    // cleanup
    setup_mocks();
    setup_free_call();
    setup_free_call();
    LOG_CONTEXT_DESTROY(result);
    LOG_CONTEXT_DESTROY(parent_context);
}

/* Tests_SRS_LOG_CONTEXT_01_014: [ If parent_context is non-NULL, the created context shall copy all the property/value pairs of parent_context. ]*/
static void LOG_CONTEXT_CREATE_with_a_parent_that_has_a_string_property_succeeds(void)
{
    // arrange
    LOG_CONTEXT_HANDLE parent_context = setup_parent_context_with_a_string_property();
    setup_mocks();
    setup_malloc_call();

    // act
    LOG_CONTEXT_HANDLE result;
    LOG_CONTEXT_CREATE(result, parent_context);

    // assert
    POOR_MANS_ASSERT(result != NULL);
    POOR_MANS_ASSERT(log_context_get_property_value_pair_count(result) == 3);
    const LOG_CONTEXT_PROPERTY_VALUE_PAIR* pairs = log_context_get_property_value_pairs(result);
    // context struct
    POOR_MANS_ASSERT(*(uint8_t*)pairs[0].value == 2);
    POOR_MANS_ASSERT(strcmp(pairs[0].name, "") == 0);
    POOR_MANS_ASSERT(pairs[0].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_struct);
    // parent context struct
    POOR_MANS_ASSERT(*(uint8_t*)pairs[1].value == 1);
    POOR_MANS_ASSERT(strcmp(pairs[1].name, "") == 0);
    POOR_MANS_ASSERT(pairs[1].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_struct);
    // parent string property
    POOR_MANS_ASSERT(strcmp(pairs[2].value , "Baba cloantza") == 0);
    POOR_MANS_ASSERT(strcmp(pairs[2].name, "parent_string_property") == 0);
    POOR_MANS_ASSERT(pairs[2].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_ascii_char_ptr);
    POOR_MANS_ASSERT(actual_and_expected_match);

    // cleanup
    setup_mocks();
    setup_free_call();
    setup_free_call();
    LOG_CONTEXT_DESTROY(result);
    LOG_CONTEXT_DESTROY(parent_context);
}

/* Tests_SRS_LOG_CONTEXT_01_014: [ If parent_context is non-NULL, the created context shall copy all the property/value pairs of parent_context. ]*/
static void LOG_CONTEXT_CREATE_with_a_parent_that_has_2_string_properties_succeeds(void)
{
    // arrange
    LOG_CONTEXT_HANDLE parent_context = setup_parent_context_with_2_string_properties();
    setup_mocks();
    setup_malloc_call();

    // act
    LOG_CONTEXT_HANDLE result;
    LOG_CONTEXT_CREATE(result, parent_context);

    // assert
    POOR_MANS_ASSERT(result != NULL);
    POOR_MANS_ASSERT(log_context_get_property_value_pair_count(result) == 4);
    const LOG_CONTEXT_PROPERTY_VALUE_PAIR* pairs = log_context_get_property_value_pairs(result);
    // context struct
    POOR_MANS_ASSERT(*(uint8_t*)pairs[0].value == 3);
    POOR_MANS_ASSERT(strcmp(pairs[0].name, "") == 0);
    POOR_MANS_ASSERT(pairs[0].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_struct);
    // parent context struct
    POOR_MANS_ASSERT(*(uint8_t*)pairs[1].value == 2);
    POOR_MANS_ASSERT(strcmp(pairs[1].name, "") == 0);
    POOR_MANS_ASSERT(pairs[1].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_struct);
    // parent 1st string property
    POOR_MANS_ASSERT(strcmp(pairs[2].value, "Baba cloantza") == 0);
    POOR_MANS_ASSERT(strcmp(pairs[2].name, "parent_string_property") == 0);
    POOR_MANS_ASSERT(pairs[2].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_ascii_char_ptr);
    // parent 2nd string property
    POOR_MANS_ASSERT(strcmp(pairs[3].value, "muuu") == 0);
    POOR_MANS_ASSERT(strcmp(pairs[3].name, "parent_string_property_2") == 0);
    POOR_MANS_ASSERT(pairs[3].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_ascii_char_ptr);
    POOR_MANS_ASSERT(actual_and_expected_match);

    // cleanup
    setup_mocks();
    setup_free_call();
    setup_free_call();
    LOG_CONTEXT_DESTROY(result);
    LOG_CONTEXT_DESTROY(parent_context);
}

/* Tests_SRS_LOG_CONTEXT_01_014: [ If parent_context is non-NULL, the created context shall copy all the property/value pairs of parent_context. ]*/
static void LOG_CONTEXT_CREATE_with_a_parent_that_has_a_context_name_succeeds(void)
{
    // arrange
    LOG_CONTEXT_HANDLE parent_context = setup_parent_context_shawarma_with_everything();
    setup_mocks();
    setup_malloc_call();

    // act
    LOG_CONTEXT_HANDLE result;
    LOG_CONTEXT_CREATE(result, parent_context);

    // assert
    POOR_MANS_ASSERT(result != NULL);
    POOR_MANS_ASSERT(log_context_get_property_value_pair_count(result) == 6);
    const LOG_CONTEXT_PROPERTY_VALUE_PAIR* pairs = log_context_get_property_value_pairs(result);
    // context struct
    POOR_MANS_ASSERT(*(uint8_t*)pairs[0].value == 5);
    POOR_MANS_ASSERT(strcmp(pairs[0].name, "") == 0);
    POOR_MANS_ASSERT(pairs[0].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_struct);
    // parent context struct
    POOR_MANS_ASSERT(*(uint8_t*)pairs[1].value == 4);
    POOR_MANS_ASSERT(strcmp(pairs[1].name, "context_name") == 0);
    POOR_MANS_ASSERT(pairs[1].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_struct);
    // parent 1st property
    POOR_MANS_ASSERT(strcmp(pairs[2].value, "Baba cloantza") == 0);
    POOR_MANS_ASSERT(strcmp(pairs[2].name, "parent_string_property") == 0);
    POOR_MANS_ASSERT(pairs[2].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_ascii_char_ptr);
    // parent 2nd property
    POOR_MANS_ASSERT(*(int32_t*)pairs[3].value == 42);
    POOR_MANS_ASSERT(strcmp(pairs[3].name, "the_answer") == 0);
    POOR_MANS_ASSERT(pairs[3].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_int32_t);
    // parent 3rd property
    POOR_MANS_ASSERT(strcmp(pairs[4].value, "43") == 0);
    POOR_MANS_ASSERT(strcmp(pairs[4].name, "another_string") == 0);
    POOR_MANS_ASSERT(pairs[4].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_ascii_char_ptr);
    // parent 4th property
    POOR_MANS_ASSERT(*(uint64_t*)pairs[5].value == UINT64_MAX);
    POOR_MANS_ASSERT(strcmp(pairs[5].name, "max_uint64_t") == 0);
    POOR_MANS_ASSERT(pairs[5].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_uint64_t);
    POOR_MANS_ASSERT(actual_and_expected_match);

    // cleanup
    setup_mocks();
    setup_free_call();
    setup_free_call();
    LOG_CONTEXT_DESTROY(result);
    LOG_CONTEXT_DESTROY(parent_context);
}

/* Tests_SRS_LOG_CONTEXT_01_015: [ LOG_CONTEXT_LOCAL_DEFINE shall store one property/value pair that with a property type of struct with as many fields as the total number of properties passed to LOG_CONTEXT_LOCAL_DEFINE in the ... arguments. ]*/
static void LOG_CONTEXT_LOCAL_DEFINE_with_no_properties_succeeds(void)
{
    // arrange
    setup_mocks();

    // act
    LOG_CONTEXT_LOCAL_DEFINE(local_context, NULL);

    // assert
    POOR_MANS_ASSERT(log_context_get_property_value_pair_count(&local_context) == 1);
    const LOG_CONTEXT_PROPERTY_VALUE_PAIR* pairs = log_context_get_property_value_pairs(&local_context);
    // context struct
    POOR_MANS_ASSERT(*(uint8_t*)pairs[0].value == 0);
    POOR_MANS_ASSERT(strcmp(pairs[0].name, "") == 0);
    POOR_MANS_ASSERT(pairs[0].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_struct);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_01_015: [ LOG_CONTEXT_LOCAL_DEFINE shall store one property/value pair that with a property type of struct with as many fields as the total number of properties passed to LOG_CONTEXT_LOCAL_DEFINE in the ... arguments. ]*/
/* Tests_SRS_LOG_CONTEXT_01_016: [ LOG_CONTEXT_LOCAL_DEFINE shall store the property types and values specified by using LOG_CONTEXT_PROPERTY in the context. ]*/
static void LOG_CONTEXT_LOCAL_DEFINE_with_1_int_property_succeeds(void)
{
    // arrange
    setup_mocks();

    // act
    LOG_CONTEXT_LOCAL_DEFINE(local_context, NULL,
        LOG_CONTEXT_PROPERTY(int32_t, the_answer, 42));

    // assert
    POOR_MANS_ASSERT(log_context_get_property_value_pair_count(&local_context) == 2);
    const LOG_CONTEXT_PROPERTY_VALUE_PAIR* pairs = log_context_get_property_value_pairs(&local_context);
    // context struct
    POOR_MANS_ASSERT(*(uint8_t*)pairs[0].value == 1);
    POOR_MANS_ASSERT(strcmp(pairs[0].name, "") == 0);
    POOR_MANS_ASSERT(pairs[0].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_struct);
    // int32_t property
    POOR_MANS_ASSERT(*(int32_t*)pairs[1].value == 42);
    POOR_MANS_ASSERT(strcmp(pairs[1].name, "the_answer") == 0);
    POOR_MANS_ASSERT(pairs[1].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_int32_t);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_01_015: [ LOG_CONTEXT_LOCAL_DEFINE shall store one property/value pair that with a property type of struct with as many fields as the total number of properties passed to LOG_CONTEXT_LOCAL_DEFINE in the ... arguments. ]*/
/* Tests_SRS_LOG_CONTEXT_01_016: [ LOG_CONTEXT_LOCAL_DEFINE shall store the property types and values specified by using LOG_CONTEXT_PROPERTY in the context. ]*/
static void LOG_CONTEXT_LOCAL_DEFINE_with_2_int_properties_succeeds(void)
{
    // arrange
    setup_mocks();

    // act
    LOG_CONTEXT_LOCAL_DEFINE(local_context, NULL,
        LOG_CONTEXT_PROPERTY(int32_t, the_answer, 42),
        LOG_CONTEXT_PROPERTY(int32_t, another_int32_t, 43));

    // assert
    POOR_MANS_ASSERT(log_context_get_property_value_pair_count(&local_context) == 3);
    const LOG_CONTEXT_PROPERTY_VALUE_PAIR* pairs = log_context_get_property_value_pairs(&local_context);
    // context struct
    POOR_MANS_ASSERT(*(uint8_t*)pairs[0].value == 2);
    POOR_MANS_ASSERT(strcmp(pairs[0].name, "") == 0);
    POOR_MANS_ASSERT(pairs[0].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_struct);
    // 1st int32_t property
    POOR_MANS_ASSERT(*(int32_t*)pairs[1].value == 42);
    POOR_MANS_ASSERT(strcmp(pairs[1].name, "the_answer") == 0);
    POOR_MANS_ASSERT(pairs[1].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_int32_t);
    // 2nd int32_t property
    POOR_MANS_ASSERT(*(int32_t*)pairs[2].value == 43);
    POOR_MANS_ASSERT(strcmp(pairs[2].name, "another_int32_t") == 0);
    POOR_MANS_ASSERT(pairs[2].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_int32_t);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_01_015: [ LOG_CONTEXT_LOCAL_DEFINE shall store one property/value pair that with a property type of struct with as many fields as the total number of properties passed to LOG_CONTEXT_LOCAL_DEFINE in the ... arguments. ]*/
/* Tests_SRS_LOG_CONTEXT_01_016: [ LOG_CONTEXT_LOCAL_DEFINE shall store the property types and values specified by using LOG_CONTEXT_PROPERTY in the context. ]*/
static void LOG_CONTEXT_LOCAL_DEFINE_with_all_int_property_types_succeeds(void)
{
    // arrange
    setup_mocks();
    setup_malloc_call();

    // act
    LOG_CONTEXT_LOCAL_DEFINE(local_context, NULL,
        LOG_CONTEXT_PROPERTY(int8_t, test_int8_t, INT8_MIN),
        LOG_CONTEXT_PROPERTY(uint8_t, test_uint8_t, UINT8_MAX),
        LOG_CONTEXT_PROPERTY(int16_t, test_int16_t, INT16_MIN),
        LOG_CONTEXT_PROPERTY(uint16_t, test_uint16_t, UINT16_MAX),
        LOG_CONTEXT_PROPERTY(int32_t, test_int32_t, INT32_MIN),
        LOG_CONTEXT_PROPERTY(uint32_t, test_uint32_t, UINT32_MAX),
        LOG_CONTEXT_PROPERTY(int64_t, test_int64_t, INT64_MIN),
        LOG_CONTEXT_PROPERTY(uint64_t, test_uint64_t, UINT64_MAX)
    );

    // assert
    POOR_MANS_ASSERT(log_context_get_property_value_pair_count(&local_context) == 9);
    const LOG_CONTEXT_PROPERTY_VALUE_PAIR* pairs = log_context_get_property_value_pairs(&local_context);
    // context struct
    POOR_MANS_ASSERT(*(uint8_t*)pairs[0].value == 8);
    POOR_MANS_ASSERT(strcmp(pairs[0].name, "") == 0);
    POOR_MANS_ASSERT(pairs[0].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_struct);
    // int8_t property
    POOR_MANS_ASSERT(*(int8_t*)pairs[1].value == INT8_MIN);
    POOR_MANS_ASSERT(strcmp(pairs[1].name, "test_int8_t") == 0);
    POOR_MANS_ASSERT(pairs[1].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_int8_t);
    // uint8_t property
    POOR_MANS_ASSERT(*(uint8_t*)pairs[2].value == UINT8_MAX);
    POOR_MANS_ASSERT(strcmp(pairs[2].name, "test_uint8_t") == 0);
    POOR_MANS_ASSERT(pairs[2].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_uint8_t);
    // int16_t property
    POOR_MANS_ASSERT(*(int16_t*)pairs[3].value == INT16_MIN);
    POOR_MANS_ASSERT(strcmp(pairs[3].name, "test_int16_t") == 0);
    POOR_MANS_ASSERT(pairs[3].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_int16_t);
    // uint16_t property
    POOR_MANS_ASSERT(*(uint16_t*)pairs[4].value == UINT16_MAX);
    POOR_MANS_ASSERT(strcmp(pairs[4].name, "test_uint16_t") == 0);
    POOR_MANS_ASSERT(pairs[4].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_uint16_t);
    // int32_t property
    POOR_MANS_ASSERT(*(int32_t*)pairs[5].value == INT32_MIN);
    POOR_MANS_ASSERT(strcmp(pairs[5].name, "test_int32_t") == 0);
    POOR_MANS_ASSERT(pairs[5].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_int32_t);
    // uint32_t property
    POOR_MANS_ASSERT(*(uint32_t*)pairs[6].value == UINT32_MAX);
    POOR_MANS_ASSERT(strcmp(pairs[6].name, "test_uint32_t") == 0);
    POOR_MANS_ASSERT(pairs[6].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_uint32_t);
    // int64_t property
    POOR_MANS_ASSERT(*(int64_t*)pairs[7].value == INT64_MIN);
    POOR_MANS_ASSERT(strcmp(pairs[7].name, "test_int64_t") == 0);
    POOR_MANS_ASSERT(pairs[7].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_int64_t);
    // uint64_t property
    POOR_MANS_ASSERT(*(uint64_t*)pairs[8].value == UINT64_MAX);
    POOR_MANS_ASSERT(strcmp(pairs[8].name, "test_uint64_t") == 0);
    POOR_MANS_ASSERT(pairs[8].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_uint64_t);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_01_017: [ The name of the struct property shall be the context name specified by using LOG_CONTEXT_NAME (if specified). ]*/
static void LOG_CONTEXT_LOCAL_DEFINE_with_LOG_CONTEXT_NAME_succeeds(void)
{
    // arrange
    setup_mocks();
    setup_malloc_call();

    // act
    LOG_CONTEXT_LOCAL_DEFINE(local_context, NULL,
        LOG_CONTEXT_NAME(my_name)
    );

    // assert
    POOR_MANS_ASSERT(log_context_get_property_value_pair_count(&local_context) == 1);
    const LOG_CONTEXT_PROPERTY_VALUE_PAIR* pairs = log_context_get_property_value_pairs(&local_context);
    // context struct
    POOR_MANS_ASSERT(*(uint8_t*)pairs[0].value == 0);
    POOR_MANS_ASSERT(strcmp(pairs[0].name, "my_name") == 0);
    POOR_MANS_ASSERT(pairs[0].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_struct);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_01_011: [ If LOG_CONTEXT_NAME is specified multiple times a compiler error shall be emitted. ]*/
#if 0
// If this code compiles we are not passing the test
static void LOG_CONTEXT_LOCAL_DEFINE_with_LOG_CONTEXT_NAME_twice_does_not_compile(void)
{
    // arrange

    // act
    LOG_CONTEXT_LOCAL_DEFINE(local_context, NULL,
        LOG_CONTEXT_NAME(grrrrr),
        LOG_CONTEXT_NAME(grrrr_again)
    );

    // assert

    // clean
}
#endif

/* Tests_SRS_LOG_CONTEXT_01_019: [ If 2 properties have the same property_name for a context a compiler error shall be emitted. ]*/
#if 0
// If this code compiles we are not passing the test
static void LOG_CONTEXT_LOCAL_DEFINE_with_2_properties_with_the_same_name_fails(void)
{
    // arrange

    // act
    LOG_CONTEXT_LOCAL_DEFINE(local_context, NULL,
        LOG_CONTEXT_NAME(grrrrr),
        LOG_CONTEXT_PROPERTY(int32_t, x, 42),
        LOG_CONTEXT_PROPERTY(int16_t, x, 43)
    );

    // assert

    // clean
}
#endif

/* Tests_SRS_LOG_CONTEXT_01_026: [ If 2 properties have the same property_name for a context a compiler error shall be emitted. ]*/
#if 0
// If this code compiles we are not passing the test
static void LOG_CONTEXT_LOCAL_DEFINE_with_2_properties_with_the_same_name_when_one_is_a_string_property_fails(void)
{
    // arrange

    // act
    LOG_CONTEXT_LOCAL_DEFINE(local_context, NULL,
        LOG_CONTEXT_NAME(grrrrr),
        LOG_CONTEXT_PROPERTY(int32_t, x, 42),
        LOG_CONTEXT_PROPERTY(x, "x")
    );

    // assert

    // clean
}
#endif

/* Tests_SRS_LOG_CONTEXT_01_018: [ If parent_context is non-NULL, the created context shall copy all the property/value pairs of parent_context. ]*/
static void LOG_CONTEXT_LOCAL_DEFINE_with_a_parent_that_has_no_properties_succeeds(void)
{
    // arrange
    LOG_CONTEXT_HANDLE parent_context = setup_parent_context_with_no_properties();
    setup_mocks();

    // act
    LOG_CONTEXT_LOCAL_DEFINE(result, parent_context);

    // assert
    POOR_MANS_ASSERT(log_context_get_property_value_pair_count(&result) == 2);
    const LOG_CONTEXT_PROPERTY_VALUE_PAIR* pairs = log_context_get_property_value_pairs(&result);
    // context struct of parent
    POOR_MANS_ASSERT(*(uint8_t*)pairs[0].value == 1);
    POOR_MANS_ASSERT(strcmp(pairs[0].name, "") == 0);
    POOR_MANS_ASSERT(pairs[0].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_struct);
    // context struct
    POOR_MANS_ASSERT(*(uint8_t*)pairs[1].value == 0);
    POOR_MANS_ASSERT(strcmp(pairs[1].name, "") == 0);
    POOR_MANS_ASSERT(pairs[1].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_struct);
    POOR_MANS_ASSERT(actual_and_expected_match);

    // cleanup
    setup_mocks();
    setup_free_call();
    LOG_CONTEXT_DESTROY(parent_context);
}

/* Tests_SRS_LOG_CONTEXT_01_018: [ If parent_context is non-NULL, the created context shall copy all the property/value pairs of parent_context. ]*/
static void LOG_CONTEXT_LOCAL_DEFINE_with_a_parent_that_has_one_int_property_succeeds(void)
{
    // arrange
    LOG_CONTEXT_HANDLE parent_context = setup_parent_context_with_int_property();
    setup_mocks();

    // act
    LOG_CONTEXT_LOCAL_DEFINE(result, parent_context);

    // assert
    POOR_MANS_ASSERT(log_context_get_property_value_pair_count(&result) == 3);
    const LOG_CONTEXT_PROPERTY_VALUE_PAIR* pairs = log_context_get_property_value_pairs(&result);
    // context struct
    POOR_MANS_ASSERT(*(uint8_t*)pairs[0].value == 2);
    POOR_MANS_ASSERT(strcmp(pairs[0].name, "") == 0);
    POOR_MANS_ASSERT(pairs[0].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_struct);
    // parent context struct
    POOR_MANS_ASSERT(*(uint8_t*)pairs[1].value == 1);
    POOR_MANS_ASSERT(strcmp(pairs[1].name, "") == 0);
    POOR_MANS_ASSERT(pairs[1].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_struct);
    // parent int32_t property
    POOR_MANS_ASSERT(*(int32_t*)pairs[2].value == 42);
    POOR_MANS_ASSERT(strcmp(pairs[2].name, "parent_int32_t_property") == 0);
    POOR_MANS_ASSERT(pairs[2].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_int32_t);
    POOR_MANS_ASSERT(actual_and_expected_match);

    // cleanup
    setup_mocks();
    setup_free_call();
    LOG_CONTEXT_DESTROY(parent_context);
}

/* Tests_SRS_LOG_CONTEXT_01_018: [ If parent_context is non-NULL, the created context shall copy all the property/value pairs of parent_context. ]*/
static void LOG_CONTEXT_LOCAL_DEFINE_with_a_parent_that_has_2_int_properties_succeeds(void)
{
    // arrange
    LOG_CONTEXT_HANDLE parent_context = setup_parent_context_with_2_int_properties();
    setup_mocks();

    // act
    LOG_CONTEXT_LOCAL_DEFINE(result, parent_context);

    // assert
    POOR_MANS_ASSERT(log_context_get_property_value_pair_count(&result) == 4);
    const LOG_CONTEXT_PROPERTY_VALUE_PAIR* pairs = log_context_get_property_value_pairs(&result);
    // context struct
    POOR_MANS_ASSERT(*(uint8_t*)pairs[0].value == 3);
    POOR_MANS_ASSERT(strcmp(pairs[0].name, "") == 0);
    POOR_MANS_ASSERT(pairs[0].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_struct);
    // parent context struct
    POOR_MANS_ASSERT(*(uint8_t*)pairs[1].value == 2);
    POOR_MANS_ASSERT(strcmp(pairs[1].name, "") == 0);
    POOR_MANS_ASSERT(pairs[1].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_struct);
    // parent 1st int32_t property
    POOR_MANS_ASSERT(*(int32_t*)pairs[2].value == 42);
    POOR_MANS_ASSERT(strcmp(pairs[2].name, "parent_int32_t_property") == 0);
    POOR_MANS_ASSERT(pairs[2].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_int32_t);
    // parent 2nd int64_t property
    POOR_MANS_ASSERT(*(int64_t*)pairs[3].value == 43);
    POOR_MANS_ASSERT(strcmp(pairs[3].name, "parent_int64_t_property") == 0);
    POOR_MANS_ASSERT(pairs[3].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_int64_t);
    POOR_MANS_ASSERT(actual_and_expected_match);

    // cleanup
    setup_mocks();
    setup_free_call();
    LOG_CONTEXT_DESTROY(parent_context);
}

/* Tests_SRS_LOG_CONTEXT_01_018: [ If parent_context is non-NULL, the created context shall copy all the property/value pairs of parent_context. ]*/
static void LOG_CONTEXT_LOCAL_DEFINE_with_a_parent_that_has_a_string_property_succeeds(void)
{
    // arrange
    LOG_CONTEXT_HANDLE parent_context = setup_parent_context_with_a_string_property();
    setup_mocks();

    // act
    LOG_CONTEXT_LOCAL_DEFINE(result, parent_context);

    // assert
    POOR_MANS_ASSERT(log_context_get_property_value_pair_count(&result) == 3);
    const LOG_CONTEXT_PROPERTY_VALUE_PAIR* pairs = log_context_get_property_value_pairs(&result);
    // context struct
    POOR_MANS_ASSERT(*(uint8_t*)pairs[0].value == 2);
    POOR_MANS_ASSERT(strcmp(pairs[0].name, "") == 0);
    POOR_MANS_ASSERT(pairs[0].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_struct);
    // parent context struct
    POOR_MANS_ASSERT(*(uint8_t*)pairs[1].value == 1);
    POOR_MANS_ASSERT(strcmp(pairs[1].name, "") == 0);
    POOR_MANS_ASSERT(pairs[1].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_struct);
    // parent string property
    POOR_MANS_ASSERT(strcmp(pairs[2].value, "Baba cloantza") == 0);
    POOR_MANS_ASSERT(strcmp(pairs[2].name, "parent_string_property") == 0);
    POOR_MANS_ASSERT(pairs[2].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_ascii_char_ptr);
    POOR_MANS_ASSERT(actual_and_expected_match);

    // cleanup
    setup_mocks();
    setup_free_call();
    LOG_CONTEXT_DESTROY(parent_context);
}

/* Tests_SRS_LOG_CONTEXT_01_018: [ If parent_context is non-NULL, the created context shall copy all the property/value pairs of parent_context. ]*/
static void LOG_CONTEXT_LOCAL_DEFINE_with_a_parent_that_has_2_string_properties_succeeds(void)
{
    // arrange
    LOG_CONTEXT_HANDLE parent_context = setup_parent_context_with_2_string_properties();
    setup_mocks();

    // act
    LOG_CONTEXT_LOCAL_DEFINE(result, parent_context);

    // assert
    POOR_MANS_ASSERT(log_context_get_property_value_pair_count(&result) == 4);
    const LOG_CONTEXT_PROPERTY_VALUE_PAIR* pairs = log_context_get_property_value_pairs(&result);
    // context struct
    POOR_MANS_ASSERT(*(uint8_t*)pairs[0].value == 3);
    POOR_MANS_ASSERT(strcmp(pairs[0].name, "") == 0);
    POOR_MANS_ASSERT(pairs[0].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_struct);
    // parent context struct
    POOR_MANS_ASSERT(*(uint8_t*)pairs[1].value == 2);
    POOR_MANS_ASSERT(strcmp(pairs[1].name, "") == 0);
    POOR_MANS_ASSERT(pairs[1].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_struct);
    // parent 1st string property
    POOR_MANS_ASSERT(strcmp(pairs[2].value, "Baba cloantza") == 0);
    POOR_MANS_ASSERT(strcmp(pairs[2].name, "parent_string_property") == 0);
    POOR_MANS_ASSERT(pairs[2].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_ascii_char_ptr);
    // parent 2nd string property
    POOR_MANS_ASSERT(strcmp(pairs[3].value, "muuu") == 0);
    POOR_MANS_ASSERT(strcmp(pairs[3].name, "parent_string_property_2") == 0);
    POOR_MANS_ASSERT(pairs[3].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_ascii_char_ptr);
    POOR_MANS_ASSERT(actual_and_expected_match);

    // cleanup
    setup_mocks();
    setup_free_call();
    LOG_CONTEXT_DESTROY(parent_context);
}

/* Tests_SRS_LOG_CONTEXT_01_018: [ If parent_context is non-NULL, the created context shall copy all the property/value pairs of parent_context. ]*/
static void LOG_CONTEXT_LOCAL_DEFINE_with_a_parent_that_has_a_context_name_succeeds(void)
{
    // arrange
    LOG_CONTEXT_HANDLE parent_context = setup_parent_context_shawarma_with_everything();
    setup_mocks();

    // act
    LOG_CONTEXT_LOCAL_DEFINE(result, parent_context);

    // assert
    POOR_MANS_ASSERT(log_context_get_property_value_pair_count(&result) == 6);
    const LOG_CONTEXT_PROPERTY_VALUE_PAIR* pairs = log_context_get_property_value_pairs(&result);
    // context struct
    POOR_MANS_ASSERT(*(uint8_t*)pairs[0].value == 5);
    POOR_MANS_ASSERT(strcmp(pairs[0].name, "") == 0);
    POOR_MANS_ASSERT(pairs[0].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_struct);
    // parent context struct
    POOR_MANS_ASSERT(*(uint8_t*)pairs[1].value == 4);
    POOR_MANS_ASSERT(strcmp(pairs[1].name, "context_name") == 0);
    POOR_MANS_ASSERT(pairs[1].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_struct);
    // parent 1st property
    POOR_MANS_ASSERT(strcmp(pairs[2].value, "Baba cloantza") == 0);
    POOR_MANS_ASSERT(strcmp(pairs[2].name, "parent_string_property") == 0);
    POOR_MANS_ASSERT(pairs[2].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_ascii_char_ptr);
    // parent 2nd property
    POOR_MANS_ASSERT(*(int32_t*)pairs[3].value == 42);
    POOR_MANS_ASSERT(strcmp(pairs[3].name, "the_answer") == 0);
    POOR_MANS_ASSERT(pairs[3].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_int32_t);
    // parent 3rd property
    POOR_MANS_ASSERT(strcmp(pairs[4].value, "43") == 0);
    POOR_MANS_ASSERT(strcmp(pairs[4].name, "another_string") == 0);
    POOR_MANS_ASSERT(pairs[4].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_ascii_char_ptr);
    // parent 4th property
    POOR_MANS_ASSERT(*(uint64_t*)pairs[5].value == UINT64_MAX);
    POOR_MANS_ASSERT(strcmp(pairs[5].name, "max_uint64_t") == 0);
    POOR_MANS_ASSERT(pairs[5].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_uint64_t);
    POOR_MANS_ASSERT(actual_and_expected_match);

    // cleanup
    setup_mocks();
    setup_free_call();
    LOG_CONTEXT_DESTROY(parent_context);
}

/* Tests_SRS_LOG_CONTEXT_01_018: [ If parent_context is non-NULL, the created context shall copy all the property/value pairs of parent_context. ]*/
static void LOG_CONTEXT_LOCAL_DEFINE_with_a_stack_log_context_succeeds(void)
{
    // arrange
    setup_mocks();

    LOG_CONTEXT_LOCAL_DEFINE(local_context_1, NULL,
        LOG_CONTEXT_PROPERTY(int32_t, a, 42)
    );

    // act
    LOG_CONTEXT_LOCAL_DEFINE(local_context_2, &local_context_1,
        LOG_CONTEXT_PROPERTY(int32_t, b, 43)
    );

    // assert
    POOR_MANS_ASSERT(log_context_get_property_value_pair_count(&local_context_2) == 4);
    const LOG_CONTEXT_PROPERTY_VALUE_PAIR* pairs = log_context_get_property_value_pairs(&local_context_2);
    // context struct
    POOR_MANS_ASSERT(*(uint8_t*)pairs[0].value == 3);
    POOR_MANS_ASSERT(strcmp(pairs[0].name, "") == 0);
    POOR_MANS_ASSERT(pairs[0].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_struct);
    // parent context struct
    POOR_MANS_ASSERT(*(uint8_t*)pairs[1].value == 1);
    POOR_MANS_ASSERT(strcmp(pairs[1].name, "") == 0);
    POOR_MANS_ASSERT(pairs[1].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_struct);
    // parent property a
    POOR_MANS_ASSERT(*(int32_t*)pairs[2].value == 42);
    POOR_MANS_ASSERT(strcmp(pairs[2].name, "a") == 0);
    POOR_MANS_ASSERT(pairs[2].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_int32_t);
    // property b
    POOR_MANS_ASSERT(*(int32_t*)pairs[3].value == 43);
    POOR_MANS_ASSERT(strcmp(pairs[3].name, "b") == 0);
    POOR_MANS_ASSERT(pairs[3].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_int32_t);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_01_014: [ If parent_context is non-NULL, the created context shall copy all the property/value pairs of parent_context. ]*/
static void LOG_CONTEXT_CREATE_with_a_stack_log_context_succeeds(void)
{
    // arrange
    setup_mocks();
    setup_malloc_call();

    LOG_CONTEXT_LOCAL_DEFINE(local_context_1, NULL,
        LOG_CONTEXT_PROPERTY(int32_t, a, 42)
    );

    // act
    LOG_CONTEXT_HANDLE context_2;
    LOG_CONTEXT_CREATE(context_2, &local_context_1,
        LOG_CONTEXT_PROPERTY(int32_t, b, 43)
    );

    // assert
    POOR_MANS_ASSERT(log_context_get_property_value_pair_count(context_2) == 4);
    const LOG_CONTEXT_PROPERTY_VALUE_PAIR* pairs = log_context_get_property_value_pairs(context_2);
    // context struct
    POOR_MANS_ASSERT(*(uint8_t*)pairs[0].value == 3);
    POOR_MANS_ASSERT(strcmp(pairs[0].name, "") == 0);
    POOR_MANS_ASSERT(pairs[0].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_struct);
    // parent context struct
    POOR_MANS_ASSERT(*(uint8_t*)pairs[1].value == 1);
    POOR_MANS_ASSERT(strcmp(pairs[1].name, "") == 0);
    POOR_MANS_ASSERT(pairs[1].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_struct);
    // parent property a
    POOR_MANS_ASSERT(*(int32_t*)pairs[2].value == 42);
    POOR_MANS_ASSERT(strcmp(pairs[2].name, "a") == 0);
    POOR_MANS_ASSERT(pairs[2].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_int32_t);
    // property b
    POOR_MANS_ASSERT(*(int32_t*)pairs[3].value == 43);
    POOR_MANS_ASSERT(strcmp(pairs[3].name, "b") == 0);
    POOR_MANS_ASSERT(pairs[3].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_int32_t);
    POOR_MANS_ASSERT(actual_and_expected_match);

    // cleanup
    setup_mocks();
    setup_free_call();
    LOG_CONTEXT_DESTROY(context_2);
}

/* log_context_get_property_value_pair_count */

/* Tests_SRS_LOG_CONTEXT_01_020: [ If log_context is NULL, log_context_get_property_value_pair_count shall return UINT32_MAX. ]*/
static void log_context_get_property_value_pair_count_with_NULL_log_context_returns_UINT32_MAX(void)
{
    // arrange
    setup_mocks();
    setup_malloc_call();

    // act
    uint32_t result = log_context_get_property_value_pair_count(NULL);

    // assert
    POOR_MANS_ASSERT(result == UINT32_MAX);
}

/* Tests_SRS_LOG_CONTEXT_01_021: [ Otherwise, log_context_get_property_value_pair_count shall return the number of property/value pairs stored by log_context. ]*/
static void log_context_get_property_value_pair_count_with_a_local_stack_context_returns_the_count(void)
{
    // arrange
    setup_mocks();
    setup_malloc_call();

    LOG_CONTEXT_LOCAL_DEFINE(local_context_1, NULL,
        LOG_CONTEXT_PROPERTY(int32_t, a, 42)
    );

    setup_mocks();

    // act
    uint32_t result = log_context_get_property_value_pair_count(&local_context_1);

    // assert
    POOR_MANS_ASSERT(result == 2);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_01_021: [ Otherwise, log_context_get_property_value_pair_count shall return the number of property/value pairs stored by log_context. ]*/
static void log_context_get_property_value_pair_count_with_a_dynamically_allocated_context_returns_the_count(void)
{
    // arrange
    setup_mocks();
    setup_malloc_call();

    LOG_CONTEXT_HANDLE context;
    LOG_CONTEXT_CREATE(context, NULL,
        LOG_CONTEXT_PROPERTY(int32_t, b, 43)
    );

    setup_mocks();

    // act
    uint32_t result = log_context_get_property_value_pair_count(context);

    // assert
    POOR_MANS_ASSERT(result == 2);
    POOR_MANS_ASSERT(actual_and_expected_match);

    // cleanup
    setup_mocks();
    setup_free_call();
    LOG_CONTEXT_DESTROY(context);
}

/* log_context_get_property_value_pairs */

/* Tests_SRS_LOG_CONTEXT_01_022: [ If log_context is NULL, log_context_get_property_value_pairs shall fail and return NULL. ]*/
static void log_context_get_property_value_pairs_with_NULL_log_context_returns_NULL(void)
{
    // arrange
    setup_mocks();
    setup_malloc_call();

    // act
    const LOG_CONTEXT_PROPERTY_VALUE_PAIR* result = log_context_get_property_value_pairs(NULL);

    // assert
    POOR_MANS_ASSERT(result == NULL);
}

/* Tests_SRS_LOG_CONTEXT_01_023: [ Otherwise, log_context_get_property_value_pairs shall return the array of property/value pairs stored by the context. ]*/
static void log_context_get_property_value_pairs_with_a_local_stack_context_returns_the_pairs(void)
{
    // arrange
    setup_mocks();
    setup_malloc_call();

    LOG_CONTEXT_LOCAL_DEFINE(local_context_1, NULL,
        LOG_CONTEXT_PROPERTY(int32_t, a, 42)
    );

    setup_mocks();

    // act
    const LOG_CONTEXT_PROPERTY_VALUE_PAIR* result = log_context_get_property_value_pairs(&local_context_1);

    // assert
    POOR_MANS_ASSERT(result != NULL);
    // context struct
    POOR_MANS_ASSERT(*(uint8_t*)result[0].value == 1);
    POOR_MANS_ASSERT(strcmp(result[0].name, "") == 0);
    POOR_MANS_ASSERT(result[0].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_struct);
    // parent context struct
    POOR_MANS_ASSERT(*(int32_t*)result[1].value == 42);
    POOR_MANS_ASSERT(strcmp(result[1].name, "a") == 0);
    POOR_MANS_ASSERT(result[1].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_int32_t);

    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_01_023: [ Otherwise, log_context_get_property_value_pairs shall return the array of property/value pairs stored by the context. ]*/
static void log_context_get_property_value_pairs_with_a_dynamically_allocated_context_returns_the_pairs(void)
{
    // arrange
    setup_mocks();
    setup_malloc_call();

    LOG_CONTEXT_HANDLE context;
    LOG_CONTEXT_CREATE(context, NULL,
        LOG_CONTEXT_PROPERTY(int32_t, a, 42)
    );

    setup_mocks();

    // act
    const LOG_CONTEXT_PROPERTY_VALUE_PAIR* result = log_context_get_property_value_pairs(context);

    // assert
    POOR_MANS_ASSERT(result != NULL);
    // context struct
    POOR_MANS_ASSERT(*(uint8_t*)result[0].value == 1);
    POOR_MANS_ASSERT(strcmp(result[0].name, "") == 0);
    POOR_MANS_ASSERT(result[0].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_struct);
    // int32_t property
    POOR_MANS_ASSERT(*(int32_t*)result[1].value == 42);
    POOR_MANS_ASSERT(strcmp(result[1].name, "a") == 0);
    POOR_MANS_ASSERT(result[1].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_int32_t);

    POOR_MANS_ASSERT(actual_and_expected_match);

    // cleanup
    setup_mocks();
    setup_free_call();
    LOG_CONTEXT_DESTROY(context);
}

/* Tests_SRS_LOG_CONTEXT_01_015: [ LOG_CONTEXT_LOCAL_DEFINE shall store one property/value pair that with a property type of struct with as many fields as the total number of properties passed to LOG_CONTEXT_LOCAL_DEFINE in the ... arguments. ]*/
static void creating_a_context_with_LOG_MAX_STACK_PROPERTY_VALUE_PAIR_COUNT_minus_one_properties_succeeds(void)
{
    // arrange
    setup_mocks();
    setup_malloc_call();

    LOG_CONTEXT_HANDLE context;
    LOG_CONTEXT_CREATE(context, NULL,
        LOG_CONTEXT_PROPERTY(int32_t, a0, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a1, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a2, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a3, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a4, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a5, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a6, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a7, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a8, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a9, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a10, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a11, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a12, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a13, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a14, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a15, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a16, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a17, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a18, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a19, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a20, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a21, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a22, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a23, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a24, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a25, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a26, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a27, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a28, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a29, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a30, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a31, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a32, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a33, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a34, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a35, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a36, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a37, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a38, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a39, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a40, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a41, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a42, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a43, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a44, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a45, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a46, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a47, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a48, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a49, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a50, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a51, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a52, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a53, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a54, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a55, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a56, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a57, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a58, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a59, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a60, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a61, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a62, 0)
    );

    // act
    const LOG_CONTEXT_PROPERTY_VALUE_PAIR* result = log_context_get_property_value_pairs(context);

    // assert
    POOR_MANS_ASSERT(result != NULL);
    // context struct
    POOR_MANS_ASSERT(*(uint8_t*)result[0].value == 63);
    POOR_MANS_ASSERT(strcmp(result[0].name, "") == 0);
    POOR_MANS_ASSERT(result[0].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_struct);

    for (size_t i = 0; i < LOG_MAX_STACK_PROPERTY_VALUE_PAIR_COUNT - 1; i++)
    {
        char expected_property_name[4];
        // parent context struct
        POOR_MANS_ASSERT(*(int32_t*)result[i + 1].value == 0);
        (void)sprintf(expected_property_name, "a%zu", i);
        POOR_MANS_ASSERT(strcmp(result[i + 1].name, expected_property_name) == 0);
        POOR_MANS_ASSERT(result[i + 1].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_int32_t);
    }

    // cleanup
    setup_mocks();
    setup_free_call();
    LOG_CONTEXT_DESTROY(context);
}

/* Tests_SRS_LOG_CONTEXT_01_024: [ If the number of properties to be stored in the log context exceeds LOG_MAX_STACK_PROPERTY_VALUE_PAIR_COUNT, an error shall be reported by calling log_internal_error_report and no properties shall be stored in the context. ]*/
static void creating_a_context_with_LOG_MAX_STACK_PROPERTY_VALUE_PAIR_COUNT_properties_reports_error(void)
{
    // arrange
    setup_mocks();
    setup_log_internal_error_report();

    LOG_CONTEXT_LOCAL_DEFINE(context, NULL,
        LOG_CONTEXT_PROPERTY(int32_t, a0, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a1, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a2, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a3, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a4, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a5, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a6, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a7, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a8, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a9, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a10, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a11, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a12, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a13, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a14, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a15, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a16, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a17, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a18, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a19, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a20, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a21, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a22, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a23, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a24, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a25, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a26, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a27, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a28, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a29, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a30, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a31, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a32, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a33, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a34, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a35, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a36, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a37, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a38, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a39, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a40, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a41, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a42, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a43, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a44, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a45, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a46, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a47, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a48, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a49, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a50, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a51, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a52, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a53, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a54, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a55, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a56, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a57, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a58, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a59, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a60, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a61, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a62, 0),
        LOG_CONTEXT_PROPERTY(int32_t, a63, 0)
    );

    // act
    uint32_t property_count = log_context_get_property_value_pair_count(&context);

    // assert
    POOR_MANS_ASSERT(property_count == 0);
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* Tests_SRS_LOG_CONTEXT_01_025: [ If the memory size needed for all properties to be stored in the context exceeds LOG_MAX_STACK_DATA_SIZE, an error shall be reported by calling log_internal_error_report and no properties shall be stored in the context. ]*/
static void creating_a_context_with_too_much_data_reports_error(void)
{
    // arrange
    char original_string[4096] = { 0 };
    setup_mocks();
    setup_log_internal_error_report();

    (void)memset(original_string, 'x', sizeof(original_string) - 1);

    // 1023 + 1 + 1 bytes (more than 1024 which is max)
    LOG_CONTEXT_LOCAL_DEFINE(context, NULL,
        LOG_CONTEXT_STRING_PROPERTY(str_property, "%s", original_string)
    );

    // act
    uint32_t property_count = log_context_get_property_value_pair_count(&context);

    // assert
    POOR_MANS_ASSERT(property_count == 0);
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);
}

/* very "poor man's" way of testing, as no test harness and mocking framework are available */
int main(void)
{
    LOG_CONTEXT_CREATE_with_no_properties_succeeds();
    when_malloc_fails_LOG_CONTEXT_CREATE_with_no_properties_also_fails();
    
    LOG_CONTEXT_DESTROY_frees_memory();
    
    LOG_CONTEXT_CREATE_with_one_int32_t_succeeds();
    LOG_CONTEXT_CREATE_with_two_properties_succeeds();
    LOG_CONTEXT_CREATE_with_all_int_property_types_succeeds();
    
    LOG_CONTEXT_CREATE_with_one_string_property_with_only_format_passed_to_it_succeeds();
    LOG_CONTEXT_CREATE_with_2_string_properties_with_only_format_passed_to_it_succeeds();
    LOG_CONTEXT_CREATE_with_a_string_property_using_printf_formatting_succeeds();
    LOG_CONTEXT_CREATE_with_a_string_property_followed_by_another_int_property_succeeds();
    LOG_CONTEXT_CREATE_with_a_string_property_preceded_by_another_int_property_succeeds();

    LOG_CONTEXT_CREATE_with_LOG_CONTEXT_NAME_uses_the_context_name();

    LOG_CONTEXT_CREATE_with_a_parent_that_has_no_properties_succeeds();
    LOG_CONTEXT_CREATE_with_a_parent_that_has_one_int_property_succeeds();
    LOG_CONTEXT_CREATE_with_a_parent_that_has_2_int_properties_succeeds();
    LOG_CONTEXT_CREATE_with_a_parent_that_has_a_string_property_succeeds();
    LOG_CONTEXT_CREATE_with_a_parent_that_has_2_string_properties_succeeds();
    LOG_CONTEXT_CREATE_with_a_parent_that_has_a_context_name_succeeds();

    LOG_CONTEXT_LOCAL_DEFINE_with_no_properties_succeeds();
    LOG_CONTEXT_LOCAL_DEFINE_with_1_int_property_succeeds();
    LOG_CONTEXT_LOCAL_DEFINE_with_2_int_properties_succeeds();
    LOG_CONTEXT_LOCAL_DEFINE_with_all_int_property_types_succeeds();

    LOG_CONTEXT_LOCAL_DEFINE_with_LOG_CONTEXT_NAME_succeeds();

    LOG_CONTEXT_LOCAL_DEFINE_with_a_parent_that_has_no_properties_succeeds();
    LOG_CONTEXT_LOCAL_DEFINE_with_a_parent_that_has_one_int_property_succeeds();
    LOG_CONTEXT_LOCAL_DEFINE_with_a_parent_that_has_2_int_properties_succeeds();
    LOG_CONTEXT_LOCAL_DEFINE_with_a_parent_that_has_a_string_property_succeeds();
    LOG_CONTEXT_LOCAL_DEFINE_with_a_parent_that_has_2_string_properties_succeeds();
    LOG_CONTEXT_LOCAL_DEFINE_with_a_parent_that_has_a_context_name_succeeds();

    LOG_CONTEXT_LOCAL_DEFINE_with_a_stack_log_context_succeeds();
    LOG_CONTEXT_CREATE_with_a_stack_log_context_succeeds();

    log_context_get_property_value_pair_count_with_NULL_log_context_returns_UINT32_MAX();
    log_context_get_property_value_pair_count_with_a_local_stack_context_returns_the_count();
    log_context_get_property_value_pair_count_with_a_dynamically_allocated_context_returns_the_count();

    log_context_get_property_value_pairs_with_NULL_log_context_returns_NULL();
    log_context_get_property_value_pairs_with_a_local_stack_context_returns_the_pairs();
    log_context_get_property_value_pairs_with_a_dynamically_allocated_context_returns_the_pairs();

    creating_a_context_with_LOG_MAX_STACK_PROPERTY_VALUE_PAIR_COUNT_minus_one_properties_succeeds();
    creating_a_context_with_LOG_MAX_STACK_PROPERTY_VALUE_PAIR_COUNT_properties_reports_error();
    creating_a_context_with_too_much_data_reports_error();

    return 0;
}
