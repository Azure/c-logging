// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include "macro_utils/macro_utils.h"
#include "c_logging/log_internal_error.h"

// defines how many mock calls we can have
#define MAX_MOCK_CALL_COUNT (128)

#define MOCK_CALL_TYPE_VALUES \
    MOCK_CALL_TYPE_abort, \
    MOCK_CALL_TYPE__set_abort_behavior \

MU_DEFINE_ENUM(MOCK_CALL_TYPE, MOCK_CALL_TYPE_VALUES)

// very poor mans mocks :-(
typedef struct _set_abort_behavior_CALL_TAG
{
    unsigned int _Flags;
    unsigned int _Mask;
} _set_abort_behavior_CALL;

typedef struct MOCK_CALL_TAG
{
    MOCK_CALL_TYPE mock_call_type;
    union
    {
        _set_abort_behavior_CALL _set_abort_behavior_call;
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

void mock_abort(void)
{
    if ((actual_call_count == expected_call_count) ||
        (expected_calls[actual_call_count].mock_call_type != MOCK_CALL_TYPE_abort))
    {
        actual_and_expected_match = false;
    }
    else
    {
        actual_call_count++;
    }
}

unsigned int mock__set_abort_behavior(unsigned int _Flags, unsigned int _Mask)
{
    unsigned int result;

    if ((actual_call_count == expected_call_count) ||
        (expected_calls[actual_call_count].mock_call_type != MOCK_CALL_TYPE__set_abort_behavior))
    {
        actual_and_expected_match = false;
        result = 0;
    }
    else
    {
        expected_calls[actual_call_count].u._set_abort_behavior_call._Flags = _Flags;
        expected_calls[actual_call_count].u._set_abort_behavior_call._Mask = _Mask;

#if defined _MSC_VER
        result = _set_abort_behavior(_Flags, _Mask);
#else
        result = 0;
#endif

        actual_call_count++;
    }

    return result;
}

#define POOR_MANS_ASSERT(cond) \
    if (!(cond)) \
    { \
        (void)printf("%s:%d test failed\r\n", __FUNCTION__, __LINE__); \
        abort(); \
    } \

/* LOG_CONTEXT_CREATE */

/* Tests_SRS_LOG_INTERNAL_ERROR_01_002: [ Otherwise log_internal_error_report shall return. ]*/
static void log_internal_error_report_returns(void)
{
    // arrange
    setup_mocks();

    // act
    log_internal_error_report();

    // assert
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
}

/* very "poor man's" way of testing, as no test harness and mocking framework are available */
int main(void)
{
    log_internal_error_report_returns();

    return 0;
}
