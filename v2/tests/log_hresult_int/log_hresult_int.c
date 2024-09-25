// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "windows.h"

#ifdef USE_VLD
#include "vld.h"
#endif

#include "macro_utils/macro_utils.h"

#include "c_logging/log_hresult.h"
#include "c_logging/log_context.h"

#define POOR_MANS_ASSERT(cond) \
    if (!(cond)) \
    { \
        (void)printf("%s:%d test failed\r\n", __FUNCTION__, __LINE__); \
        abort(); \
    } \

/* LOGGER_LOG */

static void LOG_HRESULT_emits_the_underlying_property(void)
{
    // arrange

    // act
    LOG_CONTEXT_LOCAL_DEFINE(log_context, NULL, LOG_HRESULT(E_FAIL));

    // assert
    uint32_t property_count = log_context_get_property_value_pair_count(&log_context);
    const LOG_CONTEXT_PROPERTY_VALUE_PAIR* properties = log_context_get_property_value_pairs(&log_context);
    POOR_MANS_ASSERT(property_count == 2);
    POOR_MANS_ASSERT(strcmp(properties[0].name, "") == 0);
    POOR_MANS_ASSERT(properties[0].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_struct);
    POOR_MANS_ASSERT(*(uint8_t*)properties[0].value == 1);
    POOR_MANS_ASSERT(strcmp(properties[1].name, "hresult") == 0);
    POOR_MANS_ASSERT(properties[1].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_ascii_char_ptr);
    POOR_MANS_ASSERT(strcmp(properties[1].value, "Unspecified error") == 0);
}

static void LOG_HRESULT_emits_the_underlying_property_with_a_preceding_and_following_property(void)
{
    // arrange

    // act
    LOG_CONTEXT_LOCAL_DEFINE(log_context, NULL, LOG_CONTEXT_STRING_PROPERTY(romanian_chuck_norris, "gigi %s", "duru"), LOG_HRESULT(E_POINTER), LOG_CONTEXT_PROPERTY(int32_t, the_answer, 42));

    // assert
    uint32_t property_count = log_context_get_property_value_pair_count(&log_context);
    const LOG_CONTEXT_PROPERTY_VALUE_PAIR* properties = log_context_get_property_value_pairs(&log_context);
    POOR_MANS_ASSERT(property_count == 4);
    POOR_MANS_ASSERT(strcmp(properties[0].name, "") == 0);
    POOR_MANS_ASSERT(properties[0].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_struct);
    POOR_MANS_ASSERT(*(uint8_t*)properties[0].value == 3);
    POOR_MANS_ASSERT(strcmp(properties[1].name, "romanian_chuck_norris") == 0);
    POOR_MANS_ASSERT(properties[1].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_ascii_char_ptr);
    POOR_MANS_ASSERT(strcmp(properties[1].value, "gigi duru") == 0);
    POOR_MANS_ASSERT(strcmp(properties[2].name, "hresult") == 0);
    POOR_MANS_ASSERT(properties[2].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_ascii_char_ptr);
    POOR_MANS_ASSERT(strcmp(properties[2].value, "Invalid pointer") == 0);
    POOR_MANS_ASSERT(strcmp(properties[3].name, "the_answer") == 0);
    POOR_MANS_ASSERT(properties[3].type->get_type() == LOG_CONTEXT_PROPERTY_TYPE_int32_t);
    POOR_MANS_ASSERT(*(int32_t*)properties[3].value == 42);
}

/* very "poor man's" way of testing, as no test harness and mocking framework are available */
int main(void)
{
    // make abort not popup
    _set_abort_behavior(_CALL_REPORTFAULT, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);

    LOG_HRESULT_emits_the_underlying_property();
    LOG_HRESULT_emits_the_underlying_property_with_a_preceding_and_following_property();

    return 0;
}
