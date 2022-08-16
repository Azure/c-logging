// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdio.h>
#include <string.h>

#include "macro_utils/macro_utils.h"

#include "c_logging/log_context_property_type_if.h"

static size_t asserts_failed = 0;

// defines how many mock calls we can have
#define MAX_MOCK_CALL_COUNT (128)

#define MOCK_CALL_TYPE_VALUES \
    MOCK_CALL_TYPE_snprintf \

MU_DEFINE_ENUM(MOCK_CALL_TYPE, MOCK_CALL_TYPE_VALUES)

#define POOR_MANS_ASSERT(cond) \
    if (!(cond)) \
    { \
        (void)printf("%s:%d test failed\r\n", __FUNCTION__, __LINE__); \
        asserts_failed++; \
    } \

/* LOG_CONTEXT_PROPERTY_TYPE_IMPL(type_name) */

/* Tests_SRS_LOG_CONTEXT_PROPERTY_TYPE_IF_01_001: [ LOG_CONTEXT_PROPERTY_TYPE_IMPL(type_name) shall produce the token {type_name}_log_context_property_type. ]*/
static void LOG_CONTEXT_PROPERTY_TYPE_IMPL_produces_the_correct_token(void)
{
    // arrange

    // act
    const char* result = MU_TOSTRING(LOG_CONTEXT_PROPERTY_TYPE_IMPL(gogu));

    // assert
    POOR_MANS_ASSERT(strcmp(result, "gogu_log_context_property_type") == 0);
}

/* very "poor man's" way of testing, as no test harness and mocking framework are available */
int main(void)
{
    LOG_CONTEXT_PROPERTY_TYPE_IMPL_produces_the_correct_token();

    return asserts_failed;
}
