// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#ifdef USE_VLD
#include "vld.h"
#endif

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
    };
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
        (expected_calls[actual_call_count].mock_call_type != MOCK_CALL_TYPE__set_abort_behavior) ||
        (expected_calls[actual_call_count]._set_abort_behavior_call._Flags != _Flags) ||
        (expected_calls[actual_call_count]._set_abort_behavior_call._Mask != _Mask))
    {
        actual_and_expected_match = false;
        result = 0;
    }
    else
    {
#if defined _MSC_VER
        result = _set_abort_behavior(_Flags, _Mask);
#else
        result = 0;
#endif

        actual_call_count++;
    }

    return result;
}

static void setup_abort(void)
{
    expected_calls[expected_call_count].mock_call_type = MOCK_CALL_TYPE_abort;
    expected_call_count++;
}

static void setup__set_abort_behavior(unsigned int _Flags, unsigned int _Mask)
{
    expected_calls[expected_call_count].mock_call_type = MOCK_CALL_TYPE__set_abort_behavior;
    expected_calls[expected_call_count]._set_abort_behavior_call._Flags = _Flags;
    expected_calls[expected_call_count]._set_abort_behavior_call._Mask = _Mask;
    expected_call_count++;
}

#define POOR_MANS_ASSERT(cond) \
    if (!(cond)) \
    { \
        (void)printf("%s:%d test failed\r\n", __FUNCTION__, __LINE__); \
        abort(); \
    } \

/* LOG_CONTEXT_CREATE */

/* Tests_SRS_LOG_INTERNAL_ERROR_01_001: [ If LOG_ABORT_ON_ERROR is defined, log_internal_error_report shall call abort the execution. ]*/
/* Tests_SRS_LOG_INTERNAL_ERROR_01_003: [ On Windows, if LOG_ABORT_ON_ERROR is defined, log_internal_error_report shall call _set_abort_behavior(_CALL_REPORTFAULT, _WRITE_ABORT_MSG | _CALL_REPORTFAULT) to disable the abort message. ]*/
static void log_internal_error_report_aborts(void)
{
    // arrange
    setup_mocks();
#if defined _MSC_VER
    setup__set_abort_behavior(_CALL_REPORTFAULT, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
#endif
    setup_abort();

    // act
    log_internal_error_report();

    // assert
    POOR_MANS_ASSERT(actual_and_expected_match);
    POOR_MANS_ASSERT(expected_call_count == actual_call_count);
}

/* very "poor man's" way of testing, as no test harness and mocking framework are available */
int main(void)
{
    log_internal_error_report_aborts();

    return 0;
}
