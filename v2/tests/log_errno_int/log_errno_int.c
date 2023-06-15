// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#ifndef errno_t
typedef int errno_t;
#endif

#include "macro_utils/macro_utils.h"

#include "c_logging/log_errno.h"
#include "c_logging/log_context.h"

// defines how many mock calls we can have
#define MAX_MOCK_CALL_COUNT (128)

#define MOCK_CALL_TYPE_VALUES \
    MOCK_CALL_TYPE_errno

MU_DEFINE_ENUM(MOCK_CALL_TYPE, MOCK_CALL_TYPE_VALUES)

// very poor mans mocks :-(

typedef struct errno_CALL_TAG
{
    bool override_result;
    errno_t call_result;
} errno_CALL;

typedef struct MOCK_CALL_TAG
{
    MOCK_CALL_TYPE mock_call_type;
    union
    {
        errno_CALL errno_call;
    };
} MOCK_CALL;

static MOCK_CALL expected_calls[MAX_MOCK_CALL_COUNT];
static size_t expected_call_count;
static size_t actual_call_count;
static bool actual_and_expected_match;

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

errno_t mock_errno(void)
{
    errno_t result;
    
    if ((actual_call_count == expected_call_count) ||
        (expected_calls[actual_call_count].mock_call_type != MOCK_CALL_TYPE_errno))
    {
        actual_and_expected_match = false;
        result = MU_FAILURE;
    }
    else
    {
        if (expected_calls[actual_call_count].errno_call.override_result)
        {
            result = expected_calls[actual_call_count].errno_call.call_result;
        }
        else
        {
            result = 0;
        }
    
        actual_call_count++;
    }
    
    return result;
}

static void setup_errno_call(void)
{
    expected_calls[expected_call_count].mock_call_type = MOCK_CALL_TYPE_errno;
    expected_calls[expected_call_count].errno_call.override_result = false;
    expected_call_count++;
}

/* LOG_ERRNO */

static void LOG_ERRNO_emits_the_underlying_property(void)
{
    // arrange
    setup_mocks();
    setup_errno_call();
    expected_calls[0].errno_call.override_result = true;
    expected_calls[0].errno_call.call_result = 42;

    // act
    LOG_CONTEXT_LOCAL_DEFINE(log_context, NULL, LOG_ERRNO());

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);

    uint32_t property_count = log_context_get_property_value_pair_count(&log_context);
    const LOG_CONTEXT_PROPERTY_VALUE_PAIR* properties = log_context_get_property_value_pairs(&log_context);
    POOR_MANS_ASSERT(property_count == 2);
    POOR_MANS_ASSERT(strcmp(properties[0].name, "") == 0);
    POOR_MANS_ASSERT(properties[0].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_struct);
    POOR_MANS_ASSERT(*(uint8_t*)properties[0].value == 1);
    POOR_MANS_ASSERT(strcmp(properties[1].name, "errno_value") == 0);
    POOR_MANS_ASSERT(properties[1].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_ascii_char_ptr);
    POOR_MANS_ASSERT(strcmp(properties[1].value, "") != 0);
}

static void LOG_ERRNO_emits_the_underlying_property_with_a_preceding_and_following_property(void)
{
    // arrange
    setup_mocks();
    setup_errno_call();
    expected_calls[0].errno_call.override_result = true;
    expected_calls[0].errno_call.call_result = 42;

    // act
    LOG_CONTEXT_LOCAL_DEFINE(log_context, NULL, LOG_CONTEXT_STRING_PROPERTY(romanian_chuck_norris, "gigi %s", "duru"), LOG_ERRNO(), LOG_CONTEXT_PROPERTY(int32_t, the_answer, 42));

    // assert
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
    POOR_MANS_ASSERT(actual_and_expected_match);

    uint32_t property_count = log_context_get_property_value_pair_count(&log_context);
    const LOG_CONTEXT_PROPERTY_VALUE_PAIR* properties = log_context_get_property_value_pairs(&log_context);
    POOR_MANS_ASSERT(property_count == 4);
    POOR_MANS_ASSERT(strcmp(properties[0].name, "") == 0);
    POOR_MANS_ASSERT(properties[0].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_struct);
    POOR_MANS_ASSERT(*(uint8_t*)properties[0].value == 3);
    POOR_MANS_ASSERT(strcmp(properties[1].name, "romanian_chuck_norris") == 0);
    POOR_MANS_ASSERT(properties[1].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_ascii_char_ptr);
    POOR_MANS_ASSERT(strcmp(properties[1].value, "gigi duru") == 0);
    POOR_MANS_ASSERT(strcmp(properties[2].name, "errno_value") == 0);
    POOR_MANS_ASSERT(strcmp(properties[2].value, "") != 0);
    POOR_MANS_ASSERT(properties[2].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_ascii_char_ptr);
    POOR_MANS_ASSERT(strcmp(properties[3].name, "the_answer") == 0);
    POOR_MANS_ASSERT(properties[3].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_int32_t);
    POOR_MANS_ASSERT(*(int32_t*)properties[3].value == 42);
}

/* very "poor man's" way of testing, as no test harness and mocking framework are available */
int main(void)
{
#ifdef _MSC_VER
    // make abort not popup
    _set_abort_behavior(_CALL_REPORTFAULT, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
#endif

    LOG_ERRNO_emits_the_underlying_property();
    LOG_ERRNO_emits_the_underlying_property_with_a_preceding_and_following_property();

    return 0;
}
