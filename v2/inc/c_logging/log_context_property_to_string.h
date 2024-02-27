// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef LOG_CONTEXT_PROPERTY_TO_STRING_H
#define LOG_CONTEXT_PROPERTY_TO_STRING_H

#ifdef __cplusplus
#include <cstddef>
#else
#include <stddef.h>
#endif

#include "c_logging/log_context_property_value_pair.h"

#ifdef __cplusplus
extern "C" {
#endif

int log_context_property_to_string(char* buffer, size_t buffer_size, const LOG_CONTEXT_PROPERTY_VALUE_PAIR* property_value_pairs, size_t property_value_pair_count);

#ifdef __cplusplus
}
#endif

#endif /* LOG_CONTEXT_PROPERTY_TO_STRING_H */
