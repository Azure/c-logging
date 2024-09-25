// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#ifdef USE_VLD
#include "vld.h"
#endif

#ifdef _MSC_VER
#include "windows.h"
#endif

#include "c_logging/format_message_no_newline.h"

#define POOR_MANS_ASSERT(cond) \
    if (!(cond)) \
    { \
        (void)printf("%s:%d test failed\r\n", __FUNCTION__, __LINE__); \
        abort(); \
    } \

static void FormatMessageA_no_newline_fills_995_last_error(void)
{
    // arrange
    char buffer[512];

    // act
    int result = FormatMessageA_no_newline(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, 995, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buffer, sizeof(buffer), NULL);

    // assert
    POOR_MANS_ASSERT(result > 0);
    POOR_MANS_ASSERT(strcmp(buffer, "The I/O operation has been aborted because of either a thread exit or an application request.") == 0);
}

static void FormatMessageA_no_newline_all_error_codes_does_not_crash(void)
{
    // arrange
    char buffer[512];

    for (uint32_t i = 35; i <= 65535; i++)
    {
        // act
        int result = FormatMessageA_no_newline(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, i, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buffer, sizeof(buffer), NULL);

        // assert
        // no assert
        // some of the error codes wil return 0, so we can't really check it
        if (result > 0)
        {
            (void)printf("%" PRIu32 ": %s\r\n", i, buffer);
        }
    }
}

/* very "poor man's" way of testing, as no test harness and mocking framework are available */
int main(void)
{
    // make abort not popup
    _set_abort_behavior(_CALL_REPORTFAULT, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);

    FormatMessageA_no_newline_fills_995_last_error();
    FormatMessageA_no_newline_all_error_codes_does_not_crash();

    return 0;
}
