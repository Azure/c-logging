// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef LOG_SINK_CALLBACK_H
#define LOG_SINK_CALLBACK_H

#include "c_logging/log_sink_if.h"

#ifdef __cplusplus
extern "C" {
#endif

    typedef void (*LOG_SINK_CALLBACK_LOG)(void* context, LOG_LEVEL log_level, const char* message);
    int log_sink_callback_set_callback(LOG_SINK_CALLBACK_LOG log_callback, void* context);
    void log_sink_callback_set_max_level(LOG_LEVEL log_level);

    extern const LOG_SINK_IF log_sink_callback;

#ifdef __cplusplus
}
#endif

#endif /* LOG_SINK_CALLBACK_H */
