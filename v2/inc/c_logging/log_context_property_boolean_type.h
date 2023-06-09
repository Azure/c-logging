// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef LOG_CONTEXT_PROPERTY_BOOLEAN_TYPE_H
#define LOG_CONTEXT_PROPERTY_BOOLEAN_TYPE_H

#ifdef __cplusplus
#include <cstdint>
#include <cstddef>
#else
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#endif

#include "c_logging/log_context_property_type_if.h"

int LOG_CONTEXT_PROPERTY_TYPE_INIT(bool)(void* dst_value, bool src_value);
int LOG_CONTEXT_PROPERTY_TYPE_GET_INIT_DATA_SIZE(bool)(void);
extern const LOG_CONTEXT_PROPERTY_TYPE_IF LOG_CONTEXT_PROPERTY_TYPE_IF_IMPL(bool);

#endif /* LOG_CONTEXT_PROPERTY_BOOLEAN_TYPE_H */
