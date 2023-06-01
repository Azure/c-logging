// Copyright (C) Microsoft Corporation. All rights reserved.

#include <stdio.h>

#include "windows.h"

#include "macro_utils/macro_utils.h"

#include "timer.h"

// This is a minimal version of the timer unit (just for tests)

static LARGE_INTEGER g_freq;
static volatile LONG g_timer_state = 0; /*0 - not "created", 1 - "created", "2" - creating*/

/*returns a time in us since "some" start.*/
double timer_global_get_elapsed_us(void)
{
    while (InterlockedCompareExchange(&g_timer_state, 2, 0) != 1)
    {
        (void)QueryPerformanceFrequency(&g_freq); /*from MSDN:  On systems that run Windows XP or later, the function will always succeed and will thus never return zero.*/
        (void)InterlockedExchange(&g_timer_state, 1);
    }

    LARGE_INTEGER now;
    (void)QueryPerformanceCounter(&now);
    return (double)now.QuadPart * 1000000.0 / (double)g_freq.QuadPart;
}
