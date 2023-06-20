// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef LOG_ERRNO_H
#define LOG_ERRNO_H

#ifndef __cplusplus
#include <errno.h>
#else
#include <cerrno>
#endif

#include "c_logging/log_context.h"

#ifdef __cplusplus
extern "C" {
#endif

int log_errno_fill_property(void* buffer, int dummy);

#define LOG_ERRNO() \
    /* Codes_SRS_LOG_ERRNO_WIN32_01_001: [ LOG_ERRNO shall expand to a LOG_CONTEXT_PROPERTY_CUSTOM_FUNCTION with name errno_value, type ascii_char_ptr and value function call being log_errno_fill_property. ] */ \
    /* Codes_SRS_LOG_ERRNO_LINUX_01_001: [ LOG_ERRNO shall expand to a LOG_CONTEXT_PROPERTY_CUSTOM_FUNCTION with name errno_value, type ascii_char_ptr and value function call being log_errno_fill_property. ] */ \
    LOG_CONTEXT_PROPERTY_CUSTOM_FUNCTION(ascii_char_ptr, errno_value, log_errno_fill_property, errno)

#ifdef __cplusplus
}
#endif

#endif /* LOG_ERRNO_H */
