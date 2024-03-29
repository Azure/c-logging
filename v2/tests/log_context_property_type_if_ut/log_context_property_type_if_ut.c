// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "macro_utils/macro_utils.h"

#include "c_logging/log_context_property_type_if.h"

// defines how many mock calls we can have
#define MAX_MOCK_CALL_COUNT (128)

#define MOCK_CALL_TYPE_VALUES \
    MOCK_CALL_TYPE_snprintf \

MU_DEFINE_ENUM(MOCK_CALL_TYPE, MOCK_CALL_TYPE_VALUES)

#define POOR_MANS_ASSERT(cond) \
    if (!(cond)) \
    { \
        (void)printf("%s:%d test failed\r\n", __FUNCTION__, __LINE__); \
        abort(); \
    } \

/* LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(type_name) */

/* Tests_SRS_LOG_CONTEXT_PROPERTY_TYPE_IF_01_001: [ LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(type_name) shall produce the token {type_name}_log_context_property_type. ]*/
static void LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL_produces_the_correct_token(void)
{
    // arrange

    // act
    const char* result = MU_TOSTRING(LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(gogu));

    // assert
    POOR_MANS_ASSERT(strcmp(result, "gogu_log_context_property_type") == 0);
}

/* LOG_CONTEXT_PROPERTY_TYPE_INIT(type_name) */

/* Tests_SRS_LOG_CONTEXT_PROPERTY_TYPE_IF_01_002: [ LOG_CONTEXT_PROPERTY_TYPE_INIT(type_name) shall produce the token {type_name}_log_context_property_type_init. ]*/
static void LOG_CONTEXT_PROPERTY_TYPE_INIT_produces_the_correct_token(void)
{
    // arrange

    // act
    const char* result = MU_TOSTRING(LOG_CONTEXT_PROPERTY_TYPE_INIT(gogu));

    // assert
    POOR_MANS_ASSERT(strcmp(result, "gogu_log_context_property_type_init") == 0);
}

/* LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(type_name) */

/* Tests_SRS_LOG_CONTEXT_PROPERTY_TYPE_IF_01_003: [ LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(type_name) shall produce the token {type_name}_log_context_property_type_get_init_data_size. ]*/
static void LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE_produces_the_correct_token(void)
{
    // arrange

    // act
    const char* result = MU_TOSTRING(LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(gogu));

    // assert
    POOR_MANS_ASSERT(strcmp(result, "gogu_log_context_property_type_get_init_data_size") == 0);
}

/* very "poor man's" way of testing, as no test harness and mocking framework are available */
int main(void)
{
    LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL_produces_the_correct_token();
    LOG_CONTEXT_PROPERTY_TYPE_INIT_produces_the_correct_token();
    LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE_produces_the_correct_token();

    return 0;
}
