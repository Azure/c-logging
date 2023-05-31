// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef TIMER_H
#define TIMER_H

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct TIMER_HANDLE_DATA_TAG* TIMER_HANDLE;

TIMER_HANDLE timer_create_new(void);
int timer_start(TIMER_HANDLE handle);
double timer_get_elapsed(TIMER_HANDLE timer);
double timer_get_elapsed_ms(TIMER_HANDLE timer);
void timer_destroy(TIMER_HANDLE timer);
double timer_global_get_elapsed_ms(void);
double timer_global_get_elapsed_us(void);

#ifdef __cplusplus
}
#endif

#endif /* TIMER_H */
