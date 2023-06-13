// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef LOG_ERRNO_H
#define LOG_ERRNO_H

#ifndef __cplusplus
#include <stdint.h>
#else
#include <cstdint>
#endif

#include "c_logging/log_context.h"

int log_errno_fill_property(void* buffer, int dummy);

#define LOG_ERRNO() \
    LOG_CONTEXT_PROPERTY_CUSTOM_FUNCTION(ascii_char_ptr, hresult, log_errno_fill_property, errno())

#endif /* LOG_ERRNO_H */
